/* $Id:$

1.13- 2 Mar 2013

    * Update RESERVED9 to DMAREQSEL in SETUP.cpp
      Thanks Bryce Chee for pointing it out.

1.12- 14 Mar 2011

    * Added example4.h that demonstrates alternately sending
      two buffers (double buffering) to the DAC. All those
      people building MP3 players may find this of interest.
      
1.11- 13 Mar 2011

    * Fixed a silly typo in the documentation of example3.h
    
1.10- 13 Mar 2011

    * The rescheduling showed the timer being stopped and restarted
      to perform a new scheduled grab. This was changed to show the 
      timer free running and the reschedules being setup.

1.9 - 13 Mar 2011

    * Improved example3.h to add rescheduling additional grabs
      based on the timer setup.
      
1.8 - 13 Mar 2011

    * Renamed example files to .h
    * Added pseudo g2m and m2g transferTypes to support GPIO 
      "memory moves" but triggered by peripheral timer. To 
      support this new operating mode added example3.h
    
1.7 - 13 Mar 2011

    * Remove the test at the beginning of the channel setup.
    
1.6 - 8 Mar 2011
      
    * Fixed a typo bug. Reported by Wim van der Vegt
      http://mbed.org/forum/mbed/topic/1798/?page=1#comment-9845
      
1.5 - 5 Feb 2011

    * Found a bug in the NXP library that I had copied over.
      http://mbed.org/forum/mbed/topic/1798
    * Added example2.cpp to support that forum thread.
      
1.4 - 23/11/2010

    * Added some extra overloaded methods to make calling certain
      userland API methods simpler.
      
1.3 - 23/10/2010

    * Added the LLI class wrapper.
    * Added checking channel's LLI for non-null before auto-disable
      of a channel with the ISR.
    * Tested with MODSERIAL which is now natively MODDMA "aware".
      MODSERIAL can now, using MODDMA, send blocks of bytes out
      of it's TX port under DMA control.
        
1.2 - 23/10/2010

    * Improved the IRQ callback attachment API to make 
      easier attachments when creating configurations.
      
1.1 - 23/10/2010

    * Tidied up example1.cpp
    * Removed some unneeded methoids that cause compiler errs.
    
1.0 - 23/11/2010

    * First release

*/
