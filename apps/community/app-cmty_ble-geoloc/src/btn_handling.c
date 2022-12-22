/*
 * btn_handling.c
 *
 *  Created on: 13 oct. 2022
 *      Author: Jean-PierreTogbe
 */
#include <stdio.h>

#include "aos_system.h"
#include "aos_board.h"
#include "aos_lpm.h"
#include "aos_dis.h"
#include "aos_rf_switch.h"
#include "aos_ble_core.h"
#include "aos_log.h"

#include "ble_scan_handler.h"

#include "srv_cli.h"
#include "srv_provisioning.h"
#include "srv_cli.h"
#include "srv_ble_scan.h"
#include "srv_ble_dtm.h"
#include "srv_ble_beaconing.h"

#include "app_custom_srvc.h"

#define BLE_SRVC_ENABLE_ALL         (0xFFFF)

struct {
	bool    drv_open;
	aos_ble_app_data_t app_info;
	uint8_t deveui[AOS_PROVISIONING_EUI_SIZE];
} btn_ctx = {.app_info.ble_srvc_mask = BLE_SRVC_ENABLE_ALL};


static void _init_ble_app_info(aos_ble_app_data_t *app_info)
{
	srv_provisioning_get_lora_device_eui(btn_ctx.deveui);
	app_info->serial_number.data = btn_ctx.deveui;
	app_info->serial_number.len = AOS_PROVISIONING_EUI_SIZE;
	app_info->app_version = 0;
	aos_ble_core_get_firmware_version(&app_info->ble_version);
	app_info->custom_srvc_count = (custom_service_count > MAX_CUSTOM_SERVICES_COUNT)? MAX_CUSTOM_SERVICES_COUNT:custom_service_count;
	app_info->app_init_char_cb = app_custom_srvc_data_init;

	app_custom_srvc_conf_set(app_info->custom_srvc_init_data);
}

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
		btn_ctx.app_info.ble_role = GAP_PERIPHERAL_ROLE | GAP_OBSERVER_ROLE;

		if (aos_rf_switch_acquire_antenna(aos_rf_switch_type_ble_wifi, RF_SWITCH_OWNER_BLE) != aos_result_success) {
				cli_printf("Fail to acquire the antenna\n");
		}


		_init_ble_app_info(&btn_ctx.app_info);
		aos_ble_core_app_init(&btn_ctx.app_info);

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

	btn_ctx.app_info.ble_role = GAP_NO_ROLE;

//	aos_lpm_set_mode(aos_lpm_requester_application, aos_lpm_mode_stop2, NULL, NULL);
	aos_rf_switch_release_antenna(aos_rf_switch_type_ble_wifi,
			RF_SWITCH_OWNER_BLE);

	cli_printf("BLE closed\n");
}

void on_button_5_press(uint8_t user_id, void *arg)
{
	aos_log_msg(aos_log_module_app, aos_log_level_status, true, "BUTTON BLE SCAN SHUTDOWN PRESSED!\n");
	btn_handling_close();

}

