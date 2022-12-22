/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    aos_spi_hal.h
  * @brief   SPI driver
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

#include "stm32wbxx_hal.h"
#include "stm32wbxx_hal_spi.h"

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN Private defines */
/*
 * This driver supports polling-, interrupt- and DMA-driven transactions. Pick your poison, according to your needs.
 *
 * XXX: Only the polling driver has currently been tested, and even that is not fully conclusive until we have a
 *      SPI device driver that actually works (ie, lr1110). Changing the mode probably requires regenerating the
 *      CubeMX SPI HAL initialization code with different options.
 *
 */
#define CONFIG_SPI_IO_POLLING 		0	// Least resource usage, keeps cpu busy (full power) until I/O is done.
#define CONFIG_SPI_IO_INTERRUPT 	1	// Needs a semaphore. Careful with low power modes.
#define CONFIG_SPI_IO_DMA 			2	// Needs a semaphore, may require proper buffer alignment. Careful with low power modes.

#if !defined(CONFIG_SPI_IO)	// If not specified in the build flags, use a sensible default.
#define CONFIG_SPI_IO (CONFIG_SPI_IO_POLLING)
#endif

/* USER CODE END Private defines */

void MX_SPI1_Init(void);
void MX_SPI2_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

