#ifndef DIGITALPIN_H
#define DIGITALPIN_H

#include <cstdint>

#include "modules/module.h"
#include "drivers/pin/pin.h"


class DigitalPin : public Module
{
	private:

		volatile uint8_t *ptrData; 	// pointer to the data source
		int bitNumber;				// location in the data source
		bool invert;
		int mask;

		int mode;
		std::string portAndPin;

		Pin *pin;

	public:

		DigitalPin(volatile uint8_t&, int, std::string, int, bool);
		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif
