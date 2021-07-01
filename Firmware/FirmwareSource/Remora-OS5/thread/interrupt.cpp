#include "interrupt.h"
#include "LPC17xx.h"

#include <cstdio>

// Define the vector table, it is only declared in the class declaration
Interrupt* Interrupt::ISRVectorTable[] = {0};

// Constructor
Interrupt::Interrupt(void){}


// Methods

void Interrupt::Register(int interruptNumber, Interrupt* intThisPtr)
{
	printf("Registering interrupt for interrupt number = %d\n", interruptNumber);
	ISRVectorTable[interruptNumber] = intThisPtr;
}

void Interrupt::TIMER0_Wrapper(void)
{
	ISRVectorTable[TIMER0_IRQn]->ISR_Handler();
}

void Interrupt::TIMER1_Wrapper(void)
{
	ISRVectorTable[TIMER1_IRQn]->ISR_Handler();
}

void Interrupt::TIMER2_Wrapper(void)
{
	ISRVectorTable[TIMER2_IRQn]->ISR_Handler();
}

void Interrupt::QEI_Wrapper(void)
{
	ISRVectorTable[QEI_IRQn]->ISR_Handler();
}