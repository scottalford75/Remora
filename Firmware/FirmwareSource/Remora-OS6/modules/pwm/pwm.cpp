#include "pwm.h"
#include "hardwarePwm.h"

#define PID_PWM_MAX 256		// 8 bit resolution

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

void createPWM()
{
    const char* comment = module["Comment"];
    printf("%s\n",comment);

    int sp = module["SP[i]"];
    int pwmMax = module["PWM Max"];
    const char* pin = module["PWM Pin"];

    const char* hardware = module["Hardware PWM"];
    const char* variable = module["Variable Freq"];
    int period_sp = module["Period SP[i]"];
    int period = module["Period us"];

    printf("Make PWM at pin %s\n", pin);
    
    ptrSetPoint[sp] = &rxData.setPoint[sp];

    if (!strcmp(hardware,"True"))
    {
        // Hardware PWM
        if (!strcmp(variable,"True"))
        {
            // Variable frequency hardware PWM
            ptrSetPoint[period_sp] = &rxData.setPoint[period_sp];

            Module* pwm = new HardwarePWM(*ptrSetPoint[period_sp], *ptrSetPoint[sp], period, pin);
            servoThread->registerModule(pwm);
        }
        else
        {
            // Fixed frequency hardware PWM
            Module* pwm = new HardwarePWM(*ptrSetPoint[sp], period, pin);
            servoThread->registerModule(pwm);
        }
    }
    else
    {
        // Software PWM
        if (pwmMax != 0) // use configuration file value for pwmMax - useful for 12V on 24V systems
        {
            Module* pwm = new PWM(*ptrSetPoint[sp], pin, pwmMax);
            servoThread->registerModule(pwm);
        }
        else // use default value of pwmMax
        {
            Module* pwm = new PWM(*ptrSetPoint[sp], pin);
            servoThread->registerModule(pwm);
        }
    }
}


/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

PWM::PWM(volatile float &ptrSP, std::string portAndPin) :
	ptrSP(&ptrSP),
	portAndPin(portAndPin)
{
	printf("Creating software PWM @ pin %s\n", this->portAndPin.c_str());
    //cout << "Creating software PWM @ pin " << this->portAndPin << endl;

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
	printf("Creating software PWM with PWM Max @ pin %s\n", this->portAndPin.c_str());
    //cout << "Creating software PWM with PWM Max @ pin " << this->portAndPin << endl;

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
