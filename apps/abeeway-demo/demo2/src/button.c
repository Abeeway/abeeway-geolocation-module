/*
 * @file app_button.c
 *
 * User button interface.
 */


#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "aos_boards_button.h"
#include "aos_log.h"

#include "board_EVK_ES2.h"
#include "button.h"
#include "events.h"

static void _button_callback(void *arg,aos_boards_button_event_info_t *bei)
{
	app_event_type_t evt;

	switch(bei->button_event) {
	case aos_boards_button_event_down:
		// Button is down
		evt = app_event_button_down;
		break;

	case aos_boards_button_event_up:
		// Button is up
		evt = app_event_button_up;
		break;

	case aos_boards_button_event_click:
		// Clicks (short presses) have been detected
		switch (bei->click_count) {
		case 1:
			evt = app_event_button_click;
			break;
		case 2:
			evt = app_event_button_2clicks;
			break;
		default:
			aos_log_status(aos_log_module_app, true, "Button %d clicks no handled\n", bei->click_count);
			break;
		}
		break;

	case aos_boards_button_event_press:
		evt = app_event_button_press;
		break;

	case aos_boards_button_event_count:
		return;
	}

	// Send the event
	event_send(evt);
}

static aos_board_button_config_t _buttons_cfg  = {
		.gpio = aos_board_pin_button1,
		.inverted = false,
		.cbfn = _button_callback,
		.cbarg = NULL
};

void button_init(void)
{
	aos_board_button_task_config_t tasks_cfg= {
			.click_collect_time = BUTTON_CLICK_COLLECT_TIME,
			.press_threshold_ms = BUTTON_PRESS_THRESHOLD,
	};

	// Initialize the driver
	aos_boards_buttons_init(&tasks_cfg);

	// Create the button
	aos_board_button_create(&_buttons_cfg);
}

