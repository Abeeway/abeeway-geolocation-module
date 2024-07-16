/*!
 * \file cli-cmd-config.c
 *
 * \brief Configuration CLI commands
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "FreeRTOS.h"
#include "srv_cli.h"

#include "config.h"



/*
 * ----------------------------------------------------------------------------
 * Parameter commands
 * ----------------------------------------------------------------------------
 */
static void _cli_config_display_param_value(const srv_config_param_descriptor_t* list)
{
	switch (list->descriptor.type) {
	case srv_config_param_type_deprecated:
		cli_printf("---");
		break;
	case srv_config_param_type_int32:
		cli_printf("%-10d (0x%08x)", list->value.integer, list->value.integer);
		break;

	case srv_config_param_type_float:
		cli_printf("%f", list->value.decimal);
		break;

	case srv_config_param_type_str:
		cli_printf("\"%s\"", list->value.ascii);
		break;

	case srv_config_param_type_byte_array:
		cli_printf("{");
		cli_print_hex_with_separator(list->value.barray, list->descriptor.length, ",", false);
		cli_printf("}");
		break;
	}
	cli_printf("\n");
}

static cli_parser_status_t _cli_config_show_all_params(void *arg, int argc, char **argv)
{
	const srv_config_param_descriptor_t* list;
	srv_config_result_t result;
	const srv_config_flash_version_t* version;
	uint32_t crc;
	uint16_t count;
	uint16_t ii;

	result = srv_config_dump_all_params(&list, &count, &crc, &version, false);
	if (result != srv_config_result_success) {
		cli_printf("Operation fails: %s\n", srv_config_result_to_str(result));
		return cli_parser_status_error;
	}

	cli_printf("Parameters\n");
	cli_printf(" %-5s %-9s %s\n","ID", "Type", "Value");
	for (ii=0; ii < count; ii ++, list ++) {
		cli_printf(" %-5d %-9s ",
				list->descriptor.identifier,
				srv_config_param_type_to_str(list->descriptor.type));
		_cli_config_display_param_value(list);
	}
	return cli_parser_status_ok;
}


static cli_parser_status_t _cli_config_param_get(void *arg, int argc, char **argv)
{
	int32_t identifier;
	bool result;
	const srv_config_param_descriptor_t* param;

	if (argc < 2) {
		cli_print_missing_argument();
		return cli_parser_status_error;
	}

	if (!cli_parse_int(argv[1], &identifier)) {
		cli_printf("Invalid identifier\n");
		return cli_parser_status_error;
	}

	result = config_read_param(identifier, &param);

	if (!result) {
		cli_printf("Operation failure\n");
		return cli_parser_status_error;
	}

	cli_printf("%2d = ", param->descriptor.identifier);
	_cli_config_display_param_value(param);
	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_config_param_set(void *arg, int argc, char **argv)
{
	srv_config_result_t result;
	int32_t identifier;
	const srv_config_param_descriptor_t* param;

	if (argc < 3) {
		cli_print_missing_argument();
		return cli_parser_status_error;
	}

	if (!cli_parse_int(argv[1], &identifier)) {
		cli_printf("Invalid identifier\n");
		return cli_parser_status_error;
	}

	result = srv_config_param_parse_value( argv[2], &param);
	if (result != srv_config_result_success) {
		cli_printf("Value error: %s\n", srv_config_result_to_str(result));
		return cli_parser_status_error;
	}
	cli_printf(" Detected type: %s\n",  srv_config_param_type_to_str(param->descriptor.type));

	if (!config_write_param(identifier, param->descriptor.type, param->value, param->descriptor.length)) {
		cli_printf("Operation error\n");
		return cli_parser_status_error;
	}

	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_config_save(void *arg, int argc, char **argv)
{
	bool result;

	result = config_save(NULL);
	cli_printf("Config saving %s\n", result?"success":"failure");

	return (result)?cli_parser_status_ok:cli_parser_status_error;
}

static cli_parser_status_t _cli_config_erase(void *arg, int argc, char **argv)
{
	bool result;

	result = config_erase();
	cli_printf("Config erasing %s\n", result?"success":"failure");

	return (result)?cli_parser_status_ok:cli_parser_status_error;
}

/*
 * ----------------------------------------------------------------------------
 * Info command
 * ----------------------------------------------------------------------------
 */
static const char* _flash_state_to_str(srv_config_flash_state_t state)
{
	switch(state) {
	case srv_config_flash_state_unknown:
	case srv_config_flash_state_count:
		break;
	case srv_config_flash_state_erased:
		return "Erased";
	case srv_config_flash_state_formated:
		return "Formated";
	}
	return "Unknown";
}

static cli_parser_status_t _cli_config_info(void *arg, int argc, char **argv)
{
	srv_config_flash_info_t info;
	srv_config_result_t result;

	result = srv_config_get_info(&info);

	if ((result != srv_config_result_success) && (result != srv_config_result_not_formated)){
		cli_printf("Operation error: %s\n", srv_config_result_to_str(result));
		return cli_parser_status_error;
	}

	cli_printf("Config information\n");
	cli_printf(" Flash base address: 0x%08x\n", info.base_address);
	cli_printf(" Flash state: %s\n", _flash_state_to_str(info.state));
	cli_printf(" Cache coherence: %ssync\n", info.in_sync?"":"not ");

	if (result == srv_config_result_not_formated) {
		return cli_parser_status_ok;
	}
	cli_printf(" Version: %d.%d.%d.%d\n", info.version.major, info.version.minor, info.version.iteration, info.version.user);
	cli_printf(" CRC: 0x%08x\n", info.crc);
	cli_printf(" Number of parameters: %d\n", info.nb_params);
	cli_printf(" Str/array area used: %d bytes\n", info.str_barray_used_size);
	cli_printf(" Str/array area free: %d bytes\n", info.str_barray_free_size);
	return cli_parser_status_ok;
}

// Main sub-commands definition
static const cli_parser_cmd_t _config_cmd_table[] = {
		PARSER_CMD_FUNC("info", "Display information in flash", _cli_config_info, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("show", "Show parameters", _cli_config_show_all_params, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("read", "<id> Read a parameter", _cli_config_param_get, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("write", "<id> <value> Write a parameter", _cli_config_param_set, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("erase", "Flash the factory default", _cli_config_erase, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("save", "Save the configuration in flash", _cli_config_save, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

/*
 * ----------------------------------------------------------------------------
 * Main Commands registration
 * ----------------------------------------------------------------------------
 */

// Register main commands against the CLI parser
CLI_COMMAND_TAB_REGISTER(config, "Configuration commands", _config_cmd_table, CLI_ACCESS_ALL_LEVELS );

