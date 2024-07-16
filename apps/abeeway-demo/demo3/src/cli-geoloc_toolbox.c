/*!
 * \file cli-geoloc_toolbox.c
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

#include "cli-geoloc-toolbox.h"

/*
 * ----------------------------------------------------------------------------
 * GNSS MT3333/LR1110
 * ----------------------------------------------------------------------------
 */
#define ABS_VAL(x) (x>0?x:-x)
typedef struct {
	const char* name;
	uint32_t value;
} gnss_conversion_t;

static const gnss_conversion_t _gnss_conv_sync[] = {
		{"none", 0},
		{"time", AOS_GNSS_SAT_SYNC_TIME},
		{"bit", AOS_GNSS_SAT_SYNC_SET_STATE(AOS_GNSS_SAT_SYNC_BIT)},
		{"frame", AOS_GNSS_SAT_SYNC_SET_STATE(AOS_GNSS_SAT_SYNC_FRAME)},
		{"exact", AOS_GNSS_SAT_SYNC_SET_STATE(AOS_GNSS_SAT_SYNC_EXACT)},
		{NULL,0}
};

static const char* _gnss_constellation_to_str[aos_gnss_constellation_count] = {
		"???",
		"GPS",
		"GLO",
		"BEI",
		"GNN",
		"GAL"
};

static const char* _conversion_to_name(const gnss_conversion_t* cv, uint32_t value)
{
	while (cv->name) {
		if (cv->value == value) {
			return cv->name;
		}
		cv ++;
	}
	// Return "unknown"
	return "unknown";
}

const char* cli_tool_gnss_constellation_name(aos_gnss_constellation_t constellation)
{
	if (constellation >= aos_gnss_constellation_count) {
		return "";
	}
	return _gnss_constellation_to_str[constellation];
}

const char* cli_tool_gnss_sync_name(uint8_t sync)
{
	const gnss_conversion_t* cv = _gnss_conv_sync;

	while (cv->name) {
		if (sync == cv->value) {
			return cv->name;
		}
		if (cv->value == AOS_GNSS_SAT_SYNC_TIME) {
			sync &=~AOS_GNSS_SAT_SYNC_TIME;
		}
		cv ++;
	}
	// Return "unknown"
	return "unknown";
}

void cli_tool_gnss_dump_time_info(aos_gnss_time_info_t* info)
{
	uint32_t val1;
	uint32_t val2;

	switch (info->type) {
	case aos_gnss_time_type_software:
		val1 = info->time / 1000000;
		val2 = info->time % 1000000;
		cli_printf("SW TIME: %d.%06d", val1, val2);
		break;
	case aos_gnss_time_type_week_tow:
		val1 = info->gnss_week_tow.tow / 1000000;
		val2 = info->gnss_week_tow.tow % 1000000;
		cli_printf("TOW: %d.%06d, week:%d", val1, val2, info->gnss_week_tow.week);
		break;
	case aos_gnss_time_type_utc:
		cli_printf("UTC: %02d/%02d/%02d %02d:%02d:%02d.%03d",
				info->utc.month, info->utc.day,info->utc.year,info->utc.hour,info->utc.min,
				info->utc.sec, info->utc.msec);
		break;
	}
}


const char* cli_tool_gnss_event_to_str(srv_gnss_mt3333_event_type_t event)
{
	static const char* _gnss_event_to_str[srv_gnss_mt3333_event_last] = {
		"Error",
		"Fix success",
		"A-GNSS success",
		"No satellite",
		"Fix acq_timeout",
		"Fix T1 timeout"
		"Abort"
	};

	if (event >= srv_gnss_mt3333_event_last) {
		return "unknown";
	}
	return _gnss_event_to_str[event];
}

const char* cli_tool_gnss_status_name(aos_gnss_fix_status_t status)
{
	static const gnss_conversion_t _conv[] = {
			{"NOK", aos_gnss_fix_invalid},
			{"OK", aos_gnss_fix_valid},
			{"2D", aos_gnss_fix_valid_2d},
			{"3D", aos_gnss_fix_valid_3d},
			{NULL, 0}
	};

	return _conversion_to_name(_conv, status);
}

void cli_tool_gnss_show_prn(aos_gnss_satellite_prn_report_t* prn_report, const char* prn_unit_str, bool display_sync)
{
	uint8_t ii;
	const char * sync_str;
	aos_gnss_satellite_prn_t* sat;
	uint8_t hundredth;
	uint32_t prn_unit;

	cli_printf("GNSS pseudo-range report\n");
	cli_print_systime(prn_report->timestamp);
	cli_printf(" GPS Time: ");
	cli_tool_gnss_dump_time_info(&prn_report->gnss_time);
	cli_printf("\n");
	cli_printf(" Number of satellites: %d\n", prn_report->nb_sat);
	cli_printf(" Solvable: %s\n", cli_str_yes_no(prn_report->solvable));
	cli_printf(" PRN compressed: %s\n", cli_str_yes_no(prn_report->compressed_prn));

	if (!prn_report->nb_sat) {
		return;
	}

	cli_printf(" %5s%18s%9s%17s%s%8s\n","Sv ID","Constellation","C/N0","Pseudo-range",
			prn_unit_str?prn_unit_str:"", display_sync?"Sync":"");

	for (ii=0; ii < prn_report->nb_sat; ii ++) {
		sat = &prn_report->sat_info[ii];
		sync_str = cli_tool_gnss_sync_name(sat->sync_flags);
		if (prn_report->compressed_prn) {
			hundredth = 0;
			prn_unit = sat->pseudo_range;
		} else {
			// Value in centimeter
			hundredth = sat->pseudo_range % 100;
			prn_unit = sat->pseudo_range / 100;
		}
		if (display_sync) {
			cli_printf(" %5d%18s%9d%14lu.%02d%12s\n",
					sat->sv_id,
					cli_tool_gnss_constellation_name(sat->constellation),
					sat->cn0,
					prn_unit,hundredth,
					sync_str);
		} else {
			cli_printf(" %5d%18s%9d%14lu.%02d\n",
					sat->sv_id,
					cli_tool_gnss_constellation_name(sat->constellation),
					sat->cn0,
					prn_unit,hundredth);
		}
	}
}

void cli_tool_gnss_show_fix(aos_gnss_fix_info_t* fix)
{
	uint8_t ii;

	if (fix->status == aos_gnss_fix_invalid) {
		cli_printf("No valid GNSS fix\n");
		return;
	}
	cli_printf("GNSS fix\n");
	cli_printf(" Status: %s\n", cli_tool_gnss_status_name(fix->status));
	cli_print_systime(fix->timestamp);
	cli_printf(" GPS Time: ");
	cli_tool_gnss_dump_time_info(&fix->gnss_time);
	cli_printf("\n");
	cli_printf(" TTFF: %ds\n", fix->ttff);
	cli_printf(" Latitude: %d.%07d\n", fix->lat / 10000000, abs(fix->lat) % 10000000);
	cli_printf(" Longitude: %d.%07d\n", fix->lon / 10000000, abs(fix->lon) % 10000000);
	cli_printf(" Altitude: %d.%02d\n", fix->alt/100, ABS_VAL(fix->alt) % 100);
	cli_printf(" Constellation: %s\n", cli_tool_gnss_constellation_name(fix->constellation));
	cli_printf(" EHPE: %d.%02d m\n", fix->ehpe / 100, fix->ehpe % 100);
	cli_printf(" HDOP: %d.%01d\n", (fix->hdop * 2) / 10,(fix->hdop * 2) % 10);
	cli_printf(" VDOP: %d.%02d\n", fix->vdop/100, fix->vdop%100);
	cli_printf(" PDOP: %d.%02d\n", fix->pdop/100, fix->pdop%100);
	cli_printf(" COG: %d.%02d degree\n", fix->cog/100, fix->cog%100);
	cli_printf(" SOG: %d.%02d m/s\n", fix->sog/100, fix->sog%100);
	cli_printf(" Number of satellites used for fix: %d\n", fix->fix_satellites);
	cli_printf(" Number of tracked satellites: %d\n", fix->track_satellites);
	cli_printf(" Satellites used for fix: {");
	for (ii = 0; ii < fix->fix_satellites; ii++) {
		if (ii == fix->fix_satellites - 1) {
			cli_printf("%d}\n", fix->sats_for_fix[ii]);
		} else {
			cli_printf("%d, ", fix->sats_for_fix[ii]);
		}
	}
}

void cli_tool_gnss_show_track(srv_gnss_mt3333_track_info_t* track_info)
{
	uint8_t ii;
	srv_gnss_mt3333_sat_track_info_t* sat;
	struct tm tm;

	aos_rtc_systime_local_time(track_info->systime.seconds, &tm);

	cli_printf("GNSS tracking data\n");
	cli_printf(" UTC time: %02d/%02d/%02d %02d:%02d:%02d\n",
			tm.tm_mon, tm.tm_mday,1900+tm.tm_year,tm.tm_hour,tm.tm_min, tm.tm_sec);
	cli_printf(" Number of satellites: %d\n", track_info->nb_sat);
	if (!track_info->nb_sat) {
		return;
	}
	cli_printf(" %5s%18s%9s\n","Sv ID","Constellation","C/N0");

	for (ii=0; ii < track_info->nb_sat; ii ++) {
		sat = &track_info->sat_info[ii];
		cli_printf(" %5d%18s%9d\n",
				sat->svid,
				cli_tool_gnss_constellation_name(sat->constellation),
				sat->cn0);
	}
}

void cli_tool_gnss_lr1110_show_result(aos_lr1110_gnss_result_t * result)
{
	uint8_t ii;
	const char* _ood_str = "out of date:";

	cli_printf("Scan done. Status: %s\n", result?srv_gnss_lr1110_scan_status_to_str(result->status):"unknown");

	cli_printf(" Nb successful scans: %d\n", result->nb_scans);
	cli_printf(" Type: NAV%d\n", result->scan_mode);
	cli_printf(" GPS Almanac %s %s\n", _ood_str, cli_str_yes_no(result->almanac_gps_update));
	cli_printf(" BEIDOU Almanac %s %s\n", _ood_str, cli_str_yes_no(result->almanac_beidou_update));
	cli_printf(" Position %s %s\n", _ood_str, cli_str_yes_no(result->position_update));

	for (ii=0; ii< result->nb_scans; ii ++) {
		cli_printf(" Scan %d\n", ii);
		cli_printf(" Nav (%d): ", result->scans[ii].nav.size);
		if (result->scan_mode == aos_lr1110_gnss_scan_mode_nav2) {
			// for nav1, the timestamp is displayed along to the PRN report.
			cli_print_systime(result->scans[ii].timestamp);
		}
		cli_print_hex(result->scans[ii].nav.data, result->scans[ii].nav.size, true);
		if (result->scan_mode == aos_lr1110_gnss_scan_mode_nav1) {
			cli_tool_gnss_show_prn(&result->scans[ii].report, "(ns)", false);
		}
		cli_printf("\n");
	}
}


/*
 * ----------------------------------------------------------------------------
 * BLE scan
 * ----------------------------------------------------------------------------
 */
static void _baswap(uint8_t *dest, const uint8_t *src, uint8_t len)
{
	uint8_t i;

	if (!dest || !src)
		return;

	for (i = 0; i < len; i++) {
		dest[i] = src[len - 1 - i];
	}
}

static const char* _ble_report_type_to_str(srv_ble_scan_report_type_t type)
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

void cli_tool_ble_show_report(srv_ble_scan_report_t* scan_report)
{
	uint32_t id_len;
	uint8_t* id_ptr ;
	int8_t rssi;
	int8_t tx_power;
	uint32_t id_display_len;

	cli_printf("Scan report\n");
	cli_print_systime(scan_report->timestamp);
	cli_printf(" Number of entries: %d\n", scan_report->entry_cnt);
	cli_printf(" Report type: %s\n", _ble_report_type_to_str(scan_report->report_type));

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

void cli_tool_ble_show_result(srv_ble_scan_result_t* scan_result)
{
	uint8_t *pdata;
	uint8_t data_len = 0;
	uint8_t mac_addr[BLE_MAC_ADDR_SIZE];

	cli_printf("Scan results\n");
	cli_print_systime(scan_result->timestamp);
	if (scan_result->scan_count > 0) {
		for (int i = 0; i < scan_result->scan_count; ++i) {
			cli_printf(" ADDR: ");
			 _baswap(mac_addr, scan_result->entries[i].header.addr, BLE_MAC_ADDR_SIZE);
			 cli_print_hex(mac_addr, BLE_MAC_ADDR_SIZE, false);
			cli_printf("- TxPow: %d ", scan_result->entries[i].binfo.tx_power);
			cli_printf("- RSSI: %d - Data: ", scan_result->entries[i].header.rssi);
			switch (scan_result->entries[i].binfo.type) {
			case srv_ble_scan_beacon_type_ibeacon:
				data_len = sizeof(srv_ble_scan_ibeacon_info_t);
				pdata = scan_result->entries[i].binfo.info.ibeacon.proximity_uuid;
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

/*
 * ----------------------------------------------------------------------------
 * WIFI scan
 * ----------------------------------------------------------------------------
 */
static const char* _unknown_str = "unknown";

static const char* _wifi_signal_to_str(aos_wifi_scan_signal_type_scan_t signal)
{
	switch(signal) {
	case aos_wifi_scan_signal_type_scan_b:
		return "B";
	case aos_wifi_scan_signal_type_scan_g:
		return "G";
	case aos_wifi_scan_signal_type_scan_n:
		return "N";
	default:
		// Note aos_wifi_scan_signal_type_scan_b_g_n used only for config
		break;
	}
	return _unknown_str;
}

static const char* _wifi_dr_to_str(aos_wifi_scan_datarate_type_t datarate)
{
	const char* _name[] = {
		    [aos_wifi_scan_datarate_1_MBPS]    ="1",
		    [aos_wifi_scan_datarate_2_MBPS]    ="2",
		    [aos_wifi_scan_datarate_6_MBPS]    = "6",
		    [aos_wifi_scan_datarate_9_MBPS]    = "9",
		    [aos_wifi_scan_datarate_12_MBPS]   = "12",
		    [aos_wifi_scan_datarate_18_MBPS]   = "18",
		    [aos_wifi_scan_datarate_24_MBPS]   = "24",
		    [aos_wifi_scan_datarate_36_MBPS]   = "36",
		    [aos_wifi_scan_datarate_48_MBPS]   = "48",
		    [aos_wifi_scan_datarate_54_MBPS]   = "54",
		    [aos_wifi_scan_datarate_6_5_MBPS]  = "6.5",
		    [aos_wifi_scan_datarate_13_MBPS]   = "13",
		    [aos_wifi_scan_datarate_19_5_MBPS] = "19.5",
		    [aos_wifi_scan_datarate_26_MBPS]   = "26",
		    [aos_wifi_scan_datarate_39_MBPS]   = "39",
		    [aos_wifi_scan_datarate_52_MBPS]   = "52",
		    [aos_wifi_scan_datarate_58_MBPS]   = "58",
		    [aos_wifi_scan_datarate_65_MBPS]   = "65",
		    [aos_wifi_scan_datarate_7_2_MBPS]  = "7.2",
		    [aos_wifi_scan_datarate_14_4_MBPS] = "14.4",
		    [aos_wifi_scan_datarate_21_7_MBPS] = "21.7",
		    [aos_wifi_scan_datarate_28_9_MBPS] = "28.9",
		    [aos_wifi_scan_datarate_43_3_MBPS] = "43.3",
		    [aos_wifi_scan_datarate_57_8_MBPS] = "57.8",
		    [aos_wifi_scan_datarate_65_2_MBPS] = "65.2",
		    [aos_wifi_scan_datarate_72_2_MBPS] = "72.2",
		};

	if (datarate > aos_wifi_scan_datarate_72_2_MBPS) {
		return _unknown_str;
	}
	return _name[datarate];
}

static const char* _wifi_origin_to_str(aos_wifi_scan_origin_type_t origin)
{
	switch (origin) {
	case aos_wifi_scan_origin_gateway:
		return "gateway";
	case aos_wifi_scan_origin_mobile:
		return "mobile";
	default:
		break;
	}
	return _unknown_str;
}

void cli_tool_wifi_show_result(aos_wifi_scan_result_data_t* result)
{
	int ii;
	aos_wifi_scan_channel_result_t* ap_list;

	cli_print_systime(result->timestamp);
	cli_printf(" %-20s%-9s%-12s%-11s%-14s%-11s\n", "MAC", "RSSI", "Signal", "Channel", "DR (Mbps)", "Origin");
	ap_list = result->data;
	for (ii=0; ii < result->nb_scan_results; ii ++) {
		cli_printf(" ");
		cli_print_hex(ap_list[ii].mac_address, AOS_WIFI_MAC_ADDRESS_SIZE, false);
		cli_printf("   %-9d%-12s%-11d%-14s%-11s\n",
				ap_list[ii].rssi,
				_wifi_signal_to_str(AOS_WIFI_SCAN_GET_WIFI_TYPE(ap_list[ii].data_rate_info_byte)),
				AOS_WIFI_SCAN_GET_CHANNEL_ID(ap_list[ii].channel_info_byte),
				_wifi_dr_to_str(AOS_WIFI_SCAN_GET_DATARATE(ap_list[ii].data_rate_info_byte)),
				_wifi_origin_to_str(AOS_WIFI_SCAN_GET_MAC_ORIGIN(ap_list[ii].data_rate_info_byte)));
	}
}


