#ifndef PWM_H
#define PWM_H

#include <cstdint>
//#include <iostream>
#include <string>

#include "modules/module.h"
#include "drivers/softPwm/softPwm.h"

#include "extern.h"

void createPWM(void);

class PWM : public Module
{

	private:

		volatile float* ptrSP; 			// pointer to the data source
		int 			SP;
		std::string 	portAndPin;
		int 			pwmMax;

		SoftPWM* 		pwm;			// pointer to PWM object - output


	public:

		PWM(volatile float&, std::string);
		PWM(volatile float&, std::string, int);

		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif
