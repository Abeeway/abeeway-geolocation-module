/*!
 * @file aos_usb_cdc.h
 * @brief USB driver (serial communication support - CDC- only).
 *
 * This module drives a CDC USB
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "aos_system.h"
#include "aos_uart.h"

/*!
 * \enum aos_usb_cdc_state_t
 *
 * \brief USB CDC state
 */
typedef enum {
	aos_usb_cdc_state_off = 0,		//!< Driver is off.
	aos_usb_cdc_state_no_cable,		//!< Driver is on. No cable connected
	aos_usb_cdc_state_addressed,	//!< Driver is on. BUS enumeration has been done.
	aos_usb_cdc_state_suspended,	//!< Driver is on. USB suspended
	aos_usb_cdc_state_count			//!< Number of states
} aos_usb_cdc_state_t;

/*!
 * \var aos_uart_usb_cdc_driver
 *
 * \brief USB CDC export
 */
extern aos_uart_driver_t aos_uart_usb_cdc_driver;

/*!
 * \brief Initialize the CDC.
 *
 * \remark Reset the USB device
 *
 */
void aos_usb_cdc_init(void);

/*!
 * \brief Get the state of the USB device
 *
 * \return the connection state
 */
aos_usb_cdc_state_t aos_usb_cdc_get_state( void);

