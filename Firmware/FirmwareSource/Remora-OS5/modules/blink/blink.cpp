#include "blink.h"

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

void createBlink()
{
    const char* pin = module["Pin"];
    int frequency = module["Frequency"];
    
    printf("Make Blink at pin %s\n", pin);
        
    Module* blink = new Blink(pin, PRU_SERVOFREQ, frequency);
    servoThread->registerModule(blink);
}


/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

Blink::Blink(std::string portAndPin, uint32_t threadFreq, uint32_t freq)
{

	this->periodCount = threadFreq / freq;
	this->blinkCount = 0;
	this->bState = false;

	this->blinkPin = new Pin(portAndPin, OUTPUT);
	this->blinkPin->set(bState);
}

void Blink::update(void)
{
	++this->blinkCount;
	if (this->blinkCount >= this->periodCount / 2)
	{
		this->blinkPin->set(this->bState=!this->bState);
		this->blinkCount = 0;
	}
}

void Blink::slowUpdate(void)
{
	return;
}
