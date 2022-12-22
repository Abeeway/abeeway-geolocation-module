/*
 * \file aos_board_evk_es2.h
 *
 * \brief Board-specific header file for the Abeeway Murata EVK ES2 board.
 *
 * This file should not be included directly by application code. Instead,
 * the application shall define the appropriate AOS_BOARD_TYPE conditional
 * and include the "aos_board.h" header file.
 *
 * Library code never includes board specific header files.
 *
 *  Created on: Jan 19, 2022
 *      Author: marc
 */
#pragma once

#if !defined(__AOS_BOARD_WAS_INCLUDED__)
#error "You may not include this file directly, instead, please include aos_board.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#include "aos_gpio.h"

/*
 * Board pin assignments to module pins.
 */
typedef enum {
	aos_board_pin_i2c_power_1 = aos_gpio_id_i2c_power,
	aos_board_pin_i2c_power_2 = aos_gpio_id_1,
	aos_board_pin_led2 = aos_gpio_id_2,
	aos_board_pin_led3 = aos_gpio_id_3,
	aos_board_pin_led4 = aos_gpio_id_4,
	aos_board_pin_led5 = aos_gpio_id_6,
	aos_board_pin_vbat_sense_control = aos_gpio_id_5,
	aos_board_pin_vbat_sense_measure = aos_gpio_id_vbat_sense,
} aos_board_pin_t;

/*
 * Indices of various LEDs present on this board,  to be used as a
 * parameter to the aos_board_led_set() and aos_board_led_toggle()
 * API functions.
 */
typedef enum {
	aos_board_led_idx_led2 = 0,	// Index of LED2 for aos_board_led API functions
	aos_board_led_idx_led3,
	aos_board_led_idx_led4,
	aos_board_led_idx_led5,
	aos_board_led_idx_count
} aos_board_led_idx_t;

#if defined(__cplusplus)
}
#endif

