#ifndef RESETPIN_H
#define RESETPIN_H

#include <cstdint>

#include "modules/module.h"
#include "drivers/pin/pin.h"

#include "extern.h"

void createResetPin(void);

class ResetPin : public Module
{
	private:

		volatile bool *ptrReset; 	// pointer to the data source
		std::string portAndPin;

		Pin *pin;

	public:

		ResetPin(volatile bool&, std::string);
		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif
