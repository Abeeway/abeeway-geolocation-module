/*
 * btn_handling.h
 *
 *  Created on: 13 oct. 2022
 *      Author: Jean-PierreTogbe
 */

#ifndef BTN_HANDLING_H_
#define BTN_HANDLING_H_

#include "aos_system.h"
#include "aos_board.h"
#include "srv_cli.h"
#include "srv_provisioning.h"
#include "aos_lpm.h"
#include "aos_dis.h"
#include "aos_rf_switch.h"
#include "aos_ble_core.h"
#include "srv_ble_scan.h"
#include "srv_ble_dtm.h"
#include "srv_ble_beaconing.h"



void btn_handling_config(aos_gpio_id_t gpio, aos_system_user_callback_t cb);
void btn_handling_open();
void btn_handling_close();
#endif /* BTN_HANDLING_H_ */
