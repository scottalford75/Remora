#include "interrupt.h"

void TIMER0_IRQHandler()
{
    // Base thread interrupt handler
    unsigned int isrMask = LPC_TIM0->IR;
    LPC_TIM0->IR = isrMask; /* Clear the Interrupt Bit */

    Interrupt::TIMER0_Wrapper();
}


void TIMER1_IRQHandler(void)
{
    // Servo thread interrupt handler
    unsigned int isrMask = LPC_TIM1->IR;
    LPC_TIM1->IR = isrMask; /* Clear the Interrupt Bit */

    Interrupt::TIMER1_Wrapper();
}


void TIMER2_IRQHandler(void)
{
    // Servo thread interrupt handler
    unsigned int isrMask = LPC_TIM2->IR;
    LPC_TIM2->IR = isrMask; /* Clear the Interrupt Bit */

    Interrupt::TIMER2_Wrapper();
}


void QEI_IRQHandler(void)
{
    // QEI (quatrature encoder interface) index interrupt handler
    LPC_QEI->QEICLR = ((uint32_t)(1<<0));   
    Interrupt:: QEI_Wrapper();
}