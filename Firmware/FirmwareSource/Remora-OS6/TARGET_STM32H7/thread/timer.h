#ifndef TIMER_H
#define TIMER_H

#include "mbed.h"
#include <stdint.h>

#define TIM_PSC 4
#define APB1CLK SystemCoreClock
#define APB2CLK SystemCoreClock/2

class TimerInterrupt; // forward declatation
class pruThread; // forward declatation

class pruTimer
{
	friend class TimerInterrupt;

	private:

		TimerInterrupt* 	interruptPtr;
		TIM_TypeDef* 	    timer;
		IRQn_Type 			irq;
		uint32_t 			frequency;
		pruThread* 			timerOwnerPtr;

		void startTimer(void);
		void timerTick();			// Private timer tiggered method

	public:

		pruTimer(TIM_TypeDef* timer, IRQn_Type irq, uint32_t frequency, pruThread* ownerPtr);
        void stopTimer(void);

};

#endif
