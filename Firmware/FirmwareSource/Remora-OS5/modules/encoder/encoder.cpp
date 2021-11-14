#include "encoder.h"

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/
void createEncoder()
{
    const char* comment = module["Comment"];
    printf("%s\n",comment);

    int pv = module["PV[i]"];
    const char* pinA = module["ChA Pin"];
    const char* pinB = module["ChB Pin"];
    const char* pinI = module["Index Pin"];
    int dataBit = module["Data Bit"];
    const char* modifier = module["Modifier"];

    printf("Creating Quadrature Encoder at pins %s and %s\n", pinA, pinB);

    int mod;

    if (!strcmp(modifier,"Open Drain"))
    {
        mod = OPENDRAIN;
    }
    else if (!strcmp(modifier,"Pull Up"))
    {
        mod = PULLUP;
    }
    else if (!strcmp(modifier,"Pull Down"))
    {
        mod = PULLDOWN;
    }
    else if (!strcmp(modifier,"Pull None"))
    {
        mod = PULLNONE;
    }
    else
    {
        mod = NONE;
    }
    
    ptrProcessVariable[pv]  = &txData.processVariable[pv];
    ptrInputs = &txData.inputs;

    if (pinI == nullptr)
    {
        Module* encoder = new Encoder(*ptrProcessVariable[pv], pinA, pinB, mod);
        baseThread->registerModule(encoder);
    }
    else
    {
        printf("  Encoder has index at pin %s\n", pinI);
        Module* encoder = new Encoder(*ptrProcessVariable[pv], *ptrInputs, dataBit, pinA, pinB, pinI, mod);
        baseThread->registerModule(encoder);
    }
}

/***********************************************************************
*                METHOD DEFINITIONS                                    *
************************************************************************/

Encoder::Encoder(volatile float &ptrEncoderCount, std::string ChA, std::string ChB, int modifier) :
	ptrEncoderCount(&ptrEncoderCount),
	ChA(ChA),
	ChB(ChB),
    modifier(modifier)
{
	this->pinA = new Pin(this->ChA, INPUT, this->modifier);			// create Pin
    this->pinB = new Pin(this->ChB, INPUT, this->modifier);			// create Pin
    this->hasIndex = false;
	this->count = 0;								                // initialise the count to 0
}

Encoder::Encoder(volatile float &ptrEncoderCount, volatile uint16_t &ptrData, int bitNumber, std::string ChA, std::string ChB, std::string Index, int modifier) :
	ptrEncoderCount(&ptrEncoderCount),
    ptrData(&ptrData),
    bitNumber(bitNumber),
	ChA(ChA),
	ChB(ChB),
    Index(Index),
    modifier(modifier)
{
	this->pinA = new Pin(this->ChA, INPUT, this->modifier);			// create Pin
    this->pinB = new Pin(this->ChB, INPUT, this->modifier);			// create Pin
    this->pinI = new Pin(this->Index, INPUT, this->modifier);		// create Pin
    this->hasIndex = true;
    this->indexPulse = (PRU_BASEFREQ / PRU_SERVOFREQ) * 3;          // output the index pulse for 3 servo thread periods so LinuxCNC sees it
    this->indexCount = 0;
	this->count = 0;								                // initialise the count to 0
    this->pulseCount = 0;                                           // number of base thread periods to pulse the index output    
    this->mask = 1 << this->bitNumber;
}

void Encoder::update()
{
    uint8_t s = this->state & 3;

    if (this->pinA->get()) s |= 4;
    if (this->pinB->get()) s |= 8;

    switch (s) {
		case 0: case 5: case 10: case 15:
			break;
		case 1: case 7: case 8: case 14:
			count++; break;
		case 2: case 4: case 11: case 13:
			count--; break;
		case 3: case 12:
			count += 2; break;
		default:
			count -= 2; break;
	}

	this->state = (s >> 2);

    if (this->hasIndex)                                     // we have an index pin
    {
        // handle index, index pulse and pulse count
        if (this->pinI->get() && (this->pulseCount == 0))    // rising edge on index pulse
        {
            this->indexCount = this->count;                 //  capture the encoder count at the index, send this to linuxCNC for one servo period 
            *(this->ptrEncoderCount) = this->indexCount;
            this->pulseCount = this->indexPulse;        
            *(this->ptrData) |= this->mask;                 // set bit in data source high
        }
        else if (this->pulseCount > 0)                      // maintain both index output and encoder count for the latch period
        {
            this->pulseCount--;                             // decrement the counter
        }
        else
        {
            *(this->ptrData) &= ~this->mask;                // set bit in data source low
            *(this->ptrEncoderCount) = this->count;         // update encoder count
        }
    }
    else
    {
        *(this->ptrEncoderCount) = this->count;             // update encoder count
    }
}


// credit to https://github.com/PaulStoffregen/Encoder/blob/master/Encoder.h

//                           _______         _______       
//               PinA ______|       |_______|       |______ PinA
// negative <---         _______         _______         __      --> positive
//               PinB __|       |_______|       |_______|   PinB

		//	new	new	old	old
		//	pinB	pinA	pinB	pinA	Result
		//	----	----	----	----	------
		//	0	0	0	0	no movement
		//	0	0	0	1	+1
		//	0	0	1	0	-1
		//	0	0	1	1	+2  (assume pinA edges only)
		//	0	1	0	0	-1
		//	0	1	0	1	no movement
		//	0	1	1	0	-2  (assume pinA edges only)
		//	0	1	1	1	+1
		//	1	0	0	0	+1
		//	1	0	0	1	-2  (assume pinA edges only)
		//	1	0	1	0	no movement
		//	1	0	1	1	-1
		//	1	1	0	0	+2  (assume pinA edges only)
		//	1	1	0	1	-1
		//	1	1	1	0	+1
		//	1	1	1	1	no movement
/*
	// Simple, easy-to-read "documentation" version :-)
	//
	void update(void) {
		uint8_t s = state & 3;
		if (digitalRead(pinA)) s |= 4;
		if (digitalRead(pinB)) s |= 8;
		switch (s) {
			case 0: case 5: case 10: case 15:
				break;
			case 1: case 7: case 8: case 14:
				position++; break;
			case 2: case 4: case 11: case 13:
				position--; break;
			case 3: case 12:
				position += 2; break;
			default:
				position -= 2; break;
		}
		state = (s >> 2);
	}
*/
