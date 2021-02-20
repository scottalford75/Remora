#include "temperature.h"

/***********************************************************************
*                METHOD DEFINITIONS                                    *
************************************************************************/

Temperature::Temperature(volatile float &ptrFeedback, int32_t threadFreq, int32_t slowUpdateFreq, std::string sensorType, std::string pinSensor, float beta, int r0, int t0) :
  Module(threadFreq, slowUpdateFreq),
  ptrFeedback(&ptrFeedback),
  sensorType(sensorType),
  pinSensor(pinSensor),
	beta(beta),
	r0(r0),
	t0(t0)
{
  if (this->sensorType == "Thermistor")
  {
    cout << "Creating Thermistor Tempearture measurement @ pin " << this->pinSensor << endl;
    this->Sensor = new Thermistor(this->pinSensor, this->beta, this->r0, this->t0);
  }
  // TODO: Add more sensor types as needed

  // Take some readings to get the ADC up and running before moving on
  this->slowUpdate();
  this->slowUpdate();
  cout << "Start temperature = " << this->temperaturePV << endl;
}

void Temperature::update()
{
  return;
}

void Temperature::slowUpdate()
{
	this->temperaturePV = this->Sensor->getTemperature();

  // check for disconnected temperature sensor
  if (this->temperaturePV > 10)
  {
     *(this->ptrFeedback) = this->temperaturePV;
  }
  else
  {
    cout << "Temperature sensor error, pin " << this->pinSensor << " reading = " << this->temperaturePV << endl;
    *(this->ptrFeedback) = 999;
  }

}
