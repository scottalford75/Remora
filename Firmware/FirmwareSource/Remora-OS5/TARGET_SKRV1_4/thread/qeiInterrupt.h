#ifndef QEIINTERRUPT_H
#define QEIINTERRUPT_H

// Derived class for timer interrupts

class QEIdriver; // forward declatation

class qeiInterrupt : public Interrupt
{
	private:
	    
		QEIdriver* InterruptOwnerPtr;
	
	public:

		qeiInterrupt(int interruptNumber, QEIdriver* ownerptr);
    
		void ISR_Handler(void);
};

#endif