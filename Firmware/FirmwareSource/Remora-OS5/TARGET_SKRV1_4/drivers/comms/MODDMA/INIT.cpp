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

namespace AjK {

extern uint32_t oldDMAHandler;
extern "C" void MODDMA_IRQHandler(void);
extern class MODDMA *moddma_p;

void
MODDMA::init(bool isConstructorCalling, int Channels, int Tc, int Err)
{
    if (isConstructorCalling) {    
        if (LPC_SC->PCONP & (1UL << 29)) {
            if (LPC_GPDMA->DMACConfig & 1) {
                error("Only one instance of MODDMA can exist.");
            }
        }
        LPC_SC->PCONP |= (1UL << 29);
        LPC_GPDMA->DMACConfig = 1;
        moddma_p = this;
        for (int i = 0; i < 8; i++) {
            setups[i] = (MODDMA_Config *)NULL;
        }        
    }
    
    // Reset channel configuration register(s)
    if (Channels & 0x01) LPC_GPDMACH0->DMACCConfig = 0;
    if (Channels & 0x02) LPC_GPDMACH1->DMACCConfig = 0;
    if (Channels & 0x04) LPC_GPDMACH2->DMACCConfig = 0;
    if (Channels & 0x08) LPC_GPDMACH3->DMACCConfig = 0;
    if (Channels & 0x10) LPC_GPDMACH4->DMACCConfig = 0;
    if (Channels & 0x20) LPC_GPDMACH5->DMACCConfig = 0;
    if (Channels & 0x40) LPC_GPDMACH6->DMACCConfig = 0;
    if (Channels & 0x80) LPC_GPDMACH7->DMACCConfig = 0;

    /* Clear DMA interrupt and error flag */
    LPC_GPDMA->DMACIntTCClear = Tc;
    LPC_GPDMA->DMACIntErrClr  = Err;
    
    if (isConstructorCalling) {    
        oldDMAHandler = NVIC_GetVector(DMA_IRQn);
        NVIC_SetVector(DMA_IRQn, (uint32_t)MODDMA_IRQHandler);
        NVIC_EnableIRQ(DMA_IRQn);
    }
}

}; // namespace AjK ends
