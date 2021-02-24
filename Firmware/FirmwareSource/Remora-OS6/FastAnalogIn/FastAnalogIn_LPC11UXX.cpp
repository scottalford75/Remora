#ifdef TARGET_LPC11UXX

#include "FastAnalogIn.h"
static inline int div_round_up(int x, int y)
{
    return (x + (y - 1)) / y;
}

#define LPC_IOCON0_BASE (LPC_IOCON_BASE)
#define LPC_IOCON1_BASE (LPC_IOCON_BASE + 0x60)
#define MAX_ADC_CLK     4500000

static const PinMap PinMap_ADC[] = {
    {P0_11, ADC0_0, 0x02},
    {P0_12, ADC0_1, 0x02},
    {P0_13, ADC0_2, 0x02},
    {P0_14, ADC0_3, 0x02},
    {P0_15, ADC0_4, 0x02},
    {P0_16, ADC0_5, 0x01},
    {P0_22, ADC0_6, 0x01},
    {P0_23, ADC0_7, 0x01},
    {NC   , NC    , 0   }
};

static int channel_usage[8] = {0,0,0,0,0,0,0,0};



FastAnalogIn::FastAnalogIn(PinName pin, bool enabled)
{
    ADCnumber = (ADCName)pinmap_peripheral(pin, PinMap_ADC);
    if (ADCnumber == (uint32_t)NC)
        error("ADC pin mapping failed");
    datareg = (uint32_t*) (&LPC_ADC->DR0 + ADCnumber);
    
    // Power up ADC
    LPC_SYSCON->PDRUNCFG &= ~ (1 << 4);
    LPC_SYSCON->SYSAHBCLKCTRL |= ((uint32_t)1 << 13);
 
    uint32_t pin_number = (uint32_t)pin;
    __IO uint32_t *reg = (pin_number < 32) ? (__IO uint32_t*)(LPC_IOCON0_BASE + 4 * pin_number) : (__IO uint32_t*)(LPC_IOCON1_BASE + 4 * (pin_number - 32));
 
    // set pin to ADC mode
    *reg &= ~(1 << 7); // set ADMODE = 0 (analog mode)
 
    uint32_t clkdiv = div_round_up(SystemCoreClock, MAX_ADC_CLK) - 1;
 
    LPC_ADC->CR = (LPC_ADC->CR & 0xFF)      // keep current channels
                | (clkdiv << 8) // max of 4.5MHz
                | (1 << 16)     // BURST = 1, hardware controlled
                | ( 0 << 17 );  // CLKS = 0, we stick to 10 bit mode
    
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
            LPC_ADC->CR |= (1<<ADCnumber);
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
            LPC_ADC->CR &= ~(1<<ADCnumber);
    }
    running = false;
}

unsigned short FastAnalogIn::read_u16( void )
{
    unsigned int retval;
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
            retval = *datareg;
            if ((retval>>31) == 1)
                break;
        }
        //Disable again
        disable();
    }
    
    //Do same thing as standard mbed lib, unused bit 0-3, replicate 4-7 in it
    retval &= ~0xFFFF003F;
    retval |= (retval >> 6) & 0x003F;
    return retval;
}
#endif //defined TARGET_LPC11UXX
