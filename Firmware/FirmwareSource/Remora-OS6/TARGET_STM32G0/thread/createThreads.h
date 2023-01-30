#include "extern.h"


void createThreads(void)
{
    // Create the thread objects and set the interrupt vectors to RAM. This is needed
    // as we are using the SD bootloader that requires a different code starting
    // address. Also set interrupt priority with NVIC_SetPriority.

    baseThread = new pruThread(TIM1, TIM1_BRK_UP_TRG_COM_IRQn, base_freq);
    NVIC_SetVector(TIM1_BRK_UP_TRG_COM_IRQn, (uint32_t)TIM1_IRQHandler);
    NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 2);

    servoThread = new pruThread(TIM2, TIM2_IRQn, servo_freq);
    NVIC_SetVector(TIM2_IRQn, (uint32_t)TIM2_IRQHandler);
    NVIC_SetPriority(TIM2_IRQn , 3);

    commsThread = new pruThread(TIM3, TIM3_TIM4_IRQn, PRU_COMMSFREQ);
    NVIC_SetVector(TIM3_TIM4_IRQn, (uint32_t)TIM3_IRQHandler);
    NVIC_SetPriority(TIM3_TIM4_IRQn, 4);
}