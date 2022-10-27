/*
 * ble_scan_handler.h
 *
 *  Created on: 21 oct. 2022
 *      Author: Jean-PierreTogbe
 */

#ifndef BLE_SCAN_HANDLER_H_
#define BLE_SCAN_HANDLER_H_

#include "srv_ble_scan.h"

//static void _print_ble_scan_result(srv_ble_scan_result_t* scan_result);
void ble_scan_handler_callback(void* user_arg, srv_ble_scan_result_t *result, srv_ble_scan_report_t* report);

#endif /* BLE_SCAN_HANDLER_H_ */
