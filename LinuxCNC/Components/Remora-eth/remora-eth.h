
#ifndef REMORA_H
#define REMORA_H


#define JOINTS				8  			// Number of joints - set this the same as Remora firmware code!!!. Max 8 joints
#define VARIABLES          	6 			// Number of command values - set this the same Remora firmware code!!!
#define DIGITAL_OUTPUTS		16
#define DIGITAL_INPUTS		16

#define BUFFER_SIZE			64 			//(4+4*JOINTS+4*COMMANDS+1) //(MAX_MSG*4) //20  SPI buffer size ......FIFO buffer size is 64 bytes?

#define PRU_DATA			0x64617461 	// "data" payload
#define PRU_READ          	0x72656164  // "read" payload
#define PRU_WRITE         	0x77726974  // "writ" payload
#define PRU_ACKNOWLEDGE		0x61636b6e	// "ackn" payload
#define PRU_ERR		        0x6572726f	// "erro" payload
#define PRU_ESTOP           0x65737470  // "estp" payload

#define STEPBIT				22			// bit location in DDS accum
#define STEP_MASK			(1L<<STEPBIT)
#define STEP_OFFSET			(1L<<(STEPBIT-1))

#define PRU_BASEFREQ		40000 		// Base freq of the PRU stepgen in Hz



#endif
