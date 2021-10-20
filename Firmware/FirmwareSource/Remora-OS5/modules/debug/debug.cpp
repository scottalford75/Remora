#include "debug.h"


Debug::Debug(std::string portAndPin, bool bstate) :
    bState(bstate)
{
	this->debugPin = new Pin(portAndPin, OUTPUT);
}

void Debug::update(void)
{
	this->debugPin->set(bState);
}

void Debug::slowUpdate(void)
{
	return;
}
