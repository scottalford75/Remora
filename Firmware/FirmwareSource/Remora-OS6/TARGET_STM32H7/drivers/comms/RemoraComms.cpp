#include "mbed.h"
#include "RemoraComms.h"

#include "stm32h7xx_hal.h"

RemoraComms::RemoraComms(volatile rxData_t* ptrRxData, volatile txData_t* ptrTxData, SPI_TypeDef* spiType, PinName interruptPin) :
    ptrRxData(ptrRxData),
    ptrTxData(ptrTxData),
    spiType(spiType),
    interruptPin(interruptPin), 
    slaveSelect(interruptPin)
{
    this->spiHandle.Instance = this->spiType;

    if (this->interruptPin == PA_4)
    {
        // interrupt pin is the NSS pin
        sharedSPI = false;
        HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
    }

    //slaveSelect.rise(callback(this, &RemoraComms::processPacket));
    slaveSelect.rise(callback(this, &RemoraComms::NSSinterrupt));
}

void RemoraComms:: update()
{
	if (this->data)
	{
		this->noDataCount = 0;
		this->CommsStatus = true;
	}
	else
	{
		this->noDataCount++;
	}

	if (this->noDataCount > DATA_ERR_MAX)
	{
		this->noDataCount = 0;
		this->CommsStatus = false;
	}

	this->data = false;    
}


void RemoraComms::init()
{
    if(this->spiHandle.Instance == SPI1)
    {
        printf("Initialising SPI1 slave\n");

        GPIO_InitTypeDef GPIO_InitStruct;

        /**SPI1 GPIO Configuration
        PA4     ------> SPI1_NSS
        PA5     ------> SPI1_SCK
        PA6     ------> SPI1_MISO
        PA7     ------> SPI1_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        __HAL_RCC_SPI1_CLK_ENABLE();

        this->spiHandle.Init.Mode           = SPI_MODE_SLAVE;
        this->spiHandle.Init.Direction      = SPI_DIRECTION_2LINES;
        this->spiHandle.Init.DataSize       = SPI_DATASIZE_8BIT;
        this->spiHandle.Init.CLKPolarity    = SPI_POLARITY_LOW;
        this->spiHandle.Init.CLKPhase       = SPI_PHASE_1EDGE;
        if (sharedSPI)
        {
            this->spiHandle.Init.NSS            = SPI_NSS_SOFT;
            printf("SPI is shared with SD card\n");
        }
        else
        {
            this->spiHandle.Init.NSS            = SPI_NSS_HARD_INPUT;
        } 
        this->spiHandle.Init.FirstBit                   = SPI_FIRSTBIT_MSB;
        this->spiHandle.Init.TIMode                     = SPI_TIMODE_DISABLE;
        this->spiHandle.Init.CRCCalculation             = SPI_CRCCALCULATION_DISABLE;
        this->spiHandle.Init.CRCPolynomial              = 0x0;
        this->spiHandle.Init.NSSPMode                   = SPI_NSS_PULSE_DISABLE;
        this->spiHandle.Init.NSSPolarity                = SPI_NSS_POLARITY_LOW;
        this->spiHandle.Init.FifoThreshold              = SPI_FIFO_THRESHOLD_01DATA;
        this->spiHandle.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
        this->spiHandle.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
        this->spiHandle.Init.MasterSSIdleness           = SPI_MASTER_SS_IDLENESS_00CYCLE;
        this->spiHandle.Init.MasterInterDataIdleness    = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
        this->spiHandle.Init.MasterReceiverAutoSusp     = SPI_MASTER_RX_AUTOSUSP_DISABLE;
        this->spiHandle.Init.MasterKeepIOState          = SPI_MASTER_KEEP_IO_STATE_DISABLE;
        this->spiHandle.Init.IOSwap                     = SPI_IO_SWAP_DISABLE;

        HAL_SPI_Init(&this->spiHandle);

        if (sharedSPI)
        {
            // set SSI (Slave Select Internal) low, ie same as NSS going low
             CLEAR_BIT(this->spiHandle.Instance->CR1, SPI_CR1_SSI);
        }

        printf("Initialising DMA for SPI\n");

        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();

        this->hdma_spi_tx.Instance                   = DMA1_Stream1;
        this->hdma_spi_tx.Init.Request               = DMA_REQUEST_SPI1_TX;
        this->hdma_spi_tx.Init.Direction             = DMA_MEMORY_TO_PERIPH;
        this->hdma_spi_tx.Init.PeriphInc             = DMA_PINC_DISABLE;
        this->hdma_spi_tx.Init.MemInc                = DMA_MINC_ENABLE;
        this->hdma_spi_tx.Init.PeriphDataAlignment   = DMA_PDATAALIGN_BYTE;
        this->hdma_spi_tx.Init.MemDataAlignment      = DMA_MDATAALIGN_BYTE;
        this->hdma_spi_tx.Init.Mode                  = DMA_CIRCULAR;
        //this->hdma_spi_tx.Init.Mode                  = DMA_NORMAL;
        this->hdma_spi_tx.Init.Priority              = DMA_PRIORITY_VERY_HIGH;
        this->hdma_spi_tx.Init.FIFOMode              = DMA_FIFOMODE_DISABLE;
        
        HAL_DMA_Init(&this->hdma_spi_tx);

        __HAL_LINKDMA(&this->spiHandle, hdmatx, this->hdma_spi_tx);

        this->hdma_spi_rx.Instance                   = DMA1_Stream2;
        this->hdma_spi_rx.Init.Request               = DMA_REQUEST_SPI1_RX;
        this->hdma_spi_rx.Init.Direction             = DMA_PERIPH_TO_MEMORY;
        this->hdma_spi_rx.Init.PeriphInc             = DMA_PINC_DISABLE;
        this->hdma_spi_rx.Init.MemInc                = DMA_MINC_ENABLE;
        this->hdma_spi_rx.Init.PeriphDataAlignment   = DMA_PDATAALIGN_BYTE;
        this->hdma_spi_rx.Init.MemDataAlignment      = DMA_MDATAALIGN_BYTE;
        this->hdma_spi_rx.Init.Mode                  = DMA_CIRCULAR;
        //this->hdma_spi_rx.Init.Mode                  = DMA_NORMAL;
        this->hdma_spi_rx.Init.Priority              = DMA_PRIORITY_VERY_HIGH;
        this->hdma_spi_rx.Init.FIFOMode              = DMA_FIFOMODE_DISABLE;

        HAL_DMA_Init(&this->hdma_spi_rx);

        __HAL_LINKDMA(&this->spiHandle,hdmarx,this->hdma_spi_rx);
    }
}

void RemoraComms::start()
{
    this->ptrTxData->header = PRU_DATA;
    SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)this->ptrTxData->txBuffer) & ~(uint32_t)0x1F), SPI_BUFF_SIZE+32);
    SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)this->spiRxBuffer.rxBuffer) & ~(uint32_t)0x1F), SPI_BUFF_SIZE+32);
    HAL_SPI_TransmitReceive_DMA(&this->spiHandle, (uint8_t *)this->ptrTxData->txBuffer, (uint8_t *)this->spiRxBuffer.rxBuffer, SPI_BUFF_SIZE);
}

void RemoraComms::NSSinterrupt()
{
    // NSS / CS has gone high, packet recieved
    this->NSS = true;
}

void RemoraComms::SPItasks()
{
    if (this->NSS)
    {
        this->NSS = false;

        this->DMArxCnt = 0;
        this->ticksStart = HAL_GetTick();

        // wait for DMA to complete and break if DMA is not complete in time
        while (this->DMArxCnt != SPI_BUFF_SIZE)
        {
            this->DMArxCnt = __HAL_DMA_GET_COUNTER(&this->hdma_spi_rx);
            this->ticks = HAL_GetTick() - this->ticksStart;

            if (this->ticks > 2)
            {
                this->resetSPI = true;
                break;
            }
        }

        if (this->resetSPI)
        {
            // for testing, not needed / implemented
            printf("  Reset SPI now\n");
            this->resetSPI = false;
        }

        switch (this->spiRxBuffer.header)
        {
            case PRU_READ:
                this->data = true;
                this->rejectCnt = 0;
                ++this->dataCnt;
                // READ so do nothing with the received data
                break;

            case PRU_WRITE:
                this->data = true;
                this->rejectCnt = 0;
                ++this->dataCnt;
                // we've got a good WRITE header, move the data to rxData

                // ensure an atomic access to the rxBuffer
                // disable thread interrupts
                __disable_irq(); 

                for (int i = 0; i < SPI_BUFF_SIZE; i++)
                {
                    this->ptrRxData->rxBuffer[i] = this->spiRxBuffer.rxBuffer[i];
                }

                // re-enable thread interrupts
                __enable_irq();
                
                break;

            default:
                this->rejectCnt++;
                if (this->rejectCnt > 5)
                {
                    this->SPIdataError = true;
                }
        }
    }
}


bool RemoraComms::getStatus(void)
{
    return this->CommsStatus;
}

void RemoraComms::setStatus(bool status)
{
    this->CommsStatus = status;
}

bool RemoraComms::getError(void)
{
    return this->SPIdataError;
}

void RemoraComms::setError(bool error)
{
    this->SPIdataError = error;
}