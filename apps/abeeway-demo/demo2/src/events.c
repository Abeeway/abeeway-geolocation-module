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
#include "button.h"
#include "buzzer.h"
#include "led.h"
#include "accelero.h"

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
			[app_event_button_up] = "button up",
			[app_event_button_down] = "button down",
			[app_event_button_click] = "button click",
			[app_event_button_2clicks] = "button double clicks",
			[app_event_button_press] = "button press",
			[app_event_buzzer_on] ="buzzer on",
			[app_event_buzzer_off] ="buzzer off",
			[app_event_motion_start] = "motion start",
			[app_event_motion_stop] = "motion stop",
			[app_event_shock] = "shock",
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
	uint32_t nb_beeps;

	// Infinite loop
	for ( ; ; ) {
		if (xQueueReceive(_app_evt_queue.hdl, &rx_event, portMAX_DELAY) == pdPASS) {

			aos_log_status(aos_log_module_app, true, "Got event: %s\n", _event_to_str(rx_event));

			// Dispatch the event
			switch (rx_event) {
			case app_event_button_up:
				break;

			case app_event_button_down:
				break;

			case app_event_button_click:
			case app_event_button_2clicks:
				if (rx_event == app_event_button_click) {
					nb_beeps = 1;
				} else {
					nb_beeps = 2;
				}
				buzzer_play_beep(nb_beeps);
				break;

			case app_event_button_press:
				buzzer_play_melody();
				// And play the LED pattern
				led_play_sequence();
				break;

			case app_event_buzzer_on:
				// Stop the accelerometer
				accelero_stop();
				break;

			case app_event_buzzer_off:
				// Restart the accelerometer
				accelero_start();
				break;

			case app_event_motion_start:
			case app_event_motion_stop:
			case app_event_shock:
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
