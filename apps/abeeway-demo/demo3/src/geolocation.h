/*
 * @file config.h
 *
 * Configuration
 */

#pragma once

void geolocation_init(void);

bool geolocation_start(void);
void geolocation_abort(void);

bool geolocation_techno_config(uint8_t* byte_array, uint8_t size);
bool geolocation_ble_config(uint8_t nb_beacons);

void geolocation_send_result(void);
