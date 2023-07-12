/********************************************************************
* Description:  remora.c
*               This file, 'remora.c', is a HAL component that
*               provides and SPI connection to a external LPC1768 running Remora PRU firmware.
*  				
*				Initially developed for RaspberryPi -> Arduino Due.
*				Further developed for RaspberryPi -> Smoothieboard and clones (LPC1768).
*
* Author: Scott Alford
* License: GPL Version 2
*
*		Credit to GP Orcullo and PICnc V2 which originally inspired this
*		and portions of this code is based on stepgen.c by John Kasunich
*		and hm2_rpspi.c by Matsche
*
* Copyright (c) 2021	All rights reserved.
*
* Last change:
********************************************************************/


#include "rtapi.h"			/* RTAPI realtime OS API */
#include "rtapi_app.h"		/* RTAPI realtime module decls */
#include "hal.h"			/* HAL public API decls */

#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Using BCM2835 driver library by Mike McCauley, why reinvent the wheel!
// http://www.airspayce.com/mikem/bcm2835/index.html
// Include these in the source directory when using "halcompile --install remora.c"
#include "bcm2835.h"
#include "bcm2835.c"


#include "remora-eth.h"


#define MODNAME "remora-eth"
#define PREFIX "remora"

MODULE_AUTHOR("Scott Alford AKA scotta");
MODULE_DESCRIPTION("Driver for Remora LPC1768 control board");
MODULE_LICENSE("GPL v2");


/***********************************************************************
*                STRUCTURES AND GLOBAL VARIABLES                       *
************************************************************************/

typedef struct {
	hal_bit_t		*SPIenable;
	hal_bit_t		*SPIreset;
	hal_bit_t		*PRUreset;
	bool			SPIresetOld;
	hal_bit_t		*SPIstatus;
	hal_bit_t 		*stepperEnable[JOINTS];
	int				pos_mode[JOINTS];
	hal_float_t 	*pos_cmd[JOINTS];			// pin: position command (position units)
	hal_float_t 	*vel_cmd[JOINTS];			// pin: velocity command (position units/sec)
	hal_float_t 	*pos_fb[JOINTS];			// pin: position feedback (position units)
	hal_s32_t		*count[JOINTS];				// pin: psition feedback (raw counts)
	hal_float_t 	pos_scale[JOINTS];			// param: steps per position unit
	float 			freq[JOINTS];				// param: frequency command sent to PRU
	hal_float_t 	*freq_cmd[JOINTS];			// pin: frequency command monitoring, available in LinuxCNC
	hal_float_t 	maxvel[JOINTS];				// param: max velocity, (pos units/sec)
	hal_float_t 	maxaccel[JOINTS];			// param: max accel (pos units/sec^2)
	hal_float_t		*pgain[JOINTS];
	hal_float_t		*ff1gain[JOINTS];
	hal_float_t		*deadband[JOINTS];
	float 			old_pos_cmd[JOINTS];		// previous position command (counts)
	float 			old_pos_cmd_raw[JOINTS];		// previous position command (counts)
	float 			old_scale[JOINTS];			// stored scale value
	float 			scale_recip[JOINTS];		// reciprocal value used for scaling
	float			prev_cmd[JOINTS];
	float			cmd_d[JOINTS];					// command derivative
	hal_float_t 	*setPoint[VARIABLES];
	hal_float_t 	*processVariable[VARIABLES];
	hal_bit_t   	*outputs[DIGITAL_OUTPUTS];
	hal_bit_t   	*inputs[DIGITAL_INPUTS];
} data_t;

static data_t *data;


#pragma pack(push, 1)

typedef union
{
  // this allow structured access to the outgoing SPI data without having to move it
  // this is the same structure as the PRU rxData structure
  struct
  {
    uint8_t txBuffer[BUFFER_SIZE];
  };
  struct
  {
	int32_t header;
    int32_t jointFreqCmd[JOINTS];
    float 	setPoint[VARIABLES];
	uint8_t jointEnable;
	uint16_t outputs;
    uint8_t spare0;
  };
} txData_t;


typedef union
{
  // this allow structured access to the incoming SPI data without having to move it
  // this is the same structure as the PRU txData structure
  struct
  {
    uint8_t rxBuffer[BUFFER_SIZE];
  };
  struct
  {
    int32_t header;
    int32_t jointFeedback[JOINTS];
    float 	processVariable[VARIABLES];
    uint16_t inputs;
  };
} rxData_t;

#pragma pack(pop)

static txData_t txData;
static rxData_t rxData;


/* other globals */
static int 			comp_id;				// component ID
static const char 	*modname = MODNAME;
static const char 	*prefix = PREFIX;
static int 			num_chan = 0;			// number of step generators configured
static long 		old_dtns;				// update_freq function period in nsec - (THIS IS RUNNING IN THE PI)
static double		dt;						// update_freq period in seconds  - (THIS IS RUNNING IN THE PI)
static double 		recip_dt;				// recprocal of period, avoids divides

static int64_t 		accum[JOINTS] = { 0 };
static int32_t 		old_count[JOINTS] = { 0 };
static int32_t		accum_diff = 0;

static int 			reset_gpio_pin = 25;				// debug pin

typedef enum CONTROL { POSITION, VELOCITY, INVALID } CONTROL;
char *ctrl_type[JOINTS] = { "p" };
RTAPI_MP_ARRAY_STRING(ctrl_type,JOINTS,"control type (pos or vel)");

int PRU_base_freq = -1;
RTAPI_MP_INT(PRU_base_freq, "PRU base thread frequency");

#define DST_PORT 27181
#define SRC_PORT 27181
#define SEND_TIMEOUT_US 50
#define RECV_TIMEOUT_US 50

static int udpSocket;
struct sockaddr_in dstAddr, srcAddr;
struct hostent *server;
static const char *dstAddress = "10.10.10.10";

/***********************************************************************
*                  LOCAL FUNCTION DECLARATIONS                         *
************************************************************************/
static int rt_bcm2835_init(void);
static int UDP_init(void);

static void update_freq(void *arg, long period);
static void spi_write();
static void spi_read();
static void spi_transfer();
static CONTROL parse_ctrl_type(const char *ctrl);



/***********************************************************************
*                       INIT AND EXIT CODE                             *
************************************************************************/

int rtapi_app_main(void)
{
    char name[HAL_NAME_LEN + 1];
	int n, retval;

	// parse stepgen control type
	for (n = 0; n < JOINTS; n++) {
		if(parse_ctrl_type(ctrl_type[n]) == INVALID) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					"STEPGEN: ERROR: bad control type '%s' for axis %i (must be 'p' or 'v')\n",
					ctrl_type[n], n);
			return -1;
		}
    }
	

	// check to see if the PRU base frequency has been set at the command line
	if (PRU_base_freq != -1)
	{
		if ((PRU_base_freq < 40000) || (PRU_base_freq > 120000))
		{
			rtapi_print_msg(RTAPI_MSG_ERR, "ERROR: PRU base frequency incorrect\n");
			return -1;
		}
	}
	else
	{
		PRU_base_freq = PRU_BASEFREQ;
	}
	

    // connect to the HAL, initialise the driver
    comp_id = hal_init(modname);
    if (comp_id < 0)
	{
		rtapi_print_msg(RTAPI_MSG_ERR, "%s ERROR: hal_init() failed \n", modname);
		return -1;
    }

	// allocate shared memory
	data = hal_malloc(sizeof(data_t));
	if (data == 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
			"%s: ERROR: hal_malloc() failed\n", modname);
		hal_exit(comp_id);
		return -1;
	}
	
		// Map the RPi BCM2835 peripherals - uses "rtapi_open_as_root" in place of "open"
	if (!rt_bcm2835_init())
    {
      rtapi_print_msg(RTAPI_MSG_ERR,"rt_bcm2835_init failed. Are you running with root privlages??\n");
      return -1;
    }
	
	bcm2835_gpio_fsel(reset_gpio_pin, BCM2835_GPIO_FSEL_OUTP);

	/* Initialize the UDP socket */
	if (UDP_init() < 0)
	{
		rtapi_print_msg(RTAPI_MSG_ERR, "Error: The board is unreachable\n");
		return -1;
	}

	// export RemoraPRU SPI enable and status bits
	retval = hal_pin_bit_newf(HAL_IN, &(data->SPIenable),
			comp_id, "%s.SPI-enable", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_IN, &(data->SPIreset),
			comp_id, "%s.SPI-reset", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_bit_newf(HAL_OUT, &(data->SPIstatus),
			comp_id, "%s.SPI-status", prefix);
	if (retval != 0) goto error;



    // export all the variables for each joint
    for (n = 0; n < JOINTS; n++) {
		// export pins

		data->pos_mode[n] = (parse_ctrl_type(ctrl_type[n]) == POSITION);
/*
This is throwing errors from axis.py for some reason...
		
		if (data->pos_mode[n]){
			rtapi_print_msg(RTAPI_MSG_ERR, "Creating pos_mode[%d] = %d\n", n, data->pos_mode[n]);
			retval = hal_pin_float_newf(HAL_IN, &(data->pos_cmd[n]),
					comp_id, "%s.joint.%01d.pos-cmd", prefix, n);
			if (retval < 0) goto error;
			*(data->pos_cmd[n]) = 0.0;
		} else {
			rtapi_print_msg(RTAPI_MSG_ERR, "Creating vel_mode[%d] = %d\n", n, data->pos_mode[n]);
			retval = hal_pin_float_newf(HAL_IN, &(data->vel_cmd[n]),
					comp_id, "%s.joint.%01d.vel-cmd", prefix, n);
			if (retval < 0) goto error;
			*(data->vel_cmd[n]) = 0.0;			
		}
*/

		retval = hal_pin_bit_newf(HAL_IN, &(data->stepperEnable[n]),
				comp_id, "%s.joint.%01d.enable", prefix, n);
		if (retval != 0) goto error;

		retval = hal_pin_float_newf(HAL_IN, &(data->pos_cmd[n]),
				comp_id, "%s.joint.%01d.pos-cmd", prefix, n);
		if (retval < 0) goto error;
		*(data->pos_cmd[n]) = 0.0;
		
		if (data->pos_mode[n] == 0){
			retval = hal_pin_float_newf(HAL_IN, &(data->vel_cmd[n]),
					comp_id, "%s.joint.%01d.vel-cmd", prefix, n);
			if (retval < 0) goto error;
			*(data->vel_cmd[n]) = 0.0;			
		}

		retval = hal_pin_float_newf(HAL_OUT, &(data->freq_cmd[n]),
		        comp_id, "%s.joint.%01d.freq-cmd", prefix, n);
		if (retval < 0) goto error;
		*(data->freq_cmd[n]) = 0.0;

		retval = hal_pin_float_newf(HAL_OUT, &(data->pos_fb[n]),
		        comp_id, "%s.joint.%01d.pos-fb", prefix, n);
		if (retval < 0) goto error;
		*(data->pos_fb[n]) = 0.0;
		
		retval = hal_param_float_newf(HAL_RW, &(data->pos_scale[n]),
		        comp_id, "%s.joint.%01d.scale", prefix, n);
		if (retval < 0) goto error;
		data->pos_scale[n] = 1.0;

		retval = hal_pin_s32_newf(HAL_OUT, &(data->count[n]),
		        comp_id, "%s.joint.%01d.counts", prefix, n);
		if (retval < 0) goto error;
		*(data->count[n]) = 0;
		
		retval = hal_pin_float_newf(HAL_IN, &(data->pgain[n]),
				comp_id, "%s.joint.%01d.pgain", prefix, n);
		if (retval < 0) goto error;
		*(data->pgain[n]) = 0.0;
		
		retval = hal_pin_float_newf(HAL_IN, &(data->ff1gain[n]),
				comp_id, "%s.joint.%01d.ff1gain", prefix, n);
		if (retval < 0) goto error;
		*(data->ff1gain[n]) = 0.0;
		
		retval = hal_pin_float_newf(HAL_IN, &(data->deadband[n]),
				comp_id, "%s.joint.%01d.deadband", prefix, n);
		if (retval < 0) goto error;
		*(data->deadband[n]) = 0.0;
		
		retval = hal_param_float_newf(HAL_RW, &(data->maxaccel[n]),
		        comp_id, "%s.joint.%01d.maxaccel", prefix, n);
		if (retval < 0) goto error;
		data->maxaccel[n] = 1.0;
	}

	for (n = 0; n < VARIABLES; n++) {
	// export pins

		retval = hal_pin_float_newf(HAL_IN, &(data->setPoint[n]),
		        comp_id, "%s.SP.%01d", prefix, n);
		if (retval < 0) goto error;
		*(data->setPoint[n]) = 0.0;

		retval = hal_pin_float_newf(HAL_OUT, &(data->processVariable[n]),
		        comp_id, "%s.PV.%01d", prefix, n);
		if (retval < 0) goto error;
		*(data->processVariable[n]) = 0.0;
	}

	for (n = 0; n < DIGITAL_OUTPUTS; n++) {
		retval = hal_pin_bit_newf(HAL_IN, &(data->outputs[n]),
				comp_id, "%s.output.%01d", prefix, n);
		if (retval != 0) goto error;
		*(data->outputs[n])=0;
	}

	for (n = 0; n < DIGITAL_INPUTS; n++) {
		retval = hal_pin_bit_newf(HAL_OUT, &(data->inputs[n]),
				comp_id, "%s.input.%01d", prefix, n);
		if (retval != 0) goto error;
		*(data->inputs[n])=0;
	}

	error:
	if (retval < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
		        "%s: ERROR: pin export failed with err=%i\n",
		        modname, retval);
		hal_exit(comp_id);
		return -1;
	}





	// Export functions
	rtapi_snprintf(name, sizeof(name), "%s.update-freq", prefix);
	retval = hal_export_funct(name, update_freq, data, 1, 0, comp_id);
	if (retval < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
		        "%s: ERROR: update function export failed\n", modname);
		hal_exit(comp_id);
		return -1;
	}

	rtapi_snprintf(name, sizeof(name), "%s.write", prefix);
	/* no FP operations */
	retval = hal_export_funct(name, spi_write, 0, 0, 0, comp_id);
	if (retval < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
		        "%s: ERROR: write function export failed\n", modname);
		hal_exit(comp_id);
		return -1;
	}

	rtapi_snprintf(name, sizeof(name), "%s.read", prefix);
	retval = hal_export_funct(name, spi_read, data, 1, 0, comp_id);
	if (retval < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
		        "%s: ERROR: read function export failed\n", modname);
		hal_exit(comp_id);
		return -1;
	}

	rtapi_print_msg(RTAPI_MSG_INFO, "%s: installed driver\n", modname);
	hal_ready(comp_id);
    return 0;
}

void rtapi_app_exit(void)
{
	int ret = shutdown(udpSocket, SHUT_RDWR);
	if (ret < 0)
      rtapi_print("ERROR: can't close socket: %s\n", strerror(errno));
	
    hal_exit(comp_id);
}


/***********************************************************************
*                   LOCAL FUNCTION DEFINITIONS                         *
************************************************************************/


// This is the same as the standard bcm2835 library except for the use of
// "rtapi_open_as_root" in place of "open"

int rt_bcm2835_init(void)
{
    int  memfd;
    int  ok;
    FILE *fp;

    if (debug) 
    {
        bcm2835_peripherals = (uint32_t*)BCM2835_PERI_BASE;

	bcm2835_pads = bcm2835_peripherals + BCM2835_GPIO_PADS/4;
	bcm2835_clk  = bcm2835_peripherals + BCM2835_CLOCK_BASE/4;
	bcm2835_gpio = bcm2835_peripherals + BCM2835_GPIO_BASE/4;
	bcm2835_pwm  = bcm2835_peripherals + BCM2835_GPIO_PWM/4;
	bcm2835_spi0 = bcm2835_peripherals + BCM2835_SPI0_BASE/4;
	bcm2835_bsc0 = bcm2835_peripherals + BCM2835_BSC0_BASE/4;
	bcm2835_bsc1 = bcm2835_peripherals + BCM2835_BSC1_BASE/4;
	bcm2835_st   = bcm2835_peripherals + BCM2835_ST_BASE/4;
	bcm2835_aux  = bcm2835_peripherals + BCM2835_AUX_BASE/4;
	bcm2835_spi1 = bcm2835_peripherals + BCM2835_SPI1_BASE/4;

	return 1; /* Success */
    }

    /* Figure out the base and size of the peripheral address block
    // using the device-tree. Required for RPi2/3/4, optional for RPi 1
    */
    if ((fp = fopen(BMC2835_RPI2_DT_FILENAME , "rb")))
    {
        unsigned char buf[16];
        uint32_t base_address;
        uint32_t peri_size;
        if (fread(buf, 1, sizeof(buf), fp) >= 8)
        {
            base_address = (buf[4] << 24) |
              (buf[5] << 16) |
              (buf[6] << 8) |
              (buf[7] << 0);
            
            peri_size = (buf[8] << 24) |
              (buf[9] << 16) |
              (buf[10] << 8) |
              (buf[11] << 0);
            
            if (!base_address)
            {
                /* looks like RPI 4 */
                base_address = (buf[8] << 24) |
                      (buf[9] << 16) |
                      (buf[10] << 8) |
                      (buf[11] << 0);
                      
                peri_size = (buf[12] << 24) |
                (buf[13] << 16) |
                (buf[14] << 8) |
                (buf[15] << 0);
            }
            /* check for valid known range formats */
            if ((buf[0] == 0x7e) &&
                    (buf[1] == 0x00) &&
                    (buf[2] == 0x00) &&
                    (buf[3] == 0x00) &&
                    ((base_address == BCM2835_PERI_BASE) || (base_address == BCM2835_RPI2_PERI_BASE) || (base_address == BCM2835_RPI4_PERI_BASE)))
            {
                bcm2835_peripherals_base = (off_t)base_address;
                bcm2835_peripherals_size = (size_t)peri_size;
                if( base_address == BCM2835_RPI4_PERI_BASE )
                {
                    pud_type_rpi4 = 1;
                }
            }
        
        }
        
	fclose(fp);
    }
    /* else we are prob on RPi 1 with BCM2835, and use the hardwired defaults */

    /* Now get ready to map the peripherals block 
     * If we are not root, try for the new /dev/gpiomem interface and accept
     * the fact that we can only access GPIO
     * else try for the /dev/mem interface and get access to everything
     */
    memfd = -1;
    ok = 0;
    if (geteuid() == 0)
    {
      /* Open the master /dev/mem device */
      if ((memfd = rtapi_open_as_root("/dev/mem", O_RDWR | O_SYNC) ) < 0) 
	{
	  fprintf(stderr, "bcm2835_init: Unable to open /dev/mem: %s\n",
		  strerror(errno)) ;
	  goto exit;
	}
      
      /* Base of the peripherals block is mapped to VM */
      bcm2835_peripherals = mapmem("gpio", bcm2835_peripherals_size, memfd, bcm2835_peripherals_base);
      if (bcm2835_peripherals == MAP_FAILED) goto exit;
      
      /* Now compute the base addresses of various peripherals, 
      // which are at fixed offsets within the mapped peripherals block
      // Caution: bcm2835_peripherals is uint32_t*, so divide offsets by 4
      */
      bcm2835_gpio = bcm2835_peripherals + BCM2835_GPIO_BASE/4;
      bcm2835_pwm  = bcm2835_peripherals + BCM2835_GPIO_PWM/4;
      bcm2835_clk  = bcm2835_peripherals + BCM2835_CLOCK_BASE/4;
      bcm2835_pads = bcm2835_peripherals + BCM2835_GPIO_PADS/4;
      bcm2835_spi0 = bcm2835_peripherals + BCM2835_SPI0_BASE/4;
      bcm2835_bsc0 = bcm2835_peripherals + BCM2835_BSC0_BASE/4; /* I2C */
      bcm2835_bsc1 = bcm2835_peripherals + BCM2835_BSC1_BASE/4; /* I2C */
      bcm2835_st   = bcm2835_peripherals + BCM2835_ST_BASE/4;
      bcm2835_aux  = bcm2835_peripherals + BCM2835_AUX_BASE/4;
      bcm2835_spi1 = bcm2835_peripherals + BCM2835_SPI1_BASE/4;

      ok = 1;
    }
    else
    {
      /* Not root, try /dev/gpiomem */
      /* Open the master /dev/mem device */
      if ((memfd = open("/dev/gpiomem", O_RDWR | O_SYNC) ) < 0) 
	{
	  fprintf(stderr, "bcm2835_init: Unable to open /dev/gpiomem: %s\n",
		  strerror(errno)) ;
	  goto exit;
	}
      
      /* Base of the peripherals block is mapped to VM */
      bcm2835_peripherals_base = 0;
      bcm2835_peripherals = mapmem("gpio", bcm2835_peripherals_size, memfd, bcm2835_peripherals_base);
      if (bcm2835_peripherals == MAP_FAILED) goto exit;
      bcm2835_gpio = bcm2835_peripherals;
      ok = 1;
    }

exit:
    if (memfd >= 0)
        close(memfd);

    if (!ok)
	bcm2835_close();

    return ok;
}

int UDP_init(void)
{
	int ret;

	// Create a UDP socket
	udpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket < 0)
	{
		rtapi_print("ERROR: can't open socket: %s\n", strerror(errno));
		return -errno;
	}

	bzero((char*) &dstAddr, sizeof(dstAddr));
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_addr.s_addr = inet_addr(dstAddress);
	dstAddr.sin_port = htons(DST_PORT);

	bzero((char*) &srcAddr, sizeof(srcAddr));
	srcAddr.sin_family = AF_INET;
	srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srcAddr.sin_port = htons(SRC_PORT);
	
	// bind the local socket to SCR_PORT
	ret = bind(udpSocket, (struct sockaddr *) &srcAddr, sizeof(srcAddr));
	if (ret < 0)
	{
		rtapi_print("ERROR: can't bind: %s\n", strerror(errno));
		return -errno;
	}
	
	// Connect to send and receive only to the server_addr
	ret = connect(udpSocket, (struct sockaddr*) &dstAddr, sizeof(struct sockaddr_in));
	if (ret < 0)
	{
		rtapi_print("ERROR: can't connect: %s\n", strerror(errno));
		return -errno;
	}

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = RECV_TIMEOUT_US;

	ret = setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
	if (ret < 0) {
	rtapi_print("ERROR: can't set receive timeout socket option: %s\n",
		strerror(errno));
	return -errno;
	}

	timeout.tv_usec = SEND_TIMEOUT_US;
	ret = setsockopt(udpSocket, SOL_SOCKET, SO_SNDTIMEO, (char*) &timeout,
	  sizeof(timeout));
	if (ret < 0) {
	rtapi_print("ERROR: can't set send timeout socket option: %s\n",
		strerror(errno));
	return -errno;
	}

	/* Disable the checksum, IPv4 allow no checksum */
	//  int opt = 1;
	//  ret = setsockopt(udpSocket, SOL_SOCKET, SO_NO_CHECK, &opt, sizeof(opt));
	//  if (ret < 0) {
	//    rtapi_print("ERROR: can't set checksum disable socket option: %s\n",
	//        strerror(errno));
	//    return -errno;
	//  }
	/* Note: Some installation does not use 'eth0' like 'ensp0' */
	//  ret = setsockopt(udpSocket, SOL_SOCKET, SO_BINDTODEVICE, "eth0", 4);
	//  if (ret < 0) {
	//    rtapi_print("ERROR: can't set bind to device socket option: %s\n",
	//        strerror(errno));
	//    return -errno;
	//  }
	return 0;
}

void update_freq(void *arg, long period)
{
	int i;
	data_t *data = (data_t *)arg;
	double max_ac, vel_cmd, dv, new_vel, max_freq, desired_freq;
		   
	double error, command, feedback;
	double periodfp, periodrecip;
	float pgain, ff1gain, deadband;

	// precalculate timing constants
    periodfp = period * 0.000000001;
    periodrecip = 1.0 / periodfp;

    // calc constants related to the period of this function (LinuxCNC SERVO_THREAD)
    // only recalc constants if period changes
    if (period != old_dtns) 			// Note!! period = LinuxCNC SERVO_PERIOD
	{
		old_dtns = period;				// get ready to detect future period changes
		dt = period * 0.000000001; 		// dt is the period of this thread, used for the position loop
		recip_dt = 1.0 / dt;			// calc the reciprocal once here, to avoid multiple divides later
    }

    // loop through generators
	for (i = 0; i < JOINTS; i++)
	{
		// check for scale change
		if (data->pos_scale[i] != data->old_scale[i])
		{
			data->old_scale[i] = data->pos_scale[i];		// get ready to detect future scale changes
			// scale must not be 0
			if ((data->pos_scale[i] < 1e-20) && (data->pos_scale[i] > -1e-20))	// validate the new scale value
				data->pos_scale[i] = 1.0;										// value too small, divide by zero is a bad thing
				// we will need the reciprocal, and the accum is fixed point with
				//fractional bits, so we precalc some stuff
			data->scale_recip[i] = (1.0 / STEP_MASK) / data->pos_scale[i];
		}

		// calculate frequency limit
		//max_freq = PRU_BASEFREQ/(4.0); 			//limit of DDS running at 80kHz
		//max_freq = PRU_BASEFREQ/(2.0); 	
		max_freq = PRU_base_freq/(2.0);

		// check for user specified frequency limit parameter
		if (data->maxvel[i] <= 0.0)
		{
			// set to zero if negative
			data->maxvel[i] = 0.0;
		}
		else
		{
			// parameter is non-zero, compare to max_freq
			desired_freq = data->maxvel[i] * fabs(data->pos_scale[i]);

			if (desired_freq > max_freq)
			{
				// parameter is too high, limit it
				data->maxvel[i] = max_freq / fabs(data->pos_scale[i]);
			}
			else
			{
				// lower max_freq to match parameter
				max_freq = data->maxvel[i] * fabs(data->pos_scale[i]);
			}
		}
		
		/* set internal accel limit to its absolute max, which is
		zero to full speed in one thread period */
		max_ac = max_freq * recip_dt;
		
		// check for user specified accel limit parameter
		if (data->maxaccel[i] <= 0.0)
		{
			// set to zero if negative
			data->maxaccel[i] = 0.0;
		}
		else 
		{
			// parameter is non-zero, compare to max_ac
			if ((data->maxaccel[i] * fabs(data->pos_scale[i])) > max_ac)
			{
				// parameter is too high, lower it
				data->maxaccel[i] = max_ac / fabs(data->pos_scale[i]);
			}
			else
			{
				// lower limit to match parameter
				max_ac = data->maxaccel[i] * fabs(data->pos_scale[i]);
			}
		}

		/* at this point, all scaling, limits, and other parameter
		changes have been handled - time for the main control */

		

		if (data->pos_mode[i]) {

			/* POSITION CONTROL MODE */

			// use Proportional control with feed forward (pgain, ff1gain and deadband)
			
			if (*(data->pgain[i]) != 0)
			{
				pgain = *(data->pgain[i]);
			}
			else
			{
				pgain = 1.0;
			}
			
			if (*(data->ff1gain[i]) != 0)
			{
				ff1gain = *(data->ff1gain[i]);
			}
			else
			{
				ff1gain = 1.0;
			}
			
			if (*(data->deadband[i]) != 0)
			{
				deadband = *(data->deadband[i]);
			}
			else
			{
				deadband = 1 / data->pos_scale[i];
			}	

			// read the command and feedback
			command = *(data->pos_cmd[i]);
			feedback = *(data->pos_fb[i]);
			
			// calcuate the error
			error = command - feedback;
			
			// apply the deadband
			if (error > deadband)
			{
				error -= deadband;
			}
			else if (error < -deadband)
			{
				error += deadband;
			}
			else
			{
				error = 0;
			}
			
			// calcuate command and derivatives
			data->cmd_d[i] = (command - data->prev_cmd[i]) * periodrecip;
			
			// save old values
			data->prev_cmd[i] = command;
				
			// calculate the output value
			vel_cmd = pgain * error + data->cmd_d[i] * ff1gain;
		
		} else {

			/* VELOCITY CONTROL MODE */
			
			// calculate velocity command in counts/sec
			vel_cmd = *(data->vel_cmd[i]);
		}	
			
		vel_cmd = vel_cmd * data->pos_scale[i];
			
		// apply frequency limit
		if (vel_cmd > max_freq) 
		{
			vel_cmd = max_freq;
		} 
		else if (vel_cmd < -max_freq) 
		{
			vel_cmd = -max_freq;
		}
		
		// calc max change in frequency in one period
		dv = max_ac * dt;
		
		// apply accel limit
		if ( vel_cmd > (data->freq[i] + dv) )
		{
			new_vel = data->freq[i] + dv;
		} 
		else if ( vel_cmd < (data->freq[i] - dv) ) 
		{
			new_vel = data->freq[i] - dv;
		}
		else
		{
			new_vel = vel_cmd;
		}
		
		// test for disabled stepgen
		if (*data->stepperEnable == 0) {
			// set velocity to zero
			new_vel = 0; 
		}
		
		data->freq[i] = new_vel;				// to be sent to the PRU
		*(data->freq_cmd[i]) = data->freq[i];	// feedback to LinuxCNC
	}

}


void spi_read()
{
	int i, ret;
	double curr_pos;

	// Data header
	txData.header = PRU_READ;
	
	if (*(data->SPIenable))
	{
		if( (*(data->SPIreset) && !(data->SPIresetOld)) || *(data->SPIstatus) )
		{
			// reset rising edge detected, try SPI transfer and reset OR PRU running
			
			// Transfer to and from the PRU
			//spi_transfer();
			
			bcm2835_gpio_set(reset_gpio_pin);
			// Send datagram
			ret = send(udpSocket, txData.txBuffer, sizeof(txData.header), 0);
			if (ret < 0)
			{
				rtapi_print("ERROR: send (READ), %s\n", strerror(errno));
				*(data->SPIstatus) = 0;
			}
			
			// Receive incoming datagram
			ret = recv(udpSocket, rxData.rxBuffer, BUFFER_SIZE, 0);
			if (ret < 0)
			{
				rtapi_print("ERROR: receive (READ), %s\n", strerror(errno));
				*(data->SPIstatus) = 0;
			}
			bcm2835_gpio_clr(reset_gpio_pin);
			
			switch (rxData.header)		// only process valid SPI payloads. This rejects bad payloads
			{
				case PRU_DATA:
					// we have received a GOOD payload from the PRU
					*(data->SPIstatus) = 1;

					for (i = 0; i < JOINTS; i++)
					{
						// the PRU DDS accumulator uses 32 bit counter, this code converts that counter into 64 bits */
						accum_diff = rxData.jointFeedback[i] - old_count[i];
						old_count[i] = rxData.jointFeedback[i];
						accum[i] += accum_diff;

						*(data->count[i]) = accum[i] >> STEPBIT;

						data->scale_recip[i] = (1.0 / STEP_MASK) / data->pos_scale[i];
						curr_pos = (double)(accum[i]-STEP_OFFSET) * (1.0 / STEP_MASK);
						*(data->pos_fb[i]) = (float)((curr_pos+0.5) / data->pos_scale[i]);
					}

					// Feedback
					for (i = 0; i < VARIABLES; i++)
					{
						*(data->processVariable[i]) = rxData.processVariable[i]; 
					}

					// Inputs
					for (i = 0; i < DIGITAL_INPUTS; i++)
					{
						if ((rxData.inputs & (1 << i)) != 0)
						{
							*(data->inputs[i]) = 1; 		// input is high
						}
						else
						{
							*(data->inputs[i]) = 0;			// input is low
						}
					}
					break;
				
				case PRU_ACKNOWLEDGE:
					// we've dropped a packet somewhere but comms are still up
					break;
					
				case PRU_ERR:
					// we've dropped a packet somewhere but comms are still up
					break;
				
				case PRU_ESTOP:
					// we have an eStop notification from the PRU
					*(data->SPIstatus) = 0;
					 rtapi_print_msg(RTAPI_MSG_ERR, "An E-stop is active");
					break;

				default:
					// we have received a BAD payload from the PRU
					*(data->SPIstatus) = 0;

					rtapi_print("Bad SPI payload = %x\n", rxData.header);
					//for (i = 0; i < BUFFER_SIZE; i++) {
					//	rtapi_print("%d\n",rxData.rxBuffer[i]);
					//}
					break;
			}
		}
	}
	else
	{
		*(data->SPIstatus) = 0;
	}
	
	data->SPIresetOld = *(data->SPIreset);
}


void spi_write()
{
	int i, ret;

	// Data header
	txData.header = PRU_WRITE;

	// Joint frequency commands
	for (i = 0; i < JOINTS; i++)
	{
		txData.jointFreqCmd[i] = data->freq[i];
	}

	for (i = 0; i < JOINTS; i++)
	{
		if (*(data->stepperEnable[i]) == 1)
		{
			txData.jointEnable |= (1 << i);		
		}
		else
		{
			txData.jointEnable &= ~(1 << i);	
		}
	}

	// Set points
	for (i = 0; i < VARIABLES; i++)
	{
		txData.setPoint[i] = *(data->setPoint[i]);
	}

	// Outputs
	for (i = 0; i < DIGITAL_OUTPUTS; i++)
	{
		if (*(data->outputs[i]) == 1)
		{
			txData.outputs |= (1 << i);		// output is high
		}
		else
		{
			txData.outputs &= ~(1 << i);	// output is low
		}
	}

	if( *(data->SPIstatus) )
	{
		// Transfer to and from the PRU
		//spi_transfer();
		
		bcm2835_gpio_set(reset_gpio_pin);
		// Send datagram
		ret = send(udpSocket, txData.txBuffer, BUFFER_SIZE, 0);
		if (ret < 0)
		{
			rtapi_print("ERROR: send (WRITE), %s\n", strerror(errno));
			*(data->SPIstatus) = 0;
		}

		// Receive incoming datagram
		ret = recv(udpSocket, rxData.rxBuffer, sizeof(rxData.header), 0);
		if (ret < 0)
		{
			rtapi_print("ERROR: receive (WRITE), %s\n", strerror(errno));
			*(data->SPIstatus) = 0;
		}
		bcm2835_gpio_clr(reset_gpio_pin);
		
		switch (rxData.header)
		{
			case PRU_DATA:
				// we've dropped a packet somewhere but comms are still up
				break;
			
			case PRU_ACKNOWLEDGE:
				// this is the response we expect
				break;
				
			case PRU_ERR:
				// there was a write error
				rtapi_print("Data write error: %x\n",rxData.header);
				break;
			
			case PRU_ESTOP:
				// we have an eStop notification from the PRU
				*(data->SPIstatus) = 0;
				 rtapi_print_msg(RTAPI_MSG_ERR, "An E-stop is active");
				break;

			default:
				// we have received a BAD payload from the PRU
				*(data->SPIstatus) = 0;

				rtapi_print("Bad SPI payload = %x\n", rxData.header);
				//for (i = 0; i < BUFFER_SIZE; i++) {
				//	rtapi_print("%d\n",rxData.rxBuffer[i]);
				//}
				break;
		}	
	}
}


void spi_transfer()
{
	int ret;

	// Send datagram
	ret = send(udpSocket, txData.txBuffer, BUFFER_SIZE, 0);
	if (ret < 0)
	{
		rtapi_print("ERROR: send %s\n", strerror(errno));
	}
	else {
	//    printf("Data sent %d\n", ret);
	}

	// Receive incoming datagram
	ret = recv(udpSocket, rxData.rxBuffer, BUFFER_SIZE, 0);
	if (ret < 0)
	{
		rtapi_print("ERROR: receive %s\n", strerror(errno));
	}
	else {
	//    rtapi_print("rxData %s\n", rxData.rx.buffer);
	}
}


static CONTROL parse_ctrl_type(const char *ctrl)
{
    if(!ctrl || !*ctrl || *ctrl == 'p' || *ctrl == 'P') return POSITION;
    if(*ctrl == 'v' || *ctrl == 'V') return VELOCITY;
    return INVALID;
}
