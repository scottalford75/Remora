#include "eStop.h"


eStop::eStop(volatile int32_t &ptrTxHeader, std::string portAndPin) :
    ptrTxHeader(&ptrTxHeader),
	portAndPin(portAndPin)
{
	this->pin = new Pin(this->portAndPin, 0);		// Input 0x0, Output 0x1
}



void eStop::update()
{
    if (this->pin->get() == 1)
    {
        *ptrTxHeader = PRU_ESTOP;
    }
    else {
        *ptrTxHeader = PRU_DATA;
    }
}

void eStop::slowUpdate()
{
	return;
}
