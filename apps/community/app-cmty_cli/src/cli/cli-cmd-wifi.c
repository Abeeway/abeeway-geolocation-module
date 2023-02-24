/*
 * \file cli-cms-wifi.c
 *
 * \brief WIFI commands
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "aos_rf_switch.h"
#include "srv_cli.h"
#include "radio_board.h"
#include "lr11xx_radio.h"
#include "lr11xx_wifi.h"

const radio_context_t *radio = NULL;

__attribute__((constructor))
static void _lr11xx_command_init(void)
{
	radio = radio_board_get_radio_context_reference();
}

#define LR11XX_CMD_ACCESS (CLI_ACCESS_ALL_LEVELS)

/*
 *
 */
#define USE_LR_WIFI_COMMANDS (1)

/*
 * USE_LR_WIFI_COMMANDS determines whether to include the "lr wifi" commands.
 *
 */
static cli_parser_status_t _missing_parameters(const char *s)
{
	cli_printf("Missing parameter(s) for '%s' command\n", s);
	return cli_parser_status_error;
}

static cli_parser_status_t _incorrect_parameters(const char *s, const char *e)
{
	cli_printf("Incorrect parameter value '%s' for '%s' command\n", e, s);
	return cli_parser_status_error;
}

#define IN_RANGE(p,mi,ma) ((p >= mi) && (p <= ma))


#define DEFAULT_WIFI_SIGNAL_TYPE LR11XX_WIFI_TYPE_SCAN_B_G_N
#define DEFAULT_WIFI_CHANNELS 0x3fff	// Channels 14 to 1 (bit 0 = channel #1)
#define DEFAULT_WIFI_SCAN_MODE LR11XX_WIFI_SCAN_MODE_BEACON
#define DEFAULT_WIFI_MAX_RESULTS 20
#define DEFAULT_WIFI_NB_SCAN_PER_CHANNEL 3
#define DEFAULT_WIFI_TIMEOUT_IN_MS 110	// preamble search timeout
#define DEFAULT_WIFI_ABORT_ON_TIMEOUT true

typedef struct {
	struct {
		lr11xx_wifi_signal_type_scan_t	signal_type;
		lr11xx_wifi_channel_mask_t		channels;
		lr11xx_wifi_mode_t				scan_mode;
		uint8_t							max_results;
		uint8_t							nb_scan_per_channel;
		uint16_t						timeout_in_ms;
		bool							abort_on_timeout;
	} settings;
} cli_lr11xx_wifi_params_t;

static cli_lr11xx_wifi_params_t _wifi = {
	.settings = {
		.signal_type = 	DEFAULT_WIFI_SIGNAL_TYPE,
		.channels = DEFAULT_WIFI_CHANNELS,
		.scan_mode = DEFAULT_WIFI_SCAN_MODE,
		.max_results = DEFAULT_WIFI_MAX_RESULTS,
		.nb_scan_per_channel = DEFAULT_WIFI_NB_SCAN_PER_CHANNEL,
		.timeout_in_ms = DEFAULT_WIFI_TIMEOUT_IN_MS,
		.abort_on_timeout = DEFAULT_WIFI_ABORT_ON_TIMEOUT,
	},
};

static cli_parser_status_t _cmd_lr11xx_wifi_scan(void *arg, int argc, char *argv[])
{
	lr11xx_status_t rc;
	uint8_t nb_results = 0;

	if (aos_rf_switch_get_owner(aos_rf_switch_type_ble_wifi) != aos_rf_switch_owner_none) {
		cli_printf("RF switch used. Check BLE.\n");
		return cli_parser_status_error;
	}

	if (aos_rf_switch_acquire_antenna(aos_rf_switch_type_ble_wifi, RF_SWITCH_OWNER_WIFI) != aos_result_success) {
		cli_printf("Fail to acquire the antenna\n");
		return cli_parser_status_error;
	}

	cli_printf("Scan start\n");

	rc = lr11xx_wifi_scan(radio, _wifi.settings.signal_type,
			_wifi.settings.channels,
			_wifi.settings.scan_mode,
			_wifi.settings.max_results,
			_wifi.settings.nb_scan_per_channel,
			_wifi.settings.timeout_in_ms,
			_wifi.settings.abort_on_timeout);

	aos_rf_switch_release_antenna(aos_rf_switch_type_ble_wifi, RF_SWITCH_OWNER_WIFI);


	if (rc != LR11XX_STATUS_OK) {
		cli_printf("Failed to initiate wifi scan, status %d\n", rc);
		return cli_parser_status_error;
	}

	rc = lr11xx_wifi_get_nb_results(radio, &nb_results);
	if (rc != LR11XX_STATUS_OK) {
		cli_printf("Failed to get scan result count, status %d\n", rc);
		return cli_parser_status_error;
	}

	cli_printf("Number of results: %u\n", nb_results);
	for (unsigned i = 0; i < nb_results; ++i) {
		lr11xx_wifi_basic_complete_result_t r;

		rc = lr11xx_wifi_read_basic_complete_results(radio, i, 1, &r);

		cli_printf(" MAC Address: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x, RSSI: %d\n",
				r.mac_address[0], r.mac_address[1],r.mac_address[2],
				r.mac_address[3], r.mac_address[4], r.mac_address[5], r.rssi);
	}
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lr11xx_wifi_info(void *arg, int argc, char *argv[])
{
	lr11xx_wifi_version_t wifi_version;
	lr11xx_status_t rc;

	rc = lr11xx_wifi_read_version(radio, &wifi_version);
	if (rc != LR11XX_STATUS_OK) {
		cli_printf("Failed to read wifi version, status %d\n", rc);
		return cli_parser_status_error;
	}

	cli_printf("WIFI version: %u.%u\n",  wifi_version.major, wifi_version.minor);

	return cli_parser_status_ok;
}

static const cli_cmd_option_t mac_origin_estimation_map[] = {
		{ "beacon, fixed AP", LR11XX_WIFI_ORIGIN_BEACON_FIX_AP },
		{ "beacon, mobile AP", LR11XX_WIFI_ORIGIN_BEACON_MOBILE_AP },
		{ "unknown", LR11XX_WIFI_ORIGIN_UNKNOWN }
};
static const unsigned mac_origin_estimation_map_count = sizeof(mac_origin_estimation_map) / sizeof(*mac_origin_estimation_map);

static cli_parser_status_t _cmd_lr11xx_wifi_results(void *arg, int argc, char *argv[])
{
	uint8_t nb_results = 0;
	lr11xx_status_t rc;

	rc = lr11xx_wifi_get_nb_results(radio, &nb_results);
	if (rc != LR11XX_STATUS_OK) {
		cli_printf("Failed to get scan result count, status %d\n", rc);
		return cli_parser_status_error;
	}

	cli_printf("Number of results: %u\n", nb_results);
	for (unsigned i = 0; i < nb_results; ++i) {
		lr11xx_wifi_basic_complete_result_t r;

		rc = lr11xx_wifi_read_basic_complete_results(radio, i, 1, &r);
		if (rc != LR11XX_STATUS_OK) {
			cli_printf("Failed to get scan results[%u], status %d\n", i, rc);
			return cli_parser_status_error;
		}

		cli_printf("Entry %d\n", i);
		cli_printf(" MAC Address: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
				r.mac_address[0], r.mac_address[1],r.mac_address[2],
				r.mac_address[3], r.mac_address[4], r.mac_address[5]);
		cli_printf(" RSSI: %d\n", r.rssi);

		lr11xx_wifi_channel_t channel;
		bool rssi_validity;
		lr11xx_wifi_mac_origin_t mac_origin_estimation;

		lr11xx_wifi_parse_channel_info(r.channel_info_byte, &channel, &rssi_validity, &mac_origin_estimation);

		cli_printf(" Channel: %d\n", channel);
		cli_printf(" MAC origin (est.): %s\n",
				cli_get_option_name(mac_origin_estimation, mac_origin_estimation_map, mac_origin_estimation_map_count));
	}
	return cli_parser_status_ok;
}

static const cli_cmd_option_t scan_mode_map[] = {
		{ "beacon-only", LR11XX_WIFI_SCAN_MODE_BEACON },
		{ "beacon-and-packet", LR11XX_WIFI_SCAN_MODE_BEACON_AND_PKT },
		{ "full-beacon", LR11XX_WIFI_SCAN_MODE_FULL_BEACON },
		{ "until-ssid", LR11XX_WIFI_SCAN_MODE_UNTIL_SSID },
};
static const unsigned scan_mode_map_count = sizeof(scan_mode_map) / sizeof(*scan_mode_map);

static const cli_cmd_option_t signal_type_map[] = {
		{ "b-only", LR11XX_WIFI_TYPE_SCAN_B },
		{ "g-only", LR11XX_WIFI_TYPE_SCAN_G },
		{ "n-only", LR11XX_WIFI_TYPE_SCAN_N },
		{ "bgn", LR11XX_WIFI_TYPE_SCAN_B_G_N },
};
static const unsigned signal_type_map_count = sizeof(signal_type_map) / sizeof(*signal_type_map);

static cli_parser_status_t _cmd_lr11xx_wifi_parameters_display(void *arg, int argc, char *argv[])
{
	cli_printf("Parameters\n");
	cli_printf(" channel-mask: 0x%02x\n", _wifi.settings.channels);
	cli_printf(" scan-mode: %s\n",
			cli_get_option_name(_wifi.settings.scan_mode, scan_mode_map, scan_mode_map_count));
	cli_printf(" signal-type: %s\n",
			cli_get_option_name(_wifi.settings.signal_type, signal_type_map, signal_type_map_count));
	cli_printf(" abort-on-timeout: %s\n", _wifi.settings.abort_on_timeout?"yes":"no");
	cli_printf(" max-results: %u\n", _wifi.settings.max_results);
	cli_printf(" nb-scan-per-channel: %u\n", _wifi.settings.nb_scan_per_channel);
	cli_printf(" timeout (ms): %u\n", _wifi.settings.timeout_in_ms);

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lr11xx_wifi_parameters_set_abort_on_timeout(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	value = cli_get_boolean(argv[1]);
	if (value < 0) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	_wifi.settings.abort_on_timeout = !!value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lr11xx_wifi_parameters_set_scan_mode(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	value = cli_get_option_index(scan_mode_map, scan_mode_map_count, argv[1]);
	if (value < 0) {
		cli_list_options("Parameters are: ", scan_mode_map, scan_mode_map_count);
		return _incorrect_parameters(argv[0], argv[1]);
	}

	_wifi.settings.scan_mode = value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lr11xx_wifi_parameters_set_channel_mask(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	if (!cli_parse_int(argv[1], &value) || !IN_RANGE(value, 0, 0x3fff)) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	_wifi.settings.channels = value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lr11xx_wifi_parameters_set_signal_type(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	value = cli_get_option_index(signal_type_map, signal_type_map_count, argv[1]);
	if (value < 0) {
		cli_list_options("Parameters are: ", signal_type_map, signal_type_map_count);
		return _incorrect_parameters(argv[0], argv[1]);
	}

	_wifi.settings.signal_type = value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lr11xx_wifi_parameters_set_max_results(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	if (!cli_parse_int(argv[1], &value) || !IN_RANGE(value, 0, UINT8_MAX)) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	_wifi.settings.max_results = value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lr11xx_wifi_parameters_set_nb_scans_per_channel(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	if (!cli_parse_int(argv[1], &value) || !IN_RANGE(value, 0, UINT8_MAX)) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	_wifi.settings.nb_scan_per_channel = value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lr11xx_wifi_parameters_set_timeout(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	if (!cli_parse_int(argv[1], &value) || !IN_RANGE(value, 0, UINT16_MAX)) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	_wifi.settings.timeout_in_ms = value;

	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _cli_lr11xx_wifi_parameters_set_command_table[] = {
	PARSER_CMD_FUNC("abort-on-timeout", "abort on preamble timeout", _cmd_lr11xx_wifi_parameters_set_abort_on_timeout, LR11XX_CMD_ACCESS),
	PARSER_CMD_FUNC("channel-mask", "channel mask (bit 0 = channel 1)", _cmd_lr11xx_wifi_parameters_set_channel_mask, LR11XX_CMD_ACCESS),
	PARSER_CMD_FUNC("max-results", "max results", _cmd_lr11xx_wifi_parameters_set_max_results, LR11XX_CMD_ACCESS),
	PARSER_CMD_FUNC("nb-scans", "number of scans per channel", _cmd_lr11xx_wifi_parameters_set_nb_scans_per_channel, LR11XX_CMD_ACCESS),
	PARSER_CMD_FUNC("scan-mode", "(beacon-only, beacon-and-packet)", _cmd_lr11xx_wifi_parameters_set_scan_mode, LR11XX_CMD_ACCESS),
	PARSER_CMD_FUNC("signal-type", "signal type (b-only, g-only, n-only, bgn)", _cmd_lr11xx_wifi_parameters_set_signal_type, LR11XX_CMD_ACCESS),
	PARSER_CMD_FUNC("timeout", "preamble search timeout (ms)", _cmd_lr11xx_wifi_parameters_set_timeout, LR11XX_CMD_ACCESS),
	PARSER_CMD_END
};


static const cli_parser_cmd_t _cli_lr11xx_wifi_parameters_command_table[] = {
	PARSER_CMD_FUNC("display", "display wifi scan parameters", _cmd_lr11xx_wifi_parameters_display, LR11XX_CMD_ACCESS),
	PARSER_CMD_TAB("set", "set wifi scan parameters", _cli_lr11xx_wifi_parameters_set_command_table, LR11XX_CMD_ACCESS),
	PARSER_CMD_END
};

static const cli_parser_cmd_t _cli_lr11xx_wifi_command_table[] = {
	PARSER_CMD_FUNC("scan", "Perform a WIFI scan", _cmd_lr11xx_wifi_scan, LR11XX_CMD_ACCESS),
	PARSER_CMD_FUNC("info", "Display WIFI information", _cmd_lr11xx_wifi_info, LR11XX_CMD_ACCESS),
	PARSER_CMD_FUNC("results", "Show WIFI scan results", _cmd_lr11xx_wifi_results, LR11XX_CMD_ACCESS),
	PARSER_CMD_TAB("parameters", "parameter related commands", _cli_lr11xx_wifi_parameters_command_table, LR11XX_CMD_ACCESS),
	PARSER_CMD_END
};


CLI_COMMAND_TAB_REGISTER(wifi, "WIFI related commands", _cli_lr11xx_wifi_command_table, LR11XX_CMD_ACCESS);
