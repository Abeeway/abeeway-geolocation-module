/*
 * @file events.h
 *
 * Application events definition
 */

#pragma once

#include <stdbool.h>

typedef enum {
	app_event_button_up,			// Button is up
	app_event_button_down,			// Button is down
	app_event_button_click,			// Single click detected
	app_event_button_2clicks,		// two clicks detected
	app_event_button_press,			// Button press detected
	app_event_buzzer_on,			// Buzzer is active
	app_event_buzzer_off,			// Buzzer is not active
	app_event_motion_start,			// Accelerometer detected a motion
	app_event_motion_stop,			// Motion duration elapsed
	app_event_shock,				// Accelerometer detected a shock
	app_event_count,				// Number of events
}app_event_type_t;


// Initialization
void event_init(void);

// run function
void event_run(void);

// Send an event
// - Return false if failure
bool event_send(app_event_type_t evt);
