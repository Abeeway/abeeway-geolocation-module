/*!
 * \file cli-cmd-lora.c
 *
 * \brief LoRa CLI commands
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "FreeRTOS.h"
#include "srv_cli.h"
#include "srv_lora.h"

#include "lora.h"


/*
 * ----------------------------------------------------------------------------
 * Info command
 * ----------------------------------------------------------------------------
 */
static const char* _stack_state_to_str(srv_lora_stack_state_t state)
{
	switch(state) {
	case srv_lora_stack_state_idle:
		return "Idle";
	case srv_lora_stack_state_busy:
		return "Busy";
	case srv_lora_stack_state_tx_wait:
		return "TX waiting";
	default:
		break;
	}
	return "Unknown";
}

static const char* _join_state_to_str(srv_lora_join_status_t status)
{
	switch(status) {
	case srv_lora_join_status_no:
		return "No";
	case srv_lora_join_status_yes:
		return "Yes";
	case srv_lora_join_status_in_progress:
		return "In progress";
	default:
		break;
	}
	return "Unknown";
}

static cli_parser_status_t _cli_lora_link_check(void *arg, int argc, char **argv)
{
	srv_lora_link_check();
	cli_printf("Sending link check\n");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_lora_set_nonce(void *arg, int argc, char **argv)
{
	int32_t value = 0;
	bool error = false;

	if (argc < 2) {
		cli_print_missing_argument();
		return cli_parser_status_error;
	}
	if (!cli_parse_int(argv[1], &value)) {
		error = true;
	} else if ((value <0 ) || (value > 0xFFFF)) {
		error = true;
	}
	if (error) {
		cli_printf("Argument must be a value in [0..65535]\n");
		return cli_parser_status_error;
	}

	if (srv_lora_set_devnonce((uint16_t) value) != aos_lr1110_mgr_status_success) {
		return cli_parser_status_error;
	}
	cli_printf("Devnonce set to %d. Reset the system to make it effective\n", value);
	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_lora_info(void *arg, int argc, char **argv)
{
	static srv_lora_info_t info; // Too big to fit in the system stack

	srv_lora_get_info(&info);
	cli_printf("LoRa information\n");
	cli_printf(" Versions\n");
	cli_printf("  LoRa: %d.%d.%d.%d\n", info.lora_version.major, info.lora_version.minor, info.lora_version.patch, info.lora_version.revision);
	cli_printf("  Regional: %d.%d.%d.%d\n", info.lora_regional_version.major, info.lora_regional_version.minor, info.lora_regional_version.patch, info.lora_regional_version.revision);
	cli_printf("  Modem: %d.%d.%d\n", info.modem_version.major, info.modem_version.minor, info.modem_version.patch);
	cli_printf("  Chip. HW: %d. Type: %d, FW: 0x%02x\n", info.chip_version.hw, info.chip_version.type, info.chip_version.fw);
	cli_printf(" EUIs\n");
	cli_printf("  DevEUI:");
	cli_print_hex(info.deveui, SRV_LORA_DEVEUI_SIZE, true);
	cli_printf("  JoinEUI:");
	cli_print_hex(info.joineui, SRV_LORA_JOINEUI_SIZE, true);
	cli_printf(" MAC\n");
	cli_printf("  Region: %s\n", info.region_name);
	cli_printf("  TX strategy: %s\n", info.tx_strategy==srv_lora_tx_strategy_custom?"Custom": "Network (ADR)");
	cli_printf("  State: %s\n", _stack_state_to_str(info.stack_state));
	cli_printf("  Joined: %s\n", _join_state_to_str(info.join_state));
	cli_printf("  DevAddr: 0x%08x\n", info.devaddr);
	cli_printf("  DevNonce: %d\n", info.devnonce);
	cli_printf("  Duty-cycle: ");
	if (info.duty_cycle_status_ms <0) {
		cli_printf("Lock. Next in %d ms\n", -info.duty_cycle_status_ms);
	} else {
		cli_printf("Accept. Remaining %d ms\n", info.duty_cycle_status_ms);
	}
	cli_printf(" Consumption: %d mAh\n", info.consumption_mah);
	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_lora_get_date(void *arg, int argc, char **argv)
{
	cli_printf("Requesting the date\n");

	 if (srv_lora_request_time() != aos_lr1110_mgr_status_success) {
		   	return cli_parser_status_error;
	 }

   	return cli_parser_status_ok;
}

// Main sub-commands definition
static const cli_parser_cmd_t _lora_cmd_table[] = {
		PARSER_CMD_FUNC("info", "Display information", _cli_lora_info, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("link", "Link check request", _cli_lora_link_check, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("nonce", "<value>. Set the devnonce value in flash", _cli_lora_set_nonce, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("date", "Request time and update the system time", _cli_lora_get_date, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

/*
 * ----------------------------------------------------------------------------
 * Main Commands registration
 * ----------------------------------------------------------------------------
 */

// Register main commands against the CLI parser
CLI_COMMAND_TAB_REGISTER(lora,"LoRaWAN commands", _lora_cmd_table, CLI_ACCESS_ALL_LEVELS );

