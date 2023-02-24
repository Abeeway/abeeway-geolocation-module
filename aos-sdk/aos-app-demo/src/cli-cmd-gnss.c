/*!
 * @file cli-cmd-gnss.c
 *
 * @brief GNSS CLI commands
 *
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

#include "srv_gnss.h"
#include "aos_nmea_parser.h" // For nmea_dump_raw
#include "srv_cli.h"

// Maximum number of simultaneous constellations
#define MAX_MGMT_CONSTELLATION 3

#define MAX_SATS_FOR_PRN_DISPLAY AOS_GNSS_MAX_SATELLITES


// Command for restart
typedef enum {
	gnss_opt_restart_full,
	gnss_opt_restart_cold,
	gnss_opt_restart_warm,
	gnss_opt_restart_hot,
	gnss_opt_restart_short_help,
	gnss_opt_restart_help,
	gnss_opt_restart_count
} gnss_restart_opt_t;

typedef struct {
	const char* name;
	uint32_t value;
} gnss_conversion_t;

#define MAX_GNSS_CONVERGENCE_MEASURE  8
// Measurement 0: no threshold
// Measurement 1: ehphe < 50m
// Measurement 2: ehphe < 25m
// Measurement 3: ehphe < 20m
// Measurement 4: ehphe < 15m
// Measurement 5: ehphe < 10m
// Measurement 6: ehphe < 5m
// Measurement 7: ehphe < 3m
// Measurement given in cm
#define GNSS_EHPE_MEASURE0_THRESHOLD 10000
#define GNSS_EHPE_MEASURE1_THRESHOLD 5000
#define GNSS_EHPE_MEASURE2_THRESHOLD 2500
#define GNSS_EHPE_MEASURE3_THRESHOLD 2000
#define GNSS_EHPE_MEASURE4_THRESHOLD 1500
#define GNSS_EHPE_MEASURE5_THRESHOLD 1000
#define GNSS_EHPE_MEASURE6_THRESHOLD 500
#define GNSS_EHPE_MEASURE7_THRESHOLD 300


static const gnss_conversion_t _gnss_measure_threshold[MAX_GNSS_CONVERGENCE_MEASURE + 1] = {
		{"None", GNSS_EHPE_MEASURE0_THRESHOLD},
		{"< 50m", GNSS_EHPE_MEASURE1_THRESHOLD},
		{"< 25m", GNSS_EHPE_MEASURE2_THRESHOLD},
		{"< 20m", GNSS_EHPE_MEASURE3_THRESHOLD},
		{"< 15m", GNSS_EHPE_MEASURE4_THRESHOLD},
		{"< 10m", GNSS_EHPE_MEASURE5_THRESHOLD},
		{"<  5m", GNSS_EHPE_MEASURE6_THRESHOLD},
		{"<  3m", GNSS_EHPE_MEASURE7_THRESHOLD},
		{NULL,0}
};


static const gnss_conversion_t _gnss_conv_sync [] = {
		{"none", 0},
		{"time", AOS_GNSS_SAT_SYNC_TIME},
		{"bit", AOS_GNSS_SAT_SYNC_SET_STATE(AOS_GNSS_SAT_SYNC_BIT)},
		{"frame", AOS_GNSS_SAT_SYNC_SET_STATE(AOS_GNSS_SAT_SYNC_FRAME)},
		{"exact", AOS_GNSS_SAT_SYNC_SET_STATE(AOS_GNSS_SAT_SYNC_EXACT)},
		{NULL,0}
};



// GNSS convergence
typedef struct {
	uint64_t time;
	uint32_t ehpe;
} gnss_convergence_t;


// GNSS almanac
// Command help
#define READ_ALMANAC_HELP "[gps | beidou] <first sat> <last sat>. Read the almanac"
#define SHOW_ALMANAC_HELP "[gps | beidou]. show the almanac"

// Number of satellites in the constellations
#define ALM_GPS_NB_SAT			32
#define ALM_GLONASS_NB_SAT		24
#define ALM_BEIDOU_NB_SAT		35

// General constant
#define BASE_LIBC_YEAR			1900
#define SEC_PER_WEEK			(7*24*3600)
#define MAX_SIZE_ALM_BUFFER						30

// Identifier Requests/answers for a Mediatek device
#define MEDIATEK_QUERY_GPS_ALMANAC			474
#define MEDIATEK_RESPONSE_GPS_ALMANAC		711
#define MEDIATEK_QUERY_BEIDOU_ALMANAC		494
#define MEDIATEK_RESPONSE_BEIDOU_ALMANAC	494

typedef enum {
	gnss_request_none = 0,			// No request in progress
	gnss_request_read_gps_alm,		// GPS almanac read in progress
	gnss_request_read_beidou_alm,	// BEIDOU almanac read in progress
} gnss_alm_request_type_t;

// Mediatek Almanac management
typedef struct {
	uint32_t last_read_gps_alm;					// Last uptime in sec we retrieved the gps almanac
	uint32_t last_read_beidou_alm;				// Last uptime in sec we retrieved the beidou almanac
	gnss_alm_request_type_t rqst_type;			// Request in progress for the almanac
	uint16_t first_sid;							// First satellite ID for the read request
	uint16_t last_sid;							// Last satellite ID for the request
	uint16_t gps_alm_last_upd[ALM_GPS_NB_SAT];		// GPS almanac last update (week number)
	uint16_t beidou_alm_last_upd[ALM_BEIDOU_NB_SAT];// BEIDOU almanac last update (week number)
	char buffer[MAX_SIZE_ALM_BUFFER];
} gnss_alm_mgmt_t;

typedef struct {
	uint64_t start_time;
	uint8_t nb_measures;
	gnss_convergence_t measures[MAX_GNSS_CONVERGENCE_MEASURE];
} gnss_measure_t;

static struct {
	bool				raw_monitor;
	bool				agps_monitor;
	bool                nav_monitor;
	bool                fix_monitor;
	bool                drv_open;
	bool                drv_ready;
	uint8_t             msg_filter;
	aos_gnss_fix_info_t gnss_fix;
	uint64_t 			last_fix_time;
	aos_gnss_track_data_t gnss_track[MAX_MGMT_CONSTELLATION];
	aos_gnss_satellite_prn_report_t gnss_prn_report;
	aos_gnss_constellation_t c_mgmt[MAX_MGMT_CONSTELLATION];
	gnss_measure_t       measure;
	aos_gnss_cfg_local_info_t  local_info;
	gnss_alm_mgmt_t	alm;
} lctx = {0};

static const char* _pmtk_prefix = "PMTK";


static const char* _gnss_event_to_str[aos_gnss_event_count] = {
	"Error",
	"power on",
	"power off",
	"power standby",
	"GNSS ready",
	"Trigger RX",
	"fix receive",
	"nav data",
	"pseudo range",
	"raw OSP",
	"raw MMEA"
};


static const char* _constellation_to_str[aos_gnss_constellation_count] = {
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

static char* _format_aos_date(uint32_t now)
{
	uint8_t s, m, h;

	s = now % 60;
	now /= 60;
	m = now % 60;
	now /= 60;
	h = now % 24;
	now /= 24;
	if (now) {
		snprintf(lctx.alm.buffer, MAX_SIZE_ALM_BUFFER, "%lud,%02u:%02u:%02u", (int32_t)(now % 0xffffffff), h, m, s);
	} else {
		snprintf(lctx.alm.buffer, MAX_SIZE_ALM_BUFFER, "%02u:%02u:%02u", h, m, s);
	}
	return lctx.alm.buffer;
}

static void _process_alm_answer(nmea_parse_msg_t* msg)
{
	uint32_t sid;
	uint32_t week;
	uint32_t max_sats;

	switch (lctx.alm.rqst_type) {
	case gnss_request_read_gps_alm:
	case gnss_request_read_beidou_alm:
		// Read the satellite ID
		if (!nmea_get_hex(msg, 1, &sid)) {
			cli_printf("Cannot retrieve the satellite ID\n");
			return;
		}
		if (lctx.alm.rqst_type == gnss_request_read_gps_alm) {
			max_sats = ALM_GPS_NB_SAT;
		} else {
			max_sats = ALM_BEIDOU_NB_SAT;
		}
		if (sid >= max_sats) {
			cli_printf("Invalid satellite ID %ld\n", sid);
			return;
		}
		// Read the week number
		if (!nmea_get_hex(msg, 2, &week)) {
			cli_printf("Cannot retrieve the week for sid %ld\n", sid);
			return;
		}
		if (lctx.alm.rqst_type == gnss_request_read_gps_alm) {
			week = week % 1024;
			lctx.alm.gps_alm_last_upd[sid] = week;
		} else {
			lctx.alm.beidou_alm_last_upd[sid] = week;
		}
		break;

	default:
		break;
	}
}

static void _request_alm_done()
{
	lctx.alm.rqst_type =  gnss_request_none;
	cli_printf("GNSS request done\n");
}

static void _request_alm_continue(void)
{
	uint32_t rqst = 0;
	uint32_t rsp = 0;
	uint32_t cur_time;


	if (lctx.alm.first_sid > lctx.alm.last_sid) {
		// We are done
		_request_alm_done();
		return;
	}

	// Time in sec
	cur_time = (pdMS_TO_TICKS(xTaskGetTickCount())/1000);

	switch (lctx.alm.rqst_type) {
	case gnss_request_none:
		return;

	case gnss_request_read_gps_alm:
		rqst = MEDIATEK_QUERY_GPS_ALMANAC;
		rsp = MEDIATEK_RESPONSE_GPS_ALMANAC;
		lctx.alm.last_read_gps_alm = cur_time;
		break;

	case gnss_request_read_beidou_alm:
		rqst = MEDIATEK_QUERY_BEIDOU_ALMANAC;
		rsp = MEDIATEK_RESPONSE_BEIDOU_ALMANAC;
		lctx.alm.last_read_beidou_alm = cur_time;
		break;

	}
	snprintf(lctx.alm.buffer, MAX_SIZE_ALM_BUFFER, "%s%lu,%u", _pmtk_prefix, rqst, lctx.alm.first_sid );
	if (srv_gnss_send_query((uint8_t*)lctx.alm.buffer, strlen(lctx.alm.buffer), rsp)) {
		cli_printf("Send request fails\n");
		return;
	}
	lctx.alm.first_sid ++;
}


static void _display_common_alm(const char* constellation, uint32_t last_alm_read)
{
	uint32_t cur_time = (pdMS_TO_TICKS(xTaskGetTickCount())/1000);

	cli_printf("%s almanac validity. Last read: %s", constellation, _format_aos_date(last_alm_read));
	cli_printf(" (%s before now)\n",_format_aos_date(cur_time - last_alm_read));
	cli_printf(" %-15s%-15s%s\n", "Satellite", "Week", "Acquisition");
}

static void _display_gps_alm(void)
{
	// Actual date of the roll over
#define GPS_WEEK_ROLLOVER_YEAR	2019
#define GPS_WEEK_ROLLOVER_MONTH	4
#define GPS_WEEK_ROLLOVER_DAY	7
	uint32_t ii;
	time_t rollover_time;
	time_t alm_time;
	struct tm date;

	memset(&date, 0, sizeof(date));
	date.tm_year = GPS_WEEK_ROLLOVER_YEAR - BASE_LIBC_YEAR;
	date.tm_mon = GPS_WEEK_ROLLOVER_MONTH;
	date.tm_mday = GPS_WEEK_ROLLOVER_DAY;
	rollover_time = mktime(&date);

	_display_common_alm("GPS", lctx.alm.last_read_gps_alm);

	for (ii=0; ii < ALM_GPS_NB_SAT; ii ++) {
		alm_time = rollover_time + (lctx.alm.gps_alm_last_upd[ii] * SEC_PER_WEEK);
		localtime_r(&alm_time, &date);
		cli_printf(" %-15d%-15d", ii, lctx.alm.gps_alm_last_upd[ii]);
		if (!lctx.alm.gps_alm_last_upd[ii]) {
			cli_printf("---\n");
		} else {
			cli_printf("%04d/%02d/%02d\n", date.tm_year + BASE_LIBC_YEAR, date.tm_mon, date.tm_mday);
		}
	}
}

static void _display_beidou_alm(void)
{
	// Actual date of the roll over
#define BEIDOU_BASE_YEAR	2006
#define BEIDOU_BASE_MONTH	1
#define BEIDOU_BASE_DAY	1
	uint32_t ii;
	time_t base_time;
	time_t alm_time;
	struct tm date;

	memset(&date, 0, sizeof(date));
	date.tm_year = BEIDOU_BASE_YEAR - BASE_LIBC_YEAR;
	date.tm_mon = BEIDOU_BASE_MONTH;
	date.tm_mday = BEIDOU_BASE_DAY;
	base_time = mktime(&date);

	_display_common_alm("BEIDOU", lctx.alm.last_read_beidou_alm);

	for (ii=0; ii < ALM_BEIDOU_NB_SAT; ii ++) {
		alm_time = base_time + (lctx.alm.beidou_alm_last_upd[ii] * SEC_PER_WEEK);
		localtime_r(&alm_time, &date);
		cli_printf(" %-15d%-15d", ii, lctx.alm.beidou_alm_last_upd[ii]);
		if (!lctx.alm.beidou_alm_last_upd[ii]) {
			cli_printf("---\n");
		} else {
			cli_printf("%04d/%02d/%02d\n", date.tm_year + BASE_LIBC_YEAR, date.tm_mon, date.tm_mday);
		}
	}
}


static const char* _gnss_request_status_name(aos_gnss_request_status_t status)
{
	static const gnss_conversion_t _conv[] = {
			{"success", aos_gnss_request_status_success},
			{"answer", aos_gnss_request_status_answer},
			{"nack", aos_gnss_request_status_nack},
			{"timeout", aos_gnss_request_status_timeout},
			{"failure", aos_gnss_request_status_failure},
			{"cmd invalid", aos_gnss_request_status_ack_cmd_error},
			{"cmd no action", aos_gnss_request_status_ack_ok_no_action},
			{"cmd action ok", aos_gnss_request_status_ack_ok_action_ok},
			{NULL, 0}
	};

	return _conversion_to_name(_conv, status);
}


static const char* _gnss_constellation_name(aos_gnss_constellation_t constellation)
{
	if (constellation >= aos_gnss_constellation_count) {
		return "";
	}
	return _constellation_to_str[constellation];
}

static const char* _gnss_status_name(aos_gnss_fix_status_t status)
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


static const char* _gnss_local_info_type_name(aos_gnss_local_info_type_t type)
{
	static const gnss_conversion_t _conv[] = {
		{"None", aos_gnss_loc_info_type_none},
		{"Time", aos_gnss_loc_info_type_time},
		{"Time/position", aos_gnss_loc_info_type_pos},
		{NULL, 0}
	};

	return _conversion_to_name(_conv, type);
}

static const char* _gnss_event_name(aos_gnss_event_t event)
{
	if (event >= aos_gnss_event_count) {
		return "";
	}
	return _gnss_event_to_str[event];
}


static void _gnss_dump_time_info(aos_gnss_time_info_t* info)
{
	switch (info->type) {
	case aos_gnss_time_type_software:
		cli_printf("SW TIME: %lld.%06lld", info->time / 1000000, info->time % 1000000);
		break;
	case aos_gnss_time_type_week_tow:
		cli_printf("TOW: %lld.%06lld, week:%d",
				info->gnss_week_tow.tow / 1000000,
				info->gnss_week_tow.tow % 1000000,
				info->gnss_week_tow.week);
		break;
	case aos_gnss_time_type_utc:
		cli_printf("UTC: %02d/%02d/%02d %02d:%02d:%02d.%03d",
				info->utc.month, info->utc.day,info->utc.year,info->utc.hour,info->utc.min,
				info->utc.sec, info->utc.msec);
		break;
	}
}

static const char* _gnss_sync_name(uint8_t sync)
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


static int8_t _sync_str_to_value(const char* str)
{
	const gnss_conversion_t* cv = _gnss_conv_sync;

	while (cv->name) {
		if (!strcmp(cv->name, str)) {
			return cv->value;
		}
		cv ++;
	}
	return -1;
}


static void _gnss_dump_fix(aos_gnss_fix_info_t* fix)
{
	cli_printf( "GNSS-FIX-%s: ", _gnss_constellation_name(fix->constellation));
	_gnss_dump_time_info(&fix->gnss_time);
	cli_printf(", S:%s, %d sats - %d.%07d %d.%07d ALT=%d, ehpe:%d.%02d, hdop:%d.%01d, sats={",
			 _gnss_status_name(fix->status),
			fix->fix_satellites,
			fix->lat / 10000000,
			abs(fix->lat) % 10000000,
			fix->lon / 10000000,
			abs(fix->lon) % 10000000,
			fix->alt,
			fix->ehpe / 100,
			fix->ehpe % 100,
			(fix->hdop * 2) / 10,
			(fix->hdop * 2) % 10);
	if (!fix->fix_satellites) {
		cli_printf("}\n");
		return;
	}
	for (uint8_t ii = 0; ii < fix->fix_satellites; ii++) {
		if (ii == fix->fix_satellites - 1) {
			cli_printf("%d}\n", fix->sats_for_fix[ii]);
		} else {
			cli_printf("%d, ", fix->sats_for_fix[ii]);
		}
	}
}


static void _gnss_show_fix(aos_gnss_fix_info_t* fix)
{
	uint32_t time_msec;
	uint8_t ii;

	if (fix->status == aos_gnss_fix_invalid) {
		cli_printf("No valid GNSS fix\n");
		return;
	}
	cli_printf("GNSS fix\n");
	cli_printf(" Status: %s\n", _gnss_status_name(fix->status));
	cli_printf(" UTC time: ");
	_gnss_dump_time_info(&fix->gnss_time);
	cli_printf("\n");
	cli_printf(" Latitude: %d.%07d\n", fix->lat / 10000000, abs(fix->lat) % 10000000);
	cli_printf(" Longitude: %d.%07d\n", fix->lon / 10000000, abs(fix->lon) % 10000000);
	cli_printf(" Altitude: %d.%02d\n", fix->alt/100, fix->alt%100);
	cli_printf(" Constellation: %s\n", _gnss_constellation_name(fix->constellation));
	cli_printf(" EHPE: %d.%02d m\n", fix->ehpe / 100, fix->ehpe % 100);
	cli_printf(" HDOP: %d.%01d\n", (fix->hdop * 2) / 10,(fix->hdop * 2) % 10);
	cli_printf(" VDOP: %d.%02d\n", fix->vdop/100, fix->vdop%100);
	cli_printf(" PDOP: %d.%02d\n", fix->pdop/100, fix->pdop%100);
	cli_printf(" COG: %d.%02d degree\n", fix->cog/100, fix->cog%100);
	cli_printf(" SOG: %d.%02d m/s\n", fix->sog/100, fix->sog%100);
	cli_printf(" Number of satellites used for fix: %d\n", fix->fix_satellites);
	cli_printf(" Number of tracked satellites: %d\n", fix->track_satellites);
	cli_printf(" Satellites used for fix: {");
	for (uint8_t ii = 0; ii < fix->fix_satellites; ii++) {
		if (ii == fix->fix_satellites - 1) {
			cli_printf("%d}\n", fix->sats_for_fix[ii]);
		} else {
			cli_printf("%d, ", fix->sats_for_fix[ii]);
		}
	}
	time_msec = lctx.measure.measures[0].time - lctx.measure.start_time;
	cli_printf(" TTFF: %d.%03d\n", time_msec/1000, time_msec %1000);

	cli_printf(" Convergence\n");
	for (ii=1; ii < MAX_GNSS_CONVERGENCE_MEASURE; ii++) {
		if (ii >= lctx.measure.nb_measures) {
			cli_printf("  Criteria: %s,  Time: NA, EHPE: NA\n",
					_gnss_measure_threshold[ii].name);
			continue;
		}
		time_msec = lctx.measure.measures[ii].time - lctx.measure.start_time;
		cli_printf("  Criteria: %s,  Time: %02d.%03d, EHPE: %d.%02d\n",
				_gnss_measure_threshold[ii].name,
				time_msec/1000, time_msec %1000,
				lctx.measure.measures[ii].ehpe / 100,
				lctx.measure.measures[ii].ehpe % 100);
	}
}


static void _gnss_dump_prn(aos_gnss_satellite_prn_report_t* prn)
{
	aos_gnss_satellite_prn_t* sat;
	uint8_t max_display;
	uint8_t ii;

	if ( prn->nb_sat > MAX_SATS_FOR_PRN_DISPLAY) {
		max_display = MAX_SATS_FOR_PRN_DISPLAY;
	} else {
		max_display = prn->nb_sat;
	}

	cli_printf( "GNSS-PRN: ");
	_gnss_dump_time_info(&prn->gnss_time);
	cli_printf(", Nb sat: %d, ", prn->nb_sat);

	for (ii=0; ii < max_display; ii ++) {
		sat = &prn->sat_info[ii];
		cli_printf( "%s/%d/%d/0x%x/%lld.%02d - ",
				_gnss_constellation_name(sat->constellation),
				sat->sv_id,
				sat->cn0,
				sat->sync_flags,
				sat->pseudo_range/100,
				sat->pseudo_range%100);
	}
	if (max_display != prn->nb_sat) {
		cli_printf(" ...\n");
	} else {
		cli_printf("\n");
	}
}


static void _gnss_show_prn(aos_gnss_satellite_prn_report_t* prn_report)
{
	uint8_t ii;
	const char * sync_str;

	cli_printf("GNSS pseudo-range report\n");
	cli_printf(" Time: ");
	_gnss_dump_time_info(&prn_report->gnss_time);
	cli_printf("\n");
	cli_printf(" Number of satellites: %d\n", prn_report->nb_sat);
	if (!prn_report->nb_sat) {
		return;
	}
	cli_printf(" Sv ID     Constellation     C/N0     Pseudo-range     Synchro\n");
	for (ii=0; ii < prn_report->nb_sat; ii ++) {
		sync_str = _gnss_sync_name(prn_report->sat_info[ii].sync_flags);
		cli_printf(" %5d%18s%9d%14lld.%02d",
				prn_report->sat_info[ii].sv_id,
				_gnss_constellation_name(prn_report->sat_info[ii].constellation),
				prn_report->sat_info[ii].cn0,
				prn_report->sat_info[ii].pseudo_range/100,
				prn_report->sat_info[ii].pseudo_range%100);
		cli_printf("%12s\n", sync_str);
	}
}



static void _gnss_dump_track(aos_gnss_track_data_t * track)
{
	uint8_t ii;

	cli_printf( "GNSS-TRACK-%s: ", _gnss_constellation_name(track->constellation));
	cli_printf(", Nb sat: %d, ", track->nb_sat);

	for (ii=0; ii < track->nb_sat; ii++) {
		cli_printf("%d/%d ", track->sat_info[ii].sv_id, track->sat_info[ii].cn0);
	}
	cli_printf("\n");
}


static void _gnss_show_track(aos_gnss_track_data_t* track)
{
	uint8_t ii;
	uint8_t jj;

	cli_printf("GNSS tracking\n");
	for (jj = 0; jj < MAX_MGMT_CONSTELLATION; jj ++, track ++) {
		if (track->constellation == aos_gnss_constellation_unknown) {
			continue;
		}
		cli_printf(" Constellation: %s\n", _gnss_constellation_name(track->constellation));
		cli_printf("  Number of satellites: %d\n", track->nb_sat);
		if (!track->nb_sat) {
			continue;
		}
		cli_printf("  Sv ID     C/N0     Elevation    Azimuth\n");
		for (ii=0; ii < track->nb_sat; ii ++) {
					cli_printf("  %5d%9d%14d%11d\n",
					track->sat_info[ii].sv_id, track->sat_info[ii].cn0,
					track->sat_info[ii].elevation, track->sat_info[ii].azimuth);
		}
	}
}


static void _gnss_set_msg_filter(uint8_t mask)
{
	aos_gnss_ioctl_t rqst;
	aos_result_t result;

	rqst.req = aos_gnss_ioctl_req_set_mask;
	rqst.value = mask;
	result = srv_gnss_ioctl(aos_gnss_type_mt3333, &rqst);
	if (result != aos_result_success) {
		cli_printf("Sending ioctl to gps driver fails (%d)", result);
	}
}


static bool _is_gnss_open(void)
{
	if ((!lctx.drv_open) || (!lctx.drv_ready)) {
		cli_printf("Command aborted: driver not %s\n", lctx.drv_open?"ready":"open");
		return false;
	}
	return true;
}


static void _clear_info(void)
{
	memset(&lctx.gnss_fix, 0, sizeof(lctx.gnss_fix));
	memset(&lctx.gnss_prn_report, 0, sizeof(lctx.gnss_prn_report));
	memset(&lctx.gnss_track, 0, sizeof(lctx.gnss_track));
	memset(&lctx.measure, 0, sizeof(lctx.measure));
	lctx.measure.start_time = pdMS_TO_TICKS(xTaskGetTickCount());
}


static void _gnss_store_local_info(void)
{
	struct tm date_time;

	lctx.local_info.lat = lctx.gnss_fix.lat;
	lctx.local_info.lon = lctx.gnss_fix.lon;
	lctx.local_info.alt = lctx.gnss_fix.alt;

	memset(&date_time, 0, sizeof(date_time));
	date_time.tm_year = lctx.gnss_fix.gnss_time.utc.year; // Year 2000 based!
	date_time.tm_mon = lctx.gnss_fix.gnss_time.utc.month;
	date_time.tm_mday = lctx.gnss_fix.gnss_time.utc.day;
	date_time.tm_hour = lctx.gnss_fix.gnss_time.utc.hour;
	date_time.tm_min = lctx.gnss_fix.gnss_time.utc.min;
	date_time.tm_sec = lctx.gnss_fix.gnss_time.utc.sec;
	lctx.local_info.time = mktime(&date_time);
}

#define MAX_GNSS_BYTES_TO_DUMP 200
static char _nmea_raw_display[MAX_GNSS_BYTES_TO_DUMP + 1];

static void _gnss_dump_raw_nmea(nmea_parse_msg_t* info)
{
	uint16_t len;

	_nmea_raw_display[0] = '\0';
	nmea_dump_raw(info, _nmea_raw_display, MAX_GNSS_BYTES_TO_DUMP-2);
	_nmea_raw_display[MAX_GNSS_BYTES_TO_DUMP] = '\0';

	// Check if we have the LF
	len = strlen(_nmea_raw_display);
	if (len) {
		if (_nmea_raw_display[len -1] != '\n') {
			_nmea_raw_display[len -1] = '\n';
			_nmea_raw_display[MAX_GNSS_BYTES_TO_DUMP] = '\0';
		}
	}
	cli_printf("GNSS RX(%3u): %s", len, _nmea_raw_display);
}

static void _send_local_info(void)
{
	aos_gnss_ioctl_t ioctl;
	uint32_t time_delta_sec;
	uint64_t time_delta_msec;

	// Get the elapsed time.
	time_delta_msec = pdMS_TO_TICKS(xTaskGetTickCount()) - lctx.last_fix_time;
	time_delta_sec = time_delta_msec / 1000;
	// Round instead of truncate
	if ((time_delta_msec % 1000) >= 500) {
		time_delta_sec ++;
	}
	lctx.local_info.time += time_delta_sec;

	// Send ioctl
	memcpy(&ioctl.local_info, &lctx.local_info, sizeof(ioctl.local_info));

	ioctl.req = aos_gnss_ioctl_req_set_local_info;
	if (srv_gnss_ioctl(aos_gnss_type_mt3333, &ioctl) != aos_result_success) {
		cli_printf("GNSS ioctl (local_info) fails\n");
	}
}

static uint8_t _get_and_set_constellation_idx(aos_gnss_constellation_t constellation)
{
	uint8_t ii;

	for (ii=0; ii < MAX_MGMT_CONSTELLATION; ii ++) {
		if ((lctx.c_mgmt[ii] == aos_gnss_constellation_unknown) || (lctx.c_mgmt[ii] ==constellation)) {
			lctx.c_mgmt[ii] = constellation;
			return ii;
		}
	}
	// Return default
	return 0;
}


static void _gnss_do_measure(void)
{
	// Check if we have all measure
	if (lctx.measure.nb_measures >= MAX_GNSS_CONVERGENCE_MEASURE) {
		return;
	}

	if (lctx.measure.nb_measures != 0) {
		// Check ehpe against measure threshold
		if (lctx.gnss_fix.ehpe > _gnss_measure_threshold[lctx.measure.nb_measures].value) {
			return;
		}
	}

	lctx.measure.measures[lctx.measure.nb_measures].time = pdMS_TO_TICKS(xTaskGetTickCount());
	lctx.measure.measures[lctx.measure.nb_measures].ehpe = lctx.gnss_fix.ehpe;
	lctx.measure.nb_measures ++;
}

static void _gnss_event_cb(aos_gnss_event_info_t *info, void * user_arg)
{
	uint8_t cidx;
	bool evt_display = true;
	nmea_parse_msg_t* msg;

	if (info->event >= aos_gnss_event_count) {
		cli_printf("GNSS callback. Invalid event %d\n", info->event);
		return;
	}

	switch(info->event) {
	case aos_gnss_event_error:
	case aos_gnss_event_power_on:
	case aos_gnss_event_power_off:
	case aos_gnss_event_power_standby:
		cli_printf("GNSS callback. Event(%d): %s\n",info->event, _gnss_event_name(info->event));
		break;
	case aos_gnss_event_ready:
		if (!lctx.drv_ready) {
			lctx.drv_ready = true;
		} else {
			evt_display = false;
		}
		if (evt_display) {
			cli_printf("GNSS callback. Event(%d): %s\n",info->event, _gnss_event_name(info->event));
		}
		if (lctx.local_info.type != aos_gnss_loc_info_type_none) {
			if (lctx.last_fix_time) {
				_send_local_info();
				cli_printf("Local info (%s) sent\n", _gnss_local_info_type_name(lctx.local_info.type));
			} else {
				cli_printf("Local info set but no previous fix\n");
			}
			// Prevent sending multiple time
			lctx.local_info.type = aos_gnss_loc_info_type_none;
		}
		break;

	case aos_gnss_event_fix:
		lctx.last_fix_time = pdMS_TO_TICKS(xTaskGetTickCount());
		memcpy(&lctx.gnss_fix, info->fix, sizeof(aos_gnss_fix_info_t));
		_gnss_store_local_info();
		// Do measure
		_gnss_do_measure();
		if (lctx.fix_monitor) {
			_gnss_dump_fix(info->fix);
		}
		break;

	case aos_gnss_event_track_data:
		cidx = _get_and_set_constellation_idx(info->track->constellation);
		memcpy(&lctx.gnss_track[cidx], info->track, sizeof(aos_gnss_track_data_t));
		if (lctx.nav_monitor) {
			_gnss_dump_track(info->track);
		}
		break;

	case aos_gnss_event_pseudo_range:
		memcpy(&lctx.gnss_prn_report, info->prn_report, sizeof(aos_gnss_satellite_prn_report_t));
		if (lctx.agps_monitor) {
			_gnss_dump_prn(info->prn_report);
		}
		break;

	case aos_gnss_event_raw_nmea_sentence:
		if (lctx.raw_monitor) {
			_gnss_dump_raw_nmea(info->raw.parsed_msg);
		}
		break;

	case aos_gnss_event_req_status:
		if (lctx.alm.rqst_type != gnss_request_none) {
			msg = info->req_info.raw.parsed_msg;
			if (info->req_info.status == aos_gnss_request_status_ack_ok_no_action) {
				// No data
				lctx.alm.gps_alm_last_upd[lctx.alm.first_sid] = 0;
				// Continue
				_request_alm_continue();
				break;
			}
			if ((msg) && (info->req_info.status == aos_gnss_request_status_success)) {
				// Answer received
				_process_alm_answer(msg);
				// Process next satellite if any
				_request_alm_continue();
			} else {
				// Stop here
				cli_printf("GNSS Error: %s\n", _gnss_request_status_name(info->req_info.status));
				_request_alm_done();
			}
			break;
		}
		cli_printf("GNSS callback. Request status: %s.\n", _gnss_request_status_name(info->req_info.status));
		if (info->req_info.raw.parsed_msg) {
			_gnss_dump_raw_nmea(info->req_info.raw.parsed_msg);
		}
		break;

	case aos_gnss_event_trigger_rx_delayed:
	case aos_gnss_event_count:
		break;
	}
}

static cli_parser_status_t _cmd_gnss_on(void* arg, int argc, char *argv[])
{
	if (srv_gnss_set_power(aos_gnss_type_mt3333, aos_gnss_power_on) == aos_result_success) {
		_clear_info();
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_gnss_off(void* arg, int argc, char *argv[])
{
	aos_result_t result;

	result = srv_gnss_close();
	if ((result == aos_result_not_init) || (result == aos_result_not_open)) {
		// Driver was not open. So force power to OFF
		cli_printf("GNSS not open. Force power off.\n");
		srv_gnss_set_power(aos_gnss_type_mt3333, aos_gnss_power_off);
	}
	lctx.drv_open = false;
	lctx.drv_ready = false;
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_gnss_standby(void* arg, int argc, char *argv[])
{
	if (srv_gnss_set_power(aos_gnss_type_mt3333, aos_gnss_power_standby) == aos_result_success) {
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_gnss_version(void* arg, int argc, char *argv[])
{
	const char* version_req = "PMTK605";
#define PMTK_VERSION_ANSWER	705

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	srv_gnss_send_query((uint8_t*)version_req, strlen(version_req), PMTK_VERSION_ANSWER);
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_gnss_monitor(void* arg, int argc, char *argv[])
{
	uint8_t mask;


	enum {opt_off, opt_debug, opt_fix, opt_track, opt_prn, opt_short_help, opt_help, opt_count };
	static const cli_cmd_option_t options[] = {
			{ "off", opt_off },
			{ "debug", opt_debug },
			{ "fix", opt_fix },
			{ "track", opt_track },
			{ "prn", opt_prn},
			{ "?", opt_short_help },
			{ "help", opt_help },
	};

	if (argc < 2) {
		cli_printf("Mandatory option missing\n");
		return cli_parser_status_error;
	}

	// by default allow relevant fields
	mask = AOS_GNSS_MSG_MSK_ALLOW_FIX | AOS_GNSS_MSG_MSK_ALLOW_TRACK | AOS_GNSS_MSG_MSK_ALLOW_PSEUDO_RANGE;

	// Clear monitoring
	lctx.raw_monitor = false;
	lctx.agps_monitor = false;
	lctx.fix_monitor = false;
	lctx.nav_monitor = false;

	for (int argn = 1; argn < argc; ++argn) {

		char *option = argv[argn];

		switch(cli_get_option_index(options, opt_count, option)) {

		case cli_cmd_option_index_not_found:
			cli_printf("Unknown option: %s\n", option);
			return cli_parser_status_error;

		case cli_cmd_option_index_ambiguous:
			cli_printf("Ambiguous option: %s\n", option);
			return cli_parser_status_error;

		case opt_off:
			break;

		case opt_debug:
			mask |= AOS_GNSS_MSG_MSK_ALLOW_ALL_RAW | AOS_GNSS_MSG_MSK_ALLOW_UNKNOWN;
			lctx.raw_monitor = true;
			lctx.agps_monitor = true;
			lctx.fix_monitor = true;
			lctx.nav_monitor = true;
			break;

		case opt_fix:
			lctx.fix_monitor = true;
			break;

		case opt_track:
			lctx.nav_monitor = true;
			break;

		case opt_prn:
			lctx.agps_monitor = true;
			break;

		case opt_short_help:
			for (int i = 0; i < opt_count; ++i) {
				cli_printf("%s ", options[i].name);
			}
			cli_printf("\n");
			return cli_parser_status_ok;

		case opt_help:
			cli_printf("usage: gps monitor [options]\n");
			cli_printf("options are:\n");
			cli_printf("    off         Stop the monitoring\n");
			cli_printf("    debug       Display all messages in raw format and other information\n");
			cli_printf("    fix         Display fix information\n");
			cli_printf("    track       Display track information\n");
			cli_printf("    prn         Display pseudo-ranges information\n");
			return cli_parser_status_ok;

		default:	// Should not happen, really, unless there is a program error.
			cli_printf("Invalid option '%s'\n", option);
			return cli_parser_status_error;
		}
	}


	// Setup the new filtering scheme
	lctx.msg_filter = mask;
	_gnss_set_msg_filter(lctx.msg_filter);

	return cli_parser_status_ok;
}


#define MIN_C_N_PRN_FILTER	15
#define MIN_SYNC_PRN_FILTER	AOS_GNSS_SAT_SYNC_BIT
const aos_gnss_configuration_t _gnss_config = {
		.constellations = aos_gnss_cfg_constellation_gps_glonass_galileo,
		.cbfn = _gnss_event_cb,
		.cbarg = NULL,
		.fix_period = AOS_GNSS_MIN_GPS_FIX_PERIOD,
		.track_period = AOS_GNSS_MIN_TRACKING_PERIOD,
		.filter_mask = AOS_GNSS_MSG_MSK_ALLOW_FIX | AOS_GNSS_MSG_MSK_ALLOW_TRACK,
		.prn_filter.min_cn = MIN_C_N_PRN_FILTER,
		.prn_filter.min_sync = MIN_SYNC_PRN_FILTER
};


static cli_parser_status_t _cmd_gnss_open(void* arg, int argc, char *argv[])
{
	// Check arg
	lctx.local_info.type = aos_gnss_loc_info_type_none;
	if (argc > 1) {
		if (!strcmp(argv[1], "time")) {
			cli_printf("Time information will be provided to the chip\n");
			lctx.local_info.type = aos_gnss_loc_info_type_time;
		} else if (!strcmp(argv[1], "position")) {
			cli_printf("Time and position information will be provided to the chip\n");
			lctx.local_info.type = aos_gnss_loc_info_type_pos;
		} else {
			cli_printf("Invalid local information\n");
			return cli_parser_status_error;
		}
	}

	// Flush previous information
	_clear_info();
	memset(&lctx.c_mgmt, 0, sizeof(lctx.c_mgmt));

	if (srv_gnss_open(aos_gnss_type_mt3333, &_gnss_config) == aos_result_success) {
		lctx.drv_open = true;
		// Set the message filtering at the driver level
		lctx.msg_filter = _gnss_config.filter_mask;
		// Allow monitoring fix and nav
		lctx.fix_monitor = true;
		lctx.nav_monitor = true;
		return cli_parser_status_ok;
	}
	cli_printf("Failed to open the GNSS service\n");
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_gnss_send_msg(void* arg, int argc, char *argv[])
{
	if (argc < 2) {
		cli_printf("usage: gps send <string>\n");
		return cli_parser_status_error;
	}
	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	// Send the request
	srv_gnss_send_msg((uint8_t*)argv[1], strlen(argv[1]), aos_gnss_rqst_type_msg);
	return cli_parser_status_ok;
}


static cli_parser_status_t _cmd_gnss_send_cmd(void* arg, int argc, char *argv[])
{
	if (argc < 2) {
		cli_printf("usage: gps cmd <string>\n");
		return cli_parser_status_error;
	}
	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	// Send the request
	srv_gnss_send_msg((uint8_t*)argv[1], strlen(argv[1]), aos_gnss_rqst_type_cmd);
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_gnss_send_query(void* arg, int argc, char *argv[])
{
	int32_t answer_id;
	if (argc < 3) {
		cli_printf("usage: gps query <answer_id> <string>\n");
		return cli_parser_status_error;
	}
	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	// Retrieve the answer ID
	if (!cli_parse_int(argv[1], &answer_id)) {
		cli_printf("<answer_id> must be an integer\n");
		return cli_parser_status_error;
	}

	// Send the query
	srv_gnss_send_query((uint8_t*)argv[2], strlen(argv[2]), answer_id);
	return cli_parser_status_ok;
}


static cli_parser_status_t _cmd_gnss_get_constell(void* arg, int argc, char *argv[])
{
	aos_gnss_ioctl_t ioctl;

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	ioctl.req = aos_gnss_ioctl_req_get_constel;
	if (srv_gnss_ioctl(aos_gnss_type_mt3333, &ioctl) != aos_result_success) {
		cli_printf("GNSS ioctl fails\n");
		return cli_parser_status_error;
	}
	cli_printf("Constellation used: ");

	switch (ioctl.constellation) {
	case aos_gnss_cfg_constellation_gps_only:
		cli_printf("gps only");
		break;
	case aos_gnss_cfg_constellation_glonass_only:
		cli_printf("glonass only");
		break;
	case aos_gnss_cfg_constellation_gps_glonass:
		cli_printf("gps + glonass");
		break;
	case aos_gnss_cfg_constellation_gps_galileo:
		cli_printf("gps + galileo");
		break;
	case aos_gnss_cfg_constellation_gps_glonass_galileo:
		cli_printf("gps + glonass + galileo");
		break;
	case aos_gnss_cfg_constellation_beidou_only:
		cli_printf("beidou only");
		break;
	case aos_gnss_cfg_constellation_gps_beidou:
		cli_printf("gps + beidou");
		break;
	}
	cli_printf("\n");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_gnss_set_constell(void* arg, int argc, char *argv[])
{
	aos_gnss_ioctl_t ioctl;
	uint8_t mask = 0;
	uint8_t i;

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	if (argc < 2) {
		cli_printf("Argument required. Can be gps, glonass, galileo beidou.\n");
		cli_printf("Constellations can be combined. Example: constellation set gps glonass\n");
		return cli_parser_status_error;
	}

	for (i=1; i<argc; ++i) {
		if (!strcmp(argv[i], "gps")) {
			mask |= AOS_GNSS_CFG_ENABLE_GPS;
		} else if (!strcmp(argv[i], "glonass")) {
			mask |= AOS_GNSS_CFG_ENABLE_GLONASS;
		} else if (!strcmp(argv[i], "galileo")) {
			mask |= AOS_GNSS_CFG_ENABLE_GALILEO;
		} else if (!strcmp(argv[i], "beidou")) {
			mask |= AOS_GNSS_CFG_ENABLE_BEIDOU;
		}
	}

	// Check if allowed combination
	if (!((mask == aos_gnss_cfg_constellation_gps_only) ||
		(mask == aos_gnss_cfg_constellation_glonass_only ) ||
	    (mask == aos_gnss_cfg_constellation_gps_glonass) ||
	    (mask == aos_gnss_cfg_constellation_gps_galileo ) ||
	    (mask == aos_gnss_cfg_constellation_gps_glonass_galileo ) ||
	    (mask == aos_gnss_cfg_constellation_beidou_only) ||
	    (mask == aos_gnss_cfg_constellation_gps_beidou))) {
		cli_printf("Combination not allowed\n");
		return cli_parser_status_error;
	}

	ioctl.req = aos_gnss_ioctl_req_set_constel;
	ioctl.constellation = mask;
	if (srv_gnss_ioctl(aos_gnss_type_mt3333, &ioctl) != aos_result_success) {
		cli_printf("GNSS ioctl fails\n");
		return cli_parser_status_error;
	}
	// Flush previous information
	_clear_info();
	memset(&lctx.c_mgmt, 0, sizeof(lctx.c_mgmt));
	return cli_parser_status_ok;
}

static const uint8_t* _gnss_restart_command(gnss_restart_opt_t restart, uint8_t* cmd_len)
{
	// Same length regardless the type
	*cmd_len = strlen("PMTK10x");

	switch(restart) {
	case gnss_opt_restart_full:
		return (const uint8_t*)"PMTK104";

	case gnss_opt_restart_cold:
		return (const uint8_t*)"PMTK103";

	case gnss_opt_restart_warm:
		return (const uint8_t*)"PMTK102";

	case gnss_opt_restart_hot:
		return (const uint8_t*)"PMTK101";

	default:
		break;
	}
	return NULL;
}


static cli_parser_status_t _cmd_gnss_restart(void* arg,  int argc, char *argv[])
{
	const uint8_t* restart_cmd;
	uint8_t cmd_len;
	int restart;
	static const cli_cmd_option_t options[] = {
			{ "full", gnss_opt_restart_full },
			{ "cold", gnss_opt_restart_cold },
			{ "warm", gnss_opt_restart_warm },
			{ "hot", gnss_opt_restart_hot },
			{ "?", gnss_opt_restart_short_help },
			{ "help", gnss_opt_restart_help },
	};
#define PMTK_RESTART_ANSWER		11

	if (argc < 2) {
		cli_printf("Mandatory option missing\n");
		return cli_parser_status_error;
	}

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	for (int argn = 1; argn < argc; ++argn) {

		char *option = argv[argn];
		restart = cli_get_option_index(options, gnss_opt_restart_count, option);
		switch(restart) {

		case cli_cmd_option_index_not_found:
			cli_printf("Unknown option: %s\n", option);
			return cli_parser_status_error;

		case cli_cmd_option_index_ambiguous:
			cli_printf("Ambiguous option: %s\n", option);
			return cli_parser_status_error;

		case gnss_opt_restart_full:
		case gnss_opt_restart_cold:
		case gnss_opt_restart_warm:
		case gnss_opt_restart_hot:
			restart_cmd = _gnss_restart_command(restart, &cmd_len);
			if (!restart_cmd) {
				cli_printf("Command build fails\n");
				return cli_parser_status_error;
			}
			break;

		case gnss_opt_restart_short_help:
			for (int i = 0; i < gnss_opt_restart_count; ++i) {
				cli_printf("%s ", options[i].name);
			}
			cli_printf("\n");
			return cli_parser_status_ok;

		case gnss_opt_restart_help:
			cli_printf("usage: gps restart [options]\n");
			cli_printf("options are:\n");
			cli_printf("    full        Full cold restart\n");
			cli_printf("    cold        Cold restart\n");
			cli_printf("    warm        Warm restart\n");
			cli_printf("    hot         Hot restart\n");
			return cli_parser_status_ok;

		default:	// Should not happen, really, unless there is a program error.
			cli_printf("Invalid option '%s'\n", option);
			return cli_parser_status_error;
		}
	}

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}


	// Send the request
	srv_gnss_send_query(restart_cmd, cmd_len, PMTK_RESTART_ANSWER);
	// Cleanup information
	_clear_info();

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_gnss_get_counters(void* arg, int argc, char *argv[])
{
	aos_gnss_ioctl_t ioctl;

	ioctl.req = aos_gnss_ioctl_req_get_counters;
	if (srv_gnss_ioctl(aos_gnss_type_mt3333, &ioctl) != aos_result_success) {
		cli_printf("GNSS ioctl fails\n");
		return cli_parser_status_error;
	}
	cli_printf("Counters:\n");
	cli_printf(" Valid messages: %u\n", ioctl.counters->msg_valid);
	cli_printf(" Fix cycle resets: %u\n", ioctl.counters->msg_fix_cycle_reset);
	cli_printf(" Too long messages: %u\n", ioctl.counters->msg_too_long);
	cli_printf(" Too much fields: %u\n", ioctl.counters->msg_too_much_fields);
	cli_printf(" Unexpected terminations: %u\n", ioctl.counters->msg_wrong_end);
	cli_printf(" CRC errors: %u\n", ioctl.counters->msg_crc_err);
	cli_printf(" Parser errors: %u\n", ioctl.counters->msg_parser_err);
	cli_printf(" Unknown message: %u\n", ioctl.counters->msg_unknown);
	cli_printf(" Non digit bytes: %u\n", ioctl.counters->non_digit_bytes);
	cli_printf(" No buffers: %u\n", ioctl.counters->no_buffer);
	cli_printf(" Overrun: %u\n", ioctl.counters->overrun);
	return cli_parser_status_ok;
}


static cli_parser_status_t _cmd_gnss_clr_counters(void* arg, int argc, char *argv[])
{
	aos_gnss_ioctl_t ioctl;

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	ioctl.req = aos_gnss_ioctl_req_clr_counters;
	if (srv_gnss_ioctl(aos_gnss_type_mt3333, &ioctl) != aos_result_success) {
		cli_printf("GNSS ioctl fails\n");
		return cli_parser_status_error;
	}
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_gnss_show_fix(void* arg, int argc, char *argv[])
{
	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}
	_gnss_show_fix(&lctx.gnss_fix);
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_gnss_show_prn(void* arg, int argc, char *argv[])
{
	_gnss_show_prn(&lctx.gnss_prn_report);
	return cli_parser_status_ok;
}


static cli_parser_status_t _cmd_gnss_show_track(void* arg, int argc, char *argv[])
{
	_gnss_show_track(lctx.gnss_track);
	return cli_parser_status_ok;
}


static cli_parser_status_t _cmd_gnss_get_prn_filter(void* arg, int argc, char *argv[])
{
	aos_gnss_ioctl_t ioctl;

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	ioctl.req = aos_gnss_ioctl_req_get_prn_filter;
	if (srv_gnss_ioctl(aos_gnss_type_mt3333, &ioctl) != aos_result_success) {
		cli_printf("GNSS ioctl fails\n");
		return cli_parser_status_error;
	}
	cli_printf("PRN filter\n");
	cli_printf(" Min C/N0: %d\n", ioctl.prn_filter.min_cn);
	cli_printf(" Min sync (0x%x): %s\n", ioctl.prn_filter.min_sync, _gnss_sync_name( ioctl.prn_filter.min_sync));
	return cli_parser_status_ok;
}


static cli_parser_status_t _cmd_gnss_set_prn_filter(void* arg, int argc, char *argv[])
{

	aos_gnss_ioctl_t ioctl;
	int value;
	const gnss_conversion_t* cv = _gnss_conv_sync;

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	if (argc !=3 ) {
		cli_printf("Usage: gps prn-filter min_cn min_syn\n");
		return cli_parser_status_error;
	}
	if (!sscanf(argv[1], "%d", &value)) {
		cli_printf("Incorrect argument 1\n");
		return cli_parser_status_error;
	}
	ioctl.prn_filter.min_cn = value;

	value = _sync_str_to_value(argv[2]);
	if (value <0) {
		cli_printf("Incorrect argument 2. Acceptable values:");
		while (cv->name) {
			cli_printf(" %s,", cv->name);
			cv ++;
		}
		cli_printf("\n");
		return cli_parser_status_error;
	}

	ioctl.prn_filter.min_sync = value;

	ioctl.req = aos_gnss_ioctl_req_set_prn_filter;
	if (srv_gnss_ioctl(aos_gnss_type_mt3333, &ioctl) != aos_result_success) {
		cli_printf("GNSS ioctl fails\n");
		return cli_parser_status_error;
	}
	cli_printf("PRN filter updated with min C/No: %d, min Sync: %s (%d)\n",
			ioctl.prn_filter.min_cn,
			_gnss_sync_name(ioctl.prn_filter.min_sync),
			ioctl.prn_filter.min_sync);
	return cli_parser_status_ok;
}


// Almanac management
static const char* _alm_accepted_constellation = "Accepted: gps, beidou";

static cli_parser_status_t _cmd_get_almanac_valid(void *arg, int argc, char **argv)
{
	int32_t sat_id;
	uint32_t max_sat;

	if (argc < 2) {
		cli_print_missing_argument();
		return cli_parser_status_error;
	}

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

	if (lctx.alm.rqst_type !=  gnss_request_none) {
		cli_printf("Request already in progress\n");
		return cli_parser_status_error;
	}

	if (!strcmp(argv[1], "gps")) {
		lctx.alm.rqst_type = gnss_request_read_gps_alm;
		max_sat = ALM_GPS_NB_SAT;
	} else if (!strcmp(argv[1], "beidou")) {
		lctx.alm.rqst_type = gnss_request_read_beidou_alm;
		max_sat = ALM_BEIDOU_NB_SAT;
	} else {
		cli_printf("Invalid constellation. %s\n", _alm_accepted_constellation);
		return cli_parser_status_error;
	}

	if (argc >= 3) {
		if (!cli_parse_int(argv[2], &sat_id)) {
			cli_printf("Invalid satellite ID: %s\n", argv[2]);
			return cli_parser_status_error;
		}
	} else {
		sat_id = 0;
	}

	if (sat_id >= max_sat) {
		cli_printf("Satellite ID %d not in range\n");
		return cli_parser_status_error;
	}

	lctx.alm.first_sid = sat_id;

	if (argc >= 4) {
		if (!cli_parse_int(argv[3], &sat_id)) {
			cli_printf("Invalid satellite ID: %s\n", argv[3]);
			return cli_parser_status_error;
		}
		if (sat_id >= max_sat) {
			return cli_parser_status_error;
		}
	} else {
		sat_id = max_sat - 1;
	}

	if (sat_id >= max_sat) {
		cli_printf("Satellite ID %d not in range\n", sat_id);
		return cli_parser_status_error;
	}

	lctx.alm.last_sid = sat_id;

	// Start
	_request_alm_continue();

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_display_almanac_valid(void *arg, int argc, char **argv)
{
	if (!strcmp(argv[1], "gps")) {
		_display_gps_alm();
	} else if (!strcmp(argv[1], "beidou")) {
		_display_beidou_alm();
	} else {
		cli_printf("Invalid constellation. %s\n", _alm_accepted_constellation);
		return cli_parser_status_error;
	}
	return cli_parser_status_ok;
}


static const cli_parser_cmd_t _cmd_tab_alm[] = {
		PARSER_CMD_FUNC("read", READ_ALMANAC_HELP, _cmd_get_almanac_valid, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("show", SHOW_ALMANAC_HELP, _cmd_display_almanac_valid, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

static const cli_parser_cmd_t _cmd_tab_gnss_constell[] = {
		PARSER_CMD_FUNC("get", "Get the constellation currently used", _cmd_gnss_get_constell, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("set <constellation>", "Set the constellation to use: gps, glonass, galileo, beidou", _cmd_gnss_set_constell, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

static const cli_parser_cmd_t _cmd_tab_gnss_counters[] = {
		PARSER_CMD_FUNC("get", "Read the counters", _cmd_gnss_get_counters, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("clear", "Clear the counters", _cmd_gnss_clr_counters, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

static const cli_parser_cmd_t _cmd_tab_gnss_show[] = {
		PARSER_CMD_FUNC("fix", "Show fix information" ,_cmd_gnss_show_fix, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("track", "Show satellites tracking information", _cmd_gnss_show_track, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("prn",	"Show pseudo-range information", _cmd_gnss_show_prn, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

static const cli_parser_cmd_t _cmd_tab_gnss_prn_filter_tab[] = {
		PARSER_CMD_FUNC("get", "Get PRN filter", _cmd_gnss_get_prn_filter, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("set <arg>", "Set PRN filter. Arg: min_cn min sync",_cmd_gnss_set_prn_filter, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

static const cli_parser_cmd_t _gnss_cmd_table[] = {
		PARSER_CMD_FUNC("open", "Open GNSS service and activate handlers", _cmd_gnss_open, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("on", "Turn GNSS supplies on", _cmd_gnss_on, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("standby", "Turn GNSS main supply off (keepalive)", _cmd_gnss_standby, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("off", "Turn all GNSS power off (reset)", _cmd_gnss_off, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("msg",	"Send a raw message to the GNSS" ,_cmd_gnss_send_msg, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("cmd","Send a command to the GNSS" ,_cmd_gnss_send_cmd, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("query","Send a query to the GNSS" ,_cmd_gnss_send_query, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("show", "Show fix, tracking or PRN",_cmd_tab_gnss_show, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("version", "Get GNSS version in the chip",_cmd_gnss_version, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("monitor", "Setup the GNSS monitoring", _cmd_gnss_monitor, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("almanac","GNSS almanac commands", _cmd_tab_alm, CLI_ACCESS_ALL_LEVELS ),
		PARSER_CMD_TAB("constel", "Get/set constellation", _cmd_tab_gnss_constell, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("restart", "Restart a GNSS acquisition", _cmd_gnss_restart, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("prn-filter", "Get/set PRN filter", _cmd_tab_gnss_prn_filter_tab, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("counters", "Get/clear counters", _cmd_tab_gnss_counters, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};


/*
 * ----------------------------------------------------------------------------
 * Main Commands registration
 * ----------------------------------------------------------------------------
 */
// Register main commands against the CLI parser
CLI_COMMAND_TAB_REGISTER(gnss,"GNSS commands (MT3333)", _gnss_cmd_table, CLI_ACCESS_ALL_LEVELS );

