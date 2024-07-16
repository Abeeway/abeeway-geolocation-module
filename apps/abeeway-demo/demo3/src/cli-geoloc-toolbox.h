/*!
 * \file cli-geoloc_toolbox.h
 *
 * \brief Geolocation toolbox
 *
 * \details This file provides facilities for geolocation technologies. It does not
 * support any CLI commands.
 *
 *
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "srv_cli.h"
#include "srv_wifi_scan.h"
#include "srv_ble_scan.h"
#include "srv_gnss_mt3333.h"
#include "srv_gnss_lr1110.h"

#pragma once

/*
 * ----------------------------------------------------------------------------
 * GNSS MT3333/LR1110
 * ----------------------------------------------------------------------------
 */
const char* cli_tool_gnss_constellation_name(aos_gnss_constellation_t constellation);

const char* cli_tool_gnss_sync_name(uint8_t sync);

void cli_tool_gnss_dump_time_info(aos_gnss_time_info_t* info);

const char* cli_tool_gnss_event_to_str(srv_gnss_mt3333_event_type_t event);

const char* cli_tool_gnss_status_name(aos_gnss_fix_status_t status);

void cli_tool_gnss_show_prn(aos_gnss_satellite_prn_report_t* prn_report, const char* prn_unit_str, bool display_sync);

void cli_tool_gnss_show_fix(aos_gnss_fix_info_t* fix);

void cli_tool_gnss_show_track(srv_gnss_mt3333_track_info_t* track_info);

void cli_tool_gnss_lr1110_show_result(aos_lr1110_gnss_result_t * result);

/*
 * ----------------------------------------------------------------------------
 * BLE scan
 * ----------------------------------------------------------------------------
 */
void cli_tool_ble_show_report(srv_ble_scan_report_t* scan_report);

void cli_tool_ble_show_result(srv_ble_scan_result_t* scan_result);

/*
 * ----------------------------------------------------------------------------
 * WIFI scan
 * ----------------------------------------------------------------------------
 */
void cli_tool_wifi_show_result(aos_wifi_scan_result_data_t* result);


