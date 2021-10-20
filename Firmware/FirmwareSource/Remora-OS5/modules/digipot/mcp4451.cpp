#include "mcp4451.h"

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/
void createMCP4451()
{
    printf("Make MCP4451 Digipot object\n");

    const char* sda = module["I2C SDA pin"];
    const char* scl = module["I2C SCL pin"];
    int address = module["I2C address"];
    float maxCurrent = module["Max current"];
    float factor = module["Factor"];
    float c0 = module["Current 0"];
    float c1 = module["Current 1"];
    float c2 = module["Current 2"];
    float c3 = module["Current 3"];

    Module* digipot = new MCP4451(sda, scl, address, maxCurrent, factor, c0, c1, c2, c3);
    digipot->update();
    delete digipot;
}


/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/


MCP4451::MCP4451(std::string sda, std::string scl, char address, float maxCurrent, float factor, float c0, float c1, float c2, float c3) :
  sda(sda),
  scl(scl),
  address(address),
  maxCurrent(maxCurrent),
  factor(factor),
  c0(c0),
  c1(c1),
  c2(c2),
  c3(c3)
{
  this->sclPin = new Pin(this->scl, -1); // dir = -1 so not configured as IO
  this->sdaPin = new Pin(this->sda, -1);

  // I2C com
  //this->i2c = new I2C(p9, p10);
  this->i2c = new I2C(this->sdaPin->pinToPinName(), this->sclPin->pinToPinName());
  this->i2c->frequency(20000);
  for (int i = 0; i < 4; i++) this->currents[i] = -1;
}

MCP4451::~MCP4451()
{
    delete this->i2c;
    delete this->sclPin;
    delete this->sdaPin;
}

void MCP4451::set_current( int channel, float current)
{
    if(current < 0) {
        currents[channel]= -1;
        return;
    }
    current = min( (float) max( current, 0.0f ), this->maxCurrent );
    currents[channel] = current;
    char addr = 0x58 + this->address;

    // Initial setup
    this->i2c_send( addr, 0x40, 0xff );
    this->i2c_send( addr, 0xA0, 0xff );

    // Set actual wiper value
    char addresses[4] = { 0x00, 0x10, 0x60, 0x70 };
    this->i2c_send( addr, addresses[channel], this->current_to_wiper(current) );
}

void MCP4451::i2c_send( char first, char second, char third )
{
    this->i2c->start();
    this->i2c->write(first);
    this->i2c->write(second);
    this->i2c->write(third);
    this->i2c->stop();
}

char MCP4451::current_to_wiper( float current )
{
    int c= ceilf(this->factor*current);
    if(c > 255) c= 255;
    return c;
}

void MCP4451::update()
{
  this->set_max_current(this->maxCurrent);
  this->set_factor(this->factor);

  this->set_current(0, this->c0);
  this->set_current(1, this->c1);
  this->set_current(2, this->c2);
  this->set_current(3, this->c3);
}
