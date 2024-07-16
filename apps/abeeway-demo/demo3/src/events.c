/*!
 * \file events.c
 *
 * \brief Application events
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "aos_log.h"

#include "events.h"
#include "led.h"
#include "accelero.h"
#include "geolocation.h"

// Max number of events in the queue
#define APP_EVENT_MAX_ENTRY		20

// Queue context (events are just 32 bits values
typedef struct {
	QueueHandle_t hdl;		// Handle
	StaticQueue_t data;		// Internal data
	uint8_t storage[sizeof(uint32_t) *  APP_EVENT_MAX_ENTRY];		// Queue storage
} app_event_queue_context_t;

static app_event_queue_context_t _app_evt_queue;


static const char* _event_to_str(app_event_type_t evt)
{
	const char* evt_name[app_event_count] = {
			[app_event_motion_start] = "motion start",
			[app_event_motion_stop] = "motion stop",
			[app_event_geolocation_done] = "geoloc done",
	};

	if (evt >= app_event_count) {
		return "unknown";
	}

	return evt_name[evt];
}

/*!
 * \brief  Application event running part.
 * \note Infinite loop
 */
void event_run(void)
{
	uint32_t rx_event;		// Application event

	// Infinite loop
	for ( ; ; ) {
		if (xQueueReceive(_app_evt_queue.hdl, &rx_event, portMAX_DELAY) == pdPASS) {

			aos_log_status(aos_log_module_app, true, "Got event: %s\n", _event_to_str(rx_event));

			// Dispatch the event
			switch (rx_event) {

			case app_event_motion_start:
				// Start the geolocation
				geolocation_start();
				break;

			case app_event_motion_stop:
				// Do what you want on this event
				break;

			case app_event_geolocation_done:
				// Blink lED
				led_play_sequence();

				// And send lora uplink
				geolocation_send_result();
				break;

			case app_event_count:
				break;
			}
		}
	}

	// Never should go here
	vTaskDelete(NULL);
}

/*!
  * \brief  Event initialization
  */
void event_init(void)
{
	// Create the application event queue
	_app_evt_queue.hdl =  xQueueCreateStatic(APP_EVENT_MAX_ENTRY,	// Max number of events
			sizeof(uint32_t),						// Size of the event
			_app_evt_queue.storage,
			&_app_evt_queue.data);
}

/*!
  * \brief  Send an event
  */
bool event_send(app_event_type_t evt)
{
	uint32_t event = evt;

	// Defer the processing under the main application task
	if (xQueueSend(_app_evt_queue.hdl, &event,0) != pdPASS) {
		// Log a warning
		aos_log_warning(aos_log_module_app, true, "Fails to queue the event %s\n", _event_to_str(evt));
		return false;
	}

	return true;
}
