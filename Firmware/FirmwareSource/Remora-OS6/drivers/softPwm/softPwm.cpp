#include "softPwm.h"

#include <algorithm>

#define confine(value, min, max) (((value) < (min))?(min):(((value) > (max))?(max):(value)))
#define PID_PWM_MAX 256		// 8 bit resolution

using namespace std;

SoftPWM::SoftPWM(std::string pin) :
	pin(pin),
	pwmMax(PID_PWM_MAX-1),
	pwmSP(0),
	SDaccumulator(0),
	SDdirection(false)
{
	this->pwmPin = new Pin(this->pin, OUTPUT);
}


void SoftPWM::setMaxPwm(int pwmMax)
{
	this->pwmMax = confine(pwmMax, 0, PID_PWM_MAX-1);
}


void SoftPWM::setPwmSP(int newPwmSP)
{
	this->pwmSP = newPwmSP; //confine(newPwmSP, 0, pwmMax);
}


void SoftPWM::update()
{
	// Use the standard Moudle interface

	if ((this->pwmSP < 0) || this->pwmSP >= PID_PWM_MAX)
	{
        return;
    }
    else if (this->pwmSP == 0)
	{
		this->pwmPin->set(false);
        return;
    }
    else if (this->pwmSP == PID_PWM_MAX-1)
	{
		this->pwmPin->set(true);
        return;
    }


    // this line should never actually do anything, it's just a sanity check in case our accumulator gets corrupted somehow.
    // If we didn't check and the accumulator is corrupted, we could leave a heater on for quite a long time
    // the accumulator is kept within these limits by the normal operation of the Sigma-Delta algorithm

    SDaccumulator = confine(SDaccumulator, -PID_PWM_MAX, PID_PWM_MAX << 1);

    // when SDdirection == false, our output is 0 and our accumulator is increasing by pwmSP
    if (this->SDdirection == false)
    {
        // increment accumulator
        this->SDaccumulator += this->pwmSP;
        // if we've reached half of max, flip our direction
        if (this->SDaccumulator >= (PID_PWM_MAX >> 1))
            this->SDdirection = true;
    }
    // when SDdirection == true, our output is 1 and our accumulator is decreasing by (maxPwm - pwmSP)
    else
    {
        // decrement accumulator
        this->SDaccumulator -= (PID_PWM_MAX - this->pwmSP);
        // if we've reached 0, flip our direction
        if (this->SDaccumulator <= 0)
            this->SDdirection = false;
    }

	this->pwmPin->set(this->SDdirection);

    return;
}

void SoftPWM::slowUpdate()
{
	return;
}
