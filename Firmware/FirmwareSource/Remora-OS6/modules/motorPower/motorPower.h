#ifndef MOTORPOWER_H
#define MOTORPOWER_H

#include <cstdint>

#include "modules/module.h"
#include "drivers/pin/pin.h"

#include "extern.h"

void createMotorPower(void);

class MotorPower : public Module
{
	private:

		std::string portAndPin;

		Pin *pin;

	public:

        MotorPower(std::string);
		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif
