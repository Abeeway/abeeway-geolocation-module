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

// SDK includes
#include "aos_system.h"
#include "aos_log.h"
#include "aos_lr1110_mgr.h"
#include "aos_gpio.h"

// SDK services includes
#include "srv_cli.h"

// Application includes
#include "board_EVK_ES2.h"
#include "button.h"
#include "buzzer.h"
#include "led.h"
#include "events.h"
#include "accelero.h"

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
 * \brief  Application thread entry point
 * \param argument User argument
 */
static void _application_task(void *argument)
{
	// Log an application message.
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "Starting application thread\n");

	// Setup the battery sense characteristics.
	aos_gpio_bat_setup(aos_board_pin_vbat_sense_control, aos_board_pin_vbat_sense_measure,
			VBAT_VOLTAGE_BRIDGE_CONTROL_RATIO,
			VBAT_VOLTAGE_BRIDGE_CONTROL_DELAY, VBAT_VOLTAGE_BRIDGE_OFFSET);

	// Initialize LR1110 manager. Required even if not used to put the LR1110 in low power mode.
	aos_lr1110_mgr_init(aos_gpio_id_last);

	// Initialize application events
	event_init();

	// Initialize the board drivers
	button_init();
	buzzer_init();
	led_init();
	accelero_init();

	// Start the accelerometer
	accelero_start();

	event_run();
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

	// Start scheduler
    vTaskStartScheduler();
}
