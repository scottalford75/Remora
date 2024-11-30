#include "mbed.h"
#include "RemoraComms.h"

#include "stm32f1xx_hal.h"

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
    else if (this->interruptPin == PA_15)
    {
        // interrupt pin is not the NSS pin, ie the board shares the SPI bus with the SD card
        // configure the SPI in software NSS mode and always on
        sharedSPI = true;
        HAL_NVIC_SetPriority(EXTI15_10_IRQn , 5, 0);
    }
    else if (this->interruptPin == PC_1)
    {
        // interrupt pin is not the NSS pin, ie the board shares the SPI bus with the SD card
        // configure the SPI in software NSS mode and always on
        sharedSPI = true;
        HAL_NVIC_SetPriority(EXTI1_IRQn , 5, 0);
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

        GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
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
        this->spiHandle.Init.BaudRatePrescaler        = SPI_BAUDRATEPRESCALER_2;
        this->spiHandle.Init.FirstBit       = SPI_FIRSTBIT_MSB;
        this->spiHandle.Init.TIMode         = SPI_TIMODE_DISABLE;
        this->spiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
        this->spiHandle.Init.CRCPolynomial  = 10;

        HAL_SPI_Init(&this->spiHandle);

        if (sharedSPI)
        {
            // set SSI (Slave Select Internal) low, ie same as NSS going low
             CLEAR_BIT(this->spiHandle.Instance->CR1, SPI_CR1_SSI);
        }
 
        printf("Initialising DMA for SPI\n");

        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        this->hdma_spi_tx.Instance                   = DMA1_Channel3;
        this->hdma_spi_tx.Init.Direction             = DMA_MEMORY_TO_PERIPH;
        this->hdma_spi_tx.Init.PeriphInc             = DMA_PINC_DISABLE;
        this->hdma_spi_tx.Init.MemInc                = DMA_MINC_ENABLE;
        this->hdma_spi_tx.Init.PeriphDataAlignment   = DMA_PDATAALIGN_BYTE;
        this->hdma_spi_tx.Init.MemDataAlignment      = DMA_MDATAALIGN_BYTE;
        this->hdma_spi_tx.Init.Mode                  = DMA_CIRCULAR;
        //this->hdma_spi_tx.Init.Mode                  = DMA_NORMAL;
        this->hdma_spi_tx.Init.Priority              = DMA_PRIORITY_VERY_HIGH;
        
         HAL_DMA_Init(&this->hdma_spi_tx);

        __HAL_LINKDMA(&this->spiHandle, hdmatx, this->hdma_spi_tx);

        //HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
        ///NVIC_SetVector(DMA1_Channel3_IRQn, (uint32_t)&DMA1_Channel3_IRQHandler);
        //HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

        this->hdma_spi_rx.Instance                   = DMA1_Channel2;
        this->hdma_spi_rx.Init.Direction             = DMA_PERIPH_TO_MEMORY;
        this->hdma_spi_rx.Init.PeriphInc             = DMA_PINC_DISABLE;
        this->hdma_spi_rx.Init.MemInc                = DMA_MINC_ENABLE;
        this->hdma_spi_rx.Init.PeriphDataAlignment   = DMA_PDATAALIGN_BYTE;
        this->hdma_spi_rx.Init.MemDataAlignment      = DMA_MDATAALIGN_BYTE;
        this->hdma_spi_rx.Init.Mode                  = DMA_CIRCULAR;
        //this->hdma_spi_rx.Init.Mode                  = DMA_NORMAL;
        this->hdma_spi_rx.Init.Priority              = DMA_PRIORITY_VERY_HIGH;

        HAL_DMA_Init(&this->hdma_spi_rx);

        __HAL_LINKDMA(&this->spiHandle,hdmarx,this->hdma_spi_rx);

        //HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
        //NVIC_SetVector(DMA1_Channel2_IRQn, (uint32_t)&DMA1_Channel2_IRQHandler);
        //HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
    }
}

void RemoraComms::start()
{
    this->ptrTxData->header = PRU_DATA;
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