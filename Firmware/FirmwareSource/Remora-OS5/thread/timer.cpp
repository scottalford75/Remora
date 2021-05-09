#include "mbed.h"
#include "LPC17xx.h"

#include <iostream>
#include <stdio.h>

#include "interrupt.h"
#include "timerInterrupt.h"
#include "timer.h"
#include "pruThread.h"


#define SBIT_TIMER0  1
#define SBIT_TIMER1  2
#define SBIT_TIMER2  22
#define SBIT_TIMER3  23

#define SBIT_MR0I    0
#define SBIT_MR0R    1
#define SBIT_CNTEN   0


// Timer constructor
pruTimer::pruTimer(LPC_TIM_TypeDef* timer, IRQn_Type irq, uint32_t frequency, pruThread* ownerPtr):
	timer(timer),
	irq(irq),
	frequency(frequency),
	timerOwnerPtr(ownerPtr)
{
	interruptPtr = new TimerInterrupt(this->irq, this);	// Instantiate a new Timer Interrupt object and pass "this" pointer

	this->startTimer();
}


void pruTimer::timerTick(void)
{
	//Do something here
	this->timerOwnerPtr->run();
}



void pruTimer::startTimer(void)
{
    if (this->timer == LPC_TIM0)
    {
        printf("	power on Timer 0\n");
        LPC_SC->PCONP |= (1<<SBIT_TIMER0);
    }
    else if (this->timer == LPC_TIM1)
    {
        printf("	power on Timer 1\n");
        LPC_SC->PCONP |= (1<<SBIT_TIMER1);
    }
    else if (this->timer == LPC_TIM2)
    {
        printf("	power on Timer 2\n");
        LPC_SC->PCONP |= (1<<SBIT_TIMER2);
    }

    printf("	timer set MCR\n");
    this->timer->MCR  = (1<<SBIT_MR0I) | (1<<SBIT_MR0R);     /* Clear TC on MR0 match and Generate Interrupt*/
    
    printf("	timer set PR\n");
    this->timer->PR   = 0x00;
    
    printf("	timer set PRO\n");
    this->timer->MR0  = SystemCoreClock/4/this->frequency;
    
    printf("	timer start\n");
    this->timer->TCR  = (1<<SBIT_CNTEN);                     /* Start timer by setting the Counter Enable*/
    
    NVIC_EnableIRQ(this->irq);
}

void pruTimer::stopTimer()
{
    NVIC_DisableIRQ(this->irq);

    printf("	timer stop\n");
    this->timer->TCR  = (0<<SBIT_CNTEN);
}