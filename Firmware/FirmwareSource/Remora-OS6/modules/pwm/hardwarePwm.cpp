#include "hardwarePwm.h"

using namespace std;

#define PWMPERIOD 200

HardwarePWM::HardwarePWM(volatile float &ptrPwmPulseWidth, int pwmPeriod, std::string pin) :
    ptrPwmPulseWidth(&ptrPwmPulseWidth),
    pwmPeriod(pwmPeriod),
	pin(pin)
{
    cout << "Creating Hardware PWM at pin " << this->pin << endl;

    variablePeriod = false;

    if (pwmPeriod == 0)
    {
        this->pwmPeriod = PWMPERIOD;
    }

    Pin* dummyPin = new Pin(pin, 1);
    this->pwmPin = dummyPin->hardware_pwm();

    if (this->pwmPin == NULL) {
        printf("  Error: Hardware PWM cannot this pin (Refer to Hardware Documents for Pin Information)\n");
        delete dummyPin;
        return;
    }

    this->pwmPin->period_us(this->pwmPeriod);
}


HardwarePWM::HardwarePWM(volatile float &ptrPwmPeriod, volatile float &ptrPwmPulseWidth, int pwmPeriod, std::string pin) :
    ptrPwmPeriod(&ptrPwmPeriod),
    ptrPwmPulseWidth(&ptrPwmPulseWidth),
    pwmPeriod(pwmPeriod),
	pin(pin)
{
    cout << "Creating variable frequency Hardware PWM at pin " << this->pin << endl;

    variablePeriod = true;

    if (pwmPeriod == 0)
    {
        this->pwmPeriod = PWMPERIOD;
    }

    Pin* dummyPin = new Pin(pin, 1);
    this->pwmPin = dummyPin->hardware_pwm();

    if (this->pwmPin == NULL) {
        printf("  Error: Hardware PWM cannot this pin (Refer to Hardware Documents for Pin Information)\n");
        delete dummyPin;
        return;
    }

    this->pwmPin->period_us(this->pwmPeriod);
}


void HardwarePWM::update()
{
    if (variablePeriod)
    {
        if (*(this->ptrPwmPeriod) != 0 && (*(this->ptrPwmPeriod) != this->pwmPeriod))
        {
            // PWM period has changed
            this->pwmPeriod = *(this->ptrPwmPeriod);
            this->pwmPin->period_us(this->pwmPeriod);
            this->pwmPulseWidth_us = (this->pwmPeriod * this->pwmPulseWidth) / 100.0;
            this->pwmPin->pulsewidth_us(this->pwmPulseWidth_us);
        }
    }

    if (*(this->ptrPwmPulseWidth) != this->pwmPulseWidth)
    {
        // PWM duty has changed
        this->pwmPulseWidth = *(this->ptrPwmPulseWidth);
        this->pwmPulseWidth_us = (this->pwmPeriod * this->pwmPulseWidth) / 100.0;
        this->pwmPin->pulsewidth_us(this->pwmPulseWidth_us);
    } 

    return;
}


void HardwarePWM::slowUpdate()
{
	return;
}
