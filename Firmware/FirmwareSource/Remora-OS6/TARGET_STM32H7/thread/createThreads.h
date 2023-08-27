#include "extern.h"


void createThreads(void)
{
    // Create the thread objects and set the interrupt vectors to RAM. This is needed
    // as we are using the SD bootloader that requires a different code starting
    // address. Also set interrupt priority with NVIC_SetPriority.

    baseThread = new pruThread(TIM3, TIM3_IRQn, base_freq);
    NVIC_SetVector(TIM3_IRQn, (uint32_t)TIM3_IRQHandler);
    NVIC_SetPriority(TIM3_IRQn, 2);

    servoThread = new pruThread(TIM4, TIM4_IRQn, servo_freq);
    NVIC_SetVector(TIM4_IRQn, (uint32_t)TIM4_IRQHandler);
    NVIC_SetPriority(TIM4_IRQn, 3);

    commsThread = new pruThread(TIM5, TIM5_IRQn, PRU_COMMSFREQ);
    NVIC_SetVector(TIM5_IRQn, (uint32_t)TIM5_IRQHandler);
    NVIC_SetPriority(TIM5_IRQn, 4);
}