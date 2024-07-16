/*
 * aos_boards_button.h
 *
 *  Created on: Dec 23, 2022
 *      Author: hamza
 */

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "aos_gpio.h"

/*
 * Button API definition.
 */

enum {
	BUTTON_DEBOUNCE_MS = 10,            /* milliseconds before processing a switch closure/opening */
	BUTTON_PRESS_THRESHOLD = 1000,      /* milliseconds before becoming a button press. */
	BUTTON_PRESS_THRESHOLD_MIN = 100,   /* BUTTON_PRESS_THRESHOLD must be larger than BUTTON_CLICK_THRESHOLD */
	BUTTON_CLICK_THRESHOLD = 50,        /* milliseconds minimum for a click to register*/
	BUTTON_CLICK_COLLECT_TIME = 300,    /* milliseconds to collect multiple clicks */
};

typedef enum {
	aos_boards_button_status_invalid,	// The button does not exist
	aos_boards_button_status_disabled,	// The button exists but is disabled
	aos_boards_button_status_down,		// The button is currently down (pressed)
	aos_boards_button_status_up			// The button is currently up (released)
} aos_boards_button_status_t;

typedef enum {
	aos_boards_gpio_state_uninitialized,	// Button gpio has not been initialized yet (likely disabled).
	aos_boards_gpio_state_enabled,			// Button gpio initialized (including pulls and interrupt)
	aos_boards_gpio_state_disabled			// Button gpio disabled (disabled, no pulls, no interrupt)
} aos_boards_gpio_state_t;

/*!
 * @brief aos_boards_button_event_t - Button callback event identifier.
 */
typedef enum {
	aos_boards_button_event_down,	// button pressed or switch closed.
	aos_boards_button_event_up,		// button released or switch opened.
	aos_boards_button_event_click,	// button clicked (short click or multi-clicks)
	aos_boards_button_event_press,	// button pressed (long press).
	aos_boards_button_event_count
} aos_boards_button_event_t;

typedef enum {
	button_state_opened = 0,	// Button or switch is currently opened (init default)
	button_state_closed = 1,	// Button or switch is currently closed
} button_state_t;

/*!
 * @brief aos_boards_button_event_info_t - Button callback event information.
 */
typedef struct {
	aos_gpio_id_t   button_id;
	aos_boards_button_event_t button_event;
	union {
		uint16_t event_duration;	// Time (in milliseconds) during which the button was closed (for an event_up) or open.
		uint16_t click_count;		// Number of clicks (for an event_click)
	};
} aos_boards_button_event_info_t;

/*!
 * @brief aos_boards_button_callback_t - Button callback function prototype.
 */
typedef void (*aos_boards_button_callback_t)(void *, aos_boards_button_event_info_t *info);

/*!
 * @brief aos_board_button_config_t - Button configuration information.
 */
typedef struct {
	aos_gpio_id_t                gpio;                  // The GPIO to be used.
	bool						inverted;				// False: GPIO=0 => down, GPIO=1 => up. True:  GPIO=1 => down, GPIO=0 => up.
	aos_boards_button_callback_t cbfn;                  // User callback function
	void                         *cbarg;                // User callback argument
} aos_board_button_config_t;

/*!
 * @brief aos_board_button_task_config_t - Button tasks timing.
 */
typedef struct {
	uint16_t click_collect_time;   // milliseconds to collect multiple clicks
	uint16_t press_threshold_ms;   // milliseconds to register a button press
} aos_board_button_task_config_t;

/*!
 * @brief aos_boards_buttons_init() - register a button callback handler.
 *
 * This function create tasks needed for the button driver.
 *
 * @param task_cfg tasks timing parameters
 *
 * @remark The user callback is called at user level for all events.
 */
void aos_boards_buttons_init(aos_board_button_task_config_t* task_cfg);

/*!
 * @brief aos_boards_button_status() - test the status of a specific button.
 *
 * This function tests the status of a specific button.
 *
 * @param btn the button gpio ID whose status to test.
 *
 * @return aos_boards_button_status_t indicating the status of the button.
 *
 */
aos_boards_button_status_t aos_boards_button_status(aos_gpio_id_t btn_id);

/*!
 * @brief aos_boards_button_set_click_collect_time() - Set the clicks collect time.
 *
 * This function set the max time between two clicks to be considered a double click.
 *
 * @param id the button id.
 *
 * @param collect_msec	the time in milliseconds of the clicks collect time. The value
 * 						of this parameter must be smaller than the button press threshold.
 *
 * @return aos_result_t indicating success or failure (ie, no such button or bad value)
 *
 */
aos_result_t aos_boards_button_set_click_collect_time(aos_gpio_id_t btn_id, uint16_t collect_msec);

/*!
 * @brief aos_boards_button_set_press_duration() - Set the minimum duration for a button press
 *
 * This function set the time (in milliseconds) the button needs to be held closed
 * for a button press to be reported.
 *
 * @param btn_id	the button id.
 *
 * @param press_ms	The time (in milliseconds) for which the button needs to be closed
 * 					for a button press. The value of this parameter must be larger than
 * 					the button click collect time.
 *
 * @return aos_result_t indicating success or failure (ie, no such button or bad value)
 */
aos_result_t aos_boards_button_set_press_duration(aos_gpio_id_t btn_id, uint16_t press_ms);

/*!
 * @brief aos_board_button_create() - Create and configure a button
 *
 * This function initialize button gpio and callback function
 *
 * @param conf configuration data (gpio, callback function and argument)
 *
 * @return aos_result_t indicating success or failure (ie, no such button or bad value)
 */
aos_result_t aos_board_button_create(aos_board_button_config_t* conf);

/*!
 * @brief aos_board_button_create() - Remove a button
 *
 * This function remove and uninitialize an already created button
 *
 * @param bid Button gpio id to be removed
 */
void aos_board_button_remove(aos_gpio_id_t bid);

#if defined(__cplusplus)
}
#endif

