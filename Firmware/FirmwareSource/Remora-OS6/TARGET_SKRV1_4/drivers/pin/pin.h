#ifndef PIN_H
#define PIN_H

#include "mbed.h"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <string>

#include "LPC17xx.h"

#define INPUT 0x0
#define OUTPUT 0x1

#define NONE        0b000
#define OPENDRAIN   0b001
#define PULLUP      0b010
#define PULLDOWN    0b011
#define PULLNONE    0b100

class Pin
{
    private:

        std::string         portAndPin;
        uint8_t             dir;
        uint8_t             modifier;
        uint8_t             portNumber;
        uint8_t             pin;
        LPC_GPIO_TypeDef*   port;

    public:

        Pin(std::string, int);
        Pin(std::string, int, int);

        PwmOut* hardware_pwm();

        void configPin();
        void setAsOutput();
        void setAsInput();
        void as_open_drain();
        void pull_none();
        void pull_up();
        void pull_down();
        PinName pinToPinName();

        inline bool get()
        {
            return ((this->port->FIOPIN >> this->pin ) & 1);
        }

        inline void set(bool value)
        {
            if (value)
                this->port->FIOSET = 1 << this->pin;
            else
                this->port->FIOCLR = 1 << this->pin;
        }
};

#endif
