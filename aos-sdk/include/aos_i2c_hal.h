/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    aos_i2c_hal.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

#include "stm32wbxx_hal.h"
#include "stm32wbxx_hal_i2c.h"

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;

/* USER CODE BEGIN Private defines */
/*
 * This driver supports polling-, interrupt- and DMA-driven transactions. Pick your poison, according to your needs.
 *
 * XXX: Only the polling driver has currently been tested. Changing the mode probably requires regenerating CubeMX
 * I2C HAL initialization code with different options.
 *
 * TODO: Interrupt driven mode is pretty much complete, however the data read appears to be off by one (ie, the
 *       contents of the WHO_AM_I register can be seen at address 0x10 instead of the expected 0x0f. Possibly an
 *       issue in the HAL implementation or me not understanding how to use it. Documentation would be nice !!
 */
#define CONFIG_I2C_IO_POLLING 		0	// Least resource usage, keeps cpu busy (full power) until I/O is done.
#define CONFIG_I2C_IO_INTERRUPT 	1	// Needs a semaphore. Careful with low power modes.
#define CONFIG_I2C_IO_DMA 			2	// Needs a semaphore, may require proper buffer alignment. Careful with low power modes.

#if !defined(CONFIG_I2C_IO)	// If not specified in the build flags, use a sensible default.
#define CONFIG_I2C_IO (CONFIG_I2C_IO_INTERRUPT)
#endif
/* USER CODE END Private defines */

void MX_I2C1_Init(void);
void MX_I2C3_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

