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
 * \var aos_uart_usb_cdc_driver
 *
 * \brief USB CDC export
 */
extern aos_uart_driver_t aos_uart_usb_cdc_driver;

