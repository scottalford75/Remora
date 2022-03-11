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


#define MODNAME "nvmpg"
#define PREFIX "nvmpg"
#define PRU_MPG	0x6D706764 	// "mpgd" mpg data payload

MODULE_AUTHOR("Scott Alford AKA scotta");
MODULE_DESCRIPTION("Driver for Remora with NVMPG");
MODULE_LICENSE("GPL v2");


/***********************************************************************
*                STRUCTURES AND GLOBAL VARIABLES                       *
************************************************************************/

typedef struct {
	hal_float_t		*updateFreq;
	hal_bit_t		*commsStatus;
	hal_float_t		*xPos;
	hal_float_t		*yPos;
	hal_float_t		*zPos;
	hal_float_t		*aPos;
	hal_float_t		*bPos;
	hal_float_t		*cPos;
	float			xPos_old;
	float			yPos_old;
	float			zPos_old;
	float			aPos_old;
	float			bPos_old;
	float			cPos_old;
	hal_bit_t		*reset;
	hal_float_t		*spindleRPM;
	hal_bit_t		*spindleOn;
	hal_s32_t		*feedOverrideCounts; 	//counts X scale = feed override percentage
	hal_s32_t		feedOverrideCounts_old;
	hal_float_t		*feedOverrideScale;
	hal_float_t		*jogOverride;			// what to do with Jog Override?
	hal_s32_t		*spindleOverrideCounts; //counts X scale = spindle override percentage
	hal_s32_t		spindleOverrideCounts_old;
	hal_float_t		*spindleOverrideScale;
	hal_bit_t		*parameterInc;			// input from push button
	hal_bit_t		*axisUp;				// input from push button
	hal_bit_t		*axisDown;				// input from push button
	hal_bit_t		*multiplierInc;			// input from push button
	hal_bit_t		*xSelect;
	hal_bit_t		*ySelect;
	hal_bit_t		*zSelect;
	hal_bit_t		*aSelect;
	hal_bit_t		*bSelect;
	hal_bit_t		*cSelect;
	hal_float_t		*mpgX1inc;
	hal_float_t		*mpgScale;
} data_t;

static data_t *data;

#pragma pack(push, 1)
typedef union
{
	struct
	{
		uint8_t payload[57];
	};
	struct
	{
		int32_t	header;
		int8_t	byte0;
		int8_t	byte1;
		int32_t xPos;
		int32_t yPos;
		int32_t zPos;
		int32_t aPos;
		int32_t bPos;
		int32_t cPos;
		int8_t	byte24;
		int8_t	reset;
		int8_t	byte26;
		int32_t spindle_rpm;
		int8_t	spindle_on;
		int8_t	feed_rate_override;
		int8_t	slow_jog_rate;
		int8_t	spindle_rate_override;
		int8_t	spare35;
		int8_t	parameter_select;
		int8_t	axis_select;
		int8_t	mpg_multiplier;
		int8_t	spare39;
		int8_t	spare40;
		int8_t	spare41;
		int8_t	spare42;
		int8_t	spare43;
		int8_t	spare44;
		int8_t	spare45;
		int8_t	spare46;
		int8_t	spare47;
		int8_t	spare48;
		int8_t	spare49;
		int8_t	spare50;
	};
} mpgData_t;

#pragma pack(pop)

static mpgData_t mpgData;

/* other globals */
static int 			comp_id;				// component ID
static const char 	*modname = MODNAME;
static const char 	*prefix = PREFIX;
static long 		old_dtns;				// update function period in nsec
static double		dt;						// update funcion period in seconds
static double 		recip_dt;				// recprocal of period

int update_count, update_counter;
bool updateFlag;
int buttonState[3];
int selectedAxis; 
int selectedMultiplier;

#define DST_PORT 27182
#define SRC_PORT 27182
#define SEND_TIMEOUT_US 50
#define RECV_TIMEOUT_US 50

static int udpSocket;
struct sockaddr_in dstAddr, srcAddr;
struct hostent *server;
static const char *dstAddress = "10.10.10.10";

/***********************************************************************
*                  LOCAL FUNCTION DECLARATIONS                         *
************************************************************************/
static int UDP_init(void);

static void update(void *arg, long period);



/***********************************************************************
*                       INIT AND EXIT CODE                             *
************************************************************************/

int rtapi_app_main(void)
{
    char name[HAL_NAME_LEN + 1];
	int n, retval;

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

	/* Initialize the UDP socket */
	if (UDP_init() < 0)
	{
		rtapi_print_msg(RTAPI_MSG_ERR, "Error: The board is unreachable\n");
		return -1;
	}

	// export variables
	retval = hal_pin_float_newf(HAL_IN, &(data->updateFreq),
			comp_id, "%s.update-freq", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_bit_newf(HAL_IN, &(data->commsStatus),
			comp_id, "%s.comms-status", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_float_newf(HAL_IN, &(data->xPos),
			comp_id, "%s.x-pos", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_float_newf(HAL_IN, &(data->yPos),
			comp_id, "%s.y-pos", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_float_newf(HAL_IN, &(data->zPos),
			comp_id, "%s.z-pos", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_float_newf(HAL_IN, &(data->aPos),
			comp_id, "%s.a-pos", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_float_newf(HAL_IN, &(data->bPos),
			comp_id, "%s.b-pos", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_float_newf(HAL_IN, &(data->cPos),
			comp_id, "%s.c-pos", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_IN, &(data->reset),
			comp_id, "%s.reset", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_float_newf(HAL_IN, &(data->spindleRPM),
			comp_id, "%s.spindle-rpm", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_IN, &(data->spindleOn),
			comp_id, "%s.spindle-on", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_s32_newf(HAL_IN, &(data->feedOverrideCounts),
			comp_id, "%s.feed-override-counts", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_float_newf(HAL_IN, &(data->feedOverrideScale),
			comp_id, "%s.feed-override-scale", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_s32_newf(HAL_IN, &(data->spindleOverrideCounts),
			comp_id, "%s.spindle-override-counts", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_float_newf(HAL_IN, &(data->spindleOverrideScale),
			comp_id, "%s.spindle-override-scale", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_bit_newf(HAL_IN, &(data->parameterInc),
			comp_id, "%s.parameter-inc", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_bit_newf(HAL_IN, &(data->axisUp),
			comp_id, "%s.axis-up", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_bit_newf(HAL_IN, &(data->axisDown),
			comp_id, "%s.axis-down", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_IN, &(data->multiplierInc),
			comp_id, "%s.multiplier-inc", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_OUT, &(data->xSelect),
			comp_id, "%s.x-select", prefix);
	if (retval != 0) goto error;
	
	*data->xSelect = 1;
	
	retval = hal_pin_bit_newf(HAL_OUT, &(data->ySelect),
			comp_id, "%s.y-select", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_OUT, &(data->zSelect),
			comp_id, "%s.z-select", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_OUT, &(data->aSelect),
			comp_id, "%s.a-select", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_OUT, &(data->bSelect),
			comp_id, "%s.b-select", prefix);
	if (retval != 0) goto error;
	
	retval = hal_pin_bit_newf(HAL_OUT, &(data->cSelect),
			comp_id, "%s.c-select", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_float_newf(HAL_IN, &(data->mpgX1inc),
			comp_id, "%s.mpg-x1-inc", prefix);
	if (retval != 0) goto error;

	retval = hal_pin_float_newf(HAL_OUT, &(data->mpgScale),
			comp_id, "%s.mpg-scale", prefix);
	if (retval != 0) goto error;
	
	*data->mpgScale = 1 * *data->mpgX1inc;


	error:
	if (retval < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
		        "%s: ERROR: pin export failed with err=%i\n",
		        modname, retval);
		hal_exit(comp_id);
		return -1;
	}

	// Export functions
	rtapi_snprintf(name, sizeof(name), "%s.update", prefix);
	retval = hal_export_funct(name, update, data, 1, 0, comp_id);
	if (retval < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
		        "%s: ERROR: update function export failed\n", modname);
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


void update(void *arg, long period)
{
	int ret;
	data_t *data = (data_t *)arg;
	
	int32_t update_freq;

    // calc constants related to the period of this function
    // only recalc constants if period changes
    if (period != old_dtns) 			
	{
		old_dtns = period;				// get ready to detect future period changes
		dt = period * 0.000000001; 		// dt is the period of this thread
		recip_dt = 1.0 / dt;			// recipt_dt is the frequency of this thread
    }

	// calculate the update_count
	update_count = recip_dt / *data->updateFreq;
	//rtapi_print("update_count = %d\n", update_count);

	// update the mpg at the updateFreq;
	if (update_counter >= update_count)
	{
		update_counter = 0;
		
		//rtapi_print("Update the mpg\n");
		
		// do updates
		if (*data->xPos != data->xPos_old) 
		{
			data->xPos_old = *data->xPos;
			mpgData.xPos = *data->xPos * 1000;
			updateFlag = true;
		}
		
		if (*data->yPos != data->yPos_old) 
		{
			data->yPos_old = *data->yPos;
			mpgData.yPos = *data->yPos * 1000;
			updateFlag = true;
		}
		
		if (*data->zPos != data->zPos_old) 
		{
			data->zPos_old = *data->zPos;
			mpgData.zPos = *data->zPos * 1000;
			updateFlag = true;
		}

		if (*data->aPos != data->aPos_old) 
		{
			data->aPos_old = *data->aPos;
			mpgData.aPos = *data->aPos * 1000;
			updateFlag = true;
		}
		
		if (*data->bPos != data->bPos_old) 
		{
			data->bPos_old = *data->bPos;
			mpgData.bPos = *data->bPos * 1000;
			updateFlag = true;
		}
		
		if (*data->cPos != data->cPos_old) 
		{
			data->cPos_old = *data->cPos;
			mpgData.cPos = *data->cPos * 1000;
			updateFlag = true;
		}
		
		if (*data->spindleRPM != mpgData.spindle_rpm) 
		{
			mpgData.spindle_rpm = *data->spindleRPM;
			updateFlag = true;
		}
		
		if (*data->spindleOn != mpgData.spindle_on) 
		{
			mpgData.spindle_on = *data->spindleOn;
			updateFlag = true;
		}
		
		if (*data->feedOverrideCounts != data->feedOverrideCounts_old)
		{
			data->feedOverrideCounts_old = *data->feedOverrideCounts;
			mpgData.feed_rate_override = *data->feedOverrideCounts * *data->feedOverrideScale;
			updateFlag = true;
		}

		if (*data->spindleOverrideCounts != data->spindleOverrideCounts_old)
		{
			data->spindleOverrideCounts_old = *data->spindleOverrideCounts;
			mpgData.spindle_rate_override = *data->spindleOverrideCounts * *data->spindleOverrideScale;
			updateFlag = true;
		}
	
		if ((*data->axisUp != buttonState[0]) || (*data->axisDown != buttonState[1]))
		{
			buttonState[0] = *data->axisUp;
			buttonState[1] = *data->axisDown;
			if (buttonState[0] == 1)
			{
				selectedAxis--;
			}
			else if (buttonState[1] == 1)
			{
				selectedAxis++;
			}	
			
			if (selectedAxis > 5) selectedAxis = 0;
			if (selectedAxis < 0) selectedAxis = 5;
			mpgData.axis_select = selectedAxis;
				
			*data->xSelect = 0;
			*data->ySelect = 0;
			*data->zSelect = 0;
			*data->aSelect = 0;
			*data->bSelect = 0;
			*data->cSelect = 0;
					
			switch (selectedAxis)
			{
				case 0:
					*data->xSelect = 1;
					break;

				case 1:
					*data->ySelect = 1;
					break;	
					
				case 2:
					*data->zSelect = 1;
					break;

				case 3:
					*data->aSelect = 1;
					break;	

				case 4:
					*data->bSelect = 1;
					break;

				case 5:
					*data->cSelect = 1;
					break;							
			}
			updateFlag = true;
		}
		
		if (*data->multiplierInc != buttonState[2])
		{
			buttonState[2] = *data->multiplierInc;
			if (buttonState[2] == 1)
			{
				selectedMultiplier++;
				if (selectedMultiplier > 3) selectedMultiplier = 0;
				mpgData.mpg_multiplier = selectedMultiplier;
				switch (selectedMultiplier)
				{
					case 0:
						*data->mpgScale = 1 * *data->mpgX1inc;
						break;
						
					case 1:
						*data->mpgScale = 10 * *data->mpgX1inc;
						break;
						
					case 2:
						*data->mpgScale = 100 * *data->mpgX1inc;
						break;
						
					case 3:
						*data->mpgScale = 1000 * *data->mpgX1inc;
						break;
				}
				updateFlag = true;
			}
		}
	
		if (updateFlag && *(data->commsStatus))
		{
			mpgData.header = PRU_MPG;
			mpgData.byte0 = 0x5a;
			mpgData.byte1 = 0x5a;
			
			// Send datagram
			ret = send(udpSocket, mpgData.payload, sizeof(mpgData.payload), 0);
			if (ret < 0)
			{
				rtapi_print("ERROR: send (WRITE), %s\n", strerror(errno));
			}
			
			updateFlag = false;
		}
	}
	
	update_counter++;
}

