/*!
 * \file geolocation.c
 *
 * \brief Manage the geolocation
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"


#include "aos_system.h"
#include "aos_log.h"

#include "srv_geoloc_basic.h"

#include "events.h"
#include "geolocation.h"
#include "lora.h"

#define APP_TRACE(...)	do { aos_log_status(aos_log_module_app, true, __VA_ARGS__);  } while (0)

/*
 * ----------------------------------------------------------------------------
 * Technologies configuration
 * ----------------------------------------------------------------------------
 */
srv_ble_scan_param_t _ble_config = {
		.scan_duration = 3000,					//!< Scan duration (milliseconds)
		.scan_window = 120,						//!< Scan window (in step of 0.625 ms)
		.scan_interval = 130,					//!< Scan interval (in step of 0.625 ms)
		.repeat_delay = 0,								//!< Continuous scanning: Duration (in seconds) between 2 scans. Value of 0 indicate single shot scan
		.ble_scan_type = srv_ble_scan_beacon_type_all,	//!< Type of beacons we consider.
		.filters[0] = {0},							//!< Filter 1 to be applied
		.filters[1] = {0},							//!< Filter 2 to be applied
		.report = {0},							//!< Report configuration
		.rssi_threshold = 0,					//!< Minimum RSSI to consider a beacon
		.adv_compensation = {0},				//!< Advertised channel compensation
};


aos_lr1110_gnss_settings_t _lr1110_gnss_config = {
	.constellation = aos_gnss_cfg_constellation_gps_beidou,
	.scan_mode = aos_lr1110_gnss_scan_mode_nav1,
	.scan_type = aos_lr1110_gnss_scan_type_assisted,
	.nb_scans = 2,
	.inter_scan_time = 2,		// 2 seconds between scans
	.position.provided = true,	// Must be provided
	.position.latitude = 43.6156166, // Put here your position latitude
	.position.longitude = 7.0663950		// Put here your position longitude
};

#define WIFI_TIMEOUT_PER_CHANNEL			500
#define WIFI_TIMEOUT_PER_SCAN_DEFAULT		130
aos_wifi_settings_t _lr1110_wifi_config = {
	    /* Init settings */
	.channels_mask       = aos_wifi_scan_channel_mask_channel_all,
	.types               = aos_wifi_scan_signal_type_scan_b_g_n,
	.max_results         = 32,
	.timeout_per_channel = WIFI_TIMEOUT_PER_CHANNEL,
	.timeout_per_scan    = WIFI_TIMEOUT_PER_SCAN_DEFAULT
};

#define MIN_C_N_PRN_FILTER	15
#define MIN_SYNC_PRN_FILTER	AOS_GNSS_SAT_SYNC_BIT
srv_gnss_mt3333_configuration_t _mt3333_gnss_config = {
		.mode = srv_gnss_mode_gnss,
		.constellations = aos_gnss_cfg_constellation_gps_beidou,
		.fix_acq_timeout = 300,
		.agnss_acq_timeout = 30,
		.t0_timeout = 60,
		.t1_timeout = 0,
		.ehpe = 20,
		.convergence_timeout = 30,
		.standby_timeout = 14400, //(4 hours)
		.update_systime_on_fix = true,	// Let's update the system time on fix reception
		.hold_gnss_on = false,
		.local_info = {
				.type = srv_gnss_mt3333_loc_info_type_none,
				.lat = 0,
				.lon = 0,
				.alt = 0,
				.ehpe = 0
		},
		.prn_cfg = {
				.min_stable_sats = AOS_GNSS_PRN_MIN_NB_SATS,
				.min_cn = MIN_C_N_PRN_FILTER,
				.min_sync = MIN_SYNC_PRN_FILTER,
				.compress_prn = true
		}
};


// Local context
typedef struct {
	srv_geoloc_basic_configuration_t cfg;
} geoloc_ctx_t;

static geoloc_ctx_t _geoloc_ctx;

/*!
  * \brief Initialization
  */
void geolocation_init(void)
{
	// Setup the default configuration
	_geoloc_ctx.cfg.nb_techno = 3;

	// First techno: GNSS
	_geoloc_ctx.cfg.scheduling[0].type = srv_geolocation_type_gnss;
	_geoloc_ctx.cfg.scheduling[0].action = srv_geoloc_basic_action_always_acquire; // Note that the first techno is always done
	_geoloc_ctx.cfg.scheduling[0].cfg.mt_gnss = &_mt3333_gnss_config;

	// Second techo: WIFI. Done only is GNSS fails
	_geoloc_ctx.cfg.scheduling[1].type = srv_geolocation_type_wifi;
	_geoloc_ctx.cfg.scheduling[1].action = srv_geoloc_basic_action_skip_if_success;
	_geoloc_ctx.cfg.scheduling[1].cfg.wifi = &_lr1110_wifi_config;

	// Third techo: BLE. Always done
	_geoloc_ctx.cfg.scheduling[2].type = srv_geolocation_type_ble_scan1;
	_geoloc_ctx.cfg.scheduling[2].action = srv_geoloc_basic_action_always_acquire;
	_geoloc_ctx.cfg.scheduling[2].cfg.ble = &_ble_config;

	// Initialize all technologies used. Note that BLE scan does not need to be initialized.
	srv_gnss_mt3333_init();									// GNSS MT3333 service
	srv_gnss_lr1110_init();									// Initialize in case the new configuration use it
	srv_wifi_scan_init();									// WIFI service

	// ... and the GBE service
	srv_geoloc_basic_init();
}

/*!
  * \brief geolocation callback
  */
static void _geolocation_callback(void* user_arg, srv_geolocation_result_t* results)
{
	geoloc_ctx_t* ctx = user_arg;

	// Switch the action of the WIFI
	if (ctx->cfg.scheduling[1].action == srv_geoloc_basic_action_skip_if_success) {
		ctx->cfg.scheduling[1].action = srv_geoloc_basic_action_always_acquire;
	} else {
		ctx->cfg.scheduling[1].action = srv_geoloc_basic_action_skip_if_success;
	}

	APP_TRACE("Geolocation complete. nb techno: %d\n", results->nb_techno);

	// Send the event
	event_send(app_event_geolocation_done);
}

/*!
  * \brief start the geolocation
  */
bool geolocation_start(void)
{
	aos_result_t result;

	result = srv_geoloc_basic_start(_geolocation_callback, &_geoloc_ctx, &_geoloc_ctx.cfg);
	APP_TRACE("Geolocation start. Result: %d\n", result);
	return true;
}

/*!
  * \brief Stop the geolocation
  */
void geolocation_abort(void)
{
	srv_geoloc_basic_abort();
}

/*!
  * \brief Configure the geolocation
  *
  * \param byte_array Each bytes of the byte-array contains the geolocation type on the first 6 bits and
  * the geolocation action on bit 7 and 6. The end of the array should have the action none
  * \param size Size of the array
  *
  * \return true if correct config, false otherwise
  */
bool geolocation_techno_config(uint8_t* byte_array, uint8_t size)
{
	uint8_t gcount = 0;
	uint8_t ii;
	srv_geolocation_type_t gtype;
	srv_geoloc_basic_action_t action;

	// Sanity
	if (!byte_array) {
		return false;
	}

	// Limit the number of techno
	if (size > srv_geolocation_type_count) {
		size = srv_geolocation_type_count;
	}

	for (ii=0; ii < size; ii ++) {
		gtype = *byte_array & 0x3F;
		action = *byte_array >> 6;
		if ((gtype == srv_geolocation_type_none) || (action == srv_geoloc_basic_action_none)) {
			// We are done
			break;
		}
		// Store
		_geoloc_ctx.cfg.scheduling[gcount].type = gtype;
		_geoloc_ctx.cfg.scheduling[gcount].action = action;
		gcount ++;
		byte_array ++;
	}

	if (gcount == 0) {
		return false;
	}
	_geoloc_ctx.cfg.nb_techno = gcount;
	return true;
}

/*!
  * \brief Configure the BLE nb beacons
  *
  * \param nb_beacons Number of beacons
  * \return true if correct config, false otherwise
  */
bool geolocation_ble_config(uint8_t nb_beacons)
{
	if (nb_beacons > SRV_BLE_MAX_SCAN_RESULT) {
		return false;
	}

	_ble_config.report.nb_beacons = nb_beacons;
	return true;
}

/*!
  * \brief Send the geolocation result via LoRa
  *
  * This is an example of what you could send.

  */
void geolocation_send_result(void)
{
	static uint8_t tx_buffer[AOS_LR1110_LORA_MAX_PAYLOAD_SIZE];
	const srv_geolocation_result_t* results;
	const srv_geolocation_result_per_techno_t* techno;
	uint8_t* tx_ptr;
	uint8_t tx_size;
	uint8_t ii;
	uint8_t max;
	uint8_t jj;
	bool max_reached = false;

	if (srv_geoloc_basic_get_results(&results) != aos_result_success) {
		APP_TRACE("Geoloc. Fails to get result\n");
		return;
	}

	// Initialize
	tx_ptr = tx_buffer;

	// First byte: number of techno
	*tx_ptr ++ = results->nb_techno;
	tx_size = 1;

	// Feed with techno
	for (ii=0; ii < results->nb_techno; ii ++) {
		techno = &results->results[ii];
		// Add mandatory techno type and status
		if (AOS_LR1110_LORA_MAX_PAYLOAD_SIZE - tx_size < 2) {
			break;
		}
		*tx_ptr ++ = techno->type;							// Add techno type
		*tx_ptr ++ = techno->status;						// Add the status
		tx_size += 2;
		if (techno->status != srv_geolocation_status_success) {
			// No results
			continue;
		}

		switch (techno->type) {
		case srv_geolocation_type_gnss:
			// latitude + longitude + EHPE
			if (AOS_LR1110_LORA_MAX_PAYLOAD_SIZE - tx_size < 10) {
				max_reached = true;
				break;
			}
			lora_copy32(tx_ptr, techno->data.mt_gnss.fix_info->lat);		// Latitude (big endian format)
			lora_copy32(tx_ptr + 4, techno->data.mt_gnss.fix_info->lon);	// Longitude (big endian format)
			lora_copy16(tx_ptr + 8, techno->data.mt_gnss.fix_info->ehpe);	// EHPE	(big endian format).
			tx_ptr += 10;
			tx_size += 10;
			break;

		case srv_geolocation_type_ble_scan1:
		case srv_geolocation_type_ble_scan2:
			// Copy the 2 first beacons MAC address
			max = (techno->data.ble->entry_cnt > 2) ? 2: techno->data.ble->report_type;
			for (jj=0; jj < max; jj++) {
				if (AOS_LR1110_LORA_MAX_PAYLOAD_SIZE - tx_size < SRV_BLE_MAX_SHORT_BEACON_ID_SIZE) {
					max_reached = true;
					break;
				}
				memcpy(tx_ptr, techno->data.ble->short_beacons[jj].identifier, SRV_BLE_MAX_SHORT_BEACON_ID_SIZE);	// MAC address
				tx_ptr += SRV_BLE_MAX_SHORT_BEACON_ID_SIZE;
				tx_size += SRV_BLE_MAX_SHORT_BEACON_ID_SIZE;
			}
			break;

		case srv_geolocation_type_wifi:
			// Copy the 2 first BSSID MAC address
			max = (techno->data.wifi->nb_scan_results > 2) ? 2: techno->data.wifi->nb_scan_results;
			for (jj=0; jj < max; jj++) {
				if (AOS_LR1110_LORA_MAX_PAYLOAD_SIZE - tx_size < AOS_WIFI_MAC_ADDRESS_SIZE) {
					max_reached = true;
					break;
				}
				memcpy(tx_ptr, techno->data.wifi->data->mac_address, AOS_WIFI_MAC_ADDRESS_SIZE);			// WIFI BSSID
				tx_ptr += AOS_WIFI_MAC_ADDRESS_SIZE;
				tx_size += AOS_WIFI_MAC_ADDRESS_SIZE;
			}
			break;

		default:
			// Not reported
			break;
		}
		if (max_reached) {
			break;
		}
	}

	lora_send(aos_lr1110_mgr_lora_dr2, 1, tx_size, tx_buffer);
}


