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
///DMA_HandleTypeDef hdma_sdmmc_rx;
///DMA_HandleTypeDef hdma_sdmmc_tx;

// simple flags for DMA pending signaling
volatile uint8_t SD_DMA_ReadPendingState = SD_TRANSFER_OK;
volatile uint8_t SD_DMA_WritePendingState = SD_TRANSFER_OK;

/* DMA Handlers are global, there is only one SDIO interface */

/**
  * @brief  This function handles SD interrupt request.
  */
void SDMMC1_IRQHandler(void)
{
  HAL_SD_IRQHandler(&hsd);
}


/**
 *
 * @param hsd:  Handle for SD handle Structure definition
 */
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    ///IRQn_Type IRQn;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hsd->Instance == SDMMC1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_SDMMC1_CLK_ENABLE();
        ///__HAL_RCC_DMA2_CLK_ENABLE();

        /* Enable GPIOs clock */
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        
        /* D0(PC8), D1(PC9), D2(PC10), D3(PC11), CK(PC12), CMD(PD2) */
        /* Common GPIO configuration */
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        
        /* GPIOC configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* GPIOD configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* D0DIR(PC6), D123DIR(PC7) */
        GPIO_InitStruct.Alternate = GPIO_AF8_SDIO1;
        /* GPIOC configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* CKIN(PB8), CDIR(PB9) */
        GPIO_InitStruct.Alternate = GPIO_AF7_SDIO1;
        /* GPIOB configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        __HAL_RCC_SDMMC1_FORCE_RESET();
        __HAL_RCC_SDMMC1_RELEASE_RESET();

        /* NVIC configuration for SDIO interrupts */
        HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
    } 
}

/**
 *
 * @param hsd:  Handle for SD handle Structure definition
 */
void HAL_SD_MspDeInit(SD_HandleTypeDef *hsd)
{
    if (hsd->Instance == SDMMC1) {
            /* Enable GPIOs clock */
        __HAL_RCC_GPIOB_CLK_DISABLE();
        __HAL_RCC_GPIOC_CLK_DISABLE();
        __HAL_RCC_GPIOD_CLK_DISABLE();
        
        /* Disable SDMMC1 clock */
        __HAL_RCC_SDMMC1_CLK_DISABLE();
    }
}



/**
  * @brief  Enables the SD wide bus mode.
  * @param  hsd pointer to SD handle
  * @retval error state
  */
static uint32_t SD_WideBus_Enable(SD_HandleTypeDef *hsd)
{
    uint32_t errorstate = HAL_SD_ERROR_NONE;

    if ((SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
    {
        return HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
    }

    /* Send CMD55 APP_CMD with argument as card's RCA.*/
    errorstate = SDMMC_CmdAppCommand(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
    errorstate = SDMMC_CmdBusWidth(hsd->Instance, 2U);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    hsd->Init.BusWide = SDMMC_BUS_WIDE_4B;
    SDMMC_Init(hsd->Instance, hsd->Init);

    return HAL_SD_ERROR_NONE;
}

/**
 * @brief  Initializes the SD card device.
 * @retval SD status
 */
uint8_t SD_Init(void)
{
    uint8_t sd_state = MSD_OK;

    hsd.Instance = SDMMC1;
    HAL_SD_DeInit(&hsd);

    hsd.Init.ClockEdge           = SDMMC_CLOCK_EDGE_FALLING;
    hsd.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.BusWide             = SDMMC_BUS_WIDE_4B;
    hsd.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv            = 2;

    /* HAL SD initialization */
    sd_state = HAL_SD_Init(&hsd);
    /* Configure SD Bus width (4 bits mode selected) */
    if (sd_state == MSD_OK)
    {
        /* Enable wide operation */
        if (SD_WideBus_Enable(&hsd) != HAL_OK)
        {
            sd_state = MSD_ERROR;
        }
    }

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
    HAL_SD_MspDeInit(&hsd);

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
