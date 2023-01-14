#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <cstdint>
#include <string>
//#include <iostream>

#include "modules/module.h"
#include "sensors/tempSensor.h"
#include "sensors/thermistor/thermistor.h"

#include "extern.h"

void createTemperature(void);

class Temperature : public Module
{
  private:

    std::string sensorType;       // temperature sensor type
    std::string pinSensor;	             // physical pins connections

    volatile float* ptrFeedback;       	   // pointer where to put the feedback

    float temperaturePV;

    // thermistor parameters
    float beta;
    float r0;
		float t0;

  public:

    Temperature(volatile float&, int32_t, int32_t, std::string, std::string, float, int, int);  // Thermistor type constructor

    TempSensor* Sensor;

    virtual void update(void);           // Module default interface
    virtual void slowUpdate(void);
};


#endif
