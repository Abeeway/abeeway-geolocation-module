/*
 * \file app_scan_report.c
 *
 * \brief This file contains
 * 			- the function that is execute when downlink message is received;
 * 			- the function on press on button 4, that start BLE scan and print the result of scan;
 * 			- and the function application task that start the application.
 *
 *  Created on: 21 oct. 2022
 *      Author: Jean-PierreTogbe
 *
 *  copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
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

#include "encode_handling.h"

#include "FreeRTOS.h"

#include "LmHandler.h"

#include "lora_handler.h"

#include "srv_ble_dtm.h"
#include "srv_lmh.h"
#include "srv_ble_scan.h"
#include "srv_cli.h"
#include "srv_ble_dtm.h"
#include "srv_ble_beaconing.h"
#include "srv_provisioning.h"

#include "task.h"

// General definitions
#define APP_MAIN_LED_PERIOD			1000	//!< Main LED blink period in ms
// Default prefixs of BLE Filter
#define ABW_MAIN1_PREFIX "ABEE"
#define ABW_MAIN2_PREFIX "WAY0"
// Application parameters stored in Flash
#define PARAM_ID_REPEAT_DELAY	0x69
#define PARAM_ID_FILTER_MAIN1	0x4E
#define PARAM_ID_FILTER_MAIN2	0x4F

static uint8_t result; // a result of the BLE scan variable
static srv_ble_scan_param_t* ble_param; // variable of the BLE scan parameter setting

void on_rx_data( LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
	uint32_t value;
	uint32_t id_value;
	DisplayRxUpdate(appData, params);

	switch (appData->Buffer[0]){
	case 11 ://Update parameters
		id_value =appData->Buffer[2];
		if (id_value == PARAM_ID_REPEAT_DELAY/*105*/){// ble scan duration parameter
			value = appData->Buffer[3]|appData->Buffer[4]|appData->Buffer[5]|appData->Buffer[6];

			value = (value > 300) ? 300 : ((value < 15) ? 15 : value); // test to sure that value is between 15 and 300s

			aos_nvm_write(PARAM_ID_REPEAT_DELAY, value);
			ble_param->repeat_delay = value;
			cli_printf("DURATION :  %d\n", ble_param->repeat_delay);
		}else if (id_value  == PARAM_ID_FILTER_MAIN1/*78*/){//ble filter 1 parameter

			//cli_printf("Buf 1 :  %x\n", appData->Buffer[3]);
			ble_param->filters[0].value[0] = appData->Buffer[3];
			ble_param->filters[0].value[1] = appData->Buffer[4];
			ble_param->filters[0].value[2] = appData->Buffer[5];
			ble_param->filters[0].value[3] = appData->Buffer[6];
			value = __builtin_bswap32(*(uint32_t *)&ble_param->filters[0].value);
			aos_nvm_write(PARAM_ID_FILTER_MAIN1, value);

			cli_printf("FILTER 1 :  %x\n", value);
		}else if (id_value  == PARAM_ID_FILTER_MAIN2/*79*/){//ble filter 2 parameter

			ble_param->filters[0].value[4] = appData->Buffer[3];
			ble_param->filters[0].value[5] = appData->Buffer[4];
			ble_param->filters[0].value[6] = appData->Buffer[5];
			ble_param->filters[0].value[7] = appData->Buffer[6];
			value = __builtin_bswap32(*(uint32_t *)&ble_param->filters[0].value[4]);
			aos_nvm_write(PARAM_ID_FILTER_MAIN2, value);

			cli_printf("FILTER 2 :  %x\n", value);
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

	result = srv_ble_scan_start(ble_scan_handler_callback, arg);
	cli_printf("ble start scan result : %d\n", result);
}


void application_task(void *argument)
{
	uint32_t value;
	bool valid;
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
	//Repeat delay at value or 30s by default
	ble_param->repeat_delay = ((aos_nvm_read(PARAM_ID_REPEAT_DELAY, &value)== aos_result_success) ? value : 30);

	memset(&ble_param->filters[0], 0, sizeof(ble_param->filters));
	ble_param->filters[0].start_offset = 13;
	// FILTRE MAIN1
	valid = false;
	if(aos_nvm_read(PARAM_ID_FILTER_MAIN1, &value)==aos_result_success){
		if (value) {
			baswap(ble_param->filters[0].value, (uint8_t*)&value, sizeof(value));
			memset(ble_param->filters[0].mask, 0xFF , sizeof(value));
			valid = true;
		}
	}
	if (!valid) {
		memcpy(ble_param->filters[0].value, ABW_MAIN1_PREFIX, strlen(ABW_MAIN1_PREFIX));
		memset(ble_param->filters[0].mask, 0xFF , strlen(ABW_MAIN1_PREFIX));
	}

	//FILTRE MAIN2
	valid = false;
	if(aos_nvm_read(PARAM_ID_FILTER_MAIN2, &value)==aos_result_success){
		if (value) {
			baswap(&ble_param->filters[0].value[4], (uint8_t*)&value, sizeof(value));
			memset(&ble_param->filters[0].mask[4], 0xFF , sizeof(value));
			valid = true;
		}
	}
	if (!value) {
		memcpy(&ble_param->filters[0].value[4], ABW_MAIN2_PREFIX, strlen(ABW_MAIN2_PREFIX));
		memset(&ble_param->filters[0].mask[4], 0xFF , strlen(ABW_MAIN2_PREFIX));
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
