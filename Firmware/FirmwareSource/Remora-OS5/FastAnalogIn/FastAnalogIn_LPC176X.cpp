#include "FastAnalogIn.h"
static inline int div_round_up(int x, int y)
{
    return (x + (y - 1)) / y;
}

static const PinMap PinMap_ADC[] = {
    P0_23, ADC0_0, 1,
    P0_24, ADC0_1, 1,
    P0_25, ADC0_2, 1,
    P0_26, ADC0_3, 1,
    P1_30, ADC0_4, 3,
    P1_31, ADC0_5, 3,
    P0_2,  ADC0_7, 2,
    P0_3,  ADC0_6, 2,
    NC,    NC,     0
};

static int channel_usage[8] = {0,0,0,0,0,0,0,0};

FastAnalogIn::FastAnalogIn(PinName pin, bool enabled)
{
    ADCnumber = (ADCName)pinmap_peripheral(pin, PinMap_ADC);
    if (ADCnumber == (uint32_t)NC)
        error("ADC pin mapping failed");
    datareg = (uint32_t*) (&LPC_ADC->ADDR0 + ADCnumber);

    // ensure power is turned on
    LPC_SC->PCONP |= (1 << 12);
    // set PCLK of ADC to /1
    LPC_SC->PCLKSEL0 &= ~(0x3 << 24);
    LPC_SC->PCLKSEL0 |= (0x1 << 24);
    uint32_t PCLK = SystemCoreClock;

    // calculate minimum clock divider
    //  clkdiv = divider - 1
    uint32_t MAX_ADC_CLK = 13000000;
    uint32_t clkdiv = div_round_up(PCLK, MAX_ADC_CLK) - 1;
    // Set the clkdiv
    LPC_ADC->ADCR &= ~(255<<8);
    LPC_ADC->ADCR |= clkdiv<<8;

    //Enable ADC:
    LPC_ADC->ADCR |= 1<<21;

    //Enable burstmode, set start as zero
    LPC_ADC->ADCR |= 1<<16;
    LPC_ADC->ADCR &= ~(7<<24);

    //Map pins
    pinmap_pinout(pin, PinMap_ADC);

    //Enable channel
    running = false;
    enable(enabled);

}

void FastAnalogIn::enable(bool enabled)
{
    //If currently not running
    if (!running) {
        if (enabled) {
            //Enable the ADC channel
            channel_usage[ADCnumber]++;
            LPC_ADC->ADCR |= (1<<ADCnumber);
            running = true;
        } else
            disable();
    }
}

void FastAnalogIn::disable( void )
{
    //If currently running
    if (running) {
        channel_usage[ADCnumber]--;
        
        if (channel_usage[ADCnumber]==0)
            LPC_ADC->ADCR &= ~(1<<ADCnumber);
    }
    running = false;
}

unsigned short FastAnalogIn::read_u16( void )
{
    volatile unsigned int retval;
    //If object is enabled return current value of datareg
    if (running)
        retval = *datareg;
 
    //If it isn't running, enable it and wait until new value is written to datareg
    else {
        //Force a read to clear done bit, enable the ADC channel
        retval = *datareg;
        enable();
        //Wait until it is converted
        while(1) {
            wait_us(1);
            retval = *datareg;
            if ((retval>>31) == 1)
                break;
        }
        
        //Do a second conversion since first one always fails for some reason
        while(1) {
            wait_us(1);
            retval = *datareg;
            if ((retval>>31) == 1)
                break;
        }

        //Disable again
        disable();
    }
    
    //Do same thing as standard mbed lib, unused bit 0-3, replicate 4-7 in it
    retval &= ~0xFFFF000F;
    retval |= (retval >> 8) & 0x000F;
    return retval;

}

