#include "mbed.h"
#include "RemoraComms.h"


RemoraComms::RemoraComms(volatile rxData_t* ptrRxData, volatile txData_t* ptrTxData) :
    ptrRxData(ptrRxData),
    ptrTxData(ptrTxData),
    spiSlave(MOSI0, MISO0, SCK0, SSEL0)
{
    spiSlave.frequency(48000000);
}

void RemoraComms::init()
{
    // Create MODDMA configuration objects for the SPI transfer and memory copy
    spiDMAmemcpy1 = new MODDMA_Config;
    spiDMAmemcpy2 = new MODDMA_Config;
    spiDMAtx1 = new MODDMA_Config;
    spiDMAtx2 = new MODDMA_Config;
    spiDMArx1 = new MODDMA_Config;
    spiDMArx2 = new MODDMA_Config;

   // Setup DMA configurations
    spiDMAtx1
        ->channelNum    ( MODDMA::Channel_0 )
        ->srcMemAddr    ( (uint32_t) ptrTxData )
        ->dstMemAddr    ( 0 )
        ->transferSize  ( SPI_BUFF_SIZE )
        ->transferType  ( MODDMA::m2p )
        ->srcConn       ( 0 )
        ->dstConn       ( MODDMA::SSP0_Tx )
        ->attach_tc     ( this, &RemoraComms::tc0_callback )
        ->attach_err    ( this, &RemoraComms::err_callback )
    ;

    spiDMAtx2
        ->channelNum    ( MODDMA::Channel_1 )
        ->srcMemAddr    ( (uint32_t) ptrTxData )
        ->dstMemAddr    ( 0 )
        ->transferSize  ( SPI_BUFF_SIZE )
        ->transferType  ( MODDMA::m2p )
        ->srcConn       ( 0 )
        ->dstConn       ( MODDMA::SSP0_Tx )
        ->attach_tc     ( this, &RemoraComms::tc1_callback )
        ->attach_err    ( this, &RemoraComms::err_callback )
    ;

    spiDMArx1
        ->channelNum    ( MODDMA::Channel_2 )
        ->srcMemAddr    ( 0 )
        ->dstMemAddr    ( (uint32_t) &spiRxBuffer1 )
        ->transferSize  ( SPI_BUFF_SIZE )
        ->transferType  ( MODDMA::p2m )
        ->srcConn       ( MODDMA::SSP0_Rx )
        ->dstConn       ( 0 )
        ->attach_tc     ( this, &RemoraComms::tc2_callback )
        ->attach_err    ( this, &RemoraComms::err_callback )
    ;

    spiDMArx2
        ->channelNum    ( MODDMA::Channel_3 )
        ->srcMemAddr    ( 0 )
        ->dstMemAddr    ( (uint32_t) &spiRxBuffer2 )
        ->transferSize  ( SPI_BUFF_SIZE )
        ->transferType  ( MODDMA::p2m )
        ->srcConn       ( MODDMA::SSP0_Rx )
        ->dstConn       ( 0 )
        ->attach_tc     ( this, &RemoraComms::tc3_callback )
        ->attach_err    ( this, &RemoraComms::err_callback )
    ;

    spiDMAmemcpy1
        ->channelNum    ( MODDMA::Channel_4 )
        ->srcMemAddr    ( (uint32_t) &spiRxBuffer1 )
        ->dstMemAddr    ( (uint32_t) &rxData )
        ->transferSize  ( SPI_BUFF_SIZE )
        ->transferType  ( MODDMA::m2m )
    ;

    spiDMAmemcpy2
        ->channelNum    ( MODDMA::Channel_5 )
        ->srcMemAddr    ( (uint32_t) &spiRxBuffer2 )
        ->dstMemAddr    ( (uint32_t) &rxData )
        ->transferSize  ( SPI_BUFF_SIZE )
        ->transferType  ( MODDMA::m2m )
    ;
}


void RemoraComms::start()
{
    this->ptrTxData->header = PRU_DATA;

    // Pass the configurations to the controller
    dma.Prepare( spiDMArx1 );
    dma.Prepare( spiDMAtx1 );

    // Enable SSP0 for DMA
    LPC_SSP0->DMACR = 0;
    LPC_SSP0->DMACR = (1<<1)|(1<<0); // TX,RX DMA Enable
}


void RemoraComms::tc0_callback()
{
    // SPI Tx
    MODDMA_Config *config = dma.getConfig();
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );

    // Clear DMA IRQ flags.
    if (dma.irqType() == MODDMA::TcIrq) dma.clearTcIrq();
    if (dma.irqType() == MODDMA::ErrIrq) dma.clearErrIrq();

    dma.Prepare( spiDMAtx2 );
}

void RemoraComms::tc1_callback()
{
    // SPI Tx
    MODDMA_Config *config = dma.getConfig();
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );

    // Clear DMA IRQ flags.
    if (dma.irqType() == MODDMA::TcIrq) dma.clearTcIrq();
    if (dma.irqType() == MODDMA::ErrIrq) dma.clearErrIrq();

    dma.Prepare( spiDMAtx1 );
}

void RemoraComms::tc2_callback()
{
    // SPI Rx
    MODDMA_Config *config = dma.getConfig();
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );

    SPIdata = false;
    SPIdataError = false;

    // Clear DMA IRQ flags.
    if (dma.irqType() == MODDMA::TcIrq) dma.clearTcIrq();
    if (dma.irqType() == MODDMA::ErrIrq) dma.clearErrIrq();

    // Check and move the recieved SPI data payload
    switch (spiRxBuffer1.header)
    {
      case PRU_READ:
        SPIdata = true;
        rejectCnt = 0;
        dma.Disable( spiDMAmemcpy2->channelNum()  );
        break;

      case PRU_WRITE:
        SPIdata = true;
        rejectCnt = 0;
        dma.Prepare( spiDMAmemcpy1 );
        break;

      default:
        rejectCnt++;
        if (rejectCnt > 5)
        {
            SPIdataError = true;
        }
        dma.Disable( spiDMAmemcpy2->channelNum()  );
    }

    // swap Rx buffers
    dma.Prepare( spiDMArx2 );
}

void RemoraComms::tc3_callback()
{
    // SPI Rx
    MODDMA_Config *config = dma.getConfig();
    dma.Disable( (MODDMA::CHANNELS)config->channelNum() );

    SPIdata = false;
    SPIdataError = false;

    // Clear DMA IRQ flags.
    if (dma.irqType() == MODDMA::TcIrq) dma.clearTcIrq();
    if (dma.irqType() == MODDMA::ErrIrq) dma.clearErrIrq();

    // Check and move the recieved SPI data payload
    switch (spiRxBuffer2.header)
    {
      case PRU_READ:
        SPIdata = true;
        rejectCnt = 0;
        dma.Disable( spiDMAmemcpy1->channelNum()  );
        break;

      case PRU_WRITE:
        SPIdata = true;
        rejectCnt = 0;
        dma.Prepare( spiDMAmemcpy2 );
        break;

      default:
        rejectCnt++;
        if (rejectCnt > 5)
        {
            SPIdataError = true;
        }
        dma.Disable( spiDMAmemcpy1->channelNum()  );
    }

    // swap Rx buffers
    dma.Prepare( spiDMArx1 );
}

void RemoraComms::err_callback()
{
    printf("err\r\n");
}


bool RemoraComms::getStatus(void)
{
    return this->SPIdata;
}

void RemoraComms::setStatus(bool status)
{
    this->SPIdata = status;
}

bool RemoraComms::getError(void)
{
    return this->SPIdataError;
}

void RemoraComms::setError(bool error)
{
    this->SPIdataError = error;
}