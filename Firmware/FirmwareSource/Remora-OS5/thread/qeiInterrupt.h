#ifndef QEIINTERRUPT_H
#define QEIINTERRUPT_H

// Derived class for timer interrupts

class QEI; // forward declatation

class qeiInterrupt : public Interrupt
{
	private:
	    
		QEI* InterruptOwnerPtr;
	
	public:

		qeiInterrupt(int interruptNumber, QEI* ownerptr);
    
		void ISR_Handler(void);
};

#endif