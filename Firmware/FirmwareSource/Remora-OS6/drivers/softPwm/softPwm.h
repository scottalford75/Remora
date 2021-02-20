#ifndef SOFTPWM_H
#define SOFTPWM_H

#include <string>

#include "modules/module.h"
#include "drivers/pin/pin.h"

class SoftPWM : public Module
{
	private:

		std::string pin;			// PWM output pin
		int pwmMax;						// maximum PWM output: 8 bit resolution (ie 0 to 255)
		int pwmSP;						// PWM setpoint as a percentage of maxPwm
		int SDaccumulator;		// Sigma-Delta accumulator
		bool SDdirection;			// direction the SD accumulator is being updated

		Pin* pwmPin;					// pin object

	public:

		SoftPWM(std::string);					// constructor

		void setMaxPwm(int pwmMax);
		void setPwmSP(int newPwmSP);

		virtual void update(void);           // Module default interface
		virtual void slowUpdate(void);           // Module default interface
};

#endif


/*
	 The following is taken from Smoothieware...

     * Sigma-Delta PWM algorithm
     *
     * This Sigma-Delta implementation works by increasing _sd_accumulator by _pwm until we reach _half_ of max,
     * then decreasing by (max - target_pwm) until we hit zero
     *
     * While we're increasing, the output is 0 and while we're decreasing the output is 1
     *
     * For example, with pwm=128 and a max of 256, we'll see the following pattern:
     * ACC  ADD OUT
     *   0  128   1 // after the add, we hit 256/2 = 128 so we change direction
     * 128 -128   0 // after the add, we hit 0 so we change direction again
     *   0  128   1
     * 128 -128   0
     *  as expected
     *
     * with a pwm value of 192 (75%) we'll see this:
     *  ACC  ADD OUT
     *    0  192   0 // after the add, we are beyond max/2 so we change direction
     *  192  -64   1 // haven't reached 0 yet
     *  128  -64   1 // haven't reached 0 yet
     *   64  -64   1 // after this add we reach 0, and change direction
     *    0  192   0
     *  192  -64   1
     *  128  -64   1
     *   64  -64   1
     *    0  192   0
     * etcetera
     *
     * with a pwm value of 75 (about 29%) we'll see this pattern:
     *  ACC  ADD OUT
     *    0   75   0
     *   75   75   0
     *  150 -181   1
     *  -31   75   0
     *   44   75   0
     *  119   75   0
     *  194 -181   1
     *   13 -181   1
     * -168   75   0
     *  -93   75   0
     *  -18   75   0
     *   57   75   0
     *  132 -181   1
     *  -49   75   0
     *   26   75   0
     *  101   75   0
     *  176 -181   1
     *   -5   75   0
     *   70   75   0
     *  145 -181   1
     *  -36   75   0
     * etcetera. This pattern has 6 '1's over a total of 21 lines which is on 28.57% of the time. If we let it run longer, it would get closer to the target as time went on
     */
