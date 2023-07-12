#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <stdint.h>
#include <string>

#include "sensors/tempSensor.h"
#include "drivers/pin/pin.h"

// Derived class from Tempsensor

class Thermistor : public TempSensor
{
	private:

		std::string pin;

        AnalogIn *adc;

		float temperatureMax, temperatureMin;
		bool useSteinhartHart;

		// Thermistor computation settings using beta, not used if using Steinhart-Hart
		float r0;
		float t0;

		// on board resistor settings
		int r1;
		int r2;

		union
		{
			// this saves memory as we only use either beta or SHH
			struct
			{
				float beta;
				float j;
				float k;
			};
			struct
			{
				float c1;
				float c2;
				float c3;
			};
		};

	public:

		Pin *thermistorPin;

		Thermistor(std::string, float, int, int);

		int newThermistorReading();
		float adcValueToTemperature();
		float getTemperature();

};

#endif
