#ifndef TEMPSENSOR_H
#define TEMPSENSOR_H

// Base class for all temperature sensor classes

class TempSensor
{
	public:
		virtual ~TempSensor() {}

		// Return temperature in degrees Celsius.
		virtual float getTemperature() { return -1.0F; }

};

#endif