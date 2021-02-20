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

uint32_t
MODDMA::Setup(MODDMA_Config *config)
{
    LPC_GPDMACH_TypeDef *pChannel = (LPC_GPDMACH_TypeDef *)Channel_p( config->channelNum() );
    
    setups[config->channelNum() & 0x7] = config;
    
    // Reset the Interrupt status
    LPC_GPDMA->DMACIntTCClear = IntTCClear_Ch( config->channelNum() );
    LPC_GPDMA->DMACIntErrClr  = IntErrClr_Ch ( config->channelNum() );

    // Clear DMA configure
    pChannel->DMACCControl = 0x00;
    pChannel->DMACCConfig  = 0x00;

    // Assign Linker List Item value 
    pChannel->DMACCLLI = config->dmaLLI();

    // Set value to Channel Control Registers 
    switch (config->transferType()) {
    
        // Memory to memory
        case m2m:
            // Assign physical source and destination address
            pChannel->DMACCSrcAddr  = config->srcMemAddr();
            pChannel->DMACCDestAddr = config->dstMemAddr();
            pChannel->DMACCControl
                = CxControl_TransferSize(config->transferSize()) 
                | CxControl_SBSize(_32) 
                | CxControl_DBSize(_32) 
                | CxControl_SWidth(config->transferWidth()) 
                | CxControl_DWidth(config->transferWidth()) 
                | CxControl_SI() 
                | CxControl_DI() 
                | CxControl_I();
            break;
        
        // Memory to peripheral
        case m2p:
            // Assign physical source
            pChannel->DMACCSrcAddr = config->srcMemAddr();
            // Assign peripheral destination address
            pChannel->DMACCDestAddr = (uint32_t)LUTPerAddr(config->dstConn());
            pChannel->DMACCControl
                = CxControl_TransferSize((uint32_t)config->transferSize()) 
                | CxControl_SBSize((uint32_t)LUTPerBurst(config->dstConn())) 
                | CxControl_DBSize((uint32_t)LUTPerBurst(config->dstConn())) 
                | CxControl_SWidth((uint32_t)LUTPerWid(config->dstConn())) 
                | CxControl_DWidth((uint32_t)LUTPerWid(config->dstConn())) 
                | CxControl_SI() 
                | CxControl_I();
            break;
            
        // Peripheral to memory
        case p2m:
            // Assign peripheral source address
            pChannel->DMACCSrcAddr = (uint32_t)LUTPerAddr(config->srcConn());
            // Assign memory destination address
            pChannel->DMACCDestAddr = config->dstMemAddr();
            pChannel->DMACCControl
                = CxControl_TransferSize((uint32_t)config->transferSize()) 
                | CxControl_SBSize((uint32_t)LUTPerBurst(config->srcConn())) 
                | CxControl_DBSize((uint32_t)LUTPerBurst(config->srcConn())) 
                | CxControl_SWidth((uint32_t)LUTPerWid(config->srcConn())) 
                | CxControl_DWidth((uint32_t)LUTPerWid(config->srcConn())) 
                | CxControl_DI() 
                | CxControl_I();
            break;
            
        // Peripheral to peripheral
        case p2p:
            // Assign peripheral source address
            pChannel->DMACCSrcAddr = (uint32_t)LUTPerAddr(config->srcConn());
            // Assign peripheral destination address
            pChannel->DMACCDestAddr = (uint32_t)LUTPerAddr(config->dstConn());
            pChannel->DMACCControl
                = CxControl_TransferSize((uint32_t)config->transferSize()) 
                | CxControl_SBSize((uint32_t)LUTPerBurst(config->srcConn())) 
                | CxControl_DBSize((uint32_t)LUTPerBurst(config->dstConn())) 
                | CxControl_SWidth((uint32_t)LUTPerWid(config->srcConn())) 
                | CxControl_DWidth((uint32_t)LUTPerWid(config->dstConn())) 
                | CxControl_I();
            break;
            
        // GPIO to memory
        case g2m:
            // Assign GPIO source address
            pChannel->DMACCSrcAddr = config->srcMemAddr();
            // Assign memory destination address
            pChannel->DMACCDestAddr = config->dstMemAddr();
            pChannel->DMACCControl
                = CxControl_TransferSize((uint32_t)config->transferSize()) 
                | CxControl_SBSize((uint32_t)LUTPerBurst(config->srcConn())) 
                | CxControl_DBSize((uint32_t)LUTPerBurst(config->srcConn())) 
                | CxControl_SWidth((uint32_t)LUTPerWid(config->srcConn())) 
                | CxControl_DWidth((uint32_t)LUTPerWid(config->srcConn())) 
                | CxControl_DI() 
                | CxControl_I();
            break;
            
        // Memory to GPIO
        case m2g:
            // Assign physical source
            pChannel->DMACCSrcAddr = config->srcMemAddr();
            // Assign peripheral destination address
            pChannel->DMACCDestAddr = config->dstMemAddr();
            pChannel->DMACCControl
                = CxControl_TransferSize((uint32_t)config->transferSize()) 
                | CxControl_SBSize((uint32_t)LUTPerBurst(config->dstConn())) 
                | CxControl_DBSize((uint32_t)LUTPerBurst(config->dstConn())) 
                | CxControl_SWidth((uint32_t)LUTPerWid(config->dstConn())) 
                | CxControl_DWidth((uint32_t)LUTPerWid(config->dstConn())) 
                | CxControl_SI() 
                | CxControl_I();
            break;
            
        // Do not support any more transfer type, return ERROR
        default:
            return 0;
    }

     // Re-Configure DMA Request Select for source peripheral 
    if (config->srcConn() > 15) {
        LPC_SC->DMAREQSEL |= (1 << (config->srcConn() - 16));
    } 
    else {
        LPC_SC->DMAREQSEL &= ~(1 << (config->srcConn() - 8));
    }

    // Re-Configure DMA Request Select for destination peripheral
    if (config->dstConn() > 15) {
        LPC_SC->DMAREQSEL |= (1 << (config->dstConn() - 16));
    } 
    else {
        LPC_SC->DMAREQSEL &= ~(1 << (config->dstConn() - 8));
    }

    // Enable DMA channels, little endian 
    LPC_GPDMA->DMACConfig = _E;
    while (!(LPC_GPDMA->DMACConfig & _E));

    // Calculate absolute value for Connection number
    uint32_t tmp1 = config->srcConn(); tmp1 = ((tmp1 > 15) ? (tmp1 - 8) : tmp1);
    uint32_t tmp2 = config->dstConn(); tmp2 = ((tmp2 > 15) ? (tmp2 - 8) : tmp2);

    if (config->dmacSync()) {
        uint32_t tmp3 = config->dmacSync(); tmp3 = ((tmp3 > 15) ? (tmp3 - 8) : tmp3);
        LPC_GPDMA->DMACSync |= Sync_Src( tmp3 );
    }
    
    uint32_t tfer_type = (uint32_t)config->transferType();
    if (tfer_type == g2m || tfer_type == m2g) {
        tfer_type -= 2; // Adjust psuedo transferType to a real transferType.
    }
    
    // Configure DMA Channel, enable Error Counter and Terminate counter
    pChannel->DMACCConfig 
        = CxConfig_IE() 
        | CxConfig_ITC() 
        | CxConfig_TransferType(tfer_type) 
        | CxConfig_SrcPeripheral(tmp1) 
        | CxConfig_DestPeripheral(tmp2);

    return pChannel->DMACCControl;
}

}; // namespace AjK ends

