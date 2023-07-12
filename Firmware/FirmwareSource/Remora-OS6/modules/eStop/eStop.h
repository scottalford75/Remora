#ifndef ESTOP_H
#define ESTOP_H

#include <cstdint>
#include <iostream>
#include <string>

#include "../../configuration.h"
#include "../../remora.h"
#include "modules/module.h"
#include "drivers/pin/pin.h"

#include "extern.h"

void createEStop(void);

class eStop : public Module
{

	private:

        volatile int32_t *ptrTxHeader;
		std::string 	portAndPin;

        Pin *pin;


	public:

		eStop(volatile int32_t&, std::string);

		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif
