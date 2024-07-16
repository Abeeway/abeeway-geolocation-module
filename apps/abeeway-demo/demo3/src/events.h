/*
 * @file events.h
 *
 * Application events definition
 */

#pragma once

#include <stdbool.h>

typedef enum {
	app_event_motion_start,			// Accelerometer detected a motion
	app_event_motion_stop,			// Motion duration elapsed
	app_event_geolocation_done,		// Geolocation is complete
	app_event_count,				// Number of events
}app_event_type_t;


// Initialization
void event_init(void);

// run function
void event_run(void);

// Send an event
// - Return false if failure
bool event_send(app_event_type_t evt);
