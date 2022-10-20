/*!
 * \file main.c
 *
 * \brief startup of application
 *
 *  * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "aos_board.h"
#include "aos_system.h"

#include "strnhex.h"

#include "ble_defs.h"
#include "aos_rf_switch.h"
#include "aos_lpm.h"
#include "aos_ble_core.h"
#include "aos_dis.h"
#include "aos_log.h"
#include "aos_nvm.h"

#include "srv_cli.h"
#include "srv_gnss.h"
#include "app_conf.h"

#include "srv_lmh.h"
#include "srv_ble_scan.h"
#include "srv_ble_dtm.h"
#include "srv_ble_beaconing.h"
#include "srv_provisioning.h"
#include "encode_handling.h"
#include "btn_handling.h"

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

//Filter definitions
#define ABW_PREFIX "ABEEWAY"


srv_ble_scan_param_t* ble_param;

// Application parameters stored in Flash
#define PARAM_ID_REPEAT_DELAY		1


static uint8_t result;

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
	uint32_t value;
	DisplayRxUpdate(appData, params);


	switch (appData->Port) {
	case 1: // The application LED can be controlled on port 1 or 2
		value = appData->Buffer[0];
		if(value > 300){
			value = 300;
			aos_nvm_write(PARAM_ID_REPEAT_DELAY, value);
			ble_param->repeat_delay = value;
		}else if (value < 15){
			value =15;
			aos_nvm_write(PARAM_ID_REPEAT_DELAY, value);
			ble_param->repeat_delay = value;
		}else{
			aos_nvm_write(PARAM_ID_REPEAT_DELAY, value);
			ble_param->repeat_delay = value;
		}

		cli_printf("DURATION :  %d\n", ble_param->repeat_delay);
		break;
	case 2:
		cli_printf("Received request to turn the Application LED O%s\n", (appData->Buffer[0] & 0x01) ? "N":"FF");
		aos_board_led_set(aos_board_led_idx_led4, appData->Buffer[0] & 0x01);
		break;
	default:
		break;
	}
}


static void _scan_callback(void* user_arg, srv_ble_scan_result_t *result, srv_ble_scan_report_t* report)
{
	_print_scan_result(result);
	_lora_ble_send(result);
}

static void _on_button_4_press(uint8_t user_id, void *arg)
{

	btn_handling_open();
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "BUTTON BLE SCAN ACTIVATE PRESSED!\n");


	cli_printf("ble start scan result : %d\n",ble_param->repeat_delay );
	ble_param->ble_scan_type = srv_ble_scan_beacon_type_eddy_uid;


	memset(ble_param->filters, 0, sizeof(ble_param->filters));

	ble_param->filters[0].start_offset = 13;

	memcpy(ble_param->filters[0].value, ABW_PREFIX, strlen(ABW_PREFIX));
	memset(ble_param->filters[0].mask, 0xFF , strlen(ABW_PREFIX));

	result = srv_ble_scan_start(_scan_callback, arg);
	cli_printf("ble start scan result : %d\n", result);
}

static void _on_button_5_press(uint8_t user_id, void *arg)
{
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "BUTTON BLE SCAN SHUTDOWN PRESSED!\n");
	btn_handling_close();

}


/*!
  * \brief  Application main thread
  * \param argument User argument
  */
static void _application_task(void *argument)
{
	uint32_t value;
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "Starting application thread\n");


	// Initiating LoRaMAC Handler service
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "Initiating LoRaMAC Handler service\n");
	srv_lmh_open(_on_rx_data);


	// Button (Board switch 04) configuration
	btn_handling_config(aos_gpio_id_7, _on_button_4_press);
	//Button (Board switch 05) configuration
	btn_handling_config(aos_gpio_id_8, _on_button_5_press);

	// >Pre-initialze the BLE param
	ble_param = srv_ble_scan_get_params();
	if (aos_nvm_read(PARAM_ID_REPEAT_DELAY, &value)== aos_result_success) {
		ble_param->repeat_delay =  value;
	} else {
		ble_param->repeat_delay =  30;
	}

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
