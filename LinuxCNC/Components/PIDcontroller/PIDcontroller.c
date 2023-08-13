/********************************************************************
* Description:  PIDcontroller.c
*
*               This file, 'PIDcontroller.c', is a HAL component that
*               provides a a generic PID controller.
*
*				Based on Arduino PID v1.2.1 by Bret Beauregard
*				pid.c used as a basis for command line passing
*
* Author: Scott Alford
* License: GPL Version 2
*
* Copyright (c) 2019	All rights reserved.
*
* Last change:
********************************************************************/

#include "rtapi.h"		/* RTAPI realtime OS API */
#include "rtapi_app.h"		/* RTAPI realtime module decls */
#include "rtapi_string.h"
#include "hal.h"		/* HAL public API decls */

/* module information */
MODULE_AUTHOR("Scott Alford");
MODULE_DESCRIPTION("Generic PID controller");
MODULE_LICENSE("GPL v2");


static int num_chan;		/* number of channels */
static int default_num_chan = 3;
RTAPI_MP_INT(num_chan, "number of channels");

static int howmany;
#define MAX_CHAN 16
char *names[MAX_CHAN] ={0,};
RTAPI_MP_ARRAY_STRING(names, MAX_CHAN,"pid names");

#define AUTOMATIC	1
#define MANUAL	    0       // default behaviour
#define DIRECT      0       // default behaviour if direction pin not connected
#define REVERSE     1
#define P_ON_M      1
#define P_ON_E      0       // default behaviour if pOnM pin not connected


/***********************************************************************
*                STRUCTURES AND GLOBAL VARIABLES                       *
************************************************************************/


typedef struct {
	hal_bit_t   *Auto;		// pin: auto enable input
	hal_bit_t   *pOnM;		// pin: Proportional on Measurement, default is Proportional on Error
    hal_bit_t   *direction; // pin: controller direction
    hal_float_t *setpoint;	// pin: setpoint (SP), ie commanded value
    hal_float_t *input;		// pin: input (PV), ie feedback value
    hal_float_t *error;		// pin: command - feedback
	hal_float_t *output;	// pin: the output value
	hal_float_t *kp;		// pin: proportional gain
    hal_float_t *ki;		// pin: integral gain
    hal_float_t *kd;		// pin: derivative gain
    hal_float_t *SPmin;	    // pin: setpoint minimum value
	hal_float_t *SPmax;	    // pin: setpoint maximum value
	hal_float_t *CVmin;	    // pin: output minimum value
	hal_float_t *CVmax;	    // pin: output maximum value
    bool         inAuto;
    double       outputSum;
    double       lastInput;
} hal_pid_t;

/* pointer to array of pid_t structs in shared memory, 1 per loop */
static hal_pid_t *pid_array;

/* other globals */
static int comp_id;		/* component ID */

/***********************************************************************
*                  LOCAL FUNCTION DECLARATIONS                         *
************************************************************************/

static int export_pid(hal_pid_t * addr,char * prefix);
static void compute(void *arg, long period);

/***********************************************************************
*                       INIT AND EXIT CODE                             *
************************************************************************/


int rtapi_app_main(void)
{
    int n, retval,i;

    if(num_chan && names[0]) {
        rtapi_print_msg(RTAPI_MSG_ERR,"num_chan= and names= are mutually exclusive\n");
        return -EINVAL;
    }
    if(!num_chan && !names[0]) num_chan = default_num_chan;

    if(num_chan) {
        howmany = num_chan;
    } else {
        howmany = 0;
        for (i = 0; i < MAX_CHAN; i++) {
            if ( (names[i] == NULL) || (*names[i] == 0) ){
                break;
            }
            howmany = i + 1;
        }
    }

    /* test for number of channels */
    if ((howmany <= 0) || (howmany > MAX_CHAN)) {
	rtapi_print_msg(RTAPI_MSG_ERR,
	    "PID: ERROR: invalid number of channels: %d\n", howmany);
	return -1;
    }

    /* have good config info, connect to the HAL */
    comp_id = hal_init("PIDcontroller");
    if (comp_id < 0) {
	rtapi_print_msg(RTAPI_MSG_ERR, "PID: ERROR: hal_init() failed\n");
	return -1;
    }

    /* allocate shared memory for pid loop data */
    pid_array = hal_malloc(howmany * sizeof(hal_pid_t));
    if (pid_array == 0) {
	rtapi_print_msg(RTAPI_MSG_ERR, "PID: ERROR: hal_malloc() failed\n");
	hal_exit(comp_id);
	return -1;
    }

    /* export variables and function for each PID loop */
    i = 0; // for names= items
    for (n = 0; n < howmany; n++) {
	/* export everything for this loop */
        if(num_chan) {
            char buf[HAL_NAME_LEN + 1];
            rtapi_snprintf(buf, sizeof(buf), "pid.%d", n);
	    retval = export_pid(&(pid_array[n]), buf);
        } else {
	    retval = export_pid(&(pid_array[n]), names[i++]);
        }

	if (retval != 0) {
	    rtapi_print_msg(RTAPI_MSG_ERR,
		"PID: ERROR: loop %d var export failed\n", n);
	    hal_exit(comp_id);
	    return -1;
	}
    }
    rtapi_print_msg(RTAPI_MSG_INFO, "PID: installed %d PID loops\n",
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

static void compute(void *arg, long period)
{
    hal_pid_t *pid;
    double minSetpoint, maxSetpoint;
    double minOutput, maxOutput; 
    double setpoint, input, output;
    double error, dInput;
    double kp, ki, kd;
    int controllerDirection, mode, pOn, pOnE;
    double periodfp, periodrecip;

    // point to the data for this PID loop
    pid = arg;

    // precalculate the timing constants
    periodfp = period * 0.000000001;
    periodrecip = 1.0 / periodfp;

	// set the controller direction, mode and "propotional on" (error or measurement)
    controllerDirection = *(pid->direction);
    mode = *(pid->Auto);
    pOn = *(pid->pOnM);
    pOnE = (pOn == P_ON_E);

	// set the controller tunnings
    kp = *(pid->kp);
    ki = *(pid->ki) * periodfp;
    kd = *(pid->kd) / periodfp;

    if(controllerDirection ==REVERSE)
    {
        kp = (0 - kp);
        ki = (0 - ki);
        kd = (0 - kd);
    }

	// set the controller input and output limits
    minSetpoint = *(pid->SPmin);
    maxSetpoint = *(pid->SPmax);
    minOutput = *(pid->CVmin);
    maxOutput = *(pid->CVmax);

    bool newAuto = (mode == AUTOMATIC);
    if(newAuto && !(pid->inAuto))
    {  /*we just went from manual to auto*/
        pid->outputSum = *(pid->output);
        pid->lastInput = *(pid->input);
        if(pid->outputSum > maxOutput) pid->outputSum = maxOutput;
        else if(pid->outputSum < minOutput) pid->outputSum = minOutput;
    }
    pid->inAuto = newAuto;

    if(pid->inAuto)
    {
        /*Compute all the working error variables*/
        setpoint = *(pid->setpoint);
        
        if(setpoint > maxSetpoint) setpoint = maxSetpoint;
        else if(setpoint < minSetpoint) setpoint = minSetpoint;

        input = *(pid->input);
        error = setpoint - input;
	    *pid->error = error;
        dInput = input - pid->lastInput;
        pid->outputSum += (ki * error);

        /*Add Proportional on Measurement, if P_ON_M is specified*/
        if(!pOnE) pid->outputSum -= kp * dInput;

        if(pid->outputSum > maxOutput) pid->outputSum = maxOutput;
        else if(pid->outputSum < minOutput) pid->outputSum= minOutput;

        /*Add Proportional on Error, if P_ON_E is specified*/
        if(pOnE) output = kp * error;
        else output = 0;

        /*Compute Rest of PID Output*/
        output += pid->outputSum - kd * dInput;

        if(output > maxOutput) output = maxOutput;
        else if(output < minOutput) output = minOutput;
	    if (setpoint == 0) output = 0;
	    *(pid->output) = output;

        /*Remember some variables for next time*/
        pid->lastInput = input;
    }
    else
    {
        // TODO: change this for manual mode, just set to zero for now
        *(pid->output) = 0;
    }

    /* done */
}

/***********************************************************************
*                   LOCAL FUNCTION DEFINITIONS                         *
************************************************************************/

static int export_pid(hal_pid_t * addr, char * prefix)
{
    int retval, msg;
    char buf[HAL_NAME_LEN + 1];

    /* This function exports a lot of stuff, which results in a lot of
       logging if msg_level is at INFO or ALL. So we save the current value
       of msg_level and restore it later.  If you actually need to log this
       function's actions, change the second line below */
    msg = rtapi_get_msg_level();
    rtapi_set_msg_level(RTAPI_MSG_WARN);

    /* export pins */
    retval = hal_pin_bit_newf(HAL_IN, &(addr->Auto), comp_id,
			      "%s.auto", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_bit_newf(HAL_IN, &(addr->pOnM), comp_id,
			      "%s.pOnM", prefix);
    if (retval != 0) {
	return retval;
    }

	retval = hal_pin_bit_newf(HAL_IN, &(addr->direction), comp_id,
			      "%s.direction", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_IN, &(addr->setpoint), comp_id,
				"%s.SP", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_IN, &(addr->input), comp_id,
				"%s.PV", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_OUT, &(addr->error), comp_id,
				"%s.ER", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_OUT, &(addr->output), comp_id,
				"%s.CV", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_IN, &(addr->kp), comp_id,
				"%s.KP", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_IN, &(addr->ki), comp_id,
				"%s.KI", prefix);
    if (retval != 0) {
	return retval;
    }

        retval = hal_pin_float_newf(HAL_IN, &(addr->kd), comp_id,
				"%s.KD", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_IN, &(addr->SPmin), comp_id,
				"%s.SPmin", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_IN, &(addr->SPmax), comp_id,
				"%s.SPmax", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_IN, &(addr->CVmin), comp_id,
				"%s.CVmin", prefix);
    if (retval != 0) {
	return retval;
    }

    retval = hal_pin_float_newf(HAL_IN, &(addr->CVmax), comp_id,
				"%s.CVmax", prefix);
    if (retval != 0) {
	return retval;
    }


    /* export function for this loop */
    rtapi_snprintf(buf, sizeof(buf), "%s.compute", prefix);
    retval =
	hal_export_funct(buf, compute, addr, 1, 0, comp_id);
    if (retval != 0) {
	rtapi_print_msg(RTAPI_MSG_ERR,
	    "PID: ERROR: do_pid_calcs funct export failed\n");
	hal_exit(comp_id);
	return -1;
    }
    /* restore saved message level */
    rtapi_set_msg_level(msg);
    return 0;
}
