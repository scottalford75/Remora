#ifndef TIMER_H
#define TIMER_H

#include "mbed.h"
#include <stdint.h>

class TimerInterrupt; // forward declatation
class pruThread; // forward declatation

class pruTimer
{
	friend class TimerInterrupt;

	private:

		TimerInterrupt* 	interruptPtr;
		LPC_TIM_TypeDef* 	timer;
		IRQn_Type 			irq;
		uint32_t 			frequency;
		pruThread* 			timerOwnerPtr;

		void startTimer(void);
		void timerTick();			// Private timer tiggered method

	public:

		pruTimer(LPC_TIM_TypeDef* timer, IRQn_Type irq, uint32_t frequency, pruThread* ownerPtr);
        void stopTimer(void);

};

#endif
