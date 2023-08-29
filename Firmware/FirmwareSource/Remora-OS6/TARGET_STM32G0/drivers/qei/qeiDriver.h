#ifndef QEIDRIVER_H
#define QEIDRIVER_H

#include "mbed.h"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <string>

#include "stm32g0xx_hal.h"


class QEIdriver
{
    private:

        TIM_HandleTypeDef       htim;
        TIM_Encoder_InitTypeDef sConfig  = {0};
        TIM_MasterConfigTypeDef sMasterConfig  = {0};

        InterruptIn             qeiIndex;
        IRQn_Type 		        irq;

        void interruptHandler();

    public:

        bool                    hasIndex;
        bool                    indexDetected;
        int32_t                 indexCount;

        QEIdriver();            // for channel A & B
        QEIdriver(bool);        // For channels A & B, and index

        void init(void);
        uint32_t get(void);

};

#endif
