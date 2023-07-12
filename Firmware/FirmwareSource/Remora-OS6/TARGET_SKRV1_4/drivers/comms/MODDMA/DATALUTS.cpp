/*
    Copyright (c) 2010 Andy Kirkham
 
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
 
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
 
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "MODDMA.h"

#ifndef MBED_H
#include "mbed.h"
#endif

#ifndef MODDMA_CONFIG_H
#include "CONFIG.h"
#endif

namespace AjK {

uint32_t
MODDMA::LUTPerAddr(int n)
{
    const uint32_t lut[] = { 
          (uint32_t)&LPC_SSP0->DR         // SSP0 Tx
        , (uint32_t)&LPC_SSP0->DR         // SSP0 Rx
        , (uint32_t)&LPC_SSP1->DR         // SSP1 Tx
        , (uint32_t)&LPC_SSP1->DR         // SSP1 Rx
        , (uint32_t)&LPC_ADC->ADGDR       // ADC
        , (uint32_t)&LPC_I2S->I2STXFIFO   // I2S Tx
        , (uint32_t)&LPC_I2S->I2SRXFIFO   // I2S Rx
        , (uint32_t)&LPC_DAC->DACR        // DAC
        , (uint32_t)&LPC_UART0->THR       // UART0 Tx
        , (uint32_t)&LPC_UART0->RBR       // UART0 Rx
        , (uint32_t)&LPC_UART1->THR       // UART1 Tx
        , (uint32_t)&LPC_UART1->RBR       // UART1 Rx
        , (uint32_t)&LPC_UART2->THR       // UART2 Tx
        , (uint32_t)&LPC_UART2->RBR       // UART2 Rx
        , (uint32_t)&LPC_UART3->THR       // UART3 Tx
        , (uint32_t)&LPC_UART3->RBR       // UART3 Rx
        , (uint32_t)&LPC_TIM0->MR0        // MAT0.0
        , (uint32_t)&LPC_TIM0->MR1        // MAT0.1
        , (uint32_t)&LPC_TIM1->MR0        // MAT1.0
        , (uint32_t)&LPC_TIM1->MR1        // MAT1.1
        , (uint32_t)&LPC_TIM2->MR0        // MAT2.0
        , (uint32_t)&LPC_TIM2->MR1        // MAT2.1
        , (uint32_t)&LPC_TIM3->MR0        // MAT3.0
        , (uint32_t)&LPC_TIM3->MR1        // MAT3.1   
    };
    return lut[n & 0xFF];    
}

uint32_t
MODDMA::Channel_p(int channel)
{
    const uint32_t lut[] = {
          (uint32_t)LPC_GPDMACH0
        , (uint32_t)LPC_GPDMACH1
        , (uint32_t)LPC_GPDMACH2
        , (uint32_t)LPC_GPDMACH3
        , (uint32_t)LPC_GPDMACH4
        , (uint32_t)LPC_GPDMACH5
        , (uint32_t)LPC_GPDMACH6
        , (uint32_t)LPC_GPDMACH7
    };
    return lut[channel & 0xFF];
}

uint8_t
MODDMA::LUTPerBurst(int n)
{
    const uint8_t lut[] = {
          (uint8_t)_4       // SSP0 Tx 
        , (uint8_t)_4       // SSP0 Rx
        , (uint8_t)_4       // SSP1 Tx
        , (uint8_t)_4       // SSP1 Rx
        , (uint8_t)_1       // ADC
        , (uint8_t)_32      // I2S channel 0
        , (uint8_t)_32      // I2S channel 1
        , (uint8_t)_1       // DAC
        , (uint8_t)_1       // UART0 Tx
        , (uint8_t)_1       // UART0 Rx
        , (uint8_t)_1       // UART1 Tx
        , (uint8_t)_1       // UART1 Rx
        , (uint8_t)_1       // UART2 Tx
        , (uint8_t)_1       // UART2 Rx
        , (uint8_t)_1       // UART3 Tx
        , (uint8_t)_1       // UART3 Rx
        , (uint8_t)_1       // MAT0.0
        , (uint8_t)_1       // MAT0.1
        , (uint8_t)_1       // MAT1.0
        , (uint8_t)_1       // MAT1.1
        , (uint8_t)_1       // MAT2.0
        , (uint8_t)_1       // MAT2.1
        , (uint8_t)_1       // MAT3.0
        , (uint8_t)_1       // MAT3.1
    };
    return lut[n & 0xFFF];
}

uint8_t
MODDMA::LUTPerWid(int n)
{
    const uint8_t lut[] = {
          (uint8_t)byte      // SSP0 Tx
        , (uint8_t)byte      // SSP0 Rx
        , (uint8_t)byte      // SSP1 Tx
        , (uint8_t)byte      // SSP1 Rx
        , (uint8_t)word      // ADC
        , (uint8_t)word      // I2S channel 0
        , (uint8_t)word      // I2S channel 1
        , (uint8_t)word      // DAC 
        , (uint8_t)byte      // UART0 Tx
        , (uint8_t)byte      // UART0 Rx
        , (uint8_t)byte      // UART1 Tx
        , (uint8_t)byte      // UART1 Rx
        , (uint8_t)byte      // UART2 Tx
        , (uint8_t)byte      // UART2 Rx
        , (uint8_t)byte      // UART3 Tx
        , (uint8_t)byte      // UART3 Rx  
        , (uint8_t)word      // MAT0.0
        , (uint8_t)word      // MAT0.1
        , (uint8_t)word      // MAT1.0
        , (uint8_t)word      // MAT1.1
        , (uint8_t)word      // MAT2.0
        , (uint8_t)word      // MAT2.1
        , (uint8_t)word      // MAT3.0
        , (uint8_t)word      // MAT3.1  
    };
    return lut[n & 0xFFF];
}

}; // namespace AjK ends
