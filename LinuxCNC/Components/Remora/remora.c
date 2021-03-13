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


// Using BCM2835 driver library by Mike McCauley, why reinvent the wheel!
// http://www.airspayce.com/mikem/bcm2835/index.html
// Include these in the source directory when using "halcompile --install remora.c"
#include "bcm2835.h"
#include "bcm2835.c"

#include "remora.h"


#define MODNAME "remora"
#define PREFIX "remora"

MODULE_AUTHOR("Scott Alford AKA scotta");
MODULE_DESCRIPTION("Driver for Remora LPC1768 control board");
MODULE_LICENSE("GPL v2");

char *ctrl_type[JOINTS] = { "p" };
RTAPI_MP_ARRAY_STRING(ctrl_type,JOINTS,"control type (pos or vel)");

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
	hal_float_t 	pos_scale[JOINTS];			// param: steps per position unit
	float 			freq[JOINTS];				// param: frequency command sent to PRU
	hal_float_t 	*freq_cmd[JOINTS];			// pin: frequency command monitoring, available in LinuxCNC
	hal_float_t 	maxvel[JOINTS];				// param: max velocity, (pos units/sec)
	hal_float_t 	maxaccel[JOINTS];			// param: max accel (pos units/sec^2)
	float 			old_pos_cmd[JOINTS];		// previous position command (counts)
	float 			old_pos_cmd_raw[JOINTS];		// previous position command (counts)
	float 			old_scale[JOINTS];			// stored scale value
	float 			scale_recip[JOINTS];		// reciprocal value used for scaling
	hal_float_t 	*setPoint[VARIABLES];
	hal_float_t 	*processVariable[VARIABLES];
	hal_bit_t   	*outputs[DIGITAL_OUTPUTS];
	hal_bit_t   	*inputs[DIGITAL_INPUTS];
} data_t;

static data_t *data;


typedef union
{
  // this allow structured access to the outgoing SPI data without having to move it
  // this is the same structure as the PRU rxData structure
  struct
  {
    uint8_t txBuffer[SPIBUFSIZE];
  };
  struct
  {
	int32_t header;
	uint8_t jointEnable;
    int32_t jointFreqCmd[JOINTS];
    float 	setPoint[VARIABLES];
    uint8_t outputs;
  };
} txData_t;

static txData_t txData;


typedef union
{
  // this allow structured access to the incoming SPI data without having to move it
  // this is the same structure as the PRU txData structure
  struct
  {
    uint8_t rxBuffer[SPIBUFSIZE];
  };
  struct
  {
    int32_t header;
    int32_t jointFeedback[JOINTS];
    float 	processVariable[VARIABLES];
    uint8_t inputs;
  };
} rxData_t;

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

typedef enum CONTROL { POSITION, VELOCITY, INVALID } CONTROL;

static int reset_gpio_pin = 25;				// RPI GPIO pin number used to force watchdog reset of the PRU 



/***********************************************************************
*                  LOCAL FUNCTION DECLARATIONS                         *
************************************************************************/
static int rt_bcm2835_init(void);

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

	for (n = 0; n < JOINTS; n++) {
		if(parse_ctrl_type(ctrl_type[n]) == INVALID) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					"STEPGEN: ERROR: bad control type '%s' for axis %i (must be 'p' or 'v')\n",
					ctrl_type[n], n);
			return -1;
		}
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

	// Set the SPI0 pins to the Alt 0 function to enable SPI0 access, setup CS register
	// and clear TX and RX fifos
	if (!bcm2835_spi_begin())
    {
      rtapi_print_msg(RTAPI_MSG_ERR,"bcm2835_spi_begin failed. Are you running with root privlages??\n");
      return -1;
    }

	// Configure SPI0
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default

	//bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);		// 3.125MHz on RPI3
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);		// 6.250MHz on RPI3
	//bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);		// 12.5MHz on RPI3

    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default


	/* RPI_GPIO_P1_19        = 10 		MOSI when SPI0 in use
     * RPI_GPIO_P1_21        =  9 		MISO when SPI0 in use
     * RPI_GPIO_P1_23        = 11 		CLK when SPI0 in use
     * RPI_GPIO_P1_24        =  8 		CE0 when SPI0 in use
     * RPI_GPIO_P1_26        =  7 		CE1 when SPI0 in use
	 */

	// Configure pullups on SPI0 pins - source termination and CS high (does this allows for higher clock frequencies??? wiring is more important here)
	bcm2835_gpio_set_pud(RPI_GPIO_P1_19, BCM2835_GPIO_PUD_DOWN);	// MOSI
	bcm2835_gpio_set_pud(RPI_GPIO_P1_21, BCM2835_GPIO_PUD_DOWN);	// MISO
	bcm2835_gpio_set_pud(RPI_GPIO_P1_24, BCM2835_GPIO_PUD_UP);		// CS0

	// export spiPRU SPI enable and status bits
	retval = hal_pin_bit_newf(HAL_IN, &(data->SPIenable),
			comp_id, "%s.SPI-enable", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_IN, &(data->SPIreset),
			comp_id, "%s.SPI-reset", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_bit_newf(HAL_OUT, &(data->SPIstatus),
			comp_id, "%s.SPI-status", prefix);
	if (retval != 0) goto error;

	bcm2835_gpio_fsel(reset_gpio_pin, BCM2835_GPIO_FSEL_OUTP);
	retval = hal_pin_bit_newf(HAL_IN, &(data->PRUreset),
			comp_id, "%s.PRU-reset", prefix);
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

void update_freq(void *arg, long period)
{
	int i;
	data_t *data = (data_t *)arg;
	double max_ac, vel_cmd, dv, new_vel, max_freq, match_ac,
	       dp, pos_cmd, curr_pos, curr_vel, match_accl, match_time, avg_v,
	       est_out, est_cmd, est_err, desired_freq;


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


		// test for disabled stepgen
		if (*data->stepperEnable == 0) {
			// disabled: keep updating old_pos_cmd (if in pos ctrl mode)
			data->old_pos_cmd[i] = *data->pos_cmd[i] * data->pos_scale[i];
			// set velocity to zero
			data->freq[i] = 0; //CHECK THAT WE DO NOT OVER WRITE THIS LATER!! This probably needs a else statement
		}
	

		// calculate frequency limit
		//max_freq = PRU_BASEFREQ/(4.0); 			//limit of DDS running at 80kHz
		max_freq = PRU_BASEFREQ/(2.0); 	


		// check for user specified frequency limit parameter
		if (data->maxvel[i] <= 0.0) {
			// set to zero if negative
			data->maxvel[i] = 0.0;
		} else {
			// parameter is non-zero, compare to max_freq
			desired_freq = data->maxvel[i] * fabs(data->pos_scale[i]);
			if (desired_freq > max_freq) {
				// parameter is too high, limit it
				data->maxvel[i] = max_freq / fabs(data->pos_scale[i]);
			} else {
			// lower max_freq to match parameter
			max_freq = data->maxvel[i] * fabs(data->pos_scale[i]);
			}
		}

		/* set internal accel limit to its absolute max, which is
		zero to full speed in one thread period */
		max_ac = max_freq * recip_dt;
		/* check for user specified accel limit parameter */
		if (data->maxaccel[i] <= 0.0) {
			/* set to zero if negative */
			data->maxaccel[i] = 0.0;
		} else {
			/* parameter is non-zero, compare to max_ac */
				if ((data->maxaccel[i] * fabs(data->pos_scale[i])) > max_ac) {
				/* parameter is too high, lower it */
				data->maxaccel[i] = max_ac / fabs(data->pos_scale[i]);
				} else {
				/* lower limit to match parameter */
				max_ac = data->maxaccel[i] * fabs(data->pos_scale[i]);
				}
		}

		/* at this point, all scaling, limits, and other parameter
		changes have been handled - time for the main control */

		

		if (data->pos_mode[i]) {

			/* POSITION CONTROL MODE */

			/* calculate position command in counts */
			pos_cmd = *(data->pos_cmd[i]) * data->pos_scale[i];
			/* calculate velocity command in counts/sec */
			vel_cmd = (pos_cmd - data->old_pos_cmd[i]) * recip_dt;
			data->old_pos_cmd[i] = pos_cmd;


			// convert from fixed point to double, after subtracting the one-half step offset
			///curr_pos = (double)(accum[i]) * (1.0 / STEP_MASK);
			//curr_pos = (double)(accum[i]-STEP_OFFSET) * (1.0 / STEP_MASK);
			//*(data->pos_fb[i]) = (float)(accum[i]-STEP_OFFSET) * (1.0 / STEP_MASK) / data->pos_scale[i];
			///*(data->pos_fb[i]) = (float)(curr_pos / data->pos_scale[i]);
			//*(data->pos_fb[i]) = (float)(curr_pos * data->scale_recip[i]);
			//*(data->pos_fb[i]) = (double)(accum[i]-STEP_OFFSET) * data->scale_recip[i];
			
			curr_pos = (double)(accum[i]-STEP_OFFSET) * (1.0 / STEP_MASK)+STEP_OFFSET*(1/STEP_MASK);
			*(data->pos_fb[i]) = (float)(curr_pos / data->pos_scale[i]);

			/* get velocity in counts/sec */
			curr_vel = data->freq[i];
			/* At this point we have good values for pos_cmd, curr_pos,
			   vel_cmd, curr_vel, max_freq and max_ac, all in counts,
			   counts/sec, or counts/sec^2.  Now we just have to do
			   something useful with them. */

			/* determine which way we need to ramp to match velocity */
			if (vel_cmd > curr_vel) {
				match_ac = max_ac;
			} else {
				match_ac = -max_ac;
			}

			/* determine how long the match would take */
			match_time = (vel_cmd - curr_vel) / match_ac;
			/* calc output position at the end of the match */
			avg_v = (vel_cmd + curr_vel) * 0.5;
			est_out = curr_pos + avg_v * match_time;
			/* calculate the expected command position at that time */
			est_cmd = pos_cmd + vel_cmd * (match_time - 1.5 * dt);
			/* calculate error at that time */
			est_err = est_out - est_cmd;

			if (match_time < dt) {
				/* we can match velocity in one period */
				if (fabs(est_err) < 0.0001) {
					/* after match the position error will be acceptable */
					/* so we just do the velocity match */
					new_vel = vel_cmd;
				} else {
					/* try to correct position error */
					new_vel = vel_cmd - 0.5 * est_err * recip_dt;
					/* apply accel limits */
					if (new_vel > (curr_vel + max_ac * dt)) {
						new_vel = curr_vel + max_ac * dt;
					} else if (new_vel < (curr_vel - max_ac * dt)) {
						new_vel = curr_vel - max_ac * dt;
					}
				}
			} else {
				/* calculate change in final position if we ramp in the
				opposite direction for one period */
				dv = -2.0 * match_ac * dt;
				dp = dv * match_time;
				/* decide which way to ramp */
				if (fabs(est_err + dp * 2.0) < fabs(est_err)) {
					match_ac = -match_ac;
				}
				/* and do it */
				new_vel = curr_vel + match_ac * dt;
			}


			/* apply frequency limit */
			if (new_vel > max_freq) {
				new_vel = max_freq;
			} else if (new_vel < -max_freq) {
				new_vel = -max_freq;
			}
			/* end of position mode */
			
			// Stop hunting - for some reason steppers shudder by 1 step when at position. Due to DDS offset?
			//if (fabs(*data->pos_cmd[i]-*(data->pos_fb[i])) < 0.01) new_vel = 0.0;
		
		} else {

			/* VELOCITY CONTROL MODE */
			
			/* velocity mode is simpler */
			/* calculate velocity command in counts/sec */
			vel_cmd = *(data->vel_cmd[i]) * data->pos_scale[i];
			/* apply frequency limit */
			if (vel_cmd > max_freq) {
			vel_cmd = max_freq;
			} else if (vel_cmd < -max_freq) {
			vel_cmd = -max_freq;
			}
			/* calc max change in frequency in one period */
			dv = max_ac * dt;
			/* apply accel limit */
			if ( vel_cmd > (data->freq[i] + dv) ) {
			new_vel = data->freq[i] + dv;
			} else if ( vel_cmd < (data->freq[i] - dv) ) {
			new_vel = data->freq[i] - dv;
			} else {
			new_vel = vel_cmd;
			}
			/* end of velocity mode */
		}

		data->freq[i] = new_vel;					// to be sent to the PRU
		*(data->freq_cmd[i]) = (float)(new_vel);	// feedback to LinuxCNC

	}

}


void spi_read()
{
	int i, j;

	// Data header
	txData.header = PRU_READ;
	
	// update the PRUreset output
	if (*(data->PRUreset))
	{ 
		bcm2835_gpio_set(reset_gpio_pin);
    }
	else
	{
		bcm2835_gpio_clr(reset_gpio_pin);
    }
	
	if (*(data->SPIenable))
	{
		if( (*(data->SPIreset) && !(data->SPIresetOld)) || *(data->SPIstatus) )
		{
			// reset rising edge detected, try SPI transfer and reset OR PRU running
			
			// Transfer to and from the PRU
			spi_transfer();

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

						//*(data->pos_fb[i]) = (float)(accum[i]-STEP_OFFSET) * data->scale_recip[i]; //-STEP_OFFSET
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

				default:
					// we have received a BAD payload from the PRU
					*(data->SPIstatus) = 0;

					rtapi_print("Bad SPI payload = %x\n", rxData.header);
					//for (i = 0; i < SPIBUFSIZE; i++) {
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
	int i;

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
		spi_transfer();
	}

}


void spi_transfer()
{
	// send and receive data to and from the Remora PRU concurrently

	int i;

	for (i = 0; i < SPIBUFSIZE; i++)
	{
		rxData.rxBuffer[i] = bcm2835_spi_transfer(txData.txBuffer[i]);
	}

}

static CONTROL parse_ctrl_type(const char *ctrl)
{
    if(!ctrl || !*ctrl || *ctrl == 'p' || *ctrl == 'P') return POSITION;
    if(*ctrl == 'v' || *ctrl == 'V') return VELOCITY;
    return INVALID;
}
