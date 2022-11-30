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

typedef enum {
	aos_spi_bus_id_internal,	// The module internal SPI bus.
	aos_spi_bus_id_external,	// The module external SPI bus.
	aos_spi_bus_id_count
} aos_spi_bus_id_t;

typedef enum {
	aos_spi_status_success = 0,
	aos_spi_status_error,
	aos_spi_status_lock_fail,
	aos_spi_status_not_implemented,
} aos_spi_status_t;

typedef enum {
	aos_spi_ioctl_req_set_io_timeout,			// I/O Timeout, value = timeout (ms)
} aos_spi_ioctl_req_t;

typedef struct {
	aos_spi_ioctl_req_t request;
	union {
		uint32_t value;
	};
} aos_spi_ioctl_t;

typedef uint32_t aos_spi_handle_t;

#define aos_spi_handle_invalid (0)		// Invalid SPI handle

typedef aos_spi_handle_t (*aos_spi_open_func_t)(aos_spi_bus_id_t);
typedef aos_spi_status_t (*aos_spi_close_func_t)(aos_spi_handle_t);
typedef aos_spi_status_t (*aos_spi_read_func_t)(aos_spi_handle_t, uint8_t *, unsigned);
typedef aos_spi_status_t (*aos_spi_write_func_t)(aos_spi_handle_t, uint8_t *, unsigned);
typedef aos_spi_status_t (*aos_spi_xfer_func_t)(aos_spi_handle_t, uint8_t *, uint8_t *, unsigned);
typedef aos_spi_status_t (*aos_spi_ioctl_func_t)(aos_spi_handle_t, aos_spi_ioctl_t *);

typedef struct {
	aos_spi_open_func_t open;
	aos_spi_close_func_t close;
	aos_spi_read_func_t read;
	aos_spi_write_func_t write;
	aos_spi_xfer_func_t transfer;	// Read/write transfer
	aos_spi_ioctl_func_t ioctl;
} aos_spi_master_t;

const aos_spi_master_t *aos_spi_master(void);

#if defined(__cplusplus)
}
#endif
