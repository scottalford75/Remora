#ifndef DIGITALPIN_H
#define DIGITALPIN_H

#include <cstdint>

#include "modules/module.h"
#include "drivers/pin/pin.h"

#include "extern.h"

void createDigitalPin(void);

class DigitalPin : public Module
{
	private:

		volatile uint16_t *ptrData; 	// pointer to the data source
		int bitNumber;				// location in the data source
		bool invert;
		int mask;

		int mode;
        int modifier;
		std::string portAndPin;

		Pin *pin;

	public:

        DigitalPin(volatile uint16_t&, int, std::string, int, bool, int);
		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif
