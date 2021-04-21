#include "digitalPin.h"


DigitalPin::DigitalPin(volatile uint8_t &ptrData, int mode, std::string portAndPin, int bitNumber, int modifier) :
	ptrData(&ptrData),
	mode(mode),
	portAndPin(portAndPin),
	bitNumber(bitNumber),
	modifier(modifier)
{
	this->pin = new Pin(this->portAndPin, this->mode);		// Input 0x0, Output 0x1
	this->mask = 1 << this->bitNumber;
    this->invert = false;
 
    if (this->mode == 0) //input
    {
        switch(this->modifier)
        {
            case INVERT:
                printf("  Setting pin as inverting\n");
                this->invert = true;
                break;
            case OPENDRAIN:
                printf("  Setting pin as open drain\n");
                this->pin->as_open_drain();
                break;
            case PULLUP:
                printf("  Setting pin as pull_up\n");
                this->pin->pull_up();
                break;
            case PULLDOWN:
                printf("  Setting pin as pull_down\n");
                this->pin->pull_down();
                break;
            case PULLNONE:
                printf("  Setting pin as pull_none\n");
                this->pin->pull_none();
                break;
        }
    }
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
