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

#if (AOS_BOARD_TYPE == AOS_BOARD_TYPE_EVK_ES2)

#if defined(__cplusplus)
extern "C" {
#endif

#include "aos_gpio.h"

// The VBAT voltage is divided by 2 via a Voltage bridge control
#define VBAT_VOLTAGE_BRIDGE_CONTROL_RATIO		2

// Voltage divider bridge delay (ms) to wait for its stabilization
#define VBAT_VOLTAGE_BRIDGE_CONTROL_DELAY		30

// voltage offset introduced by the voltage divider bride
#define VBAT_VOLTAGE_BRIDGE_OFFSET				20

/*
 * Board pin assignments to module pins.
 */
typedef enum {
	aos_board_pin_accelero_power = aos_gpio_id_1,
	aos_board_pin_led1 = aos_gpio_id_2,			// LED 1
	aos_board_pin_led2 = aos_gpio_id_3,			// LED 2
	aos_board_pin_led3 = aos_gpio_id_4,			// LED 3
	aos_board_pin_led4 = aos_gpio_id_6,			// LED 4
	aos_board_pin_button1 = aos_gpio_id_7,		// SW3
	aos_board_pin_button2 = aos_gpio_id_8,		// SW4
	aos_board_pin_vbat_sense_control = aos_gpio_id_5,
	aos_board_pin_vbat_sense_measure = aos_gpio_id_vbat_sense,
	aos_board_pin_baro_power= aos_gpio_id_i2c_power,
	aos_board_pin_accelero_int = aos_gpio_id_i2c_int2,
} aos_board_pin_t;

#if defined(__cplusplus)
}
#endif

#endif
