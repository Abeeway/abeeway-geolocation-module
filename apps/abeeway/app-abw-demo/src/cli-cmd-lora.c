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
#include <stdio.h>
#include "srv_cli.h"
#include "srv_provisioning.h"
#include "aos_board.h"

// XXX These includes come from the library includes. This is needed as we
// XXX want to validate that functionality, but should be moved elsewhere,
// XXX unless the lmhandler code is provided as application or service ?

#include "board.h"
#include "LmHandler.h"
#include "LmHandlerMsgDisplay.h"	// TODO: This is definitely application code


#define DEF_UPLINK_DR DR_0
#define DEF_UPLINK_PORT 100
#define MIN_UPLINK_PORT 1
#define MAX_UPLINK_PORT 223	// Port values 1-223 are application specific

static uint8_t _lmhandler_buffer[255];

static LmHandlerParams_t _lmhandler_params = {
		.Region = LORAMAC_REGION_EU868,
		.AdrEnable = true,
		.IsTxConfirmed = LORAMAC_HANDLER_UNCONFIRMED_MSG,
		.TxDatarate = DEF_UPLINK_DR,
		.PublicNetworkEnable = true,
		.DutyCycleEnabled = true,
		.DataBufferMaxSize = sizeof(_lmhandler_buffer),
		.DataBuffer = _lmhandler_buffer,
};

static uint8_t _lmh_uplink_port = DEF_UPLINK_PORT;

// Remember the state, as e.g. trying a join before initializing the handler crashes.
static enum { lmh_state_closed, lmh_state_opened } _lmh_state = lmh_state_closed;

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
	if (_lmh_state != lmh_state_opened) {
		return _must_open_lmh_first();
	}

	LmHandlerErrorStatus_t rc;

	rc = LmHandlerDeviceTimeReq();
	return (rc == LORAMAC_HANDLER_SUCCESS) ? cli_parser_status_ok : cli_parser_status_error;
}

static cli_parser_status_t _cmd_lmhandler_join(void *arg, int argc, char *argv[])
{
	if (_lmh_state != lmh_state_opened) {
		return _must_open_lmh_first();
	}

	cli_printf("Initiating join...\n");
	LmHandlerJoin();
	return cli_parser_status_ok;
}

// LM handlers. Display the LmHandler debug messages people are familiar with
// from the github examples. Handled using the stackforce demo code, modified
// to use cli_printf() instead of printf().

static void _lm_on_beacon_status_change(LoRaMacHandlerBeaconParams_t *params)
{
	DisplayBeaconUpdate(params);
}

static void _lm_on_class_change(DeviceClass_t deviceClass)
{
	DisplayClassUpdate(deviceClass);

    // Inform the server as soon as possible that the end-device has switched to ClassB
    LmHandlerAppData_t appData =
    {
        .Buffer = NULL,
        .BufferSize = 0,
        .Port = 0,
    };
	LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG);
}

static void _lm_on_join_request(LmHandlerJoinParams_t *params)
{
	DisplayJoinRequestUpdate(params);
	if (params->Status == LORAMAC_HANDLER_ERROR) {
		// Do not retry for ABP activation, that would blow our stack !
		if (params->CommissioningParams->IsOtaaActivation) {
			cli_printf("Retrying join...\n");
			LmHandlerJoin();
		}
	} else {
		LmHandlerRequestClass(CLASS_A);
	}
}

static void _lm_on_mac_mcps_request(LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxDelay)
{
	DisplayMacMcpsRequestUpdate(status, mcpsReq, nextTxDelay);
}

static void _lm_on_mac_mlme_request(LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxDelay)
{
	DisplayMacMlmeRequestUpdate(status, mlmeReq, nextTxDelay);
}

static void _lm_on_mac_process(void)
{
	// No need for trampoline since mac timers are already decoupled from interrupts.
	LmHandlerProcess();
}

static void _lm_on_network_parameters_change(CommissioningParams_t *params)
{
	DisplayNetworkParametersUpdate(params);
}

static void _lm_on_nvm_data_change(LmHandlerNvmContextStates_t state, uint16_t size)
{
	DisplayNvmDataChange(state, size);
}

static void _lm_on_rx_data(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
	DisplayRxUpdate(appData, params);

	switch (appData->Port) {
	case 1: // The application LED can be controlled on port 1 or 2
	case 2:
		cli_printf("Received request to turn the Application LED O%s\n", (appData->Buffer[0] & 0x01) ? "N":"FF");
		aos_board_led_set(aos_board_led_idx_led4, appData->Buffer[0] & 0x01);
		break;
	default:
		break;
	}
}

static void _lm_on_sys_time_update(bool isSynchronized, int32_t timeCorrection)
{
	cli_printf("%s(%d, %lu)\n", isSynchronized, timeCorrection);
}

static void _lm_on_tx_data(LmHandlerTxParams_t *params)
{
	DisplayTxUpdate(params);
}

static cli_parser_status_t _cmd_lmhandler_open(void *arg, int argc, char *argv[])
{
	LmHandlerErrorStatus_t rc;
	static LmHandlerCallbacks_t lc;

	if (srv_provisioning_data_state() == srv_provisioning_data_state_invalid) {
		cli_printf("Restoring provisioning settings...\n");

		if (srv_provisioning_read() != srv_provisioning_status_success) {
			cli_printf("No provisioning settings found\n");
			return cli_parser_status_error;
		}
	}

	srv_provisioning_mac_region_t region;

	if (srv_provisioning_get_lora_mac_region(&region) != srv_provisioning_status_success) {
		cli_printf("Failed to get provisioned region\n");
		return cli_parser_status_error;
	}

	switch(region) {
	case srv_provisioning_mac_region_eu868:
		_lmhandler_params.Region = LORAMAC_REGION_EU868;
		break;
	case srv_provisioning_mac_region_ru864:
		_lmhandler_params.Region = LORAMAC_REGION_RU864;
		break;
	case srv_provisioning_mac_region_us915:
		_lmhandler_params.Region = LORAMAC_REGION_US915;
		break;
	case srv_provisioning_mac_region_as923:
		_lmhandler_params.Region = LORAMAC_REGION_AS923;
		break;
	case srv_provisioning_mac_region_au915:
		_lmhandler_params.Region = LORAMAC_REGION_AU915;
		break;
	case srv_provisioning_mac_region_kr920:
		_lmhandler_params.Region = LORAMAC_REGION_KR920;
		break;
	case srv_provisioning_mac_region_in865:
		_lmhandler_params.Region = LORAMAC_REGION_IN865;
		break;
	case srv_provisioning_mac_region_count:
		cli_printf("Unknown provisioning region %u\n", region);
		return cli_parser_status_error;
	}

	lc.GetBatteryLevel = BoardGetBatteryLevel;
	lc.GetRandomSeed = BoardGetRandomSeed;
	lc.GetTemperature = NULL;	// BoardGetTemperature;
	lc.OnBeaconStatusChange = _lm_on_beacon_status_change;
	lc.OnClassChange = _lm_on_class_change;
	lc.OnJoinRequest = _lm_on_join_request;
	lc.OnMacMcpsRequest = _lm_on_mac_mcps_request;
	lc.OnMacMlmeRequest = _lm_on_mac_mlme_request;
	lc.OnMacProcess = _lm_on_mac_process;
	lc.OnNetworkParametersChange = _lm_on_network_parameters_change;
	lc.OnNvmDataChange = _lm_on_nvm_data_change;
	lc.OnRxData = _lm_on_rx_data;
	lc.OnSysTimeUpdate = _lm_on_sys_time_update;
	lc.OnTxData = _lm_on_tx_data;

	rc = LmHandlerInit(&lc, &_lmhandler_params);

	if (rc == LORAMAC_HANDLER_SUCCESS) {
		_lmh_state = lmh_state_opened;
	}
	return (rc == LORAMAC_HANDLER_SUCCESS) ? cli_parser_status_ok : cli_parser_status_error;
}

#include "strnhex.h"

static cli_parser_status_t _cmd_lmhandler_send(void *arg, int argc, char *argv[])
{
	if (LmHandlerIsBusy() == true) {
		cli_printf("LoRa is busy\n");
		return cli_parser_status_error;
	}

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

	cli_xdump(payload_buf, payload_len); // XXX debug

	LmHandlerAppData_t payload;

	payload.Buffer = payload_buf;	// de-constify.
	payload.BufferSize = payload_len;
	payload.Port = _lmh_uplink_port;

	LmHandlerErrorStatus_t rc;

	rc = LmHandlerSend(&payload, _lmhandler_params.IsTxConfirmed);
	if (rc != LORAMAC_HANDLER_SUCCESS) {
		cli_printf("Send failed, status %d\n", rc);
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
	LmHandlerParams_t *p = &_lmhandler_params;

	cli_printf(" ADR Enabled: %s\n", p->AdrEnable?"yes":"no");
	cli_printf(" Duty Cycle Enabled: %s\n", p->DutyCycleEnabled?"yes":"no");
	cli_printf(" Confirmed Uplinks: %s\n", p->IsTxConfirmed?"yes":"no");
	cli_printf(" Public Network: %s\n", p->PublicNetworkEnable?"yes":"no");
	cli_printf(" Uplink Port: %u \n", _lmh_uplink_port);
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

	_lmhandler_params.AdrEnable = !!value;

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

	_lmhandler_params.DutyCycleEnabled = !!value;

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

	_lmhandler_params.IsTxConfirmed = !!value;

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

	_lmhandler_params.PublicNetworkEnable = !!value;

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

	_lmh_uplink_port = value;

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

	_lmhandler_params.TxDatarate = value;

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

