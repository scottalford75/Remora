#ifndef HARDWAREPWM_H
#define HARDWAREPWM_H

#include <string>
#include <iostream>

#include "modules/module.h"
#include "drivers/pin/pin.h"

class HardwarePWM : public Module
{
	private:

		std::string pin;			        // PWM output pin
		int pwmMax;					        // maximum PWM output
		int pwmSP;					        // PWM setpoint as a percentage of maxPwm

		Pin* dummyPin;				        // pin object
        PwmOut *pwmPin;                     // PWM out object

        volatile float *ptrPwmPeriod; 	    // pointer to the data source
        volatile float *ptrPwmPulseWidth; 	// pointer to the data source

        int pwmPeriod;                      // Period (us)
        float pwmPulseWidth;                // Pulse width (%)
        int pwmPulseWidth_us;               // Pulse width (us)

        bool variablePeriod;

	public:

		HardwarePWM(volatile float&, int, std::string);			        
        HardwarePWM(volatile float&, volatile float&, int, std::string);	

		virtual void update(void);          // Module default interface
		virtual void slowUpdate(void);      // Module default interface
};

#endif

