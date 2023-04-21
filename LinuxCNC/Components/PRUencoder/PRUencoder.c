/********************************************************************
* Description:  PRUencoder.c
*
*               This file, 'PRUencoder.c', is a HAL component that
*               provides an interface to the RemoraPRU raw encoder count.
*
*
* Author: Scott Alford
* License: GPL Version 2
*
* Copyright (c) 2020	All rights reserved.
*
* Last change:
********************************************************************/

#include "rtapi.h"		/* RTAPI realtime OS API */
#include "rtapi_app.h"		/* RTAPI realtime module decls */
#include "rtapi_string.h"
#include "hal.h"		/* HAL public API decls */

#define MODNAME "PRUencoder"
#define PREFIX "PRUencoder"

/* module information */
MODULE_AUTHOR("Scott Alford");
MODULE_DESCRIPTION("Remora PRU raw encoder interface");
MODULE_LICENSE("GPL v2");


static int num_encoder;		/* number of encoders */
static int default_num_encoder = 3;
static int howmany;
RTAPI_MP_INT(num_encoder, "number of encoder");

#define MAX_ENCODER 8

char *names[MAX_ENCODER] ={0,};
RTAPI_MP_ARRAY_STRING(names, MAX_ENCODER,"encoder names");



/***********************************************************************
*                STRUCTURES AND GLOBAL VARIABLES                       *
************************************************************************/


typedef struct {
    hal_bit_t 		*reset;				// counter reset input
	hal_float_t 	*raw_count;			// pin: input, raw encoder count
	hal_bit_t 		*phaseZ;			// index pulse input
	hal_bit_t 		*index_ena;			// index enable input
	hal_float_t 	count;				
	hal_float_t 	index_count;		// raw count value the encoder index position
	hal_float_t 	*pos;				// scaled position (floating point)
	hal_float_t		old_pos;
	hal_float_t 	*pos_scale;			// scaling factor for pos
	hal_float_t		*vel;
	double 			old_scale;			
	double 			scale;
} hal_encoder_t;

/* pointer to array of pid_t structs in shared memory, 1 per encoder */
static hal_encoder_t *encoder_array;

/* other globals */
static int comp_id;		/* component ID */
static const char 	*modname = MODNAME;
static const char 	*prefix = PREFIX;

/***********************************************************************
*                  LOCAL FUNCTION DECLARATIONS                         *
************************************************************************/

static int export_encoder(hal_encoder_t * addr,char * prefix);
static void capture(void *arg, long period);

/***********************************************************************
*                       INIT AND EXIT CODE                             *
************************************************************************/


int rtapi_app_main(void)
{
    int n, retval,i;
	char name[HAL_NAME_LEN + 1];

    if(num_encoder && names[0]) {
        rtapi_print_msg(RTAPI_MSG_ERR,"num_encoder= and names= are mutually exclusive\n");
        return -EINVAL;
    }
    if(!num_encoder && !names[0]) num_encoder = default_num_encoder;

    if(num_encoder) {
        howmany = num_encoder;
    } else {
        howmany = 0;
        for (i = 0; i < MAX_ENCODER; i++) {
            if ( (names[i] == NULL) || (*names[i] == 0) ){
                break;
            }
            howmany = i + 1;
        }
    }

    /* test for number of channels */
    if ((howmany <= 0) || (howmany > MAX_ENCODER)) {
	rtapi_print_msg(RTAPI_MSG_ERR,
	    "encoder: ERROR: invalid number of encoders: %d\n", howmany);
	return -1;
    }

    /* have good config info, connect to the HAL */
    comp_id = hal_init("PRUencoder");
    if (comp_id < 0) {
	rtapi_print_msg(RTAPI_MSG_ERR, "encoder: ERROR: hal_init() failed\n");
	return -1;
    }

    /* allocate shared memory for data */
    encoder_array = hal_malloc(howmany * sizeof(hal_encoder_t));
    if (encoder_array == 0) {
	rtapi_print_msg(RTAPI_MSG_ERR, "encoder: ERROR: hal_malloc() failed\n");
	hal_exit(comp_id);
	return -1;
    }

    /* export variables and function for each encoder */
    i = 0; // for names= items
    for (n = 0; n < howmany; n++) {
	/* export everything for this loop */
        if(num_encoder) {
            char buf[HAL_NAME_LEN + 1];
            rtapi_snprintf(buf, sizeof(buf), "encoder.%d", n);
	    retval = export_encoder(&(encoder_array[n]), buf);
        } else {
	    retval = export_encoder(&(encoder_array[n]), names[i++]);
        }

		if (retval != 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
			"PID: ERROR: loop %d var export failed\n", n);
			hal_exit(comp_id);
			return -1;
		}
    }
	
	
	rtapi_snprintf(name, sizeof(name), "%s.capture-position", prefix);
	retval = hal_export_funct(name, capture, encoder_array, 1, 0, comp_id);
    if (retval != 0) {
	rtapi_print_msg(RTAPI_MSG_ERR,
	    "ENCODER: ERROR: capture funct export failed\n");
	hal_exit(comp_id);
	return -1;
    }
	
    rtapi_print_msg(RTAPI_MSG_INFO, "PRUencoder: installed %d encoders\n",
	howmany);
    hal_ready(comp_id);
    return 0;
}

void rtapi_app_exit(void)
{
    hal_exit(comp_id);
}

/***********************************************************************
*                   REALTIME PID LOOP CALCULATIONS                     *
************************************************************************/

static void capture(void *arg, long period)
{
	hal_encoder_t *encoder;
	int n;
	float scale, delta_pos, delta_time;
	
	encoder = arg;
	
    for (n = 0; n < howmany; n++) {
	
		/* done interacting with update() */
		/* check for change in scale value */
		if ( *(encoder->pos_scale) != encoder->old_scale ) {
			/* save new scale to detect future changes */
			encoder->old_scale = *(encoder->pos_scale);
			/* scale value has changed, test and update it */
			if ((*(encoder->pos_scale) < 1e-20) && (*(encoder->pos_scale) > -1e-20)) {
			/* value too small, divide by zero is a bad thing */
			*(encoder->pos_scale) = 1.0;
			}
			/* we actually want the reciprocal */
			encoder->scale = 1.0 / *(encoder->pos_scale);
		}
	
		// check reset input
		if (*(encoder->reset)) {
			// reset is active, reset the counter
			// note: we NEVER reset raw_counts, that is always a
			// running count of edges seen since startup.  The
			// public "count" is the difference between raw_count
			// and index_count, so it will become zero.
			encoder->index_count = *(encoder->raw_count);
		}
		
		// check for index enabled and rising edges
		if (*(encoder->phaseZ) && *(encoder->index_ena)) {
			encoder->index_count = *(encoder->raw_count);
			*(encoder->index_ena) = 0;
		}
			
		// compute net counts
		encoder->count = *(encoder->raw_count) - encoder->index_count;
				
		// scale count to make floating point position
		encoder->old_pos = *(encoder->pos);
		*(encoder->pos) = encoder->count * encoder->scale;
		
		// compute velocity
		delta_pos = *(encoder->pos) - encoder->old_pos;
		delta_time = period * 1e-9;
		*(encoder->vel) = delta_pos / delta_time;  // position units per second
	
	// move on to next encoder
	encoder++;
	}
    /* done */
}

/***********************************************************************
*                   LOCAL FUNCTION DEFINITIONS                         *
************************************************************************/

static int export_encoder(hal_encoder_t * addr, char * prefix)
{
    int retval, msg;
    

    /* This function exports a lot of stuff, which results in a lot of
       logging if msg_level is at INFO or ALL. So we save the current value
       of msg_level and restore it later.  If you actually need to log this
       function's actions, change the second line below */
    msg = rtapi_get_msg_level();
    rtapi_set_msg_level(RTAPI_MSG_WARN);

    /* export pins */
	
    retval = hal_pin_bit_newf(HAL_IN, &(addr->phaseZ), comp_id,
            "%s.phase-Z", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_bit_newf(HAL_IO, &(addr->index_ena), comp_id,
            "%s.index-enable", prefix);
    if (retval != 0) {
	return retval;
    }
	
	retval = hal_pin_bit_newf(HAL_IN, &(addr->reset), comp_id,
            "%s.reset", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_IN, &(addr->raw_count), comp_id,
			"%s.raw_count", prefix);
    if (retval != 0) {
	return retval;
    }
	
	retval = hal_pin_float_newf(HAL_IN, &(addr->pos_scale), comp_id,
			"%s.position-scale", prefix);
    if (retval != 0) {
	return retval;
    }
	
    retval = hal_pin_float_newf(HAL_OUT, &(addr->pos), comp_id,
            "%s.position", prefix);
    if (retval != 0) {
	return retval;
    }
	
	retval = hal_pin_float_newf(HAL_OUT, &(addr->vel), comp_id,
            "%s.velocity", prefix);
    if (retval != 0) {
	return retval;
    }
	
    /* restore saved message level */
    rtapi_set_msg_level(msg);
    return 0;
}
