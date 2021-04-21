#ifndef DIGITALPIN_H
#define DIGITALPIN_H

#include <cstdint>

#include "modules/module.h"
#include "drivers/pin/pin.h"

#define NONE        0b000
#define INVERT      0b001
#define OPENDRAIN   0b010
#define PULLUP      0b011
#define PULLDOWN    0b100
#define PULLNONE    0b101


class DigitalPin : public Module
{
	private:

		volatile uint8_t *ptrData; 	// pointer to the data source
		int bitNumber;				// location in the data source
		bool invert;
		int mask;

		int mode;
        int modifier;
		std::string portAndPin;

		Pin *pin;

	public:

        DigitalPin(volatile uint8_t&, int, std::string, int, int);
		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif
