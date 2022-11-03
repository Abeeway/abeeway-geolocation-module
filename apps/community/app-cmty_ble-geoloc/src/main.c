/*!
 * \file main.c
 *
 * \brief startup of application
 *
 *  * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>

#include "app_scan_report.h"

#include "aos_board.h"
#include "aos_system.h"
#include "aos_ble_core.h"
#include "aos_log.h"

#include "FreeRTOS.h"

#include "srv_cli.h"
#include "srv_gnss.h"

#include "task.h"

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
	_app_task = xTaskCreateStatic(application_task, "Application", APP_TASK_STACK_SIZE, NULL,
			APP_TASK_PRIORITY, _app_task_stack, &_app_task_info);
	// Start scheduler
    vTaskStartScheduler();
}
