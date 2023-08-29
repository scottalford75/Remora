#include "digitalPin.h"

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

void createDigitalPin()
{
    const char* comment = module["Comment"];
    printf("%s\n",comment);

    const char* pin = module["Pin"];
    const char* mode = module["Mode"];
    const char* invert = module["Invert"];
    const char* modifier = module["Modifier"];
    int dataBit = module["Data Bit"];

    int mod;
    bool inv;

    if (!strcmp(modifier,"Open Drain"))
    {
        mod = OPENDRAIN;
    }
    else if (!strcmp(modifier,"Pull Up"))
    {
        mod = PULLUP;
    }
    else if (!strcmp(modifier,"Pull Down"))
    {
        mod = PULLDOWN;
    }
    else if (!strcmp(modifier,"Pull None"))
    {
        mod = PULLNONE;
    }
    else
    {
        mod = NONE;
    }

    if (!strcmp(invert,"True"))
    {
        inv = true;
    }
    else inv = false;

    ptrOutputs = &rxData.outputs;
    ptrInputs = &txData.inputs;

    printf("Make Digital %s at pin %s\n", mode, pin);

    if (!strcmp(mode,"Output"))
    {
        //Module* digitalPin = new DigitalPin(*ptrOutputs, 1, pin, dataBit, invert);
        Module* digitalPin = new DigitalPin(*ptrOutputs, 1, pin, dataBit, inv, mod);
        servoThread->registerModule(digitalPin);
    }
    else if (!strcmp(mode,"Input"))
    {
        //Module* digitalPin = new DigitalPin(*ptrInputs, 0, pin, dataBit, invert);
        Module* digitalPin = new DigitalPin(*ptrInputs, 0, pin, dataBit, inv, mod);
        servoThread->registerModule(digitalPin);
    }
    else
    {
        printf("Error - incorrectly defined Digital Pin\n");
    }
}


/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

DigitalPin::DigitalPin(volatile uint16_t &ptrData, int mode, std::string portAndPin, int bitNumber, bool invert, int modifier) :
	ptrData(&ptrData),
	mode(mode),
	portAndPin(portAndPin),
	bitNumber(bitNumber),
    invert(invert),
	modifier(modifier)
{
	this->pin = new Pin(this->portAndPin, this->mode, this->modifier);		// Input 0x0, Output 0x1
	this->mask = 1 << this->bitNumber;
}


void DigitalPin::update()
{
	bool pinState;

	if (this->mode == 0)									// the pin is configured as an input
	{
		pinState = this->pin->get();
		if(this->invert)
		{
			pinState = !pinState;
		}

		if (pinState == 1)								// input is high
		{
			*(this->ptrData) |= this->mask;
		}
		else											// input is low
		{
			*(this->ptrData) &= ~this->mask;
		}
	}
	else												// the pin is configured as an output
	{
		pinState = *(this->ptrData) & this->mask;		// get the value of the bit in the data source
		if(this->invert)
		{
			pinState = !pinState;
		}
		this->pin->set(pinState);			// simple conversion to boolean
	}
}

void DigitalPin::slowUpdate()
{
	return;
}
