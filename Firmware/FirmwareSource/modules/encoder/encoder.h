#ifndef ENCODER_H
#define ENCODER_H

#include <cstdint>
#include <iostream>
#include <string>

#include "modules/module.h"
#include "drivers/pin/pin.h"

class Encoder : public Module
{

	private:

		std::string ChA;			// physical pin connection
        std::string ChB;			// physical pin connection

		volatile float *ptrEncoderCount; 	// pointer to the data source

        uint8_t state;
        int32_t count;

	public:

		Pin* pin1;
        Pin* pin2;

		Encoder(volatile float&, std::string, std::string);

		virtual void update(void);	// Module default interface
};

#endif
