/*!
 * \file app_config.c
 *
 * \brief Application configuration
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "aos_log.h"
#include "aos_gnss_common.h"

#include "srv_geoloc_basic.h"

#include "config.h"
#include "geolocation.h"

/*
 * ----------------------------------------------------------------------------
 * Common definition
 * ----------------------------------------------------------------------------
 */
#define CONFIG_STATUS(s...) do { aos_log_status(aos_log_module_config, true, s); } while (0);

#define CONFIG_TOTAL_NB_PARAMS (sizeof(_factory_descriptor)/sizeof(srv_config_param_descriptor_t))

// External coming from the linker
extern uint32_t __user_param_flash_base_addr;


/*
 * ----------------------------------------------------------------------------
 * Current version of the parameters
 * ----------------------------------------------------------------------------
 */
static srv_config_flash_version_t _config_current_version =  { 2, 0, 0, 0};



/*
 * ----------------------------------------------------------------------------
 * Parameter definition
 * ----------------------------------------------------------------------------
 */
#define MAX_GEOLOC_TECHNO srv_geolocation_type_count
static uint8_t _geoloc_default[srv_geolocation_type_count] = {
		(srv_geoloc_basic_action_always_acquire << 6) | srv_geolocation_type_gnss,
		(srv_geoloc_basic_action_skip_if_success << 6) | srv_geolocation_type_wifi,
		(srv_geoloc_basic_action_always_acquire << 6) | srv_geolocation_type_ble_scan1,
		0,
		0
};

static const srv_config_param_descriptor_t _factory_descriptor[]  = {
		{
				.descriptor = {config_param_id_geoloc_techno, srv_config_param_type_byte_array, MAX_GEOLOC_TECHNO},
				.value.barray = _geoloc_default},		// Default value
		{
				.descriptor = {config_param_id_ble_nb_beacon, srv_config_param_type_int32, 0},
				.value.integer = 4},					// Default value
};


/*
 * ----------------------------------------------------------------------------
 * Internal API
 * ----------------------------------------------------------------------------
 */

static void _apply_configuration(const 	srv_config_param_descriptor_t* param)
{
	switch(param->descriptor.identifier) {
	case config_param_id_geoloc_techno:
		geolocation_techno_config(param->value.barray, param->descriptor.length);
		break;
	case config_param_id_ble_nb_beacon:
		geolocation_ble_config(param->value.integer);
		break;
	}
}


static void _apply_all_configuration(void)
{
	const srv_config_param_descriptor_t* list;
	srv_config_result_t result;
	const srv_config_flash_version_t* version;
	uint32_t crc;
	uint16_t count;
	uint16_t ii;

	result = srv_config_dump_all_params(&list, &count, &crc, &version, false);
	if (result != srv_config_result_success) {
		return;
	}

	for (ii=0; ii < count; ii ++, list ++) {
		_apply_configuration(list);
	}
}


/*
 * ----------------------------------------------------------------------------
 * External API
 * ----------------------------------------------------------------------------
 */
void config_init(void)
{
	srv_config_result_t result;
	srv_config_flash_version_t version;
	srv_config_flash_info_t info;
	bool format = false;

	// Initialize the service
	srv_config_init((uint8_t*)&__user_param_flash_base_addr);

	// Read the flash information
	result = srv_config_get_info(&info);
	if ((result != srv_config_result_success) && (result != srv_config_result_not_formated)) {
		format = true;
		memset(&version, 0, sizeof(version));
	} else {
		memcpy(&version, &info.version, sizeof(version));
		// Process the factory configuration
		if ((_config_current_version.major != info.version.major) ||		// Incompatible version
			(_config_current_version.minor < info.version.minor)) {			// FW downgraded
			format = true;
		} else if (_config_current_version.minor > info.version.minor) {	// FW Upgraded
			format = true;
		}
	}

	if (format ) {
		config_erase();
	}
	_apply_all_configuration();
}

// Format the flash and reinitialize with the factory default
bool config_erase(void)
{
	// We don't keep deprecated
	return (srv_config_format_and_init(&_factory_descriptor[0], CONFIG_TOTAL_NB_PARAMS, &_config_current_version, false) == srv_config_result_success);
}

bool config_write_param(uint16_t param_id, srv_config_param_type_t type, srv_config_param_value_t value, uint8_t value_len)
{
	srv_config_param_descriptor_t param;

	param.descriptor.identifier = param_id;
	param.descriptor.type = type;
	param.descriptor.length = value_len;
	param.value = value;

	if (srv_config_param_set(param.descriptor.identifier, &param) == srv_config_result_success) {
		_apply_configuration(&param);
		return true;
	}
	return false;
}

bool config_read_param(uint16_t param_id, const srv_config_param_descriptor_t** param)
{
	return (srv_config_param_get(param_id, param) == srv_config_result_success);
}

bool config_save(srv_config_flash_version_t *version)
{
	return (srv_config_save(version, false) == srv_config_result_success);
}

void config_get_version(srv_config_flash_version_t *version)
{
	srv_config_flash_info_t info;

	if (!version) {
		return;
	}
	if (srv_config_get_info(&info) != srv_config_result_success) {
		memset(version, 0, sizeof(srv_config_flash_version_t));
		return;
	}

	memcpy(version, &info.version, sizeof(srv_config_flash_version_t));
}

