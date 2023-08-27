#include "extern.h"


void createThreads(void)
{
    // Create the thread objects and set the interrupt vectors to RAM. This is needed
    // as we are using the USB bootloader that requires a different code starting
    // address. Also set interrupt priority with NVIC_SetPriority.
    //
    // Note: DMAC has highest priority, then Base thread and then Servo thread
    //       to ensure SPI data transfer is reliable

    NVIC_SetPriority(DMA_IRQn, 1);

    baseThread = new pruThread(LPC_TIM0, TIMER0_IRQn, base_freq);
    NVIC_SetVector(TIMER0_IRQn, (uint32_t)TIMER0_IRQHandler);
    NVIC_SetPriority(TIMER0_IRQn, 2);

    servoThread = new pruThread(LPC_TIM1, TIMER1_IRQn, servo_freq);
    NVIC_SetVector(TIMER1_IRQn, (uint32_t)TIMER1_IRQHandler);
    NVIC_SetPriority(TIMER1_IRQn, 3);

    commsThread = new pruThread(LPC_TIM2, TIMER2_IRQn, PRU_COMMSFREQ);
    NVIC_SetVector(TIMER2_IRQn, (uint32_t)TIMER2_IRQHandler);
    NVIC_SetPriority(TIMER2_IRQn, 4);

    // for QEI modudule
    NVIC_SetVector(QEI_IRQn, (uint32_t)QEI_IRQHandler);
    NVIC_SetPriority(QEI_IRQn, 5);
}