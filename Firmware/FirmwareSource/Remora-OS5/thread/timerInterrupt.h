#ifndef TIMERINTERRUPT_H
#define TIMERINTERRUPT_H

// Derived class for timer interrupts

class pruTimer; // forward declatation

class TimerInterrupt : public Interrupt
{
	private:
	    
		pruTimer* InterruptOwnerPtr;
	
	public:

		TimerInterrupt(int interruptNumber, pruTimer* ownerptr);
    
		void ISR_Handler(void);
};

#endif