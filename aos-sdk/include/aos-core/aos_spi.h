/*
 * aos_spi.h
 *
 *  Created on: Jan 28, 2022
 *      Author: marc
 */
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
/*!
 * \defgroup aos_spi SPI driver
 *
 * \addtogroup aos_spi
 * @{
 */

/*!
 * \enum aos_spi_bus_id_t
 *
 * \brief SPI BUS identifier.
 *
 * \note Do not use the internal SPI bus. Reserved for AOS
 */
typedef enum {
	aos_spi_bus_id_internal,	//!< The module internal SPI bus.
	aos_spi_bus_id_external,	//!< The module external SPI bus.
	aos_spi_bus_id_count		//!< Number of SPI bus
} aos_spi_bus_id_t;

/*!
 * \enum aos_spi_status_t
 *
 * \brief Return status of API functions
 */
typedef enum {
	aos_spi_status_success = 0,		//!< SPI operation success
	aos_spi_status_error,			//!< SPI operation error.
	aos_spi_status_lock_fail,		//!< Fail to acquire the lock.
	aos_spi_status_not_implemented,	//!< Feature/command not implemented
} aos_spi_status_t;

/*!
 * \enum aos_spi_ioctl_req_t
 *
 * \brief IO controls supported but the driver
 */
typedef enum {
	aos_spi_ioctl_req_set_io_timeout,			//!< I/O Timeout, value = timeout (ms)
} aos_spi_ioctl_req_t;

/*!
 * \struct aos_spi_ioctl_t
 *
 * \brief IO control request and data
 */
typedef struct {
	aos_spi_ioctl_req_t request;			//!< IO control request type
	union {
		uint32_t value;						//!< Data belonging to the request.
	};
} aos_spi_ioctl_t;

/*!
 * \brief SPI handle used to access the driver
 */
typedef uint32_t aos_spi_handle_t;

/*!
 * \def aos_spi_handle_invalid
 *
 * \brief Invalid SPI handle
 */
#define aos_spi_handle_invalid (0)

/*!
 * \brief Generic SPI opening function
 *
 * \param bus_id SPI bus identifier
 * \return the SPI handle that will be used to access the other functions
 */
typedef aos_spi_handle_t (*aos_spi_open_func_t)(aos_spi_bus_id_t bus_id);

/*!
 * \fn aos_spi_status_t (*aos_spi_close_func_t)(aos_spi_handle_t spi_hdl)
 *
 * \brief Generic SPI closing function
 *
 * \param spi_hdl SPI bus handle
 * \return the status of the operation
 */
typedef aos_spi_status_t (*aos_spi_close_func_t)(aos_spi_handle_t spi_hdl);

/*!
 * \fn aos_spi_status_t (*aos_spi_read_func_t)(aos_spi_handle_t spi_hdl, uint8_t *data, unsigned len)
 *
 * \brief Generic SPI Read function
 *
 * \param spi_hdl SPI bus handle
 * \param data Buffer where to store the data read.
 * \param len Maximum size that can handle the data buffer.
 * \return the status of the operation
 */
typedef aos_spi_status_t (*aos_spi_read_func_t)(aos_spi_handle_t spi_hdl, uint8_t *data, unsigned len);

/*!
 * \fn aos_spi_status_t (*aos_spi_write_func_t)(aos_spi_handle_t spi_hdl, uint8_t *data, unsigned len)
 *
 * \brief Generic SPI Write function
 *
 * \param spi_hdl SPI bus handle
 * \param data Buffer containing the data to be written
 * \param len Number of data to write
 * \return the status of the operation
 */
typedef aos_spi_status_t (*aos_spi_write_func_t)(aos_spi_handle_t spi_hdl, uint8_t *data, unsigned len);

/*!
 * \fn aos_spi_status_t (*aos_spi_xfer_func_t)(aos_spi_handle_t spi_hdl, uint8_t *tx_data, uint8_t *rx_data, unsigned len)
 *
 * \brief Generic SPI transfer (read/write) function
 *
 * \param spi_hdl SPI bus handle
 * \param tx_data Buffer containing the data to be written
 * \param rx_data Buffer where to store the data read.
 * \param len Number of data to write and read
 * \return the status of the operation
 */
typedef aos_spi_status_t (*aos_spi_xfer_func_t)(aos_spi_handle_t spi_hdl, uint8_t *tx_data, uint8_t *rx_data, unsigned len);

/*!
 * \fn aos_spi_status_t (*aos_spi_ioctl_func_t)(aos_spi_handle_tspi_hdl, aos_spi_ioctl_t *ioctl)
 *
 * \brief Generic SPI IO control function
 *
 * \param spi_hdl SPI bus handle
 * \param ioctl IO control information
 * \return the status of the operation
 */
typedef aos_spi_status_t (*aos_spi_ioctl_func_t)(aos_spi_handle_t spi_hdl, aos_spi_ioctl_t *ioctl);

/*!
 * \struct aos_spi_master_t
 *
 * \brief Structure describing a SPI bus acting as master
 */
typedef struct {
	aos_spi_open_func_t open;		//!< Open the driver
	aos_spi_close_func_t close;		//!< Close the driver
	aos_spi_read_func_t read;		//!< Read data
	aos_spi_write_func_t write;		//!< Write data
	aos_spi_xfer_func_t transfer;	//!< Read/write transfer
	aos_spi_ioctl_func_t ioctl;		//!< IO control function
} aos_spi_master_t;

/*!
 * \brief Export the generic driver
 */
const aos_spi_master_t *aos_spi_master(void);

/*! @}*/
#if defined(__cplusplus)
}
#endif
