/*
 * aos_board_evk_es2.c
 *
 * Implementation of board-specific functionality for the EVK ES2 board.
 *
 *  Created on: Mar 24, 2022
 *      Author: marc
 */
#include <stdbool.h>

#include "aos_board.h"
#include "loramac_radio.h"	// Radio driver (from LoRaWAN MAC)

#if (AOS_BOARD_TYPE == AOS_BOARD_TYPE_EVK_ES2)

// The VBAT voltage is divided by 2 via a Voltage bridge control
#define VBAT_VOLTAGE_BRIDGE_CONTROL_RATIO		2

// Voltage divider bridge delay (ms) to wait for its stabilization
#define VBAT_VOLTAGE_BRIDGE_CONTROL_DELAY		30

// voltage offset introduced by the voltage divider bride
#define VBAT_VOLTAGE_BRIDGE_OFFSET				20

typedef enum {
	aos_board_led_state_off = aos_gpio_state_set,
	aos_board_led_state_on = aos_gpio_state_reset,
} aos_board_led_state_t;

static const aos_gpio_id_t _leds[] = {
	aos_gpio_id_2,
	aos_gpio_id_3,
	aos_gpio_id_4,
	aos_gpio_id_6,
};

static const unsigned _led_count = sizeof(_leds) / sizeof(*_leds);

_Static_assert(sizeof(_leds) / sizeof(*_leds) == aos_board_led_idx_count, "Mismatch between led count and led index");

static void _board_init_leds(void)
{
	// Set up board LEDs
	for (unsigned i = 0; i < _led_count; ++i) {
		aos_gpio_open(_leds[i], aos_gpio_mode_output);
		aos_board_led_set(i, false);
	}
}

int aos_board_init(void)
{
	_board_init_leds();

	// Set up the battery sense characteristics. XXX IMV, this should be board specific.
	aos_gpio_bat_setup(aos_board_pin_vbat_sense_control, aos_board_pin_vbat_sense_measure,
			VBAT_VOLTAGE_BRIDGE_CONTROL_RATIO,
			VBAT_VOLTAGE_BRIDGE_CONTROL_DELAY, VBAT_VOLTAGE_BRIDGE_OFFSET);

	// Place the radio in sleep mode to reduce power consumption.

	loramac_radio_init((void *)0);
	loramac_radio_set_sleep();

//	aos_debug_printf("Board Initialized\r\n");

	return 0;
}

unsigned aos_board_led_count(void)
{
	return aos_board_led_idx_count;
}

void aos_board_led_set(aos_board_led_idx_t led_idx, int led_on)
{
	if (led_idx < _led_count) {
		aos_gpio_write(_leds[led_idx], led_on ? aos_board_led_state_on : aos_board_led_state_off);
	}
}

void aos_board_led_toggle(aos_board_led_idx_t led_idx)
{
	if (led_idx < _led_count) {
		aos_gpio_toggle(_leds[led_idx]);
	}
}

#endif // (AOS_BOARD_TYPE == AOS_BOARD_TYPE_EVK_ES2)
