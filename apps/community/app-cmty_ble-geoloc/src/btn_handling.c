/*
 * btn_handling.c
 *
 *  Created on: 13 oct. 2022
 *      Author: Jean-PierreTogbe
 */
#include "aos_system.h"
#include "aos_board.h"
#include "srv_cli.h"
#include "srv_provisioning.h"
#include "aos_lpm.h"
#include "aos_dis.h"
#include "srv_cli.h"
#include "aos_rf_switch.h"
#include "aos_ble_core.h"
#include "srv_ble_scan.h"
#include "srv_ble_dtm.h"
#include "srv_ble_beaconing.h"
#include <stdio.h>

struct {
	bool    drv_open;
	uint8_t ble_role;
} btn_ctx = {0};

void btn_handling_config(aos_gpio_id_t gpio, aos_system_user_callback_t cb)
{
	aos_gpio_config_t gpio_cfg;

	gpio_cfg.mode = aos_gpio_mode_input;
	gpio_cfg.pull = aos_gpio_pull_type_pulldown;
	gpio_cfg.output_type = aos_gpio_output_type_last; // not applicable in input mode
	gpio_cfg.irq_mode = aos_gpio_irq_mode_rising_edge;
	gpio_cfg.irq_prio = aos_gpio_irq_priority_medium;
	gpio_cfg.irq_servicing = aos_gpio_irq_service_type_thread;
	gpio_cfg.irq_handler.sys_cb = cb;
	gpio_cfg.user_arg = NULL;
	aos_gpio_open_ext(gpio, &gpio_cfg);
}

void btn_handling_open()
{
		btn_ctx.ble_role = GAP_PERIPHERAL_ROLE | GAP_OBSERVER_ROLE;

		if (aos_rf_switch_acquire_antenna(aos_rf_switch_type_ble_wifi, RF_SWITCH_OWNER_BLE) != aos_result_success) {
				cli_printf("Fail to acquire the antenna\n");
		}

		uint8_t deveui[PROVISIONING_EUI_SIZE];
		srv_provisioning_get_lora_device_eui(deveui);
		aos_dis_set_dev_eui(deveui);

		APP_BLE_Init(btn_ctx.ble_role);
		aos_lpm_set_mode(aos_lpm_requester_application, aos_lpm_mode_no_sleep, NULL, NULL);

		btn_ctx.drv_open = true;

		cli_printf("BLE opened\n");
}

void btn_handling_close()
{
	btn_ctx.drv_open = false;

	if (srv_ble_scan_is_active()) {
		srv_ble_scan_stop();
	}


	btn_ctx.ble_role = GAP_NO_ROLE;

//	aos_lpm_set_mode(aos_lpm_requester_application, aos_lpm_mode_stop2, NULL, NULL);
	aos_rf_switch_release_antenna(aos_rf_switch_type_ble_wifi,
			RF_SWITCH_OWNER_BLE);

	cli_printf("BLE closed\n");
}

