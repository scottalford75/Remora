#ifndef SWITCH_H
#define SWITCH_H

#include <cstdint>
//#include <iostream>
#include <string>

#include "modules/module.h"
#include "drivers/pin/pin.h"

#include "extern.h"

void createSwitch(void);

class Switch : public Module
{

	private:

		volatile float* ptrPV; 			// pointer to the data source
		float 			PV;
		float 			SP;
		bool			mode;			// 0 switch off, 1 switch on
		std::string 	portAndPin;

		Pin 			*pin;


	public:

		Switch(float, volatile float&, std::string, bool);

		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif
