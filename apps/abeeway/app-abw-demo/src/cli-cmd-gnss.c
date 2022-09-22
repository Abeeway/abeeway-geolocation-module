/*!
 * @file cli-command-gnss.c
 *
 * @brief Simplified GNSS CLI commands
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

// GNSS convergence
typedef struct {
	uint64_t time;
	uint32_t ehpe;
} gnss_convergence_t;

typedef struct {
	uint64_t start_time;
	uint8_t nb_measures;
	gnss_convergence_t measures[MAX_GNSS_CONVERGENCE_MEASURE];
} gnss_measure_t;

static struct {
	bool				raw_monitor : 1;
	bool				agps_monitor : 1;
	bool                nav_monitor : 1;
	bool                fix_monitor : 1;
	bool                drv_open    : 1;
	bool                drv_ready   : 1;
	bool                test_running : 1;
	uint8_t             msg_filter;
	aos_gnss_fix_info_t gnss_fix;
	uint64_t 			last_fix_time;
	aos_gnss_track_data_t gnss_track[MAX_MGMT_CONSTELLATION];
	aos_gnss_constellation_t c_mgmt[MAX_MGMT_CONSTELLATION];
	gnss_measure_t       measure;
	aos_gnss_cfg_local_info_t  local_info;
} lctx = {0};

static const char* _gnss_event_to_str[aos_gnss_event_count] = {
	"Error",
	"power on",
	"power off",
	"power standby",
	"GNSS ready",
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


static const char* _gnss_request_status_name(aos_gnss_request_status_t status)
{
	static const gnss_conversion_t _conv[] = {
			{"success", aos_gnss_request_status_success},
			{"answer", aos_gnss_request_status_answer},
			{"nack", aos_gnss_request_status_nack},
			{"timeout", aos_gnss_request_status_timeout},
			{"failure", aos_gnss_request_status_failure},
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
	memset(&lctx.gnss_track, 0, sizeof(lctx.gnss_track));
	memset(&lctx.measure, 0, sizeof(lctx.measure));
	lctx.measure.start_time = xTaskGetTickCount();
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

	if (lctx.msg_filter & AOS_GNSS_MSG_MSK_ALLOW_UNKNOWN) {
		// Answer of the request has been done. Clear it
		lctx.msg_filter &= ~AOS_GNSS_MSG_MSK_ALLOW_UNKNOWN;
		_gnss_set_msg_filter(lctx.msg_filter);
	}
}

static void _send_local_info(void)
{
	aos_gnss_ioctl_t ioctl;
	uint32_t time_delta_sec;
	uint64_t time_delta_msec;

	// Get the elapsed time.
	time_delta_msec = xTaskGetTickCount() - lctx.last_fix_time;
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

	lctx.measure.measures[lctx.measure.nb_measures].time = xTaskGetTickCount();
	lctx.measure.measures[lctx.measure.nb_measures].ehpe = lctx.gnss_fix.ehpe;
	lctx.measure.nb_measures ++;
}

static void _gnss_event_cb(aos_gnss_event_info_t *info, void * user_arg)
{
	uint8_t cidx;
	bool evt_display = true;

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
		lctx.last_fix_time = xTaskGetTickCount();
		memcpy(&lctx.gnss_fix, info->fix, sizeof(aos_gnss_fix_info_t));
		_gnss_store_local_info();
		// Do measure
		_gnss_do_measure();
		if (lctx.fix_monitor) {
			_gnss_dump_fix(info->fix);
		}
		// Stop navigation monitoring
		lctx.nav_monitor = false;
		break;

	case aos_gnss_event_track_data:
		cidx = _get_and_set_constellation_idx(info->track->constellation);
		memcpy(&lctx.gnss_track[cidx], info->track, sizeof(aos_gnss_track_data_t));
		if (lctx.nav_monitor) {
			_gnss_dump_track(info->track);
		}
		break;

	case aos_gnss_event_pseudo_range:
		break;

	case aos_gnss_event_raw_nmea_sentence:
		if ((lctx.raw_monitor) || (lctx.msg_filter & AOS_GNSS_MSG_MSK_ALLOW_UNKNOWN)){
			_gnss_dump_raw_nmea(info->raw.parsed_msg);
		}
		break;

	case aos_gnss_event_req_status:
		cli_printf("GNSS callback. Request status: %s.\n", _gnss_request_status_name(info->req_status));
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
	srv_gnss_close();
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

static cli_parser_status_t _cmd_gnss_monitor(void* arg, int argc, char *argv[])
{
	uint8_t mask;


	enum {opt_off, opt_fix, opt_track, opt_short_help, opt_help, opt_count };
	static const cli_cmd_option_t options[] = {
			{ "off", opt_off },
			{ "fix", opt_fix },
			{ "track", opt_track },
			{ "?", opt_short_help },
			{ "help", opt_help },
	};

	if (argc < 2) {
		cli_printf("Mandatory option missing\n");
		return cli_parser_status_error;
	}

	// by default allow relevant fields
	mask = AOS_GNSS_MSG_MSK_ALLOW_FIX | AOS_GNSS_MSG_MSK_ALLOW_TRACK;

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

		case opt_fix:
			lctx.fix_monitor = true;
			break;

		case opt_track:
			lctx.nav_monitor = true;
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
			cli_printf("    fix         Display fix information\n");
			cli_printf("    track       Display track information\n");
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
		cli_printf("GPS only");
		break;
	case aos_gnss_cfg_constellation_glonass_only:
		cli_printf("GLONASS only");
		break;
	case aos_gnss_cfg_constellation_gps_glonass:
		cli_printf("GPS + GLONASS");
		break;
	case aos_gnss_cfg_constellation_gps_galileo:
		cli_printf("GPS + GALILEO");
		break;
	case aos_gnss_cfg_constellation_gps_glonass_galileo:
		cli_printf("GPS + GLONASS + GALILEO");
		break;
	case aos_gnss_cfg_constellation_beidou_only:
		cli_printf("BEIDOU only");
		break;
	case aos_gnss_cfg_constellation_gps_beidou:
		cli_printf("GPS + BEIDOU");
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
		cli_printf("Argument required. Can be GPS, GLONASS, GALILEO or BEIDOU.\n");
		cli_printf("Constellations can be combined. Example: constellation set GPS GLONASS\n");
		return cli_parser_status_error;
	}

	for (i=1; i<argc; ++i) {
		if (!strcmp(argv[i], "GPS")) {
			mask |= AOS_GNSS_CFG_ENABLE_GPS;
		} else if (!strcmp(argv[i], "GLONASS")) {
			mask |= AOS_GNSS_CFG_ENABLE_GLONASS;
		} else if (!strcmp(argv[i], "GALILEO")) {
			mask |= AOS_GNSS_CFG_ENABLE_GALILEO;
		} else if (!strcmp(argv[i], "BEIDOU")) {
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
	static const uint8_t* restart_cmd;
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


	// Indicate we want the unknown messages
	lctx.msg_filter |= AOS_GNSS_MSG_MSK_ALLOW_UNKNOWN;
	_gnss_set_msg_filter(lctx.msg_filter);
	// Send the request
	srv_gnss_send_msg(restart_cmd, cmd_len, aos_gnss_rqst_type_msg);
	// Cleanup information
	_clear_info();

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_gnss_get_counters(void* arg, int argc, char *argv[])
{
	aos_gnss_ioctl_t ioctl;

	if (!_is_gnss_open()) {
		return cli_parser_status_error;
	}

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


static cli_parser_status_t _cmd_gnss_show_track(void* arg, int argc, char *argv[])
{
	_gnss_show_track(lctx.gnss_track);
	return cli_parser_status_ok;
}

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
		PARSER_CMD_END
};

static const cli_parser_cmd_t _gnss_cmd_table[] = {
		PARSER_CMD_FUNC("open", "Open GNSS service and activate handlers", _cmd_gnss_open, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("on", "Turn GNSS supplies on", _cmd_gnss_on, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("standby", "Turn GNSS main supply off (keepalive)", _cmd_gnss_standby, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("off", "Turn all GNSS power off (reset)", _cmd_gnss_off, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("show", "Show fix, tracking or PRN",_cmd_tab_gnss_show, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("monitor", "Setup the GNSS monitoring", _cmd_gnss_monitor, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("constel", "Get/set constellation", _cmd_tab_gnss_constell, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("restart", "Restart a GNSS acquisition", _cmd_gnss_restart, CLI_ACCESS_ALL_LEVELS),
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

