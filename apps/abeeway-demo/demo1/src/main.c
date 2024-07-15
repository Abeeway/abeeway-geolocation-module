/*!
 * \file main.c
 *
 * \brief startup of application
 *
 *  * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

// SDK includes
#include "aos_system.h"
#include "aos_log.h"
#include "aos_lr1110_mgr.h"
#include "aos_gpio.h"

// SDK services includes
#include "srv_cli.h"

// Application includes
#include "board_EVK_ES2.h"


/*!
 * \brief Application thread
 */
// Application Thread
#define APP_TASK_STACK_SIZE 		(4096/sizeof(StackType_t))
#define APP_TASK_PRIORITY 			AOS_THREAD_PRIO_APPLICATION
static TaskHandle_t _app_task;
static StaticTask_t _app_task_info;
static StackType_t _app_task_stack[APP_TASK_STACK_SIZE];


/*!
 * \brief Define the CLI configuration
 */
// Buffers
#define SRV_CLI_UART_TX_BUFFER 1024
#define SRV_CLI_UART_RX_BUFFER 4096

static uint8_t _srv_cli_tx_buffer[SRV_CLI_UART_TX_BUFFER];
static uint8_t _srv_cli_rx_buffer[SRV_CLI_UART_RX_BUFFER];

static cli_config_param_t _cli_cfg = {
		123,		// User password -PIN code-
		456,		// Super password -PIN code-
		{			// Serial port configuration
				.speed = aos_uart_speed_57600,		// Allow very low power: STOP2 and HSI switched off during sleep
				.stop = aos_uart_stop_bit_1,
				.parity = aos_uart_parity_none,
				.data_format = aos_uart_data_size_8_bits,
				.hard_flow_control = aos_uart_hw_flow_control_off,
				.tx_buffer_size = SRV_CLI_UART_TX_BUFFER,
				.rx_buffer_size = SRV_CLI_UART_RX_BUFFER,
				.tx_buffer = _srv_cli_tx_buffer,
				.rx_buffer = _srv_cli_rx_buffer,
				.user_rx_cb = _NULL, // Will be Fed by the CLI service
				.user_arg = NULL	 // Will be Fed by the CLI service
		}
};


/*!
 * \brief Application event queue
 */
// Events that will be received by the application thread
typedef enum {
	app_event_type_led2_toggle = 0,		// Toggle the LED2
} app_event_type_t;

// Max number of events in the queue
#define APP_EVENT_MAX_ENTRY		20

// Queue context (events are just 32 bits values
typedef struct {
	QueueHandle_t hdl;		// Handle
	StaticQueue_t data;		// Internal data
	uint8_t storage[sizeof(uint32_t) *  APP_EVENT_MAX_ENTRY];		// Queue storage
} app_event_queue_context_t;

static app_event_queue_context_t _app_evt_queue;


/*!
 * \brief LED2 blinking under freeRTOS timer
 */
#define APP_MAIN_LED_PERIOD			1000	//!< Main LED blink period in ms

// Structure for a FreeRTOS timer statically allocated
typedef struct {
	TimerHandle_t timer_hdl;			// Timer handle
	StaticTimer_t timer_local_data;		// Timer internal data
	UBaseType_t timer_count;			// Timer count
} app_freertos_timer_t;

static app_freertos_timer_t _led2_timer;

static void _led2_timer_timeout( TimerHandle_t xExpiredTimer )
{

	uint32_t event = app_event_type_led2_toggle;

	// Defer the processing under the main application task
	if (xQueueSend(_app_evt_queue.hdl, &event,0) != pdPASS) {
		// Log a warning
		aos_log_warning(aos_log_module_app, true, "Fails to queue the event\n");
	}
}

/*!
 * \brief Digital input GPIO (button SW3) handling
 */
// Catch the event. Remind that we are on system thread!
static void _gpio_input_exception(uint8_t user_id, void* arg)
{
	aos_log_status(aos_log_module_app, true, "SW3 pressed\n");
}

static void _gpio_input_create(void)
{
	aos_gpio_config_t gpio_conf;

	gpio_conf.mode = aos_gpio_mode_input;

	// On the EVK, the button is not inverted: Pressed = 1, Released = 0
	gpio_conf.pull = aos_gpio_pull_type_pulldown;
	gpio_conf.output_type = aos_gpio_output_type_push_pull;
	gpio_conf.irq_mode = aos_gpio_irq_mode_falling_edge;	// A press generates an exception
	gpio_conf.irq_prio = aos_gpio_irq_priority_low;
	gpio_conf.irq_servicing = aos_gpio_irq_service_type_thread;
	gpio_conf.irq_handler.sys_cb = _gpio_input_exception;
	gpio_conf.user_arg = NULL;

	if (aos_gpio_open_ext(aos_board_pin_button1, &gpio_conf) != aos_result_success) {
		aos_log_warning(aos_log_module_app, true, "Input GPIO creation failure\n");
	}
}


/*!
 * \brief  Application thread entry point
 * \param argument User argument
 */
static void _application_task(void *argument)
{
	uint32_t rx_event;		// Application event

	// Log an application message.
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "Starting application thread\n");

	// Setup the battery sense characteristics.
	aos_gpio_bat_setup(aos_board_pin_vbat_sense_control, aos_board_pin_vbat_sense_measure,
			VBAT_VOLTAGE_BRIDGE_CONTROL_RATIO,
			VBAT_VOLTAGE_BRIDGE_CONTROL_DELAY, VBAT_VOLTAGE_BRIDGE_OFFSET);

	// Initialize LR1110 manager. Required even if not used to put the LR1110 in low power mode.
	aos_lr1110_mgr_init(aos_gpio_id_last);

	// Open the output GPIO associated to the LED2
	aos_gpio_open(aos_board_pin_led2, aos_gpio_mode_output);

	// Create the FreeRTOS timer (start automatically when created)
	_led2_timer.timer_hdl =  xTimerCreateStatic("LED2",
			pdMS_TO_TICKS(APP_MAIN_LED_PERIOD),
			pdTRUE,			// Make it periodic
			&_led2_timer.timer_count,
			_led2_timer_timeout,
			&_led2_timer.timer_local_data);

	// ... and start it
	xTimerStart(_led2_timer.timer_hdl, 0);

	// Open the input GPIO associated to the SW3
	_gpio_input_create();

	// Infinite loop
	for ( ; ; ) {
		if (xQueueReceive(_app_evt_queue.hdl, &rx_event, portMAX_DELAY) == pdPASS) {
			// Process the event
			switch (rx_event) {
			case app_event_type_led2_toggle:
				aos_gpio_toggle(aos_board_pin_led2);
				break;
			}
		}
	}

	// Never should go here
	vTaskDelete(NULL);
}

/*!
  * \brief  Application entry point.
  * \return unused returned value
  */
int main(void)
{
	// Initialize the system
	aos_system_init(true);

	// Initialize the CLI service
	srv_cli_init();
	srv_cli_open(aos_uart_type_lpuart1, &_cli_cfg);		//!< Command Line Interface

	// Open the log facility and redirect log message on the CLI.
	aos_log_init(cli_log);

	// Create the system task. Since it is statically allocated. it should not fail
	_app_task = xTaskCreateStatic(_application_task, "Application", APP_TASK_STACK_SIZE, NULL,
			APP_TASK_PRIORITY, _app_task_stack, &_app_task_info);

	// Create the application event queue
	_app_evt_queue.hdl =  xQueueCreateStatic(APP_EVENT_MAX_ENTRY,	// Max number of events
			sizeof(uint32_t),						// Size of the event
			_app_evt_queue.storage,
			&_app_evt_queue.data);

	// Start scheduler
    vTaskStartScheduler();
}
