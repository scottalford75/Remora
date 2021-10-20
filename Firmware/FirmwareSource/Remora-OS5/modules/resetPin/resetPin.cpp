#include "resetPin.h"

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

void createResetPin()
{
    const char* comment = module["Comment"];
    printf("%s\n",comment);

    const char* pin = module["Pin"];

    ptrPRUreset = &PRUreset;

    printf("Make Reset Pin at pin %s\n", pin);

    Module* resetPin = new ResetPin(*ptrPRUreset, pin);
    servoThread->registerModule(resetPin);
}


/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

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
