/*
 * @file led.c
 *
 * Manage the LED 2
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "aos_gpio.h"
#include "board_EVK_ES2.h"

#include "board_drv_led.h"
#include "led.h"

// led2_pattern
static const board_drv_led_sequencer_entry_t _led2_pattern[] = {
	{ board_drv_seq_led_on, 5 }, { board_drv_seq_led_off, 200 },
	{ board_drv_seq_led_on, 5 }, { board_drv_seq_led_off, 200 },
	{ board_drv_seq_led_on, 10 }, { board_drv_seq_led_off, 150 },
	{ board_drv_seq_led_on, 20 }, { board_drv_seq_led_off, 100 },
    { board_drv_seq_led_on, 50 }, { board_drv_seq_led_off, 70 },
    { board_drv_seq_led_on, 70 }, { board_drv_seq_led_off, 50 },
    { board_drv_seq_led_on, 100 }, { board_drv_seq_led_off, 30 },
    { board_drv_seq_led_on, 1000 }, { board_drv_seq_led_off, 1 },
    { board_drv_seq_done }
};

/*****************************************************************************
 * MODULE INTERFACE                                                          *
 *****************************************************************************/
void led_stop()
{
	board_drv_led_sequencer_stop(aos_board_pin_led2);
}

void led_play_sequence(void)
{
	board_drv_led_sequencer_start(aos_board_pin_led2, _led2_pattern);
}

void led_init ()
{
	static aos_board_led_config_t cfg = {
			aos_board_pin_led2,
			NULL,					// No callback
			NULL,					// No callback user argument
			BOARD_LED2_CONSUMPTION
	};
	board_led_create(&cfg);

	// Put the LED off
	board_drv_led_set(aos_board_pin_led2, false);
}

uint64_t led_get_consumption(void)
{
	return board_drv_led_get_consumption_uah();
}

void led_clear_consumption(void)
{
	board_drv_led_clear_consumption();
}
