/*!
 * \file aos_spi_hal.h
 *
 * \brief SPI HAL
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
/*!
 * \defgroup aos_spi SPI driver
 *
 * \addtogroup aos_spi
 * @{
 */

#include "stm32wbxx_hal.h"
#include "stm32wbxx_hal_spi.h"


/*!
 * \brief STM32 SPI definitions
 */
extern SPI_HandleTypeDef hspi1;			//!< SPI bus 1
extern SPI_HandleTypeDef hspi2;			//!< SPI bus 2

/*!
 * \brief Driver mode
 *
 * This driver supports polling-, interrupt- and DMA-driven transactions. Pick your poison, according to your needs.
 *
 * XXX: Only the polling driver has currently been tested, and even that is not fully conclusive until we have a
 *      SPI device driver that actually works (ie, lr1110). Changing the mode probably requires regenerating the
 *      CubeMX SPI HAL initialization code with different options.
 *
 */
#define CONFIG_SPI_IO_POLLING 		0	//!< Least resource usage, keeps cpu busy (full power) until I/O is done.
#define CONFIG_SPI_IO_INTERRUPT 	1	//!< Needs a semaphore. Careful with low power modes.
#define CONFIG_SPI_IO_DMA 			2	//!< Needs a semaphore, may require proper buffer alignment. Careful with low power modes.

#if !defined(CONFIG_SPI_IO)	//<! If not specified in the build flags, use a sensible default.
#define CONFIG_SPI_IO (CONFIG_SPI_IO_POLLING)		//!< Default: polling
#endif

/*!
 *\brief Initialize the SPI 1 hardware
 */
void MX_SPI1_Init(void);

/*!
 *\brief Initialize the SPI 2 hardware
 */
void MX_SPI2_Init(void);

/*! @}*/

#ifdef __cplusplus
}
#endif

