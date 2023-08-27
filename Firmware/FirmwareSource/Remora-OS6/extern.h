#ifndef EXTERN_H
#define EXTERN_H

#include "configuration.h"
#include "remora.h"

#include "ArduinoJson.h"
#include "pruThread.h"

extern uint32_t base_freq;
extern uint32_t servo_freq;

extern JsonObject module;

extern volatile bool PRUreset;

// unions for RX and TX data
extern volatile rxData_t rxData;
extern volatile txData_t txData;

// pointers to objects with global scope
extern pruThread* baseThread;
extern pruThread* servoThread;
extern pruThread* commsThread;

// pointers to data
extern volatile rxData_t*  ptrRxData;
extern volatile txData_t*  ptrTxData;
extern volatile int32_t*   ptrTxHeader;  
extern volatile bool*      ptrPRUreset;
extern volatile int32_t*   ptrJointFreqCmd[JOINTS];
extern volatile int32_t*   ptrJointFeedback[JOINTS];
extern volatile uint8_t*   ptrJointEnable;
extern volatile float*     ptrSetPoint[VARIABLES];
extern volatile float*     ptrProcessVariable[VARIABLES];
extern volatile uint16_t*   ptrInputs;
extern volatile uint16_t*   ptrOutputs;


#endif