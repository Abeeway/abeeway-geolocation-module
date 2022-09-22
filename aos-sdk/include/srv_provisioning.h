/*
 * srv_provisioning.h - Murata Module provisioning
 *
 * This source component takes care of storing and retrieving all the
 * persistent module parameters, notably all the LoRaWAN parameters but
 * also a number of board- and application-specific parameters that are
 * set during module provisioning (manufacturing) and will not change
 * over the lifetime of the device.
 *
 * It relies on the LR1110 crypto engine for storing keys and parameters.
 *
 * Copyright 2021 Marc Bongartz <mbong@free.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */
 #pragma once

#include <stdint.h>
#include <stdbool.h>

#define PROVISIONING_EUI_SIZE 8
#define PROVISIONING_KEY_SIZE 16

typedef uint8_t srv_provisioning_eui_t[PROVISIONING_EUI_SIZE];
typedef uint8_t srv_provisioning_key_t[PROVISIONING_KEY_SIZE];

typedef enum {
	srv_provisioning_status_success = 0,
	srv_provisioning_status_failure,			// Generic failure
	srv_provisioning_status_bad_parameter,	// Invalid parameter passed to API function
	srv_provisioning_status_crypto_failure,	// Crypto engine failure
	srv_provisioning_status_no_data_found,	// No provisioning data in flash
} srv_provisioning_status_t;

typedef enum {
	srv_provisioning_parameter_0,
	srv_provisioning_parameter_1,
	srv_provisioning_parameter_2,
	srv_provisioning_parameter_3,
	srv_provisioning_parameter_4,
	srv_provisioning_parameter_5,
	srv_provisioning_parameter_6,
	srv_provisioning_parameter_7,
	srv_provisioning_parameter_8,
	srv_provisioning_parameter_9,
	srv_provisioning_parameter_10,
	srv_provisioning_parameter_11,
	srv_provisioning_parameter_12,
	srv_provisioning_parameter_13,
	srv_provisioning_parameter_14,
	srv_provisioning_parameter_15,
	srv_provisioning_parameter_count
} srv_provisioning_parameter_t;

srv_provisioning_status_t srv_provisioning_set_parameter(srv_provisioning_parameter_t id, uint32_t value);
srv_provisioning_status_t srv_provisioning_get_parameter(srv_provisioning_parameter_t id, uint32_t *value);

typedef enum {
	srv_provisioning_data_state_invalid = 0,	// No valid provisioning data in flash or ram
	srv_provisioning_data_state_updated,		// RAM settings updated (needs to be saved)
	srv_provisioning_data_state_saved,		// RAM settings match those saved in flash.
} srv_provisioning_data_state_t;

srv_provisioning_data_state_t srv_provisioning_data_state(void);


typedef enum {
	srv_provisioning_mac_region_eu868 = 0,	// Default region is EU868
	srv_provisioning_mac_region_us915,
	srv_provisioning_mac_region_ru864,
	srv_provisioning_mac_region_as923,
	srv_provisioning_mac_region_au915,
	srv_provisioning_mac_region_kr920,
	srv_provisioning_mac_region_in865,
	srv_provisioning_mac_region_count
} srv_provisioning_mac_region_t;

srv_provisioning_status_t srv_provisioning_factory_settings(srv_provisioning_mac_region_t region);

typedef enum {
	srv_provisioning_activation_otaa = 0,		// Default activation is OTAA
	srv_provisioning_activation_abp,
	srv_provisioning_activation_count
} srv_provisioning_activation_t;

/*
 * For security reasons, keys can only be written, but not read back. To
 * determine whether a particular key has been stored, a bitmask is used.
 *
 */
typedef enum {
	srv_provisioning_key_id_app_s_key = 0,	// ABP
	srv_provisioning_key_id_nwk_s_key,		// ABP
	srv_provisioning_key_id_app_key,			// OTAA
	srv_provisioning_key_id_nwk_key,			// OTAA
	srv_provisioning_key_id_count
} srv_provisioning_key_id_t;

srv_provisioning_status_t srv_provisioning_set_key(srv_provisioning_key_id_t id, const srv_provisioning_key_t key);
srv_provisioning_status_t srv_provisioning_erase_key(srv_provisioning_key_id_t id);
bool srv_provisioning_key_is_set(srv_provisioning_key_id_t k);

srv_provisioning_status_t srv_provisioning_erase(void);
srv_provisioning_status_t srv_provisioning_read(void);
srv_provisioning_status_t srv_provisioning_save(void);

srv_provisioning_status_t srv_provisioning_get_lora_device_eui(srv_provisioning_eui_t ret_eui);
srv_provisioning_status_t srv_provisioning_set_lora_device_eui(srv_provisioning_eui_t eui);

srv_provisioning_status_t srv_provisioning_get_lora_join_eui(srv_provisioning_eui_t ret_eui);
srv_provisioning_status_t srv_provisioning_set_lora_join_eui(srv_provisioning_eui_t eui);

srv_provisioning_status_t srv_provisioning_set_lora_devaddr(uint32_t devaddr);
srv_provisioning_status_t srv_provisioning_get_lora_devaddr(uint32_t *devaddr);

srv_provisioning_status_t srv_provisioning_set_lora_activation(srv_provisioning_activation_t type);
srv_provisioning_status_t srv_provisioning_get_lora_activation(srv_provisioning_activation_t *type);

srv_provisioning_status_t srv_provisioning_set_lora_mac_region(srv_provisioning_mac_region_t region);
srv_provisioning_status_t srv_provisioning_get_lora_mac_region(srv_provisioning_mac_region_t *region);
