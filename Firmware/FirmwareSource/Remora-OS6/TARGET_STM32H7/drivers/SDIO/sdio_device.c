/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "sdio_device.h"
#include "platform/mbed_error.h"

/* Extern variables ---------------------------------------------------------*/

SD_HandleTypeDef hsd;

// simple flags for DMA pending signaling
volatile uint8_t SD_DMA_ReadPendingState = SD_TRANSFER_OK;
volatile uint8_t SD_DMA_WritePendingState = SD_TRANSFER_OK;

static void _SDMMC1_IRQHandler(void)
{
    HAL_SD_IRQHandler(&hsd);
}


/**
 *
 * @param hsd:  Handle for SD handle Structure definition
 */
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    IRQn_Type IRQn;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    if (hsd->Instance == SDMMC1)
    {
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
        PeriphClkInitStruct.PLL2.PLL2M = 5;         // 25 MHz / 5 = 5 MHz
        PeriphClkInitStruct.PLL2.PLL2N = 160;       // 5 MHz * 160 = 800 MHz (140: 700 MHz, 120: 600 MHz)
        PeriphClkInitStruct.PLL2.PLL2P = 4;
        PeriphClkInitStruct.PLL2.PLL2Q = 8;
        PeriphClkInitStruct.PLL2.PLL2R = 4;        // 64: 12.5 MHz  32: 25 MHz 16: 50 MHz   8: 100 MHz  4: 200 MHz
        PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
        PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
        PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            error("SDMMC clock Init error at %d in %s", __LINE__, __FILE__);
        }

        /* Peripheral clock enable */
        __HAL_RCC_SDMMC1_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**SDMMC1 GPIO Configuration
        PC8     ------> SDMMC1_D0
        PC9     ------> SDMMC1_D1
        PC10     ------> SDMMC1_D2
        PC11     ------> SDMMC1_D3
        PC12     ------> SDMMC1_CK
        PD2     ------> SDMMC1_CMD
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        // SDMMC1 interrupt Init
        IRQn = SDMMC1_IRQn;
        HAL_NVIC_SetPriority(IRQn, 0, 0);   // 0: highest prio, 15 lowest
        NVIC_SetVector(IRQn, (uint32_t)&_SDMMC1_IRQHandler);
        HAL_NVIC_EnableIRQ(IRQn);
    }
    
}

/**
 *
 * @param hsd:  Handle for SD handle Structure definition
 */
void HAL_SD_MspDeInit(SD_HandleTypeDef *hsd)
{
    if (hsd->Instance == SDMMC1) {
        __HAL_RCC_SDMMC1_CLK_DISABLE();

        /**SDMMC1 GPIO Configuration
        PC8     ------> SDMMC1_D0
        PC9     ------> SDMMC1_D1
        PC10     ------> SDMMC1_D2
        PC11     ------> SDMMC1_D3
        PC12     ------> SDMMC1_CK
        PD2     ------> SDMMC1_CMD
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                            |GPIO_PIN_12);

        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
    }
}

/**
 * @brief  DeInitializes the SD MSP.
 * @param  hsd: SD handle
 * @param  Params : pointer on additional configuration parameters, can be NULL.
 */
void SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params)
{

#if 0
    if (hsd->Instance == SDMMC2) {
    }
#endif
}



/**
 * @brief  Initializes the SD card device.
 * @retval SD status
 */
uint8_t SD_Init(void)
{
    uint8_t sd_state = MSD_OK;

    hsd.Instance = SDMMC1;
    hsd.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_ENABLE;
    hsd.Init.BusWide = SDMMC_BUS_WIDE_4B;
    hsd.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
    hsd.Init.ClockDiv = 2;          // SDMMC kernel clock / (2 * 2) = 50 MHz

    /* HAL SD initialization */
    sd_state = HAL_SD_Init(&hsd);

    sd_state = HAL_SD_ConfigSpeedBusOperation(&hsd, SDMMC_SPEED_MODE_AUTO);

   
    return sd_state;
}

/**
 * @brief  DeInitializes the SD card device.
 * @retval SD status
 */
uint8_t SD_DeInit(void)
{
    uint8_t sd_state = MSD_OK;

    /* HAL SD deinitialization */
    if (HAL_SD_DeInit(&hsd) != HAL_OK)
    {
        sd_state = MSD_ERROR;
    }

    /* Msp SD deinitialization */
    SD_MspDeInit(&hsd, NULL);

    return sd_state;
}

/**
 * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  ReadAddr: Address from where data is to be read
 * @param  NumOfBlocks: Number of SD blocks to read
 * @param  Timeout: Timeout for read operation
 * @retval SD status
 */
uint8_t SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_ReadBlocks(&hsd, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK)
    {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
 * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  WriteAddr: Address from where data is to be written
 * @param  NumOfBlocks: Number of SD blocks to write
 * @param  Timeout: Timeout for write operation
 * @retval SD status
 */
uint8_t SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_WriteBlocks(&hsd, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK)
    {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
 * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  ReadAddr: Address from where data is to be read
 * @param  NumOfBlocks: Number of SD blocks to read
 * @retval SD status
 */
uint8_t SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
    uint8_t sd_state = MSD_OK;
    SD_DMA_ReadPendingState = SD_TRANSFER_BUSY;

    if ((uint32_t)pData & 3) {
        __BKPT(0);
    }

    /*
    the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
    adjust the address and the D-Cache size to invalidate accordingly.
    */
    uint32_t alignedAddr = (uint32_t)pData & ~0x1F;
    SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr, NumOfBlocks*BLOCKSIZE + ((uint32_t)pData - alignedAddr));
    
    /* Read block(s) in DMA transfer mode */
    if (HAL_SD_ReadBlocks_DMA(&hsd, (uint8_t *)pData, ReadAddr, NumOfBlocks) != HAL_OK)
    {
        sd_state = MSD_ERROR;
        SD_DMA_ReadPendingState = SD_TRANSFER_OK;
    }

    return sd_state;
}

/**
 * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  WriteAddr: Address from where data is to be written
 * @param  NumOfBlocks: Number of SD blocks to write
 * @retval SD status
 */
uint8_t SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{
    // Ensure the data is flushed to main memory
    SCB_CleanDCache_by_Addr(pData, NumOfBlocks * 512);      // Todo: use real blocksize

    uint8_t sd_state = MSD_OK;
    SD_DMA_WritePendingState = SD_TRANSFER_BUSY;

    /* Write block(s) in DMA transfer mode */
    if (HAL_SD_WriteBlocks_DMA(&hsd, (uint8_t *)pData, WriteAddr, NumOfBlocks) != HAL_OK)
    {
        sd_state = MSD_ERROR;
        SD_DMA_WritePendingState = SD_TRANSFER_OK;
    }

    return sd_state;
}

/**
 * @brief  Erases the specified memory area of the given SD card.
 * @param  StartAddr: Start byte address
 * @param  EndAddr: End byte address
 * @retval SD status
 */
uint8_t SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
{
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_Erase(&hsd, StartAddr, EndAddr) != HAL_OK)
    {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
 * @brief  Gets the current SD card data status.
 * @param  None
 * @retval Data transfer state.
 *          This value can be one of the following values:
 *            @arg  SD_TRANSFER_OK: No data transfer is acting
 *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
 */
uint8_t SD_GetCardState(void)
{
    return ((HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

/**
 * @brief  Get SD information about specific SD card.
 * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
 * @retval None
 */
void SD_GetCardInfo(SD_Cardinfo_t *CardInfo)
{
    /* Get SD card Information, copy structure for portability */
    HAL_SD_CardInfoTypeDef HAL_CardInfo;

    HAL_SD_GetCardInfo(&hsd, &HAL_CardInfo);

    if (CardInfo)
    {
        CardInfo->CardType = HAL_CardInfo.CardType;
        CardInfo->CardVersion = HAL_CardInfo.CardVersion;
        CardInfo->Class = HAL_CardInfo.Class;
        CardInfo->RelCardAdd = HAL_CardInfo.RelCardAdd;
        CardInfo->BlockNbr = HAL_CardInfo.BlockNbr;
        CardInfo->BlockSize = HAL_CardInfo.BlockSize;
        CardInfo->LogBlockNbr = HAL_CardInfo.LogBlockNbr;
        CardInfo->LogBlockSize = HAL_CardInfo.LogBlockSize;
    }
}

/**
 * @brief  Check if a DMA operation is pending
 * @retval DMA operation is pending
 *          This value can be one of the following values:
 *            @arg  SD_TRANSFER_OK: No data transfer is acting
 *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
 */
uint8_t SD_DMA_ReadPending(void)
{
    return SD_DMA_ReadPendingState;
}

/**
 * @brief  Check if a DMA operation is pending
 * @retval DMA operation is pending
 *          This value can be one of the following values:
 *            @arg  SD_TRANSFER_OK: No data transfer is acting
 *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
 */
uint8_t SD_DMA_WritePending(void)
{
    return SD_DMA_WritePendingState;
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd Pointer SD handle
  * @retval None
  */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    SD_DMA_ReadPendingState = SD_TRANSFER_OK;
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd Pointer to SD handle
  * @retval None
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    SD_DMA_WritePendingState = SD_TRANSFER_OK;
}

/**
  * @brief SD Abort callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
      //BSP_SD_AbortCallback();
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
    printf("SD_Error: 0x%x\n", hsd->ErrorCode);
}