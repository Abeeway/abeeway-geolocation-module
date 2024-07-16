/*!
 * \file aos_i2c_hal.h
 *
 * \brief I2C HAL
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32wbxx_hal.h"
#include "stm32wbxx_hal_i2c.h"

/*!
 * \brief STM32 I2C definitions
 */
extern I2C_HandleTypeDef hi2c1;			//!< I2C bus 1
extern I2C_HandleTypeDef hi2c3;			//!< I2C bus 2

/*
 * \brief Driver mode
 *
 * This driver supports polling-, interrupt- and DMA-driven transactions. Pick your poison, according to your needs.
 *
 * XXX: Only the polling driver has currently been tested. Changing the mode probably requires regenerating CubeMX
 * I2C HAL initialization code with different options.
 *
 * TODO: Interrupt driven mode is pretty much complete, however the data read appears to be off by one (ie, the
 *       contents of the WHO_AM_I register can be seen at address 0x10 instead of the expected 0x0f. Possibly an
 *       issue in the HAL implementation or me not understanding how to use it. Documentation would be nice !!
 */
#define CONFIG_I2C_IO_POLLING 		0	//!< Least resource usage, keeps cpu busy (full power) until I/O is done.
#define CONFIG_I2C_IO_INTERRUPT 	1	//!< Needs a semaphore. Careful with low power modes.
#define CONFIG_I2C_IO_DMA 			2	//!< Needs a semaphore, may require proper buffer alignment. Careful with low power modes.

#if !defined(CONFIG_I2C_IO)	// If not specified in the build flags, use a sensible default.
#define CONFIG_I2C_IO (CONFIG_I2C_IO_INTERRUPT)	//!< By default use the interruptions
#endif

/*!
 *\brief Initialize the I2C 1 hardware
 */
void MX_I2C1_Init(void);

/*!
 *\brief Initialize the I2C 2 hardware
 */
void MX_I2C3_Init(void);


#ifdef __cplusplus
}
#endif

