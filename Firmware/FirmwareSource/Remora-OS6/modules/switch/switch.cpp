#include "switch.h"


Switch::Switch(float SP, volatile float &ptrPV, std::string portAndPin, bool mode) :
	SP(SP),
	ptrPV(&ptrPV),
	portAndPin(portAndPin),
	mode(mode)
{
	cout << "Creating a Switch @ pin " << this->portAndPin << endl;
	int output = 0x1; // an output
	this->pin = new Pin(this->portAndPin, output);
}


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
