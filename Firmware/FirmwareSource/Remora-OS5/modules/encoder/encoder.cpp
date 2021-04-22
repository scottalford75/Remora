#include "encoder.h"


/***********************************************************************
*                METHOD DEFINITIONS                                    *
************************************************************************/

// credit to https://github.com/PaulStoffregen/Encoder/blob/master/Encoder.h

//                           _______         _______       
//               Pin1 ______|       |_______|       |______ Pin1
// negative <---         _______         _______         __      --> positive
//               Pin2 __|       |_______|       |_______|   Pin2

		//	new	new	old	old
		//	pin2	pin1	pin2	pin1	Result
		//	----	----	----	----	------
		//	0	0	0	0	no movement
		//	0	0	0	1	+1
		//	0	0	1	0	-1
		//	0	0	1	1	+2  (assume pin1 edges only)
		//	0	1	0	0	-1
		//	0	1	0	1	no movement
		//	0	1	1	0	-2  (assume pin1 edges only)
		//	0	1	1	1	+1
		//	1	0	0	0	+1
		//	1	0	0	1	-2  (assume pin1 edges only)
		//	1	0	1	0	no movement
		//	1	0	1	1	-1
		//	1	1	0	0	+2  (assume pin1 edges only)
		//	1	1	0	1	-1
		//	1	1	1	0	+1
		//	1	1	1	1	no movement
/*
	// Simple, easy-to-read "documentation" version :-)
	//
	void update(void) {
		uint8_t s = state & 3;
		if (digitalRead(pin1)) s |= 4;
		if (digitalRead(pin2)) s |= 8;
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

Encoder::Encoder(volatile float &ptrEncoderCount, std::string ChA, std::string ChB, int modifier) :
	ptrEncoderCount(&ptrEncoderCount),
	ChA(ChA),
	ChB(ChB),
    modifier(modifier)
{
	this->pin1 = new Pin(this->ChA, INPUT, this->modifier);			// create Pin
    this->pin2 = new Pin(this->ChB, INPUT, this->modifier);			// create Pin
	this->count = 0;								                // initialise the count to 0
}

void Encoder::update()
{
    uint8_t s = this->state & 3;

    if (this->pin1->get()) s |= 4;
    if (this->pin2->get()) s |= 8;

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

    *(this->ptrEncoderCount) = this->count;
}

