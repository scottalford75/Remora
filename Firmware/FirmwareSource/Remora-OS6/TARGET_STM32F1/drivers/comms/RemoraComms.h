#ifndef REMORASPI_H
#define REMORASPI_H

#include "mbed.h"
#include "configuration.h"
#include "remora.h"

#include "stm32f1xx_hal.h"

#include "modules/module.h"

class RemoraComms : public Module
{
    private:

        SPI_TypeDef*        spiType;
        SPI_HandleTypeDef   spiHandle;
        DMA_HandleTypeDef   hdma_spi_tx;
        DMA_HandleTypeDef   hdma_spi_rx;
        DMA_HandleTypeDef   hdma_memtomem_dma2_stream1;
        HAL_StatusTypeDef   status;

        uint32_t            transferCompleteFlag;

        volatile rxData_t*  ptrRxData;
        volatile txData_t*  ptrTxData;
        rxData_t            spiRxBuffer;

        uint8_t		        noDataCount;
        uint8_t             rejectCnt;
        uint8_t             dataCnt;

        uint8_t             DMArxCnt;
        uint32_t            ticksStart;
        uint32_t            ticks;

        bool                NSS;
        bool                resetSPI;

        bool                data;
        bool                CommsStatus;

        bool                SPIdata;
        bool                SPIdataError;
        
        PinName             interruptPin;
        InterruptIn         slaveSelect;
        bool                sharedSPI;
        
        void NSSinterrupt(void);

    public:

        RemoraComms(volatile rxData_t*, volatile txData_t*, SPI_TypeDef*, PinName);

        virtual void update(void);

        void init(void);
        void start(void);
        void SPItasks(void);
        bool getStatus(void);
        void setStatus(bool);
        bool getError(void);
        void setError(bool);

};

#endif
