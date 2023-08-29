#include "switch.h"

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

void createSwitch()
{
    const char* comment = module["Comment"];
    printf("%s\n",comment);

    const char* pin = module["Pin"];
    const char* mode = module["Mode"];
    int pv = module["PV[i]"];
    float sp = module["SP"];

    printf("Make Switch (%s) at pin %s\n", mode, pin);

    if (!strcmp(mode,"On"))
    {
        Module* SoftSwitch = new Switch(sp, *ptrProcessVariable[pv], pin, 1);
        servoThread->registerModule(SoftSwitch);
    }
    else if (!strcmp(mode,"Off"))
    {
        Module* SoftSwitch = new Switch(sp, *ptrProcessVariable[pv], pin, 0);
        servoThread->registerModule(SoftSwitch);
    }
    else
    {
        printf("Error - incorrectly defined Switch\n");
    }
}

Switch::Switch(float SP, volatile float &ptrPV, std::string portAndPin, bool mode) :
	SP(SP),
	ptrPV(&ptrPV),
	portAndPin(portAndPin),
	mode(mode)
{
    printf("Creating a Switch\n");
	//cout << "Creating a Switch @ pin " << this->portAndPin << endl;
	int output = 0x1; // an output
	this->pin = new Pin(this->portAndPin, output);
}


/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

void Switch::update()
{
	bool pinState;

	pinState = this->mode;

	// update the SP
	this->PV = *(this->ptrPV);

	if (this->PV > this->SP)
	{
		this->pin->set(pinState);
	}
	else
	{
		pinState = !pinState;
		this->pin->set(pinState);
	}

}


void Switch::slowUpdate()
{
	return;
}
