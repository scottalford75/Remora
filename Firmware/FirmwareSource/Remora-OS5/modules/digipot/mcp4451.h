#ifndef MCP4451_H
#define MCP4451_H

#include "mbed.h"
#include <string>

#include "module.h"
#include "pin.h"

#include "extern.h"


void createMCP4451(void);

class MCP4451 : public Module
{
  private:

    std::string scl, sda; // i2c SCL and SDA portAndPin
    Pin *sclPin, *sdaPin;
    I2C *i2c;

    char address;       // on the i2c bus, set by address bits A1:A0
    float c0, c1, c2, c3;
    float currents[4];  // the mcp4451 only has 4 wipers

    float factor;
    float maxCurrent;

    void i2c_send(char, char, char);
    char current_to_wiper(float);

  public:

    MCP4451(std::string, std::string, char, float, float, float, float, float, float);
    //mcp4451(std::string, std::string, char);
    ~MCP4451();

    void set_max_current(float c) { maxCurrent= c; }
    void set_factor(float f) { factor= f; }
    void set_current(int, float);

    virtual void update(void);           // Module default interface
};


#endif
