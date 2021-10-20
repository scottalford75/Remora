#ifndef DEBUG_H
#define DEBUG_H

#include <cstdint>
#include <string>

#include "modules/module.h"
#include "drivers/pin/pin.h"

class Debug : public Module
{

	private:

		bool 		bState;

		Pin*        debugPin;	// class object members - Pin objects

	public:

		Debug(std::string, bool);

		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif
