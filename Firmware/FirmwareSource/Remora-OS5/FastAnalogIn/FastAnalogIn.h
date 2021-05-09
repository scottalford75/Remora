#ifndef FASTANALOGIN_H
#define FASTANALOGIN_H

/*
 * Includes
 */
#include "mbed.h"
#include "pinmap.h"

 /** A class similar to AnalogIn, only faster, for LPC176X
 *
 * AnalogIn does a single conversion when you read a value (actually several conversions and it takes the median of that).
 * This library runns the ADC conversion automatically in the background.
 * When read is called, it immediatly returns the last sampled value.
 *
 * LPC176X
 * Using more ADC pins in continuous mode will decrease the conversion rate (LPC176X:200kHz/LPC4088:400kHz).
 * If you need to sample one pin very fast and sometimes also need to do AD conversions on another pin,
 * you can disable the continuous conversion on that ADC channel and still read its value.
 *
 *
 * When continuous conversion is disabled, a read will block until the conversion is complete
 * (much like the regular AnalogIn library does).
 * Each ADC channel can be enabled/disabled separately.
 *
 * IMPORTANT : It does not play nicely with regular AnalogIn objects, so either use this library or AnalogIn, not both at the same time!!
 *
 * Example for the LPC176X processor:
 * @code
 * // Print messages when the AnalogIn is greater than 50%
 *
 * #include "mbed.h"
 *
 * FastAnalogIn temperature(p20);
 *
 * int main() {
 *     while(1) {
 *         if(temperature > 0.5) {
 *             printf("Too hot! (%f)", temperature.read());
 *         }
 *     }
 * }
 * @endcode
*/
class FastAnalogIn {

public:
     /** Create a FastAnalogIn, connected to the specified pin
     *
     * @param pin AnalogIn pin to connect to
     * @param enabled Enable the ADC channel (default = true)
     */
    FastAnalogIn( PinName pin, bool enabled = true );
    
    ~FastAnalogIn( void )
    {
        disable();
    }
    
    /** Enable the ADC channel
    *
    * @param enabled Bool that is true for enable, false is equivalent to calling disable
    */
    void enable(bool enabled = true);
    
    /** Disable the ADC channel
    *
    * Disabling unused channels speeds up conversion in used channels. 
    * When disabled you can still call read, that will do a single conversion (actually two since the first one always returns 0 for unknown reason).
    * Then the function blocks until the value is read. This is handy when you sometimes needs a single conversion besides the automatic conversion
    */
    void disable( void );
    
    /** Returns the raw value
    *
    * @param return Unsigned integer with converted value
    */
    unsigned short read_u16( void );
    
    /** Returns the scaled value
    *
    * @param return Float with scaled converted value to 0.0-1.0
    */
    float read( void )
    {
        unsigned short value = read_u16();
        return (float)value * (1.0f/65535.0f);
    }
    
    /** An operator shorthand for read()
    */
    operator float() {
        return read();
    }

    
private:
    bool running;    
    char ADCnumber;
    volatile uint32_t *datareg;
};

#endif
