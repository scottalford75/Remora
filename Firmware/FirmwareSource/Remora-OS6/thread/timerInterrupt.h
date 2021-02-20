#ifndef TIMERINTERRUPT_H
#define TIMERINTERRUPT_H

// Derived class for timer interrupts

class spiPRUtimer; // forward declatation

class TimerInterrupt : public Interrupt
{
	private:
	    
		spiPRUtimer* InterruptOwnerPtr;
	
	public:

		TimerInterrupt(int interruptNumber, spiPRUtimer* ownerptr);
    
		void ISR_Handler(void);
};

#endif