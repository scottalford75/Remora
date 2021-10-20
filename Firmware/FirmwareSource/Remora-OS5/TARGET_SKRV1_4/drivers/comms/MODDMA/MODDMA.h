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
    
    @file          MODDMA.h 
    @purpose       Adds DMA controller and multiple transfer configurations
    @version       see ChangeLog.c
    @date          Nov 2010
    @author        Andy Kirkham    
*/

#ifndef MODDMA_H
#define MODDMA_H

/** @defgroup API The MODDMA API */
/** @defgroup MISC Misc MODSERIAL functions */
/** @defgroup INTERNALS MODSERIAL Internals */

#include "mbed.h"
#include "iomacros.h"

namespace AjK {

/**
 * @brief The MODDMA configuration system
 * @author Andy Kirkham
 * @see http://mbed.org/cookbook/MODDMA_Config
 * @see MODDMA
 * @see API 
 *
 * <b>MODDMA_Config</b> defines a configuration that can be passed to the MODDMA controller
 * instance to perform a GPDMA data transfer.
 */
class  MODDMA_Config {
protected:

    // *****************************************
    // From GPDMA by NXP MCU SW Application Team
    // *****************************************
    
    uint32_t ChannelNum;        //!< DMA channel number, should be in range from 0 to 7. 
    uint32_t TransferSize;      //!< Length/Size of transfer 
    uint32_t TransferWidth;     //!< Transfer width - used for TransferType is GPDMA_TRANSFERTYPE_m2m only 
    uint32_t SrcMemAddr;        //!< Physical Src Addr, used in case TransferType is chosen as MODDMA::GPDMA_TRANSFERTYPE::m2m or MODDMA::GPDMA_TRANSFERTYPE::m2p 
    uint32_t DstMemAddr;        //!< Physical Destination Address, used in case TransferType is chosen as MODDMA::GPDMA_TRANSFERTYPE::m2m or MODDMA::GPDMA_TRANSFERTYPE::p2m 
    uint32_t TransferType;      //!< Transfer Type
    uint32_t SrcConn;           //!< Peripheral Source Connection type, used in case TransferType is chosen as
    uint32_t DstConn;           //!< Peripheral Destination Connection type, used in case TransferType is chosen as
    uint32_t DMALLI;            //!< Linker List Item structure data address if there's no Linker List, set as '0'
    uint32_t DMACSync;          //!< DMACSync if required.
    
    // Mbed specifics.

public: 
   
    MODDMA_Config() {
        isrIntTCStat  = new FunctionPointer;
        isrIntErrStat = new FunctionPointer;
        ChannelNum    = 0xFFFF;
        TransferSize  = 0;
        TransferWidth = 0;
        SrcMemAddr    = 0;
        DstMemAddr    = 0;
        TransferType  = 0;
        SrcConn       = 0;
        DstConn       = 0;
        DMALLI        = 0;
        DMACSync      = 0;
    }
    
    ~MODDMA_Config() {
        delete(isrIntTCStat);
        delete(isrIntErrStat);
    }
        
    class MODDMA_Config * channelNum(uint32_t n)    { ChannelNum = n & 0x7;  return this; }
    class MODDMA_Config * transferSize(uint32_t n)  { TransferSize = n;      return this; }
    class MODDMA_Config * transferWidth(uint32_t n) { TransferWidth = n;     return this; }
    class MODDMA_Config * srcMemAddr(uint32_t n)    { SrcMemAddr = n;        return this; }
    class MODDMA_Config * dstMemAddr(uint32_t n)    { DstMemAddr = n;        return this; }
    class MODDMA_Config * transferType(uint32_t n)  { TransferType = n;      return this; }
    class MODDMA_Config * srcConn(uint32_t n)       { SrcConn = n;           return this; }
    class MODDMA_Config * dstConn(uint32_t n)       { DstConn = n;           return this; }
    class MODDMA_Config * dmaLLI(uint32_t n)        { DMALLI = n;            return this; }
    class MODDMA_Config * dmacSync(uint32_t n)      { DMACSync = n;          return this; }
    
    uint32_t channelNum(void)    { return ChannelNum;    }
    uint32_t transferSize(void)  { return TransferSize;  }
    uint32_t transferWidth(void) { return TransferWidth; }
    uint32_t srcMemAddr(void)    { return SrcMemAddr;    }
    uint32_t dstMemAddr(void)    { return DstMemAddr;    }
    uint32_t transferType(void)  { return TransferType;  }
    uint32_t srcConn(void)       { return SrcConn;       }
    uint32_t dstConn(void)       { return DstConn;       }
    uint32_t dmaLLI(void)        { return DMALLI;        }
    uint32_t dmacSync(void)      { return DMACSync; }
    
    /**
     * Attach a callback to the TC IRQ configuration.
     *
     * @param fptr A function pointer to call
     * @return this
     */
    class MODDMA_Config * attach_tc(void (*fptr)(void)) {  
        isrIntTCStat->attach(fptr); 
        return this;
    }
    
    /**
     * Attach a callback to the ERR IRQ configuration.
     *
     * @param fptr A function pointer to call
     * @return this
     */
    class MODDMA_Config * attach_err(void (*fptr)(void)) {  
        isrIntErrStat->attach(fptr);         
        return this;
    }
    
    /**
     * Attach a callback to the TC IRQ configuration.
     *
     * @param tptr A template pointer to the calling object
     * @param mptr A method pointer within the object to call.
     * @return this
     */
    template<typename T>
    class MODDMA_Config * attach_tc(T* tptr, void (T::*mptr)(void)) {  
        if((mptr != NULL) && (tptr != NULL)) {
            isrIntTCStat->attach(tptr, mptr);
        }
        return this;
    }
    
    /**
     * Attach a callback to the ERR IRQ configuration.
     *
     * @param tptr A template pointer to the calling object
     * @param mptr A method pointer within the object to call.
     * @return this
     */
    template<typename T>
    class MODDMA_Config * attach_err(T* tptr, void (T::*mptr)(void)) {  
        if((mptr != NULL) && (tptr != NULL)) {
            isrIntErrStat->attach(tptr, mptr);
        }
        return this;
    }
    FunctionPointer *isrIntTCStat;                        
    FunctionPointer *isrIntErrStat;                        
};

/**
 * @brief The MODDMA configuration system (linked list items)
 * @author Andy Kirkham
 * @see http://mbed.org/cookbook/MODDMA_Config
 * @see MODDMA
 * @see MODDMA_Config
 * @see API 
 */
class MODDMA_LLI {
public:
    class MODDMA_LLI *srcAddr(uint32_t n) { SrcAddr = n; return this; }
    class MODDMA_LLI *dstAddr(uint32_t n) { DstAddr = n; return this; }
    class MODDMA_LLI *nextLLI(uint32_t n) { NextLLI = n; return this; }
    class MODDMA_LLI *control(uint32_t n) { Control = n; return this; }
    uint32_t srcAddr(void) { return SrcAddr; }
    uint32_t dstAddr(void) { return DstAddr; }
    uint32_t nextLLI(void) { return NextLLI; }
    uint32_t control(void) { return Control; }

    uint32_t SrcAddr;    //!< Source Address 
    uint32_t DstAddr;    //!< Destination address 
    uint32_t NextLLI;    //!< Next LLI address, otherwise set to '0' 
    uint32_t Control;    //!< GPDMA Control of this LLI 
};



 /**
 * @brief MODDMA GPDMA Controller
 * @author Andy Kirkham
 * @see http://mbed.org/cookbook/MODDMA
 * @see example1.cpp
 * @see API 
 *
 * <b>MODDMA</b> defines a GPDMA controller and multiple DMA configurations that allow for DMA
 * transfers from memory to memory, memory to peripheral or peripheral to memory.
 *
 * At the heart of the library is the MODDMA class that defines a single instance controller that
 * manages all the GPDMA hardware registers and interrupts. The controller can accept multiple
 * configurations that define the channel transfers. Each configuration specifies the source and 
 * destination information and other associated parts to maintain the transfer process.
 *
 * Standard example:
 * @code
 * #include "mbed.h"
 * #include "MODDMA.h"
 *
 * DigitalOut led1(LED1);
 * Serial pc(USBTX, USBRX); // tx, rx
 * MODDMA dma;
 *
 * int main() {
 *
 *     // Create a string buffer to send directly to a Uart/Serial
 *     char s[] = "***DMA*** ABCDEFGHIJKLMNOPQRSTUVWXYZ ***DMA***";
 *
 *     // Create a transfer configuarion
 *     MODDMA_Config *config = new MODDMA_Config;
 *
 *     // Provide a "minimal" setup for demo purposes.
 *     config
 *      ->channelNum    ( MODDMA::Channel_0 )   // The DMA channel to use.
 *      ->srcMemAddr    ( (uint32_t) &s )       // A pointer to the buffer to send.
 *      ->transferSize  ( sizeof(s) )           // The size of that buffer.
 *      ->transferType  ( MODDMA::m2p )         // Source is memory, destination is peripheral
 *      ->dstConn       ( MODDMA::UART0_Tx )    // Specifically, peripheral is Uart0 TX (USBTX, USBRX)
 *    ; // config end.
 *
 *    // Pass the configuration to the MODDMA controller.
 *    dma.Setup( config );
 *
 *    // Enable the channel and begin transfer.
 *    dma.Enable( config->channelNum() );
 *
 *    while(1) {
 *         led1 = !led1;
 *         wait(0.25);
 *     }
 * }
 * @endcode
 */
class MODDMA
{
public:

    //! Channel definitions.
    enum CHANNELS {
          Channel_0 = 0     /*!< Channel 0 */ 
        , Channel_1         /*!< Channel 1 */ 
        , Channel_2         /*!< Channel 2 */ 
        , Channel_3         /*!< Channel 3 */ 
        , Channel_4         /*!< Channel 4 */ 
        , Channel_5         /*!< Channel 5 */ 
        , Channel_6         /*!< Channel 6 */ 
        , Channel_7         /*!< Channel 7 */ 
    };
    
    //! Interrupt callback types.
    enum IrqType_t {
          TcIrq = 0     /*!< Terminal Count interrupt */
        , ErrIrq        /*!< Error interrupt */
    };
    
    //! Return status codes.
    enum Status {
          Ok            = 0     /*!< Ok, suceeded */
        , Error         = -1    /*!< General error */
        , ErrChInUse    = -2    /*!< Specific error, channel in use */
    };
    
    //! DMA Connection number definitions 
    enum GPDMA_CONNECTION {
          SSP0_Tx       = 0UL   /*!< SSP0 Tx */
        , SSP0_Rx       = 1UL   /*!< SSP0 Rx */
        , SSP1_Tx       = 2UL   /*!< SSP1 Tx */
        , SSP1_Rx       = 3UL   /*!< SSP1 Rx */
        , ADC           = 4UL   /*!< ADC */
        , I2S_Channel_0 = 5UL   /*!< I2S channel 0 */
        , I2S_Channel_1 = 6UL   /*!< I2S channel 1 */
        , DAC           = 7UL   /*!< DAC */
        , UART0_Tx      = 8UL   /*!< UART0 Tx */
        , UART0_Rx      = 9UL   /*!< UART0 Rx */
        , UART1_Tx      = 10UL  /*!< UART1 Tx */
        , UART1_Rx      = 11UL  /*!< UART1 Rx */
        , UART2_Tx      = 12UL  /*!< UART2 Tx */
        , UART2_Rx      = 13UL  /*!< UART2 Rx */
        , UART3_Tx      = 14UL  /*!< UART3 Tx */
        , UART3_Rx      = 15UL  /*!< UART3 Rx */
        , MAT0_0        = 16UL  /*!< MAT0.0 */
        , MAT0_1        = 17UL  /*!< MAT0.1 */
        , MAT1_0        = 18UL  /*!< MAT1.0 */
        , MAT1_1        = 19UL  /*!< MAT1.1 */
        , MAT2_0        = 20UL  /**< MAT2.0 */
        , MAT2_1        = 21UL  /*!< MAT2.1 */
        , MAT3_0        = 22UL  /*!< MAT3.0 */
        , MAT3_1        = 23UL  /*!< MAT3.1 */
    };

    //! GPDMA Transfer type definitions 
    enum  GPDMA_TRANSFERTYPE {
          m2m = 0UL     /*!< Memory to memory - DMA control */
        , m2p = 1UL     /*!< Memory to peripheral - DMA control */
        , p2m = 2UL     /*!< Peripheral to memory - DMA control */
        , p2p = 3UL     /*!< Src peripheral to dest peripheral - DMA control */         
        , g2m = 4UL     /*!< Psuedo special case for reading "peripheral GPIO" that's memory mapped. */
        , m2g = 5UL     /*!< Psuedo Special case for writing "peripheral GPIO" that's memory mapped. */        
    };   

    //! Burst size in Source and Destination definitions */
    enum GPDMA_BSIZE {
          _1    = 0UL   /*!< Burst size = 1 */
        , _4    = 1UL   /*!< Burst size = 4 */
        , _8    = 2UL   /*!< Burst size = 8 */
        , _16   = 3UL   /*!< Burst size = 16 */
        , _32   = 4UL   /*!< Burst size = 32 */
        , _64   = 5UL   /*!< Burst size = 64 */
        , _128  = 6UL   /*!< Burst size = 128 */
        , _256  = 7UL   /*!< Burst size = 256 */
    };
    
    //! Width in Src transfer width and Dest transfer width definitions */
    enum GPDMA_WIDTH {
          byte     = 0UL    /*!< Width = 1 byte */
        , halfword = 1UL    /*!< Width = 2 bytes */
        , word     = 2UL    /*!< Width = 4 bytes */
    };
    
    //! DMA Request Select Mode definitions. */
    enum GPDMA_REQSEL {
          uart  = 0UL   /*!< UART TX/RX is selected */
        , timer = 1UL   /*!< Timer match is selected */
    };

    //! GPDMA Control register bits.
    enum Config {
          _E = 1        /*!< DMA Controller enable */
        , _M = 2        /*!< AHB Master endianness configuration */
    };    

    //! GPDMA Channel config register bits.
    enum CConfig {
          _CE  = (1UL << 0)     /*!< Channel enable */
        , _IE  = (1UL << 14)    /*!< Interrupt error mask */
        , _ITC = (1UL << 15)    /*!< Terminal count interrupt mask */
        , _L   = (1UL << 16)    /*!< Lock */
        , _A   = (1UL << 17)    /*!< Active */
        , _H   = (1UL << 18)    /*!< Halt */
    };
    
    /**
     * The MODDMA constructor is used to initialise the DMA controller object.
     */    
    MODDMA() { init(true); }
    
    /**
     * The MODDMA destructor.
     */    
    ~MODDMA() {}
    
    /**
     * Used to setup the DMA controller to prepare for a data transfer.
     *
     * @ingroup API
     * @param isConstructorCalling Set true when called from teh constructor
     * @param 
     */
    void init(bool isConstructorCalling, int Channels = 0xFF, int Tc = 0xFF, int Err = 0xFF);
    
    /**
     * Used to setup and enable the DMA controller.
     *
     * @see Setup
     * @see Enable
     * @ingroup API
     * @param c A pointer to an instance of MODDMA_Config to setup.
     */
    uint32_t Prepare(MODDMA_Config *c) {
        uint32_t u = Setup(c);
        if (u) Enable(c);
        return u;
    }
    
    /**
     * Used to setup the DMA controller to prepare for a data transfer.
     *
     * @ingroup API
     * @param c A pointer to an instance of MODDMA_Config to setup.
     */
    uint32_t Setup(MODDMA_Config *c);
    
    /**
     * Enable and begin data transfer.
     *
     * @ingroup API
     * @param ChannelNumber Type CHANNELS, the channel number to enable
     */
    void Enable(CHANNELS ChannelNumber);
    
    /**
     * Enable and begin data transfer (overloaded function)
     *
     * @ingroup API
     * @param ChannelNumber Type uin32_t, the channel number to enable
     */
    void Enable(uint32_t ChannelNumber) { Enable((CHANNELS)(ChannelNumber & 0x7)); }
    
    /**
     * Enable and begin data transfer (overloaded function)
     *
     * @ingroup API
     * @param config A pointer to teh configuration
     */
    void Enable(MODDMA_Config *config) { Enable( config->channelNum() ); }
        
    
    /**
     * Disable a channel and end data transfer.
     *
     * @ingroup API
     * @param ChannelNumber Type CHANNELS, the channel number to enable
     */
    void Disable(CHANNELS ChannelNumber);
    
    /**
     * Disable a channel and end data transfer (overloaded function)
     *
     * @ingroup API
     * @param ChannelNumber Type uin32_t, the channel number to disable
     */
    void Disable(uint32_t ChannelNumber) { Disable((CHANNELS)(ChannelNumber & 0x7)); }
    
    /**
     * Is the specified channel enabled?
     *
     * @ingroup API
     * @param ChannelNumber Type CHANNELS, the channel number to test
     * @return bool true if enabled, false otherwise.
     */
    bool Enabled(CHANNELS ChannelNumber);
    
    /**
     * Is the specified channel enabled? (overloaded function)
     *
     * @ingroup API
     * @param ChannelNumber Type uin32_t, the channel number to test
     * @return bool true if enabled, false otherwise.
     */
    bool Enabled(uint32_t ChannelNumber) { return Enabled((CHANNELS)(ChannelNumber & 0x7)); }
    
    __INLINE uint32_t IntStat(uint32_t n)            { return (1UL << n) & 0xFF; }
    __INLINE uint32_t IntTCStat_Ch(uint32_t n)       { return (1UL << n) & 0xFF; }
    __INLINE uint32_t IntTCClear_Ch(uint32_t n)      { return (1UL << n) & 0xFF; }
    __INLINE uint32_t IntErrStat_Ch(uint32_t n)      { return (1UL << n) & 0xFF; }
    __INLINE uint32_t IntErrClr_Ch(uint32_t n)       { return (1UL << n) & 0xFF; }
    __INLINE uint32_t RawIntErrStat_Ch(uint32_t n)   { return (1UL << n) & 0xFF; }
    __INLINE uint32_t EnbldChns_Ch(uint32_t n)       { return (1UL << n) & 0xFF; }
    __INLINE uint32_t SoftBReq_Src(uint32_t n)       { return (1UL << n) & 0xFFFF; }
    __INLINE uint32_t SoftSReq_Src(uint32_t n)       { return (1UL << n) & 0xFFFF; }
    __INLINE uint32_t SoftLBReq_Src(uint32_t n)      { return (1UL << n) & 0xFFFF; }
    __INLINE uint32_t SoftLSReq_Src(uint32_t n)      { return (1UL << n) & 0xFFFF; }
    __INLINE uint32_t Sync_Src(uint32_t n)           { return (1UL << n) & 0xFFFF; }
    __INLINE uint32_t ReqSel_Input(uint32_t n)       { return (1UL << (n - 8)) & 0xFF; }
    

    __INLINE uint32_t CxControl_TransferSize(uint32_t n)     { return (n & 0xFFF) << 0; }
    __INLINE uint32_t CxControl_SBSize(uint32_t n)           { return (n & 0x7) << 12; }
    __INLINE uint32_t CxControl_DBSize(uint32_t n)           { return (n & 0x7) << 15; }
    __INLINE uint32_t CxControl_SWidth(uint32_t n)           { return (n & 0x7) << 18; }
    __INLINE uint32_t CxControl_DWidth(uint32_t n)           { return (n & 0x7) << 21; }
    __INLINE uint32_t CxControl_SI()                         { return (1UL << 26); }
    __INLINE uint32_t CxControl_DI()                         { return (1UL << 27); }
    __INLINE uint32_t CxControl_Prot1()                      { return (1UL << 28); }
    __INLINE uint32_t CxControl_Prot2()                      { return (1UL << 29); }
    __INLINE uint32_t CxControl_Prot3()                      { return (1UL << 30); }
    __INLINE uint32_t CxControl_I()                          { return (1UL << 31); }
    __INLINE uint32_t CxControl_E()                          { return (1UL << 0); }
    __INLINE uint32_t CxConfig_SrcPeripheral(uint32_t n)     { return (n & 0x1F) << 1; }
    __INLINE uint32_t CxConfig_DestPeripheral(uint32_t n)    { return (n & 0x1F) << 6; }
    __INLINE uint32_t CxConfig_TransferType(uint32_t n)      { return (n & 0x7) << 11; }
    __INLINE uint32_t CxConfig_IE()                          { return (1UL << 14); }
    __INLINE uint32_t CxConfig_ITC()                         { return (1UL << 15); }
    __INLINE uint32_t CxConfig_L()                           { return (1UL << 16); }
    __INLINE uint32_t CxConfig_A()                           { return (1UL << 17); }
    __INLINE uint32_t CxConfig_H()                           { return (1UL << 18); }
    
    /**
     * A store for up to 8 (8 channels) of configurations.
     * @see MODDMA_Config
     */
    MODDMA_Config *setups[8];
    
    /**
     * Get a pointer to the current configuration the ISR is servicing.
     *
     * @ingroup API
     * @return MODDMA_Config * A pointer to the setup the ISR is currently servicing.
     */
    MODDMA_Config *getConfig(void) { return setups[IrqProcessingChannel]; }
    
    /**
     * Set which channel the ISR is currently servicing.
     *
     * *** USED INTERNALLY. DO NOT CALL FROM USER PROGRAMS ***
     *
     * Must be public so the extern "C" ISR can use it.
     */
    void setIrqProcessingChannel(CHANNELS n) { IrqProcessingChannel = n; }
    
    /**
     * Gets which channel the ISR is currently servicing.
     *
     * @ingroup API
     * @return CHANNELS The current channel the ISR is servicing.
     */
    CHANNELS irqProcessingChannel(void) { return IrqProcessingChannel; }
    
    /**
     * Sets which type of IRQ the ISR is making a callback for.
     *
     * *** USED INTERNALLY. DO NOT CALL FROM USER PROGRAMS ***
     *
     * Must be public so the extern "C" ISR can use it.
     */ 
    void setIrqType(IrqType_t n) { IrqType = n; }
    
    /**
     * Get which type of IRQ the ISR is calling you about,
     * terminal count or error.
     */
    IrqType_t irqType(void) { return IrqType; }
    
    /**
     * Clear the interrupt after handling.
     *
     * @param CHANNELS The channel the IQR occured on.
     */
    void clearTcIrq(CHANNELS n) { LPC_GPDMA->DMACIntTCClear = (uint32_t)(1UL << n); } 
    
    /**
     * Clear the interrupt the ISR is currently handing..
     */
    void clearTcIrq(void) { clearTcIrq( IrqProcessingChannel ); }
    
    /**
     * Clear the error interrupt after handling.
     *
     * @ingroup API
     * @param CHANNELS The channel the IQR occured on.
     */
    void clearErrIrq(CHANNELS n) { LPC_GPDMA->DMACIntTCClear = (uint32_t)(1UL << n); } 
    
    /**
     * Clear the error interrupt the ISR is currently handing.
     * @ingroup API
     */
    void clearErrIrq(void) { clearErrIrq( IrqProcessingChannel ); }
   
    /**
     * Is the supplied channel currently active?
     *
     * @ingroup API
     * @param CHANNELS The channel to inquire about.
     * @return bool true if active, false otherwise.
     */     
    bool isActive(CHANNELS ChannelNumber);
    
    /**
     * Halt the supplied channel. 
     *
     * @ingroup API
     * @param CHANNELS The channel to halt.
     */
    void haltChannel(CHANNELS ChannelNumber);
    
    /**
     * get a channels control register. 
     *
     * @ingroup API
     * @param CHANNELS The channel to get the control register for.
     */
    uint32_t getControl(CHANNELS ChannelNumber);
    
    /**
     * Wait for channel transfer to complete and then halt.
     *
     * @ingroup API
     * @param CHANNELS The channel to wait for then halt.
     */
    void haltAndWaitChannelComplete(CHANNELS n) { haltChannel(n); while (isActive(n)); }
    
    /**
     * Attach a callback to the TC IRQ controller.
     *
     * @ingroup API
     * @param fptr A function pointer to call
     * @return this
     */
    void attach_tc(void (*fptr)(void)) {  
        isrIntTCStat.attach(fptr);         
    }
    
    /**
     * Attach a callback to the TC IRQ controller.
     *
     * @ingroup API
     * @param tptr A template pointer to the calling object
     * @param mptr A method pointer within the object to call.
     * @return this
     */
    template<typename T>
    void attach_tc(T* tptr, void (T::*mptr)(void)) {  
        if((mptr != NULL) && (tptr != NULL)) {
            isrIntTCStat.attach(tptr, mptr);         
        }        
    }
       
    /**
     * The MODDMA controllers terminal count interrupt callback.
     */
    FunctionPointer isrIntTCStat;                        
    
    /**
     * Attach a callback to the ERR IRQ controller.
     *
     * @ingroup API
     * @param fptr A function pointer to call
     * @return this
     */
    void attach_err(void (*fptr)(void)) {  
        isrIntErrStat.attach(fptr);                 
    }
    
    /**
     * Attach a callback to the ERR IRQ controller.
     *
     * @ingroup API
     * @param tptr A template pointer to the calling object
     * @param mptr A method pointer within the object to call.
     * @return this
     */
    template<typename T>
    void attach_err(T* tptr, void (T::*mptr)(void)) {  
        if((mptr != NULL) && (tptr != NULL)) {
            isrIntErrStat.attach(tptr, mptr);         
        }
    }
    
    /**
     * Get the Linked List index regsiter for the requested channel.
     *
     * @param channelNum The channel number.
     * @return uint32_t The value of the DMACCLLI register
     */
    uint32_t lli(CHANNELS ChannelNumber, MODDMA_LLI *set = 0) { 
        LPC_GPDMACH_TypeDef *pChannel = (LPC_GPDMACH_TypeDef *)Channel_p( ChannelNumber & 0x7 );
        if (set) pChannel->DMACCLLI = (uint32_t)set;
        return pChannel->DMACCLLI; 
    }
    
    /**
     * The MODDMA controllers error interrupt callback.
     */
    FunctionPointer isrIntErrStat;                        
    
    uint32_t Channel_p(int channel);
    
protected:
   
    // Data LUTs.
    uint32_t LUTPerAddr(int n);
    uint8_t  LUTPerBurst(int n);
    uint8_t  LUTPerWid(int n);    
    //uint32_t Channel_p(int channel);
    
    CHANNELS IrqProcessingChannel;
    
    IrqType_t IrqType;
};

}; // namespace AjK ends.

using namespace AjK;

#endif
