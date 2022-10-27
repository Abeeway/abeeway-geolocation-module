/*
 * app_scan_report.h
 *
 *  Created on: 21 oct. 2022
 *      Author: Jean-PierreTogbe
 */

#ifndef APP_SCAN_REPORT_H_
#define APP_SCAN_REPORT_H_


#include "LmHandler.h"


void on_rx_data( LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);
void on_button_4_press(uint8_t user_id, void *arg);
void application_task(void *argument);

#endif /* APP_SCAN_REPORT_H_ */
