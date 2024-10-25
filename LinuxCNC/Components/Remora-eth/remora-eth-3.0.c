/********************************************************************
* Description:  remora-eth.c
*               This file, 'remora-eth.c', is a HAL component that
*               provides an ethernet connection to a external Enthernet
* 			 	controller running Remora PRU firmware.
*  				
*
* Author: Scott Alford
* License: GPL Version 2
*
*		Credit to GP Orcullo and PICnc V2 which originally inspired this
*		and portions of this code is based on stepgen.c by John Kasunich
*		and hm2_rpspi.c by Matsche
*
* Copyright (c) 2023 All rights reserved.
*
* Last change:
********************************************************************/


#include "rtapi.h"			/* RTAPI realtime OS API */
#include "rtapi_app.h"		/* RTAPI realtime module decls */
#include "hal.h"			/* HAL public API decls */

#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <math.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#include "remora-eth-3.0.h"

#define MODNAME "remora-eth-3.0"
#define PREFIX "remora"

MODULE_AUTHOR("Scott Alford AKA scotta");
MODULE_DESCRIPTION("Driver for Remora Ethernet capable control board");
MODULE_LICENSE("GPL v2");


/***********************************************************************
*                STRUCTURES AND GLOBAL VARIABLES                       *
************************************************************************/

typedef struct {
	hal_bit_t		*enable;
	hal_bit_t		*reset;
	hal_bit_t		*PRUreset;
	bool			resetOld;
	hal_bit_t		*status;
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
	float 			old_pos_cmd_raw[JOINTS];	// previous position command (counts)
	float 			old_scale[JOINTS];			// stored scale value
	float 			scale_recip[JOINTS];		// reciprocal value used for scaling
	float			prev_cmd[JOINTS];
	float			cmd_d[JOINTS];				// command derivative
	hal_float_t 	*setPoint[VARIABLES];
	hal_float_t 	*processVariable[VARIABLES];
	hal_bit_t   	*outputs[DIGITAL_OUTPUTS];
	hal_bit_t   	*inputs[DIGITAL_INPUTS*2];
	hal_bit_t   	*NVMPGinputs[NVMPG_INPUTS];
} data_t;

static data_t *data;


#pragma pack(push, 1)

typedef union
{
  // this allow structured access to the outgoing data without having to move it
  // this is the same structure as the Remora rxData structure
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
	uint32_t outputs;
    uint8_t spare0;
  };
} txData_t;


typedef union
{
  // this allow structured access to the incoming data without having to move it
  // this is the same structure as the Remora txData structure
  struct
  {
    uint8_t rxBuffer[BUFFER_SIZE];
  };
  struct
  {
    int32_t header;
    int32_t jointFeedback[JOINTS];
    float 	processVariable[VARIABLES];
    uint32_t inputs;
	uint16_t NVMPGinputs;
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

static int32_t 		count[JOINTS] = { 0 };
static int32_t 		old_count[JOINTS] = { 0 };
static int32_t 		filter_count[JOINTS] = { 0 };
static int32_t		accum_diff = 0;

static int 			reset_gpio_pin = 25;				// debug pin

typedef enum CONTROL { POSITION, VELOCITY, INVALID } CONTROL;
char *ctrl_type[JOINTS] = { "p" };
RTAPI_MP_ARRAY_STRING(ctrl_type,JOINTS,"control type (pos or vel)");

int PRU_base_freq = -1;
RTAPI_MP_INT(PRU_base_freq, "PRU base thread frequency");

#define DST_PORT 27181
#define SRC_PORT 27181
#define SEND_TIMEOUT_US 10
#define RECV_TIMEOUT_US 10
#define READ_PCK_DELAY_NS 10000

static int udpSocket;
static int errCount;
struct sockaddr_in dstAddr, srcAddr;
struct hostent *server;
static const char *dstAddress = "10.10.10.10";

/***********************************************************************
*                  LOCAL FUNCTION DECLARATIONS                         *
************************************************************************/

static int UDP_init(void);

static void update_freq(void *arg, long period);
static void pru_write();
static void pru_read();
static void pru_transfer(int txSize, int rxSize);
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
		if ((PRU_base_freq < 40000) || (PRU_base_freq > 500000))
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
	
	// Initialize the UDP socket
	if (UDP_init() < 0)
	{
		rtapi_print_msg(RTAPI_MSG_ERR, "Error: The board is unreachable\n");
		return -1;
	}

	// export spiPRU SPI enable and status bits
	retval = hal_pin_bit_newf(HAL_IN, &(data->enable),
			comp_id, "%s.enable", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_IN, &(data->reset),
			comp_id, "%s.reset", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_bit_newf(HAL_OUT, &(data->status),
			comp_id, "%s.status", prefix);
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
				comp_id, "%s.output.%02d", prefix, n);
		if (retval != 0) goto error;
		*(data->outputs[n])=0;
	}

	for (n = 0; n < DIGITAL_INPUTS; n++) {
		retval = hal_pin_bit_newf(HAL_OUT, &(data->inputs[n]),
				comp_id, "%s.input.%02d", prefix, n);
		if (retval != 0) goto error;
		*(data->inputs[n])=0;

		retval = hal_pin_bit_newf(HAL_OUT, &(data->inputs[n+DIGITAL_INPUTS]),
				comp_id, "%s.input.%02d.not", prefix, n);
		if (retval != 0) goto error;
		*(data->inputs[n+DIGITAL_INPUTS])=1;

	}
	
	for (n = 0; n < NVMPG_INPUTS; n++) {
		retval = hal_pin_bit_newf(HAL_OUT, &(data->NVMPGinputs[n]),
				comp_id, "%s.NVMPGinput.%01d", prefix, n);
		if (retval != 0) goto error;
		*(data->NVMPGinputs[n])=0;
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
	retval = hal_export_funct(name, pru_write, 0, 0, 0, comp_id);
	if (retval < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
		        "%s: ERROR: write function export failed\n", modname);
		hal_exit(comp_id);
		return -1;
	}

	rtapi_snprintf(name, sizeof(name), "%s.read", prefix);
	retval = hal_export_funct(name, pru_read, data, 1, 0, comp_id);
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
		max_freq = PRU_base_freq;

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
				deadband = fabs(1/data->pos_scale[i]);
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


void pru_read()
{
	int i, ret;
	double curr_pos;
	
	// following error spike filter parameters
	int n = 2;
	int M = 250;

	// Data header
	txData.header = PRU_READ;
	
	if (*(data->enable))
	{
		if( (*(data->reset) && !(data->resetOld)) || *(data->status) )
		{
			// reset rising edge detected, try transfer and reset OR PRU running
			
			// Transfer to and from the PRU
			pru_transfer(sizeof(txData.header), BUFFER_SIZE);
			
			switch (rxData.header)		// only process valid SPI payloads. This rejects bad payloads
			{
				case PRU_DATA:
					// we have received a GOOD payload from the PRU
					*(data->status) = 1;

					for (i = 0; i < JOINTS; i++)
					{
						old_count[i] = count[i];
						count[i] = rxData.jointFeedback[i];
						accum_diff = count[i] - old_count[i];
						
						// spike filter
						if (abs(count[i] - old_count[i]) > M && filter_count[i] < n)
						{
							// recent big change: hold previous value
							++filter_count[i];
							count[i] = old_count[i];
							rtapi_print("Spike filter active[%d][%d]: %d\n", i, filter_count[i], accum_diff);
						}
						else
						{
							// normal operation, or the big change must be real after all
							filter_count[i] = 0;
						}
						
						*(data->count[i]) = count[i];
						*(data->pos_fb[i]) = (float)(count[i]) / data->pos_scale[i];
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
							*(data->inputs[i+DIGITAL_INPUTS]) = 0; 		// inverted
						}
						else
						{
							*(data->inputs[i]) = 0;			// input is low
							*(data->inputs[i+DIGITAL_INPUTS]) = 1; 		// inverted
						}
					}
					
					// NVMPG Inputs
					for (i = 0; i < NVMPG_INPUTS; i++)
					{
						if ((rxData.NVMPGinputs & (1 << i)) != 0)
						{
							*(data->NVMPGinputs[i]) = 1; 		// input is high
						}
						else
						{
							*(data->NVMPGinputs[i]) = 0;			// input is low
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
					*(data->status) = 0;
					 rtapi_print_msg(RTAPI_MSG_ERR, "An E-stop is active");
					break;

				default:
					// we have received a BAD payload from the PRU
					*(data->status) = 0;
					rtapi_print("Bad payload = %x\n", rxData.header);
					break;
			}
		}
	}
	else
	{
		*(data->status) = 0;
	}
	
	data->resetOld = *(data->reset);
}


void pru_write()
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

	if( *(data->status) )
	{
		// Transfer to and from the PRU
		pru_transfer(BUFFER_SIZE, sizeof(rxData.header));
		
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
				*(data->status) = 0;
				 rtapi_print_msg(RTAPI_MSG_ERR, "An E-stop is active");
				break;

			default:
				// we have received a BAD payload from the PRU
				*(data->status) = 0;
				rtapi_print("Bad payload = %x\n", rxData.header);
				break;
		}	
	}
}


void pru_transfer(int txSize, int rxSize)
{
	int ret;
	long long t1, t2;

	// Send datagram
	ret = send(udpSocket, txData.txBuffer, txSize, 0);

	// Receive incoming datagram
    t1 = rtapi_get_time();
    do {
        ret = recv(udpSocket, rxData.rxBuffer, rxSize, 0);
        if(ret < 0) rtapi_delay(READ_PCK_DELAY_NS);
        t2 = rtapi_get_time();
    } while ((ret < 0) && ((t2 - t1) < 200*1000*1000));

	if (ret > 0)
	{
		errCount = 0;
	}
	else
	{
		errCount++;
	}
	
	if (errCount > 2)
	{
		*(data->status) = 0;
		rtapi_print("Ethernet ERROR: %s\n", strerror(errno));
	}
}


static CONTROL parse_ctrl_type(const char *ctrl)
{
    if(!ctrl || !*ctrl || *ctrl == 'p' || *ctrl == 'P') return POSITION;
    if(*ctrl == 'v' || *ctrl == 'V') return VELOCITY;
    return INVALID;
}
