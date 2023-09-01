#include "extern.h"


void createThreads(void)
{
    // Create the thread objects and set the interrupt vectors to RAM. This is needed
    // as we are using the SD bootloader that requires a different code starting
    // address. Also set interrupt priority with NVIC_SetPriority.

    baseThread = new pruThread(TIM9, TIM1_BRK_TIM9_IRQn, base_freq);
    NVIC_SetVector(TIM1_BRK_TIM9_IRQn, (uint32_t)TIM9_IRQHandler);
    NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 2);

    servoThread = new pruThread(TIM10, TIM1_UP_TIM10_IRQn, servo_freq);
    NVIC_SetVector(TIM1_UP_TIM10_IRQn, (uint32_t)TIM10_IRQHandler);
    NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 3);

    commsThread = new pruThread(TIM11, TIM1_TRG_COM_TIM11_IRQn, PRU_COMMSFREQ);
    NVIC_SetVector(TIM1_TRG_COM_TIM11_IRQn, (uint32_t)TIM11_IRQHandler);
    NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 4);
}