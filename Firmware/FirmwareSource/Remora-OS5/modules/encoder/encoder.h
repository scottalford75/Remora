#ifndef ENCODER_H
#define ENCODER_H

#include <cstdint>
#include <iostream>
#include <string>

#include "configuration.h"
#include "modules/module.h"
#include "drivers/pin/pin.h"

#include "extern.h"

void createEncoder(void);

class Encoder : public Module
{

	private:

		std::string ChA;			// physical pin connection
        std::string ChB;			// physical pin connection
        
        std::string Index;			// physical pin connection
        bool hasIndex;
        volatile uint16_t *ptrData; 	// pointer to the data source
		int bitNumber;				// location in the data source
        int mask;

		volatile float *ptrEncoderCount; 	// pointer to the data source

        int8_t  modifier;
        uint8_t state;
        int32_t count;
        int32_t indexCount;
        int8_t  indexPulse;
        int8_t  pulseCount;

	public:

		Pin* pinA;      // channel A
        Pin* pinB;      // channel B
        Pin* pinI;      // index       

		Encoder(volatile float&, std::string, std::string, int);
        Encoder(volatile float&, volatile uint16_t&, int, std::string, std::string, std::string, int);

		virtual void update(void);	// Module default interface
};

#endif
