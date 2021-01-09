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

#ifdef NOCOMPILE

#ifndef MODDMA_CONFIG_H
#define MODDMA_CONFIG_H

#include "mbed.h"

namespace AjK {

// Forward reference.
class MODDMA;

class  MODDMA_Channel_CFG_t {
public:

    // *****************************************
    // From GPDMA by NXP MCU SW Application Team
    // *****************************************
    
    uint32_t ChannelNum;        //!< DMA channel number, should be in range from 0 to 7. 
    uint32_t TransferSize;      //!< Length/Size of transfer 
    uint32_t TransferWidth;     //!< Transfer width - used for TransferType is GPDMA_TRANSFERTYPE_m2m only 
    uint32_t SrcMemAddr;        //!< Physical Src Addr, used in case TransferType is chosen as MODDMA::GPDMA_TRANSFERTYPE::m2m or MODDMA::GPDMA_TRANSFERTYPE::m2p 
    uint32_t DstMemAddr;        //!< Physical Destination Address, used in case TransferType is chosen as MODDMA::GPDMA_TRANSFERTYPE::m2m or MODDMA::GPDMA_TRANSFERTYPE::p2m 
    uint32_t TransferType;      //!< Transfer Type
    uint32_t SrcConn;           ///!< Peripheral Source Connection type, used in case TransferType is chosen as
    uint32_t DstConn;           //!< Peripheral Destination Connection type, used in case TransferType is chosen as
    uint32_t DMALLI;            //!< Linker List Item structure data address if there's no Linker List, set as '0'
    
    // Mbed specifics.
    
    MODDMA_Channel_CFG_t() {
        isrIntTCStat  = new FunctionPointer;
        isrIntErrStat = new FunctionPointer;
    }
    
    ~MODDMA_Channel_CFG_t() {
        delete(isrIntTCStat);
        delete(isrIntErrStat);
    }
        
    class MODDMA_Channel_CFG_t * channelNum(uint32_t n)    { ChannelNum = n;    return this; }
    class MODDMA_Channel_CFG_t * transferSize(uint32_t n)  { TransferSize = n;  return this; }
    class MODDMA_Channel_CFG_t * transferWidth(uint32_t n) { TransferWidth = n; return this; }
    class MODDMA_Channel_CFG_t * srcMemAddr(uint32_t n)    { SrcMemAddr = n;    return this; }
    class MODDMA_Channel_CFG_t * dstMemAddr(uint32_t n)    { DstMemAddr = n;    return this; }
    class MODDMA_Channel_CFG_t * transferType(uint32_t n)  { TransferType = n;  return this; }
    class MODDMA_Channel_CFG_t * srcConn(uint32_t n)       { SrcConn = n;       return this; }
    class MODDMA_Channel_CFG_t * dstConn(uint32_t n)       { DstConn = n;       return this; }
    class MODDMA_Channel_CFG_t * dmaLLI(uint32_t n)        { DMALLI = n;        return this; }
    
    uint32_t channelNum(void) { return ChannelNum; }
    
    FunctionPointer *isrIntTCStat;                        
    FunctionPointer *isrIntErrStat;                        
};

/**
 * @brief GPDMA Linker List Item structure type definition
 */
class GPDMA_LLI_t 
{
public:
    uint32_t SrcAddr;    //!< Source Address 
    uint32_t DstAddr;    //!< Destination address 
    uint32_t NextLLI;    //!< Next LLI address, otherwise set to '0' 
    uint32_t Control;    //!< GPDMA Control of this LLI 
};

}; // namespace AjK ends.

#endif 
#endif
