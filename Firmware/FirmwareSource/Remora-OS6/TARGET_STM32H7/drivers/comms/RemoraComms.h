#ifndef REMORASPI_H
#define REMORASPI_H

#include "mbed.h"
#include "configuration.h"
#include "remora.h"

#include "stm32h7xx_hal.h"



class RemoraComms
{
    private:

        SPI_TypeDef*        spiType;
        SPI_HandleTypeDef   spiHandle;
        DMA_HandleTypeDef   hdma_spi_tx;
        DMA_HandleTypeDef   hdma_spi_rx;
        DMA_HandleTypeDef   hdma_memtomem_dma2_stream1;
        HAL_StatusTypeDef   status;

        volatile rxData_t*  ptrRxData;
        volatile txData_t*  ptrTxData;
        rxData_t            spiRxBuffer;
        uint8_t             rejectCnt;
        bool                SPIdata;
        bool                SPIdataError;
        
        PinName             interruptPin;
        InterruptIn         slaveSelect;
        bool                sharedSPI;
        
        void processPacket(void);

    public:

        RemoraComms(volatile rxData_t*, volatile txData_t*, SPI_TypeDef*, PinName);
        void init(void);
        void start(void);
        bool getStatus(void);
        void setStatus(bool);
        bool getError(void);
        void setError(bool);

};

#endif
