/*!
 *
 * \file   srv_geoloc_common.h
 *
 * \brief  Common definition to all geolocation engines
 *
 * Copyright (C) 2023, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "aos_log.h"
#include "srv_gnss_lr1110.h"
#include "srv_gnss_mt3333.h"
#include "srv_wifi_scan.h"
#include "srv_ble_scan.h"

/*!
 * \defgroup srv_geoloc_common Geolocation common definitions
 *
 * \addtogroup srv_geoloc_common
 * @{
 *
 */

// Trace facility
#define GEOLOC_WARNING(...)	do { aos_log_warning(aos_log_module_geolocation, true, __VA_ARGS__);  } while (0)	//!< Warning
#define GEOLOC_STATUS(...)	do { aos_log_status(aos_log_module_geolocation, true, __VA_ARGS__);  } while (0)		//!< status
#define GEOLOC_TRACE(...)	do { aos_log_msg(aos_log_module_geolocation, aos_log_level_debug, true, __VA_ARGS__);  } while (0) //!< Trace (debug)

/*!
 * \enum srv_geolocation_type_t
 * \brief Geolocation technologies
 *
 * \note Do not modify the order
 */
typedef enum {
	srv_geolocation_type_lr_agnss,      	//!< Aided-GNSS using the LR1110.
	srv_geolocation_type_wifi,      		//!< WIFI scan using the LR1110
	srv_geolocation_type_ble_scan1,			//!< BLE beacons scan 1
	srv_geolocation_type_ble_scan2,			//!< BLE beacons scan 2 (with a configuration different from ble1)
	srv_geolocation_type_mt_agnss,      	//!< Aided-GNSS using the MT3333
	srv_geolocation_type_gnss,      		//!< GNSS using the MT3333
	srv_geolocation_type_count,		 		//!< Number of technologies.Last in the list
	srv_geolocation_type_none = srv_geolocation_type_count,      		//!< No technology
} srv_geolocation_type_t;

/*!
 * \union srv_geolocation_techno_cfg_t
 * \brief Geolocation technologies configuration
 */
typedef union {
	aos_lr1110_gnss_settings_t* lr_agnss;		//!< Configuration of the Aided-GNSS using the LR1110
	srv_gnss_mt3333_configuration_t* mt_gnss;	//!< Configuration of the GNSS/Aided GNSS using the MT3333
	aos_wifi_settings_t* wifi;					//!< Configuration of the WIFI scan using the LR1110
	srv_ble_scan_param_t* ble;					//!< Configuration of the BLE scan
} srv_geolocation_techno_cfg_t;

/*!
 * \enum srv_geolocation_status_t
 * \brief Geolocation service status
 */
typedef enum {
	srv_geolocation_status_success,			//!< Done and success
	srv_geolocation_status_not_solvable,	//!< Done and success but not solvable
	srv_geolocation_status_skipped,			//!< Not done (skipped due to the configuration)
	srv_geolocation_status_failure,      	//!< Done but fail
	srv_geolocation_status_cancel,      	//!< Not done due to a user abort
	srv_geolocation_status_error_start,		//!< Not done, MT3333 or BLE fails to start
	srv_geolocation_status_error_lr_start_scan,	//!< LR GNSS start scan or WIFI start scan failure
	srv_geolocation_status_error_lr_cancel_scan,//!< LR GNSS cancel scan or WIFI cancel scan failure
} srv_geolocation_status_t;

/*!
 * \union srv_geolocation_result_data_t
 * \brief Geolocation result data
 */
typedef union {
	aos_lr1110_gnss_result_t* lr_gnss;		//!< Result for LR1110 aided-GNSS
	srv_gnss_mt3333_event_info_t mt_gnss;	//!< Result for MT3333 GNSS/aided-GNSS
	aos_wifi_scan_result_data_t* wifi;		//!< Results for WIFI
	srv_ble_scan_report_t* ble;				//!< Result for BLE scan
} srv_geolocation_result_data_t;

/*!
 * \struct srv_geolocation_result_per_techno_t
 * \brief Geolocation result per technology
 */
typedef struct {
	srv_geolocation_type_t type;				//!< Geolocation technology
	srv_geolocation_status_t status;			//!< Status
	srv_geolocation_result_data_t data;			//!< Geolocation data and local status
} srv_geolocation_result_per_techno_t;

/*!
 * \struct srv_geolocation_result_t
 * \brief Geolocation result per technology
 */
typedef struct {
	uint8_t nb_techno;						//!< Number of technologies in the results
	srv_geolocation_result_per_techno_t results[srv_geolocation_type_count]; //!< Results per technology
} srv_geolocation_result_t;


/*!
 * \fn void (* srv_geolocation_callback_t)(void* user_arg, srv_geolocation_event_t* event)
 *
 * \brief User callback used for query
 *
 * \param user_arg User argument (opaque for the service)
 * \param results Results provided by the geolocation engine
 */
typedef void (* srv_geolocation_callback_t)(void* user_arg, srv_geolocation_result_t* results);


/*!
 * \fn const char* srv_geoloc_common_status_to_str(srv_geolocation_status_t status)
 *
 * \brief Convert the geolocation status to a displayable ASCII string
 *
 * \param status Status to convert
 *
 * \return The string
 */
const char* srv_geoloc_common_status_to_str(srv_geolocation_status_t status);

/*!
 * \fn const char* srv_geoloc_common_type_to_str(srv_geolocation_type_t type)
 *
 * \brief Convert the geolocation type to a displayable ASCII string
 *
 * \param type type to convert
 *
 * \return The string
 */
const char* srv_geoloc_common_type_to_str(srv_geolocation_type_t type);

/*! @}*/
#ifdef __cplusplus
}
#endif

