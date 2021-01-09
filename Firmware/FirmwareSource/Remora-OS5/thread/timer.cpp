#include "mbed.h"
#include "LPC17xx.h"

#include <iostream>
#include <stdio.h>

#include "interrupt.h"
#include "timerInterrupt.h"
#include "timer.h"
#include "PRUthread.h"


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
	printf("	power on timer\n");
	LPC_SC->PCONP |= (1<<SBIT_TIMER0) | (1<<SBIT_TIMER1); /* Power ON Timer0,1 */
	//LPC_SC->PCONP |= (1<<SBIT_TIMER0) | (1<<SBIT_TIMER1) | (1<<SBIT_TIMER2) | (1<<SBIT_TIMER3); // Power ON Timers

	printf("	timer set MCR\n");
	this->timer->MCR  = (1<<SBIT_MR0I) | (1<<SBIT_MR0R);     /* Clear TC on MR0 match and Generate Interrupt*/
	//LPC_TIM1->MCR  = (1<<SBIT_MR0I) | (1<<SBIT_MR0R);
	printf("	timer set PR\n");
	this->timer->PR   = 0x00;
	//LPC_TIM1->PR   = 0x00;
	printf("	timer set PRO\n");
    this->timer->MR0  = SystemCoreClock/4/this->frequency;
	//LPC_TIM1->MR0  = SystemCoreClock/4/this->frequency;
	printf("	timer start\n");
    this->timer->TCR  = (1<<SBIT_CNTEN);                     /* Start timer by setting the Counter Enable*/
	//LPC_TIM1->TCR  = (1<<SBIT_CNTEN);

    NVIC_EnableIRQ(this->irq);
}
