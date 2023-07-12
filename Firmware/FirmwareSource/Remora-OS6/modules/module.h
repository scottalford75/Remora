#ifndef MODULE_H
#define MODULE_H

#include <cstdint>

// Module base class
// All modules are derived from this base class

class Module
{
	protected:

		int32_t threadFreq;
		int32_t slowUpdateFreq;
		int32_t updateCount;
		int32_t counter;


	public:

		Module();					// constructor to run the module at the thread frequency
		Module(int32_t, int32_t);	// constructor to run the module at a "slow update frequency" < thread frequency

		virtual ~Module();
		void runModule();			// the standard interface that the thread runs at the thread frequency, this calls update() at the module frequency
		virtual void update();		// the standard interface for update of the module - use for stepgen, PWM etc
		virtual void slowUpdate();	// the standard interface for the slow update - use for PID controller etc
        virtual void configure();   // the standard interface for one off configuration

};

#endif

