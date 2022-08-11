/*!
 * \file aos_i2c.h
 *
 * \brief I2C driver
 *
 *  Created on: Jan 18, 2022
 *      Author: marc
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "aos_common.h"	// aos_result_t

/*!
 * \defgroup aos_i2c I2C driver
 *
 * \addtogroup aos_i2c
 * @{
 */

/*!
 * \enum aos_i2c_bus_id_t
 *
 * \brief I2C bus supported by the driver
 */
typedef enum {
	aos_i2c_bus_id_internal,	//!< Module internal I2C bus.
	aos_i2c_bus_id_external,	//!< Module external I2C bus.
	aos_i2c_bus_id_count		//!< Last item in the enumerated.
} aos_i2c_bus_id_t;

/*!
 * \enum aos_i2c_ioctl_req_t
 *
 * \brief Driver control request
 */
typedef enum {
	aos_i2c_ioctl_req_set_io_timeout,			//!< I/O Timeout, value = timeout (ms)
	aos_i2c_ioctl_req_set_16_bit_addresses,		//!< Use 16 bit addresses, value = boolean
	aos_i2c_ioctl_req_device_exists,			//!< Check if a device exists, value = device address
} aos_i2c_ioctl_req_t;

/*!
 * \struct aos_i2c_ioctl_t
 *
 * \brief Information passed to the IO control
 */
typedef struct {
	aos_i2c_ioctl_req_t request;		//!< Request type
	union {
		uint32_t value;					//!< Integer data. Meaning depends on the request.
	};
} aos_i2c_ioctl_t;

/*!
 * \typedef aos_i2c_handle_t
 *
 * \brief Define an opaque pointer to the driver access structure.
 */
typedef uint32_t aos_i2c_handle_t;

/*!
 * \def aos_i2c_handle_invalid
 *
 * \brief Invalid I2C handle
 */
#define aos_i2c_handle_invalid (0)

/*!
 * \typedef aos_i2c_devaddr_t
 *
 * \brief I2C device address
 */
typedef uint16_t aos_i2c_devaddr_t;

/*!
 * \typedef aos_i2c_regaddr_t
 *
 * \brief I2C device register address
 */
typedef uint16_t aos_i2c_regaddr_t;

/*!
 * \fn aos_i2c_open_func_t(aos_i2c_bus_id_t bid)
 *
 * \brief Open the I2C driver
 *
 * \param bid Bus identifier
 *
 * \return An I2C opaque handle that will be used by the other driver functions
 */
typedef aos_i2c_handle_t (*aos_i2c_open_func_t)(aos_i2c_bus_id_t bid);

/*!
 * \fn aos_i2c_close_func_t(aos_i2c_handle_t h)
 *
 * \brief Close the I2C driver
 *
 * \param h Handle returned by the open function
 *
 * \return The result of the operation
 */
typedef aos_result_t (*aos_i2c_close_func_t)(aos_i2c_handle_t h);

/*!
 * \fn aos_i2c_read_func_t(aos_i2c_handle_t h, aos_i2c_devaddr_t da, aos_i2c_regaddr_t ra, uint8_t *buffer, unsigned len)
 *
 * \brief Read device registers
 *
 * \param h Handle returned by the open function
 * \param da Device address
 * \param ra First register address to read
 * \param buffer Area where to store the read values
 * \param count Number of registers to read
 *
 * \return The result of the operation
 */
typedef aos_result_t (*aos_i2c_read_func_t)(aos_i2c_handle_t h, aos_i2c_devaddr_t da, aos_i2c_regaddr_t ra, uint8_t *buffer, unsigned len);

/*!
 * \fn aos_i2c_write_func_t(aos_i2c_handle_t h, aos_i2c_devaddr_t da, aos_i2c_regaddr_t ra, uint8_t *buffer, unsigned len);
 *
 * \brief Write device registers
 *
 * \param h Handle returned by the open function
 * \param da Device address
 * \param ra First register address to write
 * \param buffer Area where to read the values
 * \param count Number of registers to write
 *
 * \return The result of the operation
 */
typedef aos_result_t (*aos_i2c_write_func_t)(aos_i2c_handle_t h, aos_i2c_devaddr_t da, aos_i2c_regaddr_t ra, uint8_t *buffer, unsigned len);

/*!
 * \fn aos_i2c_ioctl_func_t(aos_i2c_handle_t h, aos_i2c_ioctl_t *info)
 *
 * \brief Send IO control request  to the driver
 *
 * \param h Handle returned by the open function
 * \param info request information block
 *
 * \return The result of the operation
 */
typedef aos_result_t (*aos_i2c_ioctl_func_t)(aos_i2c_handle_t h, aos_i2c_ioctl_t *info);

/*!
 * \struct aos_i2c_master_t
 *
 * \brief I2C master driver access
 */
typedef struct {
	aos_i2c_open_func_t open;		//!< Open the driver
	aos_i2c_close_func_t close;		//!< Close the driver
	aos_i2c_read_func_t read;		//!< Read I2C device registers
	aos_i2c_write_func_t write;		//!< Write I2C device registers
	aos_i2c_ioctl_func_t ioctl;		//!< Control the I2C driver
} aos_i2c_master_t;

/*!
 * \var aos_i2c_master
 *
 * \brief Export the I2C master driver
 */
const aos_i2c_master_t *aos_i2c_master(void);

/*! @}*/

#if defined(__cplusplus)
}
#endif
