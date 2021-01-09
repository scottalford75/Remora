#include "resetPin.h"


ResetPin::ResetPin(volatile bool &ptrReset, std::string portAndPin) :
	ptrReset(&ptrReset),
	portAndPin(portAndPin)
{
	this->pin = new Pin(this->portAndPin, 0);		// Input 0x0, Output 0x1
}


void ResetPin::update()
{
	*(this->ptrReset) = this->pin->get();
}

void ResetPin::slowUpdate()
{
	return;
}
