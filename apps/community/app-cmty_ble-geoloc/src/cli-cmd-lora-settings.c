/*
 * \file cli-cmd-lora-settings.c
 *
 * \brief This file implements the LoRa provisioning commands.
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> // strtoul
#include "srv_provisioning.h"
#include "srv_cli.h"
#include "strnhex.h"

#define CLI_CMD_ACCESS (cli_access_mask_super)

#define DEFAULT_MAC_REGION srv_provisioning_mac_region_eu868

#define RETURN_ON_PROVISIONING_ERROR(cs,msg) do { \
	srv_provisioning_status_t status = (cs); \
	if (status != srv_provisioning_status_success) { \
		return _failure_status(msg, status); \
	} \
} while (0)
/*
 * enum / name mapping tables
 */
static const cli_cmd_option_t _mac_region_map[srv_provisioning_mac_region_count] = {
		{ "AS923-1", srv_provisioning_mac_region_as923_1 },
		{ "AS923-2", srv_provisioning_mac_region_as923_2 },
		{ "AS923-3", srv_provisioning_mac_region_as923_3 },
		{ "AS923-4", srv_provisioning_mac_region_as923_4 },
		{ "AS923-JP", srv_provisioning_mac_region_as923_1_jp },
		{ "AU915", srv_provisioning_mac_region_au915},
		{ "EU868", srv_provisioning_mac_region_eu868 },
		{ "IN865", srv_provisioning_mac_region_in865 },
		{ "KR920", srv_provisioning_mac_region_kr920 },
		{ "RU864", srv_provisioning_mac_region_ru864 },
		{ "US915", srv_provisioning_mac_region_us915 },
};
const unsigned _mac_region_map_size = sizeof(_mac_region_map) / sizeof(*_mac_region_map);

static const cli_cmd_option_t _activation_map[] = {
		{ "ABP", srv_provisioning_activation_abp },
		{ "OTAA", srv_provisioning_activation_otaa },
};
const unsigned _activation_map_size = sizeof(_activation_map) / sizeof(*_activation_map);


static const char *_prov_status_name(srv_provisioning_status_t rc)
{
	switch(rc) {
	case srv_provisioning_status_success:	return "success";
	case srv_provisioning_status_bad_parameter: return "bad parameter";
	case srv_provisioning_status_crypto_failure: return "crypto engine failure";
	case srv_provisioning_status_failure: return "generic failure";
	case srv_provisioning_status_no_data_found: return "no settings";
	}
	return "unknown";
}
static cli_parser_status_t _failure_status(const char *what_failed, srv_provisioning_status_t status)
{
	cli_printf("%s error %d(%s)\n", what_failed, status, _prov_status_name(status));
	return cli_parser_status_error;
}

static cli_parser_status_t _parse_and_set_prov_key(int argc, char *argv[], srv_provisioning_key_id_t id)
{
	if (argc > 1) {
		srv_provisioning_key_t key;
		if (strnhex(key,  PROVISIONING_KEY_SIZE, argv[1]) == PROVISIONING_KEY_SIZE) {
			RETURN_ON_PROVISIONING_ERROR(srv_provisioning_set_key(id, key), "Set KEY");
			return cli_parser_status_ok;
		} else {
			cli_printf("Invalid key value '%s'\n", argv[1]);
			return cli_parser_status_error;
		}
	}
	cli_printf("Missing key value\n");
	return cli_parser_status_error;
}

static bool _parse_prov_eui(int argc, char *argv[], srv_provisioning_eui_t ret_eui)
{
	if (argc > 1) {
		if (strnhex(ret_eui,  PROVISIONING_EUI_SIZE, argv[1]) == PROVISIONING_EUI_SIZE) {
			return true;
		} else {
			cli_printf("Invalid EUI value '%s'\n", argv[1]);
			return false;
		}
	}
	cli_printf("Missing EUI value\n");
	return false;
}

/*
 * The settings defined here correspond to the device provisioning, and are to be
 * stored in a ciphered format inside their own parameter section.
 */
static cli_parser_status_t _cmd_settings_erase(void *arg, int argc, char *argv[])
{
	RETURN_ON_PROVISIONING_ERROR(srv_provisioning_erase(), "Erase provisioning:");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_settings_read(void *arg, int argc, char *argv[])
{
	RETURN_ON_PROVISIONING_ERROR(srv_provisioning_read(), "Read provisioning:");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_settings_write(void *arg, int argc, char *argv[])
{
	RETURN_ON_PROVISIONING_ERROR(srv_provisioning_save(), "Save provisioning");
	return cli_parser_status_ok;
}

static void _cli_list_enum_names(const cli_cmd_option_t *map, size_t map_count)
{
	for (unsigned i = 0; i < map_count; ++i) {
		cli_printf(" %s", map[i].name);
	}
	cli_printf("\r\n");
}


static bool _parse_mac_region(void *arg, int argc, char *argv[], srv_provisioning_mac_region_t *ret_region)
{
	int32_t region;	// should be an enum, but needs to be 32 bits for ...set_enum().

	if (argc < 2) {
		region = DEFAULT_MAC_REGION;
	} else {
		region = cli_get_option_index(_mac_region_map, _mac_region_map_size, argv[1]);
		if (region < 0) {
			cli_printf("Invalid MAC region '%s'\n", argv[1]);
			_cli_list_enum_names(_mac_region_map, _mac_region_map_size);
			return false;
		}
	}
	*ret_region = region;
	return true;
}

static cli_parser_status_t _cmd_settings_factory(void *arg, int argc, char *argv[])
{
	srv_provisioning_mac_region_t region = srv_provisioning_mac_region_eu868;

	if (_parse_mac_region(arg, argc, argv, &region)) {
		RETURN_ON_PROVISIONING_ERROR(srv_provisioning_factory_settings(region), "Factory settings:");
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_settings_display(void *arg, int argc, char *argv[])
{
	srv_provisioning_eui_t eui;
	srv_provisioning_activation_t activation;
	srv_provisioning_mac_region_t region;
	uint32_t devaddr;

	srv_provisioning_data_state_t dstate = srv_provisioning_data_state();

	switch(dstate) {

	case srv_provisioning_data_state_invalid:
		cli_printf("No valid provisioning data exists.\n");
		return cli_parser_status_error;

	case srv_provisioning_data_state_updated:
		cli_printf("Provisioning data (unsaved):\n");
		break;

	case srv_provisioning_data_state_saved:
		cli_printf("Provisioning data (saved):\n");
		break;
	}

	RETURN_ON_PROVISIONING_ERROR(srv_provisioning_get_lora_activation(&activation), "Get Activation");
	RETURN_ON_PROVISIONING_ERROR(srv_provisioning_get_lora_mac_region(&region), "Get MAC region");

	cli_printf(" MAC Region: %s\n", cli_get_option_name(region, _mac_region_map, _mac_region_map_size));
	cli_printf(" Activation: %s\n", cli_get_option_name(activation, _activation_map, _activation_map_size));

	RETURN_ON_PROVISIONING_ERROR(srv_provisioning_get_lora_device_eui(eui), "Get deveui");

	cli_printf(" Device EUI: ");
	cli_print_hex(eui, sizeof(eui), true);

	switch(activation) {

	case srv_provisioning_activation_otaa:
		RETURN_ON_PROVISIONING_ERROR(srv_provisioning_get_lora_join_eui(eui), "Get join EUI");
		cli_printf(" Join EUI: ");
		cli_print_hex(eui, sizeof(eui), true);
		cli_printf(" nwkkey defined: %s\n", srv_provisioning_key_is_set(srv_provisioning_key_id_nwk_key)?"yes":"no");
		cli_printf(" appkey defined: %s\n", srv_provisioning_key_is_set(srv_provisioning_key_id_app_key)?"yes":"no");
		break;

	case srv_provisioning_activation_abp:
		RETURN_ON_PROVISIONING_ERROR(srv_provisioning_get_lora_devaddr(&devaddr), "Get devaddr");
		cli_printf(" ABP devaddr: 0x%08x\n", devaddr);
		cli_printf(" appskey defined: %s\n", srv_provisioning_key_is_set(srv_provisioning_key_id_app_s_key)?"yes":"no");
		cli_printf(" nwkskey defined: %s \n", srv_provisioning_key_is_set(srv_provisioning_key_id_nwk_s_key)?"yes":"no");
		break;

	case srv_provisioning_activation_count:
		break;
	}

	for (unsigned i = 0; i < srv_provisioning_parameter_count; ++i) {
		uint32_t value;

		RETURN_ON_PROVISIONING_ERROR(srv_provisioning_get_parameter(i, &value), "Get param");

		cli_printf(" Parameter %u: 0x%08x\n", i, value);

	}

	return cli_parser_status_void;
}

static cli_parser_status_t _cmd_settings_set_device_eui(void *arg, int argc, char *argv[])
{
	srv_provisioning_eui_t eui;

	if (_parse_prov_eui(argc, argv, eui)) {

		RETURN_ON_PROVISIONING_ERROR(srv_provisioning_set_lora_device_eui(eui), "Set EUI");
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_settings_set_activation_abp(void *arg, int argc, char *argv[])
{
	RETURN_ON_PROVISIONING_ERROR(srv_provisioning_set_lora_activation(srv_provisioning_activation_abp), "Set ABP");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_settings_set_activation_otaa(void *arg, int argc, char *argv[])
{
	RETURN_ON_PROVISIONING_ERROR(srv_provisioning_set_lora_activation(srv_provisioning_activation_otaa), "Set OTAA");
	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _cli_settings_set_activation_command_table[] = {
	PARSER_CMD_FUNC("abp", "Use Activation By Personalization", _cmd_settings_set_activation_abp, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("otaa", "Use Over-The-Air Activation", _cmd_settings_set_activation_otaa, CLI_CMD_ACCESS),
	PARSER_CMD_END
};

static bool _read_uint32(const char *cp, uint32_t *retval)
{
	char *eptr;

	*retval = strtoul(cp, &eptr, 0);
	return ((*cp != '\0') && (*eptr == '\0'));
}

static cli_parser_status_t _cmd_settings_set_devaddr(void *arg, int argc, char *argv[])
{
	uint32_t devaddr;

	if ((argc > 1)
		&& (_read_uint32(argv[1], &devaddr))) {

		RETURN_ON_PROVISIONING_ERROR(srv_provisioning_set_lora_devaddr(devaddr), "Set devaddr");
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_settings_set_appskey(void *arg, int argc, char *argv[])
{
	return _parse_and_set_prov_key(argc, argv, srv_provisioning_key_id_app_s_key);
}

static cli_parser_status_t _cmd_settings_set_netskey(void *arg, int argc, char *argv[])
{
	return _parse_and_set_prov_key(argc, argv, srv_provisioning_key_id_nwk_s_key);
}

static cli_parser_status_t _cmd_settings_set_join_eui(void *arg, int argc, char *argv[])
{
	srv_provisioning_eui_t eui;

	if (_parse_prov_eui(argc, argv, eui)) {

		RETURN_ON_PROVISIONING_ERROR(srv_provisioning_set_lora_join_eui(eui), "Set EUI");
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_settings_set_appkey(void *arg, int argc, char *argv[])
{
	return _parse_and_set_prov_key(argc, argv, srv_provisioning_key_id_app_key);
}

static cli_parser_status_t _cmd_settings_set_nwkkey(void *arg, int argc, char *argv[])
{
	return _parse_and_set_prov_key(argc, argv, srv_provisioning_key_id_nwk_key);
}

static cli_parser_status_t _cmd_settings_set_parameter(void *arg, int argc, char *argv[])
{
	uint32_t pidx;
	uint32_t pval;

	if ((argc > 2)
		&& _read_uint32(argv[1], &pidx)
		&& _read_uint32(argv[2], &pval)) {

		RETURN_ON_PROVISIONING_ERROR(srv_provisioning_set_parameter(pidx, pval), "Set Parameter");
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_settings_set_region(void *arg, int argc, char *argv[])
{
	srv_provisioning_mac_region_t region = srv_provisioning_mac_region_eu868;

	if (_parse_mac_region(arg, argc, argv, &region)) {
		RETURN_ON_PROVISIONING_ERROR(srv_provisioning_set_lora_mac_region(region), "Set region:");
		return cli_parser_status_ok;
	}
	return cli_parser_status_error;
}

static const cli_parser_cmd_t _cli_settings_set_command_table[] = {
	PARSER_CMD_TAB("activation", "Set activation {abp|otaa}", _cli_settings_set_activation_command_table, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("appkey", "Set OTAA appkey",_cmd_settings_set_appkey, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("appskey", "Set ABP appskey",_cmd_settings_set_appskey, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("devaddr", "Set ABP devaddr",_cmd_settings_set_devaddr, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("deveui", "Set Device EUI",_cmd_settings_set_device_eui, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("joineui", "Set OTAA Join (Application) EUI",_cmd_settings_set_join_eui, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("nwkkey", "Set OTAA nwkkey",_cmd_settings_set_nwkkey, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("nwkskey", "Set ABP nwkskey",_cmd_settings_set_netskey, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("parameter", "Set parameter value {index} {value}",_cmd_settings_set_parameter, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("region", "Set LoRaWAN MAC region",_cmd_settings_set_region, CLI_CMD_ACCESS),
	PARSER_CMD_END
};

static const cli_parser_cmd_t _cli_settings_command_table[] = {
	PARSER_CMD_FUNC("display", "Display current settings", _cmd_settings_display, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("erase", "Erase settings from flash", _cmd_settings_erase, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("factory", "Reset settings to factory defaults", _cmd_settings_factory, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("restore", "Restore settings from flash", _cmd_settings_read, CLI_CMD_ACCESS),
	PARSER_CMD_FUNC("save", "Save settings to flash", _cmd_settings_write, CLI_CMD_ACCESS),
	PARSER_CMD_TAB("set", "Set various provisioning parameters", _cli_settings_set_command_table, CLI_CMD_ACCESS),
	PARSER_CMD_END
};

CLI_COMMAND_TAB_REGISTER(settings, "LoRa settings related commands", _cli_settings_command_table, CLI_CMD_ACCESS);
