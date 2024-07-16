/*!
 * \file cli-cmd-geoloc.c
 *
 * \brief Geolocation CLI commands
 *
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "cli-geoloc-toolbox.h"
#include "srv_geoloc_basic.h"

#include "geolocation.h"


/*
 * ----------------------------------------------------------------------------
 * Commands
 * ----------------------------------------------------------------------------
 */

static cli_parser_status_t _cmd_geoloc_start(void* arg, int argc, char *argv[])
{
	cli_printf("Start geolocation\n");

	if (!geolocation_start()) {
		return cli_parser_status_error;
	}
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_geoloc_abort(void* arg, int argc, char *argv[])
{

	geolocation_abort();
	cli_printf("Aborting geoloc\n");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_geoloc_show(void* arg, int argc, char *argv[])
{
	aos_result_t result;
	const srv_geolocation_result_t* geo_results;
	const srv_geolocation_result_per_techno_t* techno;
	uint8_t ii;

	result = srv_geoloc_basic_get_results(&geo_results);

	if (result!= aos_result_success) {
		cli_print_aos_result(result);
		return cli_parser_status_error;
	}

	cli_printf("Geolocation results. Nb techno: %d", geo_results->nb_techno);

	for (ii=0; ii < geo_results->nb_techno; ii ++) {
		cli_printf("\n");
		techno = &geo_results->results[ii];
		if (techno->type == srv_geolocation_type_none) {
			break;
		}
		cli_printf("Techno %s, geoloc status: %s. Techno status: ", srv_geoloc_common_type_to_str(techno->type),
				srv_geoloc_common_status_to_str(techno->status));

		if (techno->status == srv_geolocation_status_skipped) {
			cli_printf("-\n");
			continue;
		}

		switch (techno->type) {
		case srv_geolocation_type_lr_agnss:
			if (!techno->data.lr_gnss) {
				break;
			}
			cli_printf("%s\n" ,srv_gnss_lr1110_scan_status_to_str(techno->data.lr_gnss->status));
			cli_tool_gnss_lr1110_show_result(techno->data.lr_gnss);
			continue;

		case srv_geolocation_type_wifi:
			if (!techno->data.wifi) {
				break;
			}
			cli_printf("success. Nb AP: %d\n", techno->data.wifi->nb_scan_results);
			cli_tool_wifi_show_result(techno->data.wifi);
			continue;

		case srv_geolocation_type_ble_scan1:
		case srv_geolocation_type_ble_scan2:
			if (!techno->data.ble->entry_cnt) {
				cli_printf("no beacons\n");
				continue;
			}
			cli_printf("success. Nb beacons: %d\n", techno->data.ble->entry_cnt);
			cli_tool_ble_show_report(techno->data.ble);
			continue;

		case srv_geolocation_type_mt_agnss:
		case srv_geolocation_type_gnss:
			switch (techno->data.mt_gnss.event) {
			case srv_gnss_mt3333_event_error:
				cli_printf("error\n");
				continue;

			case srv_gnss_mt3333_event_fix_success:
				if (!techno->data.mt_gnss.fix_info) {
					break;
				}
				cli_printf("fix success\n");
				cli_tool_gnss_show_fix(techno->data.mt_gnss.fix_info);
				continue;

			case srv_gnss_mt3333_event_agps_success:
				if (!techno->data.mt_gnss.prn_report) {
					break;
				}
				cli_printf("MT agnss success\n");
				cli_tool_gnss_show_prn(techno->data.mt_gnss.prn_report,"", true);
				break;

			case srv_gnss_mt3333_event_no_sat_timeout:
				cli_printf("MT no satellites in view (T0 timeout)\n");
				continue;

			case srv_gnss_mt3333_event_fix_acq_timeout:
				cli_printf("MT acquisition timeout\n");
				continue;

			case srv_gnss_mt3333_event_fix_t1_timeout:
				cli_printf("MT T1 timeout\n");
				continue;

			case srv_gnss_mt3333_event_abort:
				cli_printf("aborted\n");
				continue;

			case srv_gnss_mt3333_event_last:
				// Make the compiler happy
				break;
			}

		default:
			break;;
		}
		cli_printf("-\n");
	}

	return cli_parser_status_ok;
}

/*
 * ----------------------------------------------------------------------------
 * Main Commands registration
 * ----------------------------------------------------------------------------
 */
static const cli_parser_cmd_t _geoloc_cmd_table[] = {
		PARSER_CMD_FUNC("start", "Start a geolocation", _cmd_geoloc_start, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("abort", "Abort the current geolocation", _cmd_geoloc_abort, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("show", "Show the results", _cmd_geoloc_show, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

// Register main commands against the CLI parser
CLI_COMMAND_TAB_REGISTER(geoloc,"Geolocation commands", _geoloc_cmd_table, CLI_ACCESS_ALL_LEVELS );
