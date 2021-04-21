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

class Pin
{
    private:

        uint8_t             portNumber;
        uint8_t             pin;
        LPC_GPIO_TypeDef*   port;

    public:

        Pin(std::string, int);

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
