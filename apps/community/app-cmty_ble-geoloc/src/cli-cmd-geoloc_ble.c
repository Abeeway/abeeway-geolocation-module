
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
// #include <stdio.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "aos_system.h"
#include "aos_board.h"
#include "aos_lpm.h"
#include "aos_dis.h"
#include "aos_rf_switch.h"
#include "aos_ble_core.h"
#include "aos_log.h"
#include "aos_nvm.h"

#include "encode_handling.h"
#include "ble_scan_handler.h"

#include "srv_cli.h"
#include "app_conf.h"
#include "ble_defs.h"

#include "strnhex.h"

#include "srv_ble_dtm.h"
#include "srv_ble_scan.h"
#include "srv_ble_beaconing.h"
#include "srv_provisioning.h"

#define PARAM_ID_REPEAT_DELAY	0x69
#define PARAM_ID_FILTER_MAIN1	0x4E
#define PARAM_ID_FILTER_MAIN2	0x4F

static uint8_t result;
int32_t value;
static cli_parser_status_t _cmd_ble_demo_start(void* arg, int argc, char *argv[])
{
		srv_ble_scan_param_t* ble_param = srv_ble_scan_get_params();
		aos_log_msg(aos_log_module_app, aos_log_level_status, true, "BLE EDDYSTONNE SCAN ACTIVATE !\n");

		cli_printf("ble start scan result : %d\n",ble_param->repeat_delay );
		ble_param->ble_scan_type = srv_ble_scan_beacon_type_eddy_uid;

		result = srv_ble_scan_start(ble_scan_handler_callback, arg);
		cli_printf("ble start scan result : %d\n", result);
		return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_ble_demo_stop(void* arg, int argc, char *argv[])
{
	srv_ble_scan_stop();
	return cli_parser_status_ok;

}

static cli_parser_status_t _cmd_ble_demo_filter(void* arg, int argc, char *argv[])
{
	srv_ble_scan_param_t* ble_param = srv_ble_scan_get_params();
	for (int argn = 0; argn < argc; ++argn) {
	if (++argn < argc) {
		if (!_set_ble_scan_filter(ble_param->filters[0].value, argv[argn])) {

			cli_printf("Filter value should be an hex value of max %d bytes\n",SRV_BLE_SCAN_FILTER_MAX_SIZE);
		}else{
			value = __builtin_bswap32(*(uint32_t *)&ble_param->filters[0].value);
			memset(ble_param->filters[0].mask, 0xFF , sizeof(value));
			aos_nvm_write(PARAM_ID_FILTER_MAIN1, value);
		}

	} else {
		cli_printf("Please specify the filter 1 value\n");
		return false;
	}
	}
	return cli_parser_status_ok;
}

/*static cli_parser_status_t _cmd_ble_demo_mask(void* arg, int argc, char *argv[])
{
	srv_ble_scan_param_t* ble_param = srv_ble_scan_get_params();
	for (int argn = 0; argn < argc; ++argn) {
		if (++argn < argc) {
			if (!_set_ble_scan_filter(ble_param->filters[0].mask, argv[argn])) {
				cli_printf("Filter mask should be an hex value of max %d bytes\n",SRV_BLE_SCAN_FILTER_MAX_SIZE);
			}
		} else {
			cli_printf("Please specify the filter 1 mask\n");
			return false;
		}
	}
	return cli_parser_status_ok;
}*/

static cli_parser_status_t _cmd_ble_demo_repeat_delay(void* arg, int argc, char *argv[])
{

	srv_ble_scan_param_t* ble_param = srv_ble_scan_get_params();
	for (int argn = 0; argn < argc; ++argn) {
		if (++argn < argc) {
			cli_parse_int(argv[argn], &value);
			if ((value != 0) && ((value < 10) || (value > 65535))) {
				cli_printf(
						"Repeat delay should be 0 or range in [10..65535 seconds]\n");
				return false;
			}
			aos_nvm_write(PARAM_ID_REPEAT_DELAY, value);
			ble_param->repeat_delay = value;
		} else {
			cli_printf("Please specify the repeat delay\n");
			return false;
		}
	}
	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _geoloc_ble_demo[] = {
		PARSER_CMD_FUNC("-start", "Start the BLE scan and reporting", _cmd_ble_demo_start, CLI_ACCESS_ALL_LEVELS), //<time>
		PARSER_CMD_FUNC("-stop", "Stop the scan and reporting", _cmd_ble_demo_stop, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("-rep ", "Set the BLE scan repeat delay", _cmd_ble_demo_repeat_delay, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("-filter ", "Set the BLE scan filter", _cmd_ble_demo_filter, CLI_ACCESS_ALL_LEVELS),
		//PARSER_CMD_FUNC("-mask ", "Set the BLE scan mask", _cmd_ble_demo_mask, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};
/*
 * ----------------------------------------------------------------------------
 * Main Commands registration
 * ----------------------------------------------------------------------------
 */
// Register main commands against the CLI parser
CLI_COMMAND_TAB_REGISTER(geolocBle,"Padawan commands", _geoloc_ble_demo, CLI_ACCESS_ALL_LEVELS );
