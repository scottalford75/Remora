#include "mbed.h"

#include "pin.h"
#include <cstdio>
#include <cerrno>
#include <string>

#include "LPC17xx.h"

Pin::Pin(std::string portAndPin, int dir) :
    portAndPin(portAndPin),
    dir(dir)
{
    this->configPin();
}

Pin::Pin(std::string portAndPin, int dir, int modifier) :
    portAndPin(portAndPin),
    dir(dir),
    modifier(modifier)
{
    this->configPin();

    if (this->dir == 0) //input
    {
        switch(this->modifier)
        {
            case OPENDRAIN:
                printf("  Setting pin as open drain\n");
                this->as_open_drain();
                break;
            case PULLUP:
                printf("  Setting pin as pull_up\n");
                this->pull_up();
                break;
            case PULLDOWN:
                printf("  Setting pin as pull_down\n");
                this->pull_down();
                break;
            case PULLNONE:
                printf("  Setting pin as pull_none\n");
                this->pull_none();
                break;
        }
    }
}

void Pin::configPin()
{
    printf("Creating Pin @\n");

    LPC_GPIO_TypeDef* gpios[5] ={LPC_GPIO0,LPC_GPIO1,LPC_GPIO2,LPC_GPIO3,LPC_GPIO4};

    // The method below to determine the port and pin from the string is taken from Smoothieware, thanks!

    // cs is the current position in the string
    const char* cs = this->portAndPin.c_str();

    // cn is the position of the next char after the number we just read
    char* cn = NULL;

    // grab first integer as port. pointer to first non-digit goes in cn
    this->portNumber = std::strtol(cs, &cn, 10);

    printf("  portNumber = %d\n", this->portNumber);

    // if cn > cs then strtol read at least one digit
    if ((cn > cs) && (this->portNumber <= 4))
    {
        // translate port index into something useful
        this->port = gpios[this->portNumber];

        // if the char after the first integer is a . then we should expect a pin index next
        if (*cn == '.')
        {
            // move pointer to first digit (hopefully) of pin index
            cs = ++cn;

            // grab pin index.
            this->pin = strtol(cs, &cn, 10);

            printf("  pin = %d\n", this->pin);

            // if strtol read some numbers, cn will point to the first non-digit
            if ((cn > cs) && (this->pin < 32) && (this->dir >= 0))
            {
                // configure pin direction: FIODIR
                if (dir == INPUT)
                {
                    this->port->FIODIR &= ~(1<<this->pin);
                }
                else
                {
                    this->port->FIODIR |= 1<<this->pin;
                }

                // configure
                this->port->FIOMASK &= ~(1 << this->pin);
            }
        }
    }
}

void Pin::setAsOutput()
{
    this->port->FIODIR |= 1<<this->pin;
}


void Pin::setAsInput()
{
    this->port->FIODIR &= ~(1<<this->pin);
}

// Configure this pin as OD
void Pin::as_open_drain(){
    if( this->portNumber == 0 ){ LPC_PINCON->PINMODE_OD0 |= (1<<this->pin); }
    if( this->portNumber == 1 ){ LPC_PINCON->PINMODE_OD1 |= (1<<this->pin); }
    if( this->portNumber == 2 ){ LPC_PINCON->PINMODE_OD2 |= (1<<this->pin); }
    if( this->portNumber == 3 ){ LPC_PINCON->PINMODE_OD3 |= (1<<this->pin); }
    if( this->portNumber == 4 ){ LPC_PINCON->PINMODE_OD4 |= (1<<this->pin); }
    pull_none(); // no pull up by default
}

// Configure this pin as no pullup or pulldown
void Pin::pull_none()
{
    // Set the two bits for this pin as 10
    if( this->portNumber == 0 && this->pin < 16  ){ LPC_PINCON->PINMODE0 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE0 &= ~(1<<( this->pin    *2)); }
    if( this->portNumber == 0 && this->pin >= 16 ){ LPC_PINCON->PINMODE1 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE1 &= ~(1<<((this->pin-16)*2)); }
    if( this->portNumber == 1 && this->pin < 16  ){ LPC_PINCON->PINMODE2 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE2 &= ~(1<<( this->pin    *2)); }
    if( this->portNumber == 1 && this->pin >= 16 ){ LPC_PINCON->PINMODE3 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE3 &= ~(1<<((this->pin-16)*2)); }
    if( this->portNumber == 2 && this->pin < 16  ){ LPC_PINCON->PINMODE4 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE4 &= ~(1<<( this->pin    *2)); }
    if( this->portNumber == 3 && this->pin >= 16 ){ LPC_PINCON->PINMODE7 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE7 &= ~(1<<((this->pin-16)*2)); }
    if( this->portNumber == 4 && this->pin >= 16 ){ LPC_PINCON->PINMODE9 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE9 &= ~(1<<((this->pin-16)*2)); }
}

// Configure this pin as a pullup
void Pin::pull_up()
{
    // Set the two bits for this pin as 00
    if( this->portNumber == 0 && this->pin < 16  ){ LPC_PINCON->PINMODE0 &= ~(3<<( this->pin    *2)); }
    if( this->portNumber == 0 && this->pin >= 16 ){ LPC_PINCON->PINMODE1 &= ~(3<<((this->pin-16)*2)); }
    if( this->portNumber == 1 && this->pin < 16  ){ LPC_PINCON->PINMODE2 &= ~(3<<( this->pin    *2)); }
    if( this->portNumber == 1 && this->pin >= 16 ){ LPC_PINCON->PINMODE3 &= ~(3<<((this->pin-16)*2)); }
    if( this->portNumber == 2 && this->pin < 16  ){ LPC_PINCON->PINMODE4 &= ~(3<<( this->pin    *2)); }
    if( this->portNumber == 3 && this->pin >= 16 ){ LPC_PINCON->PINMODE7 &= ~(3<<((this->pin-16)*2)); }
    if( this->portNumber == 4 && this->pin >= 16 ){ LPC_PINCON->PINMODE9 &= ~(3<<((this->pin-16)*2)); }
}

// Configure this pin as a pulldown
void Pin::pull_down()
{
    // Set the two bits for this pin as 11
    if( this->portNumber == 0 && this->pin < 16  ){ LPC_PINCON->PINMODE0 |= (3<<( this->pin    *2)); }
    if( this->portNumber == 0 && this->pin >= 16 ){ LPC_PINCON->PINMODE1 |= (3<<((this->pin-16)*2)); }
    if( this->portNumber == 1 && this->pin < 16  ){ LPC_PINCON->PINMODE2 |= (3<<( this->pin    *2)); }
    if( this->portNumber == 1 && this->pin >= 16 ){ LPC_PINCON->PINMODE3 |= (3<<((this->pin-16)*2)); }
    if( this->portNumber == 2 && this->pin < 16  ){ LPC_PINCON->PINMODE4 |= (3<<( this->pin    *2)); }
    if( this->portNumber == 3 && this->pin >= 16 ){ LPC_PINCON->PINMODE7 |= (3<<((this->pin-16)*2)); }
    if( this->portNumber == 4 && this->pin >= 16 ){ LPC_PINCON->PINMODE9 |= (3<<((this->pin-16)*2)); }
}

// Convert a PortAndPin into a mBed Pin
// allows use of standard mbed libraries, eg FastAnalogIn
PinName Pin::pinToPinName()
{
  if( this->port == LPC_GPIO0 && this->pin == 0 ) {
      return p9;
  } else if( this->port == LPC_GPIO0 && this->pin == 1 ) {
      return p10;
  } else if( this->port == LPC_GPIO0 && this->pin == 23 ) {
      return p15;
  } else if( this->port == LPC_GPIO0 && this->pin == 24 ) {
      return p16;
  } else if( this->port == LPC_GPIO0 && this->pin == 25 ) {
      return p17;
  } else if( this->port == LPC_GPIO0 && this->pin == 26 ) {
      return p18;
  } else if( this->port == LPC_GPIO1 && this->pin == 30 ) {
      return p19;
  } else if( this->port == LPC_GPIO1 && this->pin == 31 ) {
      return p20;
  } else {
      //TODO: Error
      return NC;
  }
}

// If available on this pin, return mbed hardware pwm class for this pin
PwmOut* Pin::hardware_pwm()
{
    if (this->portNumber == 1)
    {
        if (this->pin == 18) { return new mbed::PwmOut(P1_18); }
        if (this->pin == 20) { return new mbed::PwmOut(P1_20); }
        if (this->pin == 21) { return new mbed::PwmOut(P1_21); }
        if (this->pin == 23) { return new mbed::PwmOut(P1_23); }
        if (this->pin == 24) { return new mbed::PwmOut(P1_24); }
        if (this->pin == 26) { return new mbed::PwmOut(P1_26); }
    }
    else if (this->portNumber == 2)
    {
        if (this->pin == 0) { return new mbed::PwmOut(P2_0); }
        if (this->pin == 1) { return new mbed::PwmOut(P2_1); }
        if (this->pin == 2) { return new mbed::PwmOut(P2_2); }
        if (this->pin == 3) { return new mbed::PwmOut(P2_3); }
        if (this->pin == 4) { return new mbed::PwmOut(P2_4); }
        if (this->pin == 5) { return new mbed::PwmOut(P2_5); }
    }
    else if (this->portNumber == 3)
    {
        if (this->pin == 25) { return new mbed::PwmOut(P3_25); }
        if (this->pin == 26) { return new mbed::PwmOut(P3_26); }
    }
    return nullptr;
}
