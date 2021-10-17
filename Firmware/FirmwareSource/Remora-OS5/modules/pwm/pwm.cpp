#include "pwm.h"


#define PID_PWM_MAX 256		// 8 bit resolution

PWM::PWM(volatile float &ptrSP, std::string portAndPin) :
	ptrSP(&ptrSP),
	portAndPin(portAndPin)
{
	cout << "Creating software PWM @ pin " << this->portAndPin << endl;

	this->pwm = new SoftPWM(this->portAndPin);
	this->pwmMax = PID_PWM_MAX-1;
	this->pwm->setMaxPwm(this->pwmMax);
}

// use the following constructor when using 12v devices on a 24v system
PWM::PWM(volatile float &ptrSP, std::string portAndPin, int pwmMax) :
	ptrSP(&ptrSP),
	portAndPin(portAndPin),
	pwmMax(pwmMax)
{
	cout << "Creating software PWM with PWM Max @ pin " << this->portAndPin << endl;

	this->pwm = new SoftPWM(this->portAndPin);
	this->pwm->setMaxPwm(this->pwmMax);
}



void PWM::update()
{
	float SP;

	// update the speed SP
	this->SP = *(this->ptrSP);

    // ensure SP is within range. LinuxCNC PID can have -ve command value
	if (this->SP > 100) this->SP = 100;
    if (this->SP < 0) this->SP = 0;

	// the SP is as a percentage (%)
	// scale the pwm output range (0 - pwmMax) = (0 - 100%)

	SP = this->pwmMax * (this->SP / 100.0);

	this->pwm->setPwmSP(int(SP));

	this->pwm->update();
}

void PWM::slowUpdate()
{
	return;
}
