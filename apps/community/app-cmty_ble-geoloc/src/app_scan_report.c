/*
 * app_scan_report.c
 *
 *  Created on: 21 oct. 2022
 *      Author: Jean-PierreTogbe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "aos_system.h"
#include "aos_board.h"
#include "aos_lpm.h"
#include "aos_dis.h"
#include "aos_rf_switch.h"
#include "aos_ble_core.h"
#include "aos_log.h"
#include "aos_nvm.h"

#include "app_scan_report.h"


#include "btn_handling.h"
#include "ble_scan_handler.h"
//#include "ble_defs.h"
//#include "app_conf.h"
#include "srv_ble_dtm.h"

#include "FreeRTOS.h"
#include "task.h"


#include "lora_handler.h"


#include "LmHandler.h"

#include "srv_lmh.h"
#include "srv_ble_scan.h"
#include "srv_cli.h"
#include "srv_ble_dtm.h"
#include "srv_ble_beaconing.h"
#include "srv_provisioning.h"



// General definitions
#define APP_MAIN_LED_PERIOD			1000	//!< Main LED blink period in ms

#define ABW_PREFIX "ABEEWAY"

#define PARAM_ID_REPEAT_DELAY	1
#define PARAM_ID_FILTER_MAIN1	2
#define PARAM_ID_FILTER_MAIN2	3
#define PARAM_ID_OFSET1			4
#define PARAM_ID_OFSET2			5


static uint8_t result;

static srv_ble_scan_param_t* ble_param;

void on_rx_data( LmHandlerAppData_t *appData, LmHandlerRxParams_t *params) //, srv_ble_scan_param_t* ble_param)
{


	uint32_t value, id_value;
	uint8_t* Buf=NULL;
	//uint8_t tab_value[SRV_BLE_SCAN_FILTER_MAX_SIZE]={0};
	int j=0;
	DisplayRxUpdate(appData, params);


	switch (appData->Buffer[0]){
	case 11 ://Update parameters
		value = appData->Buffer[3]+appData->Buffer[4]+appData->Buffer[5]+appData->Buffer[6];
		id_value =appData->Buffer[2];

		if (id_value == 105){// ble scan duration parameter

			if (value > 300) {
				value = 300;
				aos_nvm_write(PARAM_ID_REPEAT_DELAY, value);
				ble_param->repeat_delay = value;
			} else if (value < 15) {
				value = 15;
				aos_nvm_write(PARAM_ID_REPEAT_DELAY, value);
				ble_param->repeat_delay = value;
			} else {
				aos_nvm_write(PARAM_ID_REPEAT_DELAY, value);
				ble_param->repeat_delay = value;
			}
			cli_printf("DURATION :  %d\n", ble_param->repeat_delay);

		}
		else if (id_value  == 78){//ble filter 1 parameter
			for (int i = 4; i <= appData->BufferSize; i++) {
				Buf[j] = appData->Buffer[i];
				j++;
			}
			aos_nvm_write(PARAM_ID_REPEAT_DELAY, *Buf);
			memcpy(ble_param->filters[0].value, Buf, 4);

			cli_printf("FILTER 1 :  %d\n", ble_param->filters[0].value);
		}
		else if (id_value  == 79){//ble filter 1 parameter
			cli_printf("FILTER 2 :  %d\n", value);
		}else{

		}
		break;
	default:
		break;
	}
}


void on_button_4_press(uint8_t user_id, void *arg)
{

	btn_handling_open();
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "BUTTON BLE SCAN ACTIVATE PRESSED!\n");


	cli_printf("ble start scan result : %d\n",ble_param->repeat_delay );
	ble_param->ble_scan_type = srv_ble_scan_beacon_type_eddy_uid;


	memset(ble_param->filters, 0, sizeof(ble_param->filters));

	ble_param->filters[0].start_offset = 13;

	memcpy(ble_param->filters[0].value, ABW_PREFIX, strlen(ABW_PREFIX));
	memset(ble_param->filters[0].mask, 0xFF , strlen(ABW_PREFIX));

	result = srv_ble_scan_start(ble_scan_handler_callback, arg);
	cli_printf("ble start scan result : %d\n", result);
}


void application_task(void *argument)
{
	uint32_t value;
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "Starting application thread\n");


	// Initiating LoRaMAC Handler service
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "Initiating LoRaMAC Handler service\n");
	srv_lmh_open(on_rx_data);


	// Button (Board switch 04) configuration
	btn_handling_config(aos_gpio_id_7, on_button_4_press);
	//Button (Board switch 05) configuration
	btn_handling_config(aos_gpio_id_8, on_button_5_press);

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
