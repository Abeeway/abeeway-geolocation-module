/*
 * @file geolocation.h
 *
 * Geolocation applicative part
 */

#pragma once

#include "srv_config_param.h"

typedef enum {
	config_param_id_geoloc_techno = 0,
	config_param_id_ble_nb_beacon = 1,
}config_param_id_t;


void config_init(void);
bool config_write_param(uint16_t param_id, srv_config_param_type_t type, srv_config_param_value_t value, uint8_t value_len);
bool config_read_param(uint16_t param_id, const srv_config_param_descriptor_t** param);
bool config_erase(void);
bool config_save(srv_config_flash_version_t *version);
void config_get_version(srv_config_flash_version_t *version);

