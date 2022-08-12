/*!
 * \file main.c
 *
 * \brief startup of application
 *
 *  * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */

#include "FreeRTOS.h"
#include "task.h"

#include "aos_board.h"
#include "aos_system.h"

#include "srv_cli.h"
#include "aos_log.h"
#include "srv_gnss.h"

#include "srv_lmh.h"

// General definitions
#define APP_MAIN_LED_PERIOD			1000	//!< Main LED blink period in ms


// Application Thread
#define APP_TASK_STACK_SIZE 		(2048/sizeof(StackType_t))
#define APP_TASK_PRIORITY 			AOS_THREAD_PRIO_APPLICATION
static TaskHandle_t _app_task;
static StaticTask_t _app_task_info;
static StackType_t _app_task_stack[APP_TASK_STACK_SIZE];


// CLI definitions
#define SRV_CLI_UART_TX_BUFFER 1024
#define SRV_CLI_UART_RX_BUFFER 1024

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


static void _on_rx_data(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
	DisplayRxUpdate(appData, params);

	switch (appData->Port) {
	case 1: // The application LED can be controlled on port 1 or 2
	case 2:
		cli_printf("Received request to turn the Application LED O%s\n", (appData->Buffer[0] & 0x01) ? "N":"FF");
		aos_board_led_set(aos_board_led_idx_led4, appData->Buffer[0] & 0x01);
		break;
	default:
		break;
	}
}

static void _on_button_press(void *arg)
{
	// aos_log_msg(aos_log_module_app, aos_log_level_status, true, "BUTTON PRESSED!\n");
	uint8_t payload[] = { 0xaa, 0xbb, 0xcc };
	srv_lmh_send(payload, sizeof(payload));
}

/*!
  * \brief  Application main thread
  * \param argument User argument
  */
static void _application_task(void *argument)
{

	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "Starting application thread\n");


	// Initiating LoRaMAC Handler service
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "Initiating LoRaMAC Handler service\n");
	srv_lmh_open(_on_rx_data);


	// Button configuration
	aos_gpio_id_t gpio = aos_gpio_id_7; // board switch 04
	aos_gpio_config_t gpio_cfg;
	gpio_cfg.mode = aos_gpio_mode_input;
	gpio_cfg.pull = aos_gpio_pull_type_pulldown;
	gpio_cfg.output_type = aos_gpio_output_type_last; // not applicable in input mode
	gpio_cfg.irq_mode = aos_gpio_irq_mode_rising_edge;
	gpio_cfg.irq_prio = aos_gpio_irq_priority_medium;
	gpio_cfg.irq_servicing = aos_gpio_irq_service_type_int;
	gpio_cfg.irq_handler = (aos_gpio_callback_t)_on_button_press;
	gpio_cfg.user_arg = NULL;
	aos_gpio_open_ext(gpio, &gpio_cfg);


	// Start blinking LED3
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "Start blinking LED3\n");
	for ( ; ; ) {
		// Toggle the LED state
		aos_board_led_toggle(aos_board_led_idx_led3);
		vTaskDelay(pdMS_TO_TICKS(APP_MAIN_LED_PERIOD));
	}
	vTaskDelete(NULL);
}

/*!
  * \brief  Application entry point.
  * \return unused returned value
  */
int main(void)
{
	// Initialize the system
	aos_system_init();

	// Initialize the services
	srv_cli_init(aos_uart_type_lpuart1, &_cli_cfg);		//!< Command Line Interface

	// Open the log facility and redirect log message on the CLI.
	aos_log_init(cli_log);

	srv_gnss_init();									//!< GNSS service

	// Create the system task. Since it is statically allocated. it should not fail
	_app_task = xTaskCreateStatic(_application_task, "Application", APP_TASK_STACK_SIZE, NULL,
			APP_TASK_PRIORITY, _app_task_stack, &_app_task_info);

	// Start scheduler
    vTaskStartScheduler();
}
