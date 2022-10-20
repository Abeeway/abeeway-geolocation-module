/*!
 * \file cli-cmd-lora.c
 *
 * \brief LORA CLI commands
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
// #include <stdio.h>
#include "srv_cli.h"
#include "srv_provisioning.h"
#include "aos_board.h"

#include "srv_lmh.h"


static const cli_cmd_option_t _loramac_region_map[] = {
		{ "AS923", LORAMAC_REGION_AS923 },
		{ "AU915", LORAMAC_REGION_AU915 },
		{ "EU868", LORAMAC_REGION_EU868 },
		{ "IN865", LORAMAC_REGION_IN865 },
		{ "KR920", LORAMAC_REGION_KR920 },
		{ "RU864", LORAMAC_REGION_RU864 },
		{ "US915", LORAMAC_REGION_US915 },
};
const unsigned _loramac_region_map_size = sizeof(_loramac_region_map) / sizeof(*_loramac_region_map);

static const cli_cmd_option_t _loramac_class_map[] = {
		{ "Class A", CLASS_A },
		{ "Class B", CLASS_B },
		{ "Class C", CLASS_C },
 };
const unsigned _loramac_class_map_size = sizeof(_loramac_class_map) / sizeof(*_loramac_class_map);

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

static cli_parser_status_t _must_open_lmh_first(void)
{
	cli_printf("LoRa must be opened first.\n");
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_lmhandler_device_time(void *arg, int argc, char *argv[])
{
	if (srv_lmh_state != lmh_state_opened) {
		return _must_open_lmh_first();
	}

	LmHandlerErrorStatus_t rc;

	rc = LmHandlerDeviceTimeReq();
	return (rc == LORAMAC_HANDLER_SUCCESS) ? cli_parser_status_ok : cli_parser_status_error;
}

static cli_parser_status_t _cmd_lmhandler_join(void *arg, int argc, char *argv[])
{
	if (srv_lmh_state != lmh_state_opened) {
		return _must_open_lmh_first();
	}

	cli_printf("Initiating join...\n");
	LmHandlerJoin();
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lmhandler_open(void *arg, int argc, char *argv[])
{

	LmHandlerErrorStatus_t rc;
	rc = srv_lmh_open(NULL);
	return (rc == LORAMAC_HANDLER_SUCCESS) ? cli_parser_status_ok : cli_parser_status_error;

}

#include "strnhex.h"

static cli_parser_status_t _cmd_lmhandler_send(void *arg, int argc, char *argv[])
{

#define HEX_BUFLEN 64

	uint8_t hexbuf[HEX_BUFLEN];

	uint8_t *payload_buf;
	uint8_t payload_len;

	if (argc > 1) {
		int len = strnhex(hexbuf, HEX_BUFLEN, argv[1]);
		if (len > 0) {
			payload_len = len;
			payload_buf = hexbuf;
		} else {
			payload_buf = (uint8_t *)argv[1];
			payload_len = strlen(argv[1]);
		}
	} else {
		static const uint8_t default_payload[] = "Hello, world";
		payload_buf = (uint8_t *)default_payload;
		payload_len = sizeof(default_payload);
	}

	LmHandlerErrorStatus_t rc;
	rc = srv_lmh_send(payload_buf, payload_len);

	if (rc != LORAMAC_HANDLER_SUCCESS) {
		return cli_parser_status_error;
	}
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lmhandler_status(void *arg, int argc, char *argv[])
{
	cli_printf(" LM Joined: %s\n", LmHandlerJoinStatus() == LORAMAC_HANDLER_SET?"yes":"no");
	cli_printf(" Active MAC region: %s\n", cli_get_option_name(LmHandlerGetActiveRegion(), _loramac_region_map, _loramac_region_map_size));
	cli_printf(" Current Class: %s\n", cli_get_option_name(LmHandlerGetCurrentClass(), _loramac_class_map, _loramac_class_map_size));
	cli_printf(" Current Datarate: %u\n", LmHandlerGetCurrentDatarate());

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lmhandler_params_display(void *arg, int argc, char *argv[])
{
	LmHandlerParams_t *p = &srv_lmh_params;

	cli_printf(" ADR Enabled: %s\n", p->AdrEnable?"yes":"no");
	cli_printf(" Duty Cycle Enabled: %s\n", p->DutyCycleEnabled?"yes":"no");
	cli_printf(" Confirmed Uplinks: %s\n", p->IsTxConfirmed?"yes":"no");
	cli_printf(" Public Network: %s\n", p->PublicNetworkEnable?"yes":"no");
	cli_printf(" Uplink Port: %u \n", srv_lmh_uplink_port);
	cli_printf(" Uplink Datarate: %u\n", p->TxDatarate);
	return cli_parser_status_void;
}

static cli_parser_status_t _cmd_lmhandler_params_set_adr(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	value = cli_get_boolean(argv[1]);
	if (value < 0) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	srv_lmh_params.AdrEnable = !!value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lmhandler_params_set_duty_cycle(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	value = cli_get_boolean(argv[1]);
	if (value < 0) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	srv_lmh_params.DutyCycleEnabled = !!value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lmhandler_params_set_confirmed_tx(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	value = cli_get_boolean(argv[1]);
	if (value < 0) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	srv_lmh_params.IsTxConfirmed = !!value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lmhandler_params_set_public_network(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	value = cli_get_boolean(argv[1]);
	if (value < 0) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	srv_lmh_params.PublicNetworkEnable = !!value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lmhandler_params_set_ul_port(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	if (!cli_parse_int(argv[1], &value)) {
		return cli_parser_status_error;
	}

	if ((value < MIN_UPLINK_PORT) || (value > MAX_UPLINK_PORT)) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	srv_lmh_uplink_port = value;

	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_lmhandler_params_set_tx_datarate(void *arg, int argc, char *argv[])
{
	int32_t value;

	if (argc < 2) {
		return _missing_parameters(argv[0]);
	}

	if (!cli_parse_int(argv[1], &value)) {
		return cli_parser_status_error;
	}

	if ((value < DR_0) || (value > DR_15)) {
		return _incorrect_parameters(argv[0], argv[1]);
	}

	srv_lmh_params.TxDatarate = value;

	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _cli_lmhandler_params_set_command_table[] = {
	PARSER_CMD_FUNC("adr", "Enable or disable ADR", _cmd_lmhandler_params_set_adr, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("confirmed", "Enable or disable confirmed uplinks", _cmd_lmhandler_params_set_confirmed_tx, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("datarate", "Set the uplink datarate", _cmd_lmhandler_params_set_tx_datarate, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("duty-cycle", "Enable or disable uplink duty Cycle (testing only)", _cmd_lmhandler_params_set_duty_cycle, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("port", "Set the uplink port number", _cmd_lmhandler_params_set_ul_port, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("public-net", "Enable or disable public network", _cmd_lmhandler_params_set_public_network, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_END
};

static const cli_parser_cmd_t _cli_lmhandler_params_command_table[] = {
	PARSER_CMD_FUNC("display", "Display LoRa Parameters", _cmd_lmhandler_params_display, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_TAB("set", "Set LoRa Parameter", _cli_lmhandler_params_set_command_table, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_END
};

static const cli_parser_cmd_t _cli_lmhandler_command_table[] = {
	PARSER_CMD_FUNC("open", "Open the LoRa driver", _cmd_lmhandler_open, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("join", "Initiate a join", _cmd_lmhandler_join, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_TAB("parameter", "Display or set parameters", _cli_lmhandler_params_command_table, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("send", "[ascii|hex] Send an uplink", _cmd_lmhandler_send, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("status", "Get status", _cmd_lmhandler_status, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("date", "Request the date/time via LoRa", _cmd_lmhandler_device_time, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_END
};


/*
 * ----------------------------------------------------------------------------
 * Main Commands registration
 * ----------------------------------------------------------------------------
 */
// Register main commands against the CLI parser
CLI_COMMAND_TAB_REGISTER(lora, "LoRa related commands", _cli_lmhandler_command_table, CLI_ACCESS_ALL_LEVELS);

