/*
 * \file ble_scan_handler.c
 *
 * \brief This file contains
 * 			- scan and print the result of BLE scan function
 * 			- and the callback function when BLE scan is done
 *
 *  Created on: 21 oct. 2022
 *      Author: Jean-PierreTogbe
 *
 *  copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "aos_log.h"
#include "aos_board.h"
#include "aos_system.h"
#include "aos_dis.h"
#include "aos_rf_switch.h"
#include "aos_lpm.h"
#include "aos_ble_core.h"
#include "aos_nvm.h"

#include "ble_scan_handler.h"

#include "encode_handling.h"

#include "lora_handler.h"

#include "srv_ble_scan.h"
#include "srv_cli.h"


static void _print_ble_scan_result(srv_ble_scan_result_t* scan_result)
{
	// Function to scan and print the result of BLE scan
	uint8_t *pdata;
	uint8_t data_len = 0;
	uint8_t mac_addr[SRV_BLE_GAP_ADDR_LEN];

	cli_printf("Scan results\n");
	if (scan_result->scan_count > 0) {
		for (int i = 0; i < scan_result->scan_count; ++i) {
			cli_printf(" ADDR: ");
			baswap(mac_addr, scan_result->entries[i].header.addr, SRV_BLE_GAP_ADDR_LEN);

			cli_print_hex(mac_addr, SRV_BLE_GAP_ADDR_LEN, false);
			cli_printf("- TxPow: %d ", scan_result->entries[i].binfo.tx_power);
			cli_printf("- RSSI: %d - Data: ", scan_result->entries[i].header.rssi);

			switch (scan_result->entries[i].binfo.type) {
			case srv_ble_scan_beacon_type_ibeacon:
				data_len = sizeof(srv_ble_scan_ibeacon_info_t);
				pdata = scan_result->entries[i].binfo.info.ibeacon.company_uuid;
				break;
			case srv_ble_scan_beacon_type_eddy_all:
			case srv_ble_scan_beacon_type_eddy_url:
			case srv_ble_scan_beacon_type_eddy_uid:
				data_len = scan_result->entries[i].binfo.info.ebeacon.data_len;
				pdata = scan_result->entries[i].binfo.info.ebeacon.data;
				break;
			case srv_ble_scan_beacon_type_altbeacon:
				data_len = sizeof(srv_ble_scan_altbeacon_info_t);
				pdata =
						scan_result->entries[i].binfo.info.abeacon.manufacturer_id;
				break;
			case srv_ble_scan_beacon_type_exposure:
				data_len = sizeof(srv_ble_scan_exposure_info_t);
				pdata = scan_result->entries[i].binfo.info.exposure_beacon.rpi;
				break;
			case srv_ble_scan_beacon_type_custom:
				data_len = sizeof(srv_ble_scan_custom_info_t);
				pdata = scan_result->entries[i].binfo.info.custom.data;
				break;
			case srv_ble_scan_beacon_type_all:
			default:
				pdata = NULL;
				return;
			}

			if (pdata) {
				for (int j = 0; j < data_len; ++j) {
						cli_printf("%02x ", pdata[j]);
				}
				cli_printf("\n");
			}
		}
	} else {
		cli_printf("No beacon detected\n");
	}
}

//The callback function when BLE scan is done
void ble_scan_handler_callback(void* user_arg, srv_ble_scan_result_t *result, srv_ble_scan_report_t* report)
{
	if (result->scan_count > 0){
	_print_ble_scan_result(result);
	lora_ble_send(result);
	}else{
		cli_printf("No beacon detected\n");
		uint8_t payload[]={0xaa,0xbb};
		srv_lmh_send(payload, sizeof(payload));
	}
}
