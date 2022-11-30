/*!
 * \file cli-cmd-ble.c
 *
 * \brief BLE CLI commands
 *
 * \verbatim
 * Using scan filter with iBeacons:
 *  ble open both						Open the driver in observer and peripheral
 *  ble scan set -fofs1 9				Set the filter offset to 9 (locating the company_uuid)
 *  ble scan set -fmask1 FFFFFFFF		Set the filter mask (8 first bytes)
 *  ble scan set -fval1 e2c56db5		set the filter value
 *  ble scan ibeacon					Start the scan
 *
 *
 *\endverbatim
 */

#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "srv_cli.h"
#include "app_conf.h"
#include "ble_defs.h"
#include "aos_rf_switch.h"
#include "aos_lpm.h"
#include "aos_ble_core.h"

#include "srv_ble_dtm.h"
#include "srv_ble_scan.h"
#include "srv_ble_beaconing.h"
#include "srv_provisioning.h"

#include "app_custom_srvc.h"

#define COMMAND_ABORTED_NOT_OPEN "Command aborted: driver not open with the correct role\n"

#define ADVERTISING_INTERVAL_IN_MS  (1000)
#define CALIBRATED_TX_POWER_AT_0_M  ((uint8_t) (-22))
#define CALIBRATED_TX_POWER_AT_1_M  ((uint8_t) (-42))
#define CFG_TX_POWER                (0x18)
#define BLE_SRVC_ENABLE_ALL         (0xFFFF)

static struct {
	bool    drv_open;
	aos_ble_app_data_t app_info;
	uint8_t deveui[AOS_PROVISIONING_EUI_SIZE];
} cli_ble_ctx = {
		.app_info.ble_srvc_mask = BLE_SRVC_ENABLE_ALL
	};



/*
 * *****************************************************************************
 * Facilities
 * *****************************************************************************
 */
// Default advertised identifier
static const uint8_t _default_adv_identifier[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06
};

static bool _strtohex_table(const char * hex, uint8_t *table, uint8_t table_len)
{
    uint8_t len = strlen(hex);
    uint8_t base;

    len = (len < (2 * table_len)) ? len : (2 * table_len);
    for (int i = 0; i < len; ++i) {
    	base = (i%2) ? 1 : 16;

        if (hex[i] >= '0' && hex[i] <= '9') {
            table[i/2] += (hex[i] - 48) * base;
        } else if (hex[i] >= 'A' && hex[i] <= 'F') {
        	table[i/2] += (hex[i] - 55) * base;
        } else if (hex[i] >= 'a' && hex[i] <= 'f') {
        	table[i/2] += (hex[i] - 87) * base;
        } else {
        	return false;
        }
    }
    return true;
}


static void _baswap(uint8_t *dest, uint8_t *src, uint8_t len)
{
	uint8_t i;

	if (!dest || !src)
		return;

	for (i = 0; i < len; i++) {
		dest[i] = src[len - 1 - i];
	}
}


/*
 * *****************************************************************************
 * Scanning
 * *****************************************************************************
 */
static void _scan_set_display_help(void)
{
	cli_printf("options are:\n");
	cli_printf("    -dur     Set scan duration in ms\n");
	cli_printf("    -win     Set scan window in ms\n");
	cli_printf("    -int     Set scan interval in ms\n");
	cli_printf("    -rep     Set repeat delay in seconds. 0 No repeat\n");
	cli_printf("    -fofs1   Set the filter 1 offset\n");
	cli_printf("    -fmask1  Set the filter 1 mask (10 bytes Hex value)\n");
	cli_printf("    -fval1   Set the filter 1 value (10 bytes Hex value)\n");
	cli_printf("    -fofs2   Set the filter 2 offset\n");
	cli_printf("    -fmask2  Set the filter 2 mask (10 bytes Hex value)\n");
	cli_printf("    -fval2   Set the filter 2 value (10 bytes Hex value)\n");
	cli_printf("    -rssi    Set the rssi threshold filter\n");
	cli_printf("    -ch1     Set channel 37 compensation\n");
	cli_printf("    -ch2     Set channel 38 compensation\n");
	cli_printf("    -ch3     Set channel 39 compensation\n");
	cli_printf("    -rbcnt   Set the max number of beacons to report\n");
	cli_printf("    -rbtype  Set beacon id type to report (0: Mac address, 1: short id, 2: long id\n");
	cli_printf("    -rbofs   Set the start id offset of the reported beacon\n");
}

bool _ble_scan_set_params(int argc, char *argv[])
{
	int32_t value;

	srv_ble_scan_param_t* params = srv_ble_scan_get_params();

	enum {	opt_scan_duration,           // given in ms
			opt_scan_window,             // given in ms
			opt_scan_interval,           // given in ms
			opt_repeat_delay,            // given in seconds. 0: single shot
			opt_filter_1_offset,
			opt_filter_1_mask,
			opt_filter_1_value,
			opt_filter_2_offset,
			opt_filter_2_mask,
			opt_filter_2_value,
			opt_rssi_threshold,
			opt_compensation_ch1,
			opt_compensation_ch2,
			opt_compensation_ch3,
			opt_report_nb_beacon,
			opt_report_beacon_id_type,
			opt_report_beacon_id_offset,
			opt_help,
			opt_short_help,
			opt_count
	};

	static const cli_cmd_option_t options[] = {
			{ "-dur", opt_scan_duration },
			{ "-win", opt_scan_window },
			{ "-int", opt_scan_interval },
			{ "-rep", opt_repeat_delay },
			{ "-fofs1", opt_filter_1_offset},
			{ "-fmask1", opt_filter_1_mask},
			{ "-fval1", opt_filter_1_value},
			{ "-fofs2", opt_filter_2_offset},
			{ "-fmask2", opt_filter_2_mask},
			{ "-fval2", opt_filter_2_value},
			{ "-rssi", opt_rssi_threshold},
			{ "-ch1", opt_compensation_ch1},
			{ "-ch2", opt_compensation_ch2},
			{ "-ch3", opt_compensation_ch3},
			{ "-rbcnt", opt_report_nb_beacon},
			{ "-rbtype", opt_report_beacon_id_type},
			{ "-rbofs", opt_report_beacon_id_offset},
			{ "?", opt_short_help },
			{ "help", opt_help },
	};

	if (!argc) {
		cli_print_missing_argument();
		_scan_set_display_help();
		return false;
	}

	for (int argn = 0; argn < argc; ++argn) {

		char *option = argv[argn];

		switch(cli_get_option_index(options, opt_count, option)) {

		case cli_cmd_option_index_not_found:
			cli_printf("Unknown option: %s\n", option);
			return false;

		case cli_cmd_option_index_ambiguous:
			cli_printf("Ambiguous option: %s\n", option);
			return false;

		case opt_scan_duration:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				if ((value < 1000) || (value > 600000)) {
					cli_printf("Scan duration should be between 1000..600000 ms\n");
					return false;
				}
				params->scan_duration = value;
			} else {
				cli_printf("Please specify the scan duration\n");
				return false;
			}
			break;

		case opt_scan_window:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				if ((value < 10) || (value > 10240)) {
					cli_printf("Scan window should be between 10..10240 ms\n");
					return false;
				}
				params->scan_window = value/0.625;
			} else {
				cli_printf("Please specify the scan window\n");
				return false;
			}
			break;

		case opt_scan_interval:
				if (++argn < argc) {
					cli_parse_int(argv[argn], &value);
					if ((value < 10) || (value > 10240)) {
						cli_printf("Scan interval should be between 10..10240 ms\n");
						return false;
					}
					params->scan_interval = value/0.625;
				} else {
					cli_printf("Please specify the scan interval\n");
					return false;
				}
				break;

		case opt_repeat_delay:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				if ((value != 0) && ((value < 10) || (value > 65535))) {
					cli_printf("Repeat delay should be 0 or range in [10..65535 seconds]\n");
					return false;
				}
				params->repeat_delay = value;
			} else {
				cli_printf("Please specify the repeat delay\n");
				return false;
			}
			break;

		case opt_filter_1_offset:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				 params->filters[0].start_offset = value;
			} else {
				cli_printf("Please specify the filter 1 start offset\n");
				return false;
			}
			break;

		case opt_filter_1_mask:
			if (++argn < argc) {
				if (!_strtohex_table(argv[argn], params->filters[0].mask, SRV_BLE_SCAN_FILTER_MAX_SIZE)) {
					cli_printf("Filter mask should be an hex value of max %d bytes\n", SRV_BLE_SCAN_FILTER_MAX_SIZE);
				}
			} else {
				cli_printf("Please specify the filter 1 mask\n");
				return false;
			}
			break;

		case opt_filter_1_value:
			if (++argn < argc) {
				if (!_strtohex_table(argv[argn], params->filters[0].value, SRV_BLE_SCAN_FILTER_MAX_SIZE)) {
					cli_printf("Filter value should be an hex value of max %d bytes\n", SRV_BLE_SCAN_FILTER_MAX_SIZE);
				}
			} else {
				cli_printf("Please specify the filter 1 value\n");
				return false;
			}
			break;

		case opt_filter_2_offset:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				 params->filters[1].start_offset = value;
			} else {
				cli_printf("Please specify the filter 2 start offset\n");
				return false;
			}
			break;

		case opt_filter_2_mask:
			if (++argn < argc) {
				if (!_strtohex_table(argv[argn], params->filters[1].mask, SRV_BLE_SCAN_FILTER_MAX_SIZE)) {
					cli_printf("Filter mask should be an hex value of max %d bytes\n", SRV_BLE_SCAN_FILTER_MAX_SIZE);
				}
			} else {
				cli_printf("Please specify the filter 2 mask\n");
				return false;
			}
			break;

		case opt_filter_2_value:
			if (++argn < argc) {
				if (!_strtohex_table(argv[argn], params->filters[1].value, SRV_BLE_SCAN_FILTER_MAX_SIZE)) {
					cli_printf("Filter value should be an hex value of max %d bytes\n", SRV_BLE_SCAN_FILTER_MAX_SIZE);
				}
			} else {
				cli_printf("Please specify the filter 2 value\n");
				return false;
			}
			break;

		case opt_rssi_threshold:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				params->rssi_threshold = value;
			} else {
				cli_printf("Please specify the rssi threshold\n");
				return false;
			}
			break;

		case opt_compensation_ch1:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				params->adv_compensation[0] = value;
			} else {
				cli_printf("Please specify the compensation on channel 37\n");
				return false;
			}
			break;

		case opt_compensation_ch2:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				params->adv_compensation[1] = value;
			} else {
				cli_printf("Please specify the compensation on channel 38\n");
				return false;
			}
			break;

		case opt_compensation_ch3:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				params->adv_compensation[2] = value;
			} else {
				cli_printf("Please specify the compensation on channel 39\n");
				return false;
			}
			break;

		case opt_report_nb_beacon:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				params->report.nb_beacons = value;
			} else {
				cli_printf("Please specify the max number of beacon to report\n");
				return false;
			}
			break;

		case opt_report_beacon_id_type:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				params->report.beacon_id_type = value;
			} else {
				cli_printf("Please specify the beacon id type to report\n");
				return false;
			}
			break;

		case opt_report_beacon_id_offset:
			if (++argn < argc) {
				cli_parse_int(argv[argn], &value);
				params->report.start_id_offset = value;
			} else {
				cli_printf("Please specify the start id offset\n");
				return false;
			}
			break;

		case opt_help:
			_scan_set_display_help();
			cli_printf("usage: ble scan set [options] [value]\n");
			return true;

		case opt_short_help:
			cli_list_options("Options: ", options, sizeof(options)/sizeof(cli_cmd_option_t));
			return true;

		default:	// Should not happen, really, unless there is a program error.
			cli_printf("Unhandled option '%s'\n", option);
			return false;
		}
	}

	return true;
}


static const char* ble_report_type_to_str(srv_ble_scan_report_type_t type)
{
	switch(type) {
	case srv_ble_scan_report_type_mac_address:
		return "mac-address";
	case srv_ble_scan_report_type_short_id:
		return "short beacon ID";
	case srv_ble_scan_report_type_long_id:
		return "long beacon ID";
	default:
		break;
	}
	return "unknown";
}


static void _print_scan_report(srv_ble_scan_report_t* scan_report)
{
	uint32_t id_len;
	uint8_t* id_ptr ;
	int8_t rssi;
	int8_t tx_power;
	uint32_t id_display_len;

	cli_printf("Scan report\n");
	cli_printf(" Number of entries: %d\n", scan_report->entry_cnt);
	cli_printf(" Report type: %s\n", ble_report_type_to_str(scan_report->report_type));

	if (scan_report->report_type == srv_ble_scan_report_type_long_id) {
		cli_printf(" %-54s%-12s%s\n","Identifier", "RSSI","Tx power");
		id_len = SRV_BLE_MAX_LONG_BEACON_ID_SIZE;
		id_display_len = 54;
	} else {
		cli_printf(" %-30s%-12s%s\n","Identifier", "RSSI","Tx power");
		id_len = SRV_BLE_MAX_SHORT_BEACON_ID_SIZE;
		id_display_len = 30;
	}

	for (int ii=0; ii < scan_report->entry_cnt; ii++) {
		switch (scan_report->report_type) {
		case srv_ble_scan_report_type_mac_address:
			id_ptr = scan_report->short_beacons[ii].identifier;
			rssi = scan_report->short_beacons[ii].rssi;
			tx_power = scan_report->short_beacons[ii].tx_power;
			break;
		case srv_ble_scan_report_type_short_id:
			id_ptr = scan_report->short_beacons[ii].identifier;
			rssi = scan_report->short_beacons[ii].rssi;
			tx_power = scan_report->short_beacons[ii].tx_power;
			break;
		case srv_ble_scan_report_type_long_id:
			id_ptr = scan_report->long_beacons[ii].identifier;
			rssi = scan_report->long_beacons[ii].rssi;
			tx_power = scan_report->long_beacons[ii].tx_power;
			break;
		default:
			return;
		}
		cli_printf(" ");
		cli_print_hex(id_ptr, id_len, false);
		cli_fill_with_byte(' ', id_display_len - (id_len * 3));
		cli_printf("%-12d", rssi);
		cli_printf("%d\n", tx_power);
	}
}

static void _print_scan_result(srv_ble_scan_result_t* scan_result)
{
	uint8_t *pdata;
	uint8_t data_len = 0;
	uint8_t mac_addr[SRV_BLE_GAP_ADDR_LEN];

	cli_printf("Scan results\n");
	if (scan_result->scan_count > 0) {
		for (int i = 0; i < scan_result->scan_count; ++i) {
			cli_printf(" ADDR: ");
			 _baswap(mac_addr, scan_result->entries[i].header.addr, SRV_BLE_GAP_ADDR_LEN);
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
				pdata = scan_result->entries[i].binfo.info.abeacon.manufacturer_id;
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

static void _ble_scan_callback(void* user_arg, srv_ble_scan_result_t *result, srv_ble_scan_report_t* report)
{
	// Print the raw result
	_print_scan_result(result);

	// Print the report
	_print_scan_report(report);
}

static const char * _ble_beacon_type_to_str(srv_ble_scan_beacon_type_t type)
{
	switch(type) {
	case srv_ble_scan_beacon_type_all:
		return "all";
	case srv_ble_scan_beacon_type_eddy_uid:
		return "eddystone UUID";
	case srv_ble_scan_beacon_type_eddy_url:
		return "eddystone URL";
	case srv_ble_scan_beacon_type_eddy_all:
		return "all eddystone";
	case srv_ble_scan_beacon_type_ibeacon:
		return "ibeacon";
	case srv_ble_scan_beacon_type_altbeacon:
		return "altbeacon";
	case srv_ble_scan_beacon_type_custom:
		return "custom";
	case srv_ble_scan_beacon_type_exposure:
		return "exposure";
	default:
		break;
	}
	return "unknown";
}

static void _ble_scan_display_params(void)
{
	srv_ble_scan_param_t* params = srv_ble_scan_get_params();
	uint ii;
	cli_printf("Scan parameters\n");
	cli_printf(" Scan duration: %dms\n", params->scan_duration);
	cli_printf(" Scan window: %dms\n", (params->scan_window*625)/1000);
	cli_printf(" Scan interval: %dms\n", (params->scan_interval*625)/1000);
	cli_printf(" Repeat delay: %ds\n", params->repeat_delay);
	cli_printf(" Accepted type: %s\n", _ble_beacon_type_to_str(params->ble_scan_type));

	for (ii=0; ii< SRV_BLE_SCAN_NB_FILTER_MAX; ii++) {
		cli_printf(" Filter %d\n", ii + 1);
		cli_printf("  Start offset: %d\n", params->filters[ii].start_offset);
		cli_printf("  Mask : ");
		cli_print_hex(params->filters[ii].mask, SRV_BLE_SCAN_FILTER_MAX_SIZE, true);
		cli_printf("  Value: ");
		cli_print_hex(params->filters[ii].value, SRV_BLE_SCAN_FILTER_MAX_SIZE, true);
	}

	cli_printf(" RSSI threshold filter: %ddB\n", params->rssi_threshold);

	for (ii = 0; ii < SRV_BLE_NB_ADV_CHANNELS; ii ++) {
		cli_printf(" Channel %d compensation: %ddB\n", ii+1, params->adv_compensation[ii]);
	}

	cli_printf(" Report\n");
	cli_printf("  Beacon count: %d\n", params->report.nb_beacons);
	cli_printf("  Beacon ID type: %s\n", ble_report_type_to_str(params->report.beacon_id_type));
	cli_printf("  Beacon ID offset: %d\n", params->report.start_id_offset);
}

static bool _is_ble_open(uint8_t role)
{
	if (role != GAP_NO_ROLE) {
		return ((cli_ble_ctx.drv_open) && (cli_ble_ctx.app_info.ble_role & role));
	} else {
		return (cli_ble_ctx.drv_open);
	}
}

static cli_parser_status_t _cmd_ble_address(void* arg, int argc, char *argv[])
{
	const uint8_t *bd_addr;
	bd_addr = aos_ble_core_get_bd_address();

	cli_printf("BLE ADDR: ");
	cli_print_hex(bd_addr, SRV_BLE_GAP_ADDR_LEN, true);
	return cli_parser_status_ok;
}

static void _display_open_help(void)
{
	cli_printf("usage: ble open <options>\n");
	cli_printf("    observer       Open BLE in observer mode only (scan)\n");
	cli_printf("    peripheral     Open BLE in peripheral mode only (advertiser)\n");
	cli_printf("    both       	   Open BLE in observer and peripheral mode (scan and advertiser)\n");
}

static void _init_ble_app_info(aos_ble_app_data_t *app_info)
{
	srv_provisioning_get_lora_device_eui(cli_ble_ctx.deveui);
	app_info->serial_number.data = cli_ble_ctx.deveui;
	app_info->serial_number.len = AOS_PROVISIONING_EUI_SIZE;
	app_info->app_version = 0;
	aos_ble_core_get_firmware_version(&app_info->ble_version);
	app_info->custom_srvc_count = (custom_service_count > MAX_CUSTOM_SERVICES_COUNT)? MAX_CUSTOM_SERVICES_COUNT:custom_service_count;
	app_info->app_init_char_cb = app_custom_srvc_data_init;

	app_custom_srvc_conf_set(app_info->custom_srvc_init_data);
}

static cli_parser_status_t _cmd_ble_open(void* arg, int argc, char *argv[])
{
	enum {opt_obs, opt_periph, opt_both, opt_short_help, opt_help, opt_count };
	static const cli_cmd_option_t options[] = {
			{ "observer", opt_obs },
			{ "peripheral", opt_periph },
			{ "both", opt_both },
			{ "?", opt_short_help },
			{ "help", opt_help },
	};

	if (argc < 2) {
		cli_printf("Mandatory option missing\n");
		_display_open_help();
		return cli_parser_status_error;
	}

	if (cli_ble_ctx.drv_open) {
		cli_printf("BLE service already open\n");
		return cli_parser_status_error;
	}

	for (int argn = 1; argn < argc; ++argn) {

		char *option = argv[argn];

		switch(cli_get_option_index(options, opt_count, option)) {

		case cli_cmd_option_index_not_found:
			cli_printf("Unknown option: %s\n", option);
			return cli_parser_status_error;

		case cli_cmd_option_index_ambiguous:
			cli_printf("Ambiguous option: %s\n", option);
			return cli_parser_status_error;

		case opt_obs:
			cli_ble_ctx.app_info.ble_role |= GAP_OBSERVER_ROLE;
			break;

		case opt_periph:
			cli_ble_ctx.app_info.ble_role |= GAP_PERIPHERAL_ROLE;
			break;

		case opt_both:
			cli_ble_ctx.app_info.ble_role = GAP_PERIPHERAL_ROLE | GAP_OBSERVER_ROLE;
			break;

		case opt_short_help:
			for (int i = 0; i < opt_count; ++i) {
				cli_printf("%s ", options[i].name);
			}
			cli_printf("\n");
			return cli_parser_status_ok;

		case opt_help:
			_display_open_help();
			return cli_parser_status_ok;

		default:	// Should not happen, really, unless there is a program error.
			cli_printf("Invalid option '%s'\n", option);
			return cli_parser_status_error;
		}
	}

	if (aos_rf_switch_acquire_antenna(aos_rf_switch_type_ble_wifi, RF_SWITCH_OWNER_BLE) != aos_result_success) {
		cli_printf("Fail to acquire the antenna\n");
		return cli_parser_status_error;
	}

	_init_ble_app_info(&cli_ble_ctx.app_info);
	aos_ble_core_app_init(&cli_ble_ctx.app_info);

	aos_lpm_set_mode(aos_lpm_requester_application, aos_lpm_mode_no_sleep, NULL, NULL);

	cli_ble_ctx.drv_open = true;

	cli_printf("BLE opened\n");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_ble_close(void* arg, int argc, char *argv[])
{
	cli_ble_ctx.drv_open = false;

	if (srv_ble_scan_is_active()) {
		srv_ble_scan_stop();
	}

	if (srv_ble_beaconing_is_active()) {
		srv_ble_beaconing_stop();
	}

	cli_ble_ctx.app_info.ble_role = GAP_NO_ROLE;

	aos_lpm_set_mode(aos_lpm_requester_application, aos_lpm_mode_stop2, NULL, NULL);
	aos_rf_switch_release_antenna(aos_rf_switch_type_ble_wifi, RF_SWITCH_OWNER_BLE);

	cli_printf("BLE closed\n");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_scan_set(void* arg, int argc, char *argv[])
{
	if (!_ble_scan_set_params(--argc, ++argv)) {
		return cli_parser_status_error;
	}
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_scan_display(void* arg, int argc, char *argv[])
{
	_ble_scan_display_params();
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_stop_scan(void* arg, int argc, char *argv[])
{
	srv_ble_scan_stop();
	return cli_parser_status_ok;
}

static void _feed_beacon_param(srv_ble_scan_beacon_type_t type)
{
	srv_ble_scan_param_t* param;

	param = srv_ble_scan_get_params();
	param->ble_scan_type = type;
}

static bool _beacon_scan(srv_ble_scan_beacon_type_t btype)
{
	if (!_is_ble_open(GAP_OBSERVER_ROLE)) {
		cli_printf(COMMAND_ABORTED_NOT_OPEN);
		return false;
	}

	// Setup the filter
	_feed_beacon_param(btype);
	if (srv_ble_scan_start(_ble_scan_callback, NULL) != aos_result_success) {
		return false;
	}
	cli_printf("Scan started\n");
	return true;
}

static cli_parser_status_t _cmd_scan_all(void* arg, int argc, char *argv[])
{
	if (_beacon_scan(srv_ble_scan_beacon_type_all)) {
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_scan_eddy(void* arg, int argc, char *argv[])
{
	uint8_t beacon_type = srv_ble_scan_beacon_type_eddy_uid;
	if (argc > 1) {
		if (!strcmp(argv[1], "uuid")) {
			beacon_type = srv_ble_scan_beacon_type_eddy_uid;
			--argc; ++argv;
		} else if (!strcmp(argv[1], "url")) {
			beacon_type = srv_ble_scan_beacon_type_eddy_url;
			--argc; ++argv;
		} else if (!strcmp(argv[1], "all")) {
			beacon_type = srv_ble_scan_beacon_type_eddy_all;
			--argc; ++argv;
		}
	}
	if (_beacon_scan(beacon_type)) {
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_scan_ibeac(void* arg, int argc, char *argv[])
{
	if (_beacon_scan(srv_ble_scan_beacon_type_ibeacon)) {
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_scan_alt(void* arg, int argc, char *argv[])
{
	if (_beacon_scan(srv_ble_scan_beacon_type_altbeacon)) {
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}


/*
 * *****************************************************************************
 * Beaconing
 * *****************************************************************************
 */
static cli_parser_status_t _cmd_stop_beaconing(void* arg, int argc, char *argv[])
{

	if (srv_ble_beaconing_is_active()) {
		srv_ble_beaconing_stop();
	}
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_adv_ebeacon(void* arg, int argc, char *argv[])
{
	srv_ble_beaconing_param_t param;

	if (!_is_ble_open(GAP_PERIPHERAL_ROLE)) {
		cli_printf(COMMAND_ABORTED_NOT_OPEN);
		return cli_parser_status_error;
	}

	if (srv_ble_beaconing_is_active()) {
		srv_ble_beaconing_stop();
	}

	param.adv_interval = ADVERTISING_INTERVAL_IN_MS;
	param.calibrated_tx_power = CALIBRATED_TX_POWER_AT_1_M;


	memset(param.eddy_uuid.name_space, 0x00, sizeof(param.eddy_uuid.name_space));
	memcpy(param.eddy_uuid.instance, _default_adv_identifier, sizeof(_default_adv_identifier));


	if (srv_ble_beaconing_start(srv_ble_beacon_type_eddy_uuid, &param) != aos_result_success) {
		cli_printf("Beaconing start failure\n");
		return cli_parser_status_error;
	}

	cli_printf("Beaconing start success\n");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_adv_ibeacon(void* arg, int argc, char *argv[])
{
	srv_ble_beaconing_param_t param;

	if (!_is_ble_open(GAP_PERIPHERAL_ROLE)) {
		cli_printf(COMMAND_ABORTED_NOT_OPEN);
		return cli_parser_status_error;
	}

	if (srv_ble_beaconing_is_active()) {
		srv_ble_beaconing_stop();
	}

	param.adv_interval = ADVERTISING_INTERVAL_IN_MS;
	param.calibrated_tx_power = CALIBRATED_TX_POWER_AT_1_M;

	// Feed the company UUID
	memset(param.ibeacon.company_uuid, 0x00, sizeof(param.ibeacon.company_uuid));
	memcpy(param.ibeacon.company_uuid, _default_adv_identifier, sizeof(_default_adv_identifier));

	// Set major/minor
	param.ibeacon.major[0] = 0x01;
	param.ibeacon.major[1] = 0x02;
	param.ibeacon.minor[0] = 0x03;
	param.ibeacon.minor[1] = 0x04;


	if (srv_ble_beaconing_start(srv_ble_beacon_type_ibeacon, &param) != aos_result_success) {
		cli_printf("Beaconing start failure\n");
		return cli_parser_status_error;
	}

	cli_printf("Beaconing start success\n");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_adv_altbeacon(void* arg, int argc, char *argv[])
{
	srv_ble_beaconing_param_t param;

	if (!_is_ble_open(GAP_PERIPHERAL_ROLE)) {
		cli_printf(COMMAND_ABORTED_NOT_OPEN);
		return cli_parser_status_error;
	}

	if (srv_ble_beaconing_is_active()) {
		srv_ble_beaconing_stop();
	}

	param.adv_interval = ADVERTISING_INTERVAL_IN_MS;
	param.calibrated_tx_power = CALIBRATED_TX_POWER_AT_1_M;

	// Standard altBeacon manufacturer ID
	param.alt_beacon.manufacturer_id[0] = 0x18;
	param.alt_beacon.manufacturer_id[1] = 0x01;
	param.alt_beacon.manufacturer_id[2] = 0xBE;
	param.alt_beacon.manufacturer_id[3] = 0xAC;

	// Feed the beacon ID
	memset(param.alt_beacon.beacon_id, 0x00, sizeof(param.alt_beacon.beacon_id));
	memcpy(param.alt_beacon.beacon_id, _default_adv_identifier, sizeof(_default_adv_identifier));

	if (srv_ble_beaconing_start(srv_ble_beacon_type_altbeacon, &param) != aos_result_success) {
		cli_printf("Beaconing start failure\n");
		return cli_parser_status_error;
	}

	cli_printf("Beaconing start success\n");
	return cli_parser_status_ok;
}


static cli_parser_status_t _cmd_adv_exposure(void* arg, int argc, char *argv[])
{
	srv_ble_beaconing_param_t param;
	srv_provisioning_eui_t deveui;

	if (!_is_ble_open(GAP_PERIPHERAL_ROLE)) {
		cli_printf(COMMAND_ABORTED_NOT_OPEN);
		return cli_parser_status_error;
	}

	if (srv_ble_beaconing_is_active()) {
		srv_ble_beaconing_stop();
	}

	param.adv_interval = ADVERTISING_INTERVAL_IN_MS;
	param.calibrated_tx_power = CALIBRATED_TX_POWER_AT_1_M;

	// Feed meta data
	param.exposure.meta_data[0] =  EXP_NOTIF_SERVICE_VERSION;
	param.exposure.meta_data[1] = CFG_TX_POWER;
	param.exposure.meta_data[2] = 0x00;
	param.exposure.meta_data[3] = 0x00;

	srv_provisioning_get_lora_device_eui(deveui);
	// Feed the last 8 bytes with the devEUI
	memcpy(&param.exposure.rpi[8], deveui, sizeof(srv_provisioning_eui_t));
	memset(param.exposure.rpi, 0x00, sizeof(param.exposure.rpi) - sizeof(srv_provisioning_eui_t));

	if (srv_ble_beaconing_start(srv_ble_beacon_type_exposure, &param) != aos_result_success) {
		cli_printf("Beaconing start failure\n");
		return cli_parser_status_error;
	}

	cli_printf("Beaconing start success\n");
	return cli_parser_status_ok;
}


static cli_parser_status_t _cmd_adv_quuppa(void* arg, int argc, char *argv[])
{
	srv_ble_beaconing_param_t param;
	srv_provisioning_eui_t deveui;

	if (!_is_ble_open(GAP_PERIPHERAL_ROLE)) {
		cli_printf(COMMAND_ABORTED_NOT_OPEN);
		return cli_parser_status_error;
	}

	if (srv_ble_beaconing_is_active()) {
		srv_ble_beaconing_stop();
	}

	param.adv_interval = ADVERTISING_INTERVAL_IN_MS;
	param.calibrated_tx_power = CALIBRATED_TX_POWER_AT_0_M;

	srv_provisioning_get_lora_device_eui(deveui);

	// Feed specific data
	param.quuppa.compensated_tx_power = CFG_TX_POWER;
	// 6 last byte of deveui
	memcpy(param.quuppa.identifier, &deveui[2], sizeof(param.quuppa.identifier));

	if (srv_ble_beaconing_start(srv_ble_beacon_type_exposure, &param) != aos_result_success) {
		cli_printf("Beaconing start failure\n");
		return cli_parser_status_error;
	}

	cli_printf("Beaconing start success\n");
	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _cmd_tab_beacon[] = {
		PARSER_CMD_FUNC("altbeacon", "Start altbeacon", _cmd_adv_altbeacon, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("ibeacon", "Start ibeacon", _cmd_adv_ibeacon, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("eddystone", "Start ebeacon", _cmd_adv_ebeacon, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("quuppa", "Start Quuppa beacon", _cmd_adv_quuppa, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("exposure", "Start exposure beacon", _cmd_adv_exposure, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("stop", "Stop beaconing", _cmd_stop_beaconing, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

static const cli_parser_cmd_t _cmd_tab_scan[] = {
		PARSER_CMD_FUNC("set", "Set scan parameters", _cmd_scan_set, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("display", "Display scan parameters", _cmd_scan_display, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("eddystone", "Scan only Eddystone beacons", _cmd_scan_eddy, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("ibeacon", "Scan only I-beacons", _cmd_scan_ibeac, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("altbeacon", "Scan only alt-beacons", _cmd_scan_alt, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("all", "Scan all beacon types", _cmd_scan_all, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("stop", "Stop beaconing scans", _cmd_stop_scan, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

static const cli_parser_cmd_t _ble_cmd_table[] = {
		PARSER_CMD_FUNC("close", "Close the BLE service", _cmd_ble_close, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("open <mode>", "Open the BLE service", _cmd_ble_open, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("beacon", "Start beaconing", _cmd_tab_beacon, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("scan", "Scan beacons", _cmd_tab_scan, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("address", "Show BLE MAC address", _cmd_ble_address, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};


/*
 * ----------------------------------------------------------------------------
 * Main Commands registration
 * ----------------------------------------------------------------------------
 */
// Register main commands against the CLI parser
CLI_COMMAND_TAB_REGISTER(ble,"BLE commands", _ble_cmd_table, CLI_ACCESS_ALL_LEVELS );
