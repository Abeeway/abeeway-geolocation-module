/**
 * \file      aos_lr1110_gnss.h
 *
 * \brief     Basic LR1110 GNSS scan driver
 *
 * \details	  This file provides a facility running on top of the Semtech GNSS middleware
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "smtc_modem_api.h"
#include "ralf.h"

#include "aos_gnss_common.h"

#define AOS_LR1110_GNSS_MAX_SATELLITES  	10		//!< Max number of detected satellites
#define AOS_LR1110_GNSS_MAX_SCAN  			4		//!< Max number of scan for one position acquisition

#define AOS_LR1110_GNSS_RAW_NAV_DATA_SIZE ( 80 ) 	//!< status (1) + constellation data (7) + max_sat * per_sat data(7) + 1 spare

#define AOS_LR11110_GNSS_ALMANAC_ENTRY_SIZE	22		//!< Size of an almanac entry including the Time of week

#define AOS_LR11110_GNSS_ALMANAC_BEIDOU_SVID_OFFSET		64 //!< Offset in the SVID


/*!
 * \brief GNSS status
 */
typedef enum {
	aos_lr1110_gnss_status_success,  		//!< The scan operation has been successfully completed
	aos_lr1110_gnss_status_in_progress,		//!< The scan is in progress
	aos_lr1110_gnss_status_error,  			//!< The scan operation has failed for an unknown reason
	aos_lr1110_gnss_status_parser_error,	//!< The scan operation has failed due to a NAV1 parsing error
	aos_lr1110_gnss_status_cancel,  		//!< The scan operation has been cancelled by the user
	aos_lr1110_gnss_status_abort_no_time,	//!< Scan aborted. Time needed.
	aos_lr1110_gnss_status_abort_almanac,	//!< Scan aborted. Almanac needed
	aos_lr1110_gnss_status_abort_position,	//!< Scan aborted. Aiding position needed.
	aos_lr1110_gnss_status_abort_rp,		//!< Scan aborted by the radio planer.
	aos_lr1110_gnss_status_last				//!< Last entry
} aos_lr1110_gnss_status_t;


/*!
 * \brief Events sent to the lr1110 manager
 */
typedef enum {
	aos_lr1110_gnss_event_scan_ended = 0,	//!< Scan ended
	aos_lr1110_gnss_event_query_done,	//!< Query done
} aos_lr1110_gnss_event_type_t;

/*!
 * \brief GNSS scan type.
 */
typedef enum {
	aos_lr1110_gnss_scan_type_autonomous,	//!< No assistance position nor almanac
	aos_lr1110_gnss_scan_type_assisted,		//!< Assistance position and almanac present
} aos_lr1110_gnss_scan_type_t;

/*!
 * \brief GNSS scan mode
 */
typedef enum {
	aos_lr1110_gnss_scan_mode_nav1 = 1,		//!< Select NAV1 method
	aos_lr1110_gnss_scan_mode_nav2,		//!< Select NAV2 method
} aos_lr1110_gnss_scan_mode_t;


/*!
 * \brief Local information: current position.
 *
 * \ Details Provided to the LR11110 as assistance data
 */
typedef struct {
	bool provided;		//!< True if the local position is provided, false otherwise
    float latitude;		//!< Latitude 12 bits: latitude in degree * 2048/90) with resolution 0.044 degree
    float longitude;	//!< Longitude 12 bits: longitude in degree * 2048/180) with resolution 0.088 degree
} aos_lr1110_gnss_local_position_t;

/*!
 * \brief LR1110 GNSS scan configuration
 */
typedef struct {
	aos_gnss_cfg_constellation_t  constellation; //!< Constellation(s) to use
	aos_lr1110_gnss_scan_mode_t	scan_mode;			//!< Scan mode
	aos_lr1110_gnss_scan_type_t scan_type;			//!< Scan type
	uint8_t nb_scans;								//!< Number of scan for one position acquisition
	uint8_t inter_scan_time;						//!< Time between each scans in seconds
	aos_lr1110_gnss_local_position_t position;		//!< Local position estimate (aiding data)
} aos_lr1110_gnss_settings_t;


/*!
 * \brief LR1110 GNSS scan configuration. Follow the LR1110 bit mask
 */
typedef enum {
	aos_lr1110_gnss_constellation_update_none = 0,		//!< No almanac update needed
	aos_lr1110_gnss_constellation_update_gps = 1,		//!< GPS almanac update needed
	aos_lr1110_gnss_constellation_update_beidou = 2,	//!< GPS almanac update needed
	aos_lr1110_gnss_constellation_update_gps_beidou = 3,//!< Both almanac update needed
}aos_lr1110_gnss_almanac_update_t;


/*!
 * \brief LR1110 GNSS scan result: navigation data
 */
typedef struct {
	uint8_t size;										//!< Size of the raw data
	uint8_t data[AOS_LR1110_GNSS_RAW_NAV_DATA_SIZE];	//!< Raw data (NAV1 or NAV2)
} aos_lr1110_gnss_nav_data_t;

/*!
 * \brief Result for a single scan
 */
typedef struct {
	uint32_t timestamp;							//!< System time in seconds
	aos_gnss_satellite_prn_report_t report;		//!< Formatted report. Valid only if NAV1
	aos_lr1110_gnss_nav_data_t nav;				//!< Non formatted navigation data provided by the LR11110 for this scan
} aos_lr1110_gnss_scan_data_t;

/*!
 * \brief GNSS data available to the user
 */
typedef struct {
	aos_lr1110_gnss_status_t status;								//!< Status of the acquisition
	aos_lr1110_gnss_scan_mode_t scan_mode;							//!< Scan mode done
	uint64_t power_consumption_uah;									//!< Consumed power during the scan
	bool almanac_gps_update;										//!< True if the GPS almanac should be updated
	bool almanac_beidou_update;										//!< True if the BEIDOU almanac should be updated
	bool position_update;											//!< True if the position should be updated
	uint8_t nb_scans;												//!< Number of scans valid
	aos_lr1110_gnss_scan_data_t scans[AOS_LR1110_GNSS_MAX_SCAN];	//!< acquisition results
} aos_lr1110_gnss_result_t;


/*!
 * \enum aos_lr1110_gnss_query_status_t
 *
 * \brief Status of a query
 */
typedef enum {
	aos_lr1110_gnss_query_status_success,		//!< Query is successful
	aos_lr1110_gnss_query_status_failure,		//!< Query has failed
} aos_lr1110_gnss_query_status_t;

/*!
 * \enum aos_lr1110_gnss_query_type_t
 *
 * \brief Type of a query
 */
typedef enum {
	aos_lr1110_gnss_query_type_none = 0,				//!< No query
	aos_lr1110_gnss_query_type_get_almanac,				//!< Request an almanac entry for a given satellite
	aos_lr1110_gnss_query_type_set_short_almanac,		//!< Set an almanac entry for a given satellite in a short format (15 bytes)
	aos_lr1110_gnss_query_type_set_long_almanac,		//!< Set an almanac entry for a given satellite in a long format (19 bytes)
} aos_lr1110_gnss_query_type_t;

/*!
 * \struct aos_lr1110_gnss_query_result_t
 *
 * \brief Data belonging to the answer of a query
 */
typedef struct {
	aos_lr1110_gnss_query_type_t type;					//!< Type of the query
	aos_lr1110_gnss_query_status_t status;				//!< Status of a query
	aos_gnss_constellation_t constellation; 			//!< Constellation
	aos_gnss_almanac_reduced_entry_t* almanac_entry;	//!< Almanac entry
} aos_lr1110_gnss_query_result_t;


/*!
 * \struct aos_lr1110_gnss_query_request_t
 *
 * \brief Data belonging to a GNSS query
 */
typedef struct {
	aos_lr1110_gnss_query_type_t type;					//!< Type of the query
	aos_gnss_constellation_t constellation;		//!< Constellation for which we want to read the almanac entry
	uint8_t svid;								//!< Satellite identifier (starting from 0), for which we want the entry
	bool last_entry;							//!< True if this is the last entry for an update.
	aos_gnss_almanac_reduced_entry_t* entry;	//!< Almanac entry to be written (applicable only to set an entry).
} aos_lr1110_gnss_query_request_t;

/*!
 *  \enum aos_lr1110_gnss_almanac_status_t
 *
 *  \brief Status of the almanac after the last update
 */
typedef enum {
	 aos_lr1110_gnss_almanac_status_ok = 0,		//!< No error
	 aos_lr1110_gnss_almanac_status_too_old,	//!< Almanac too old
	 aos_lr1110_gnss_almanac_status_crc_error,	//!< CRC mismatch for the updated entry
	 aos_lr1110_gnss_almanac_status_mem_error,	//!< Memory integrity error
	 aos_lr1110_gnss_almanac_status_upd_error	///!< Impossible to update more than one constellation at a time
} aos_lr1110_gnss_almanac_status_t;

/*!
 *  \struct aos_lr1110_gnss_info_t
 *
 *  \brief General information
 */
typedef struct {
    uint8_t firmware_version;						//!< GNSS Firmware version running on the LR1110
    aos_lr1110_gnss_almanac_status_t alm_status;	//!< Almanac status
 }  aos_lr1110_gnss_info_t;

/*!
 * \brief Initialize the driver
 *
 * \param[in] modem_radio The interface of the modem
 *
 * \return The operation result
 */
smtc_modem_return_code_t aos_lr1110_gnss_init(ralf_t* modem_radio);

/*!
 * \brief Start a GNSS scan
 *
 * \param settings Configuration of the scan
 *
 * \return The operation result
 */
smtc_modem_return_code_t aos_lr1110_gnss_start(aos_lr1110_gnss_settings_t* settings);

/*!
 * \brief Stop a GNSS scan
 *
 * \details. If a scan was actually in progress, it is aborted and the user callback
 * will be triggered with an abort status. Otherwise the user callback is not triggered,
 * \return The operation result:
 * SMTC_MODEM_RC_OK if the scan can be aborted,
 * SMTC_MODEM_RC_FAIL otherwise
 */
smtc_modem_return_code_t aos_lr1110_gnss_stop(void);

/*!
 * \brief Get the scan result
 *
 * \param scan_result: Handle to the result stored in aos_lr1110_gnss,
 *
 * \return The operation result
 */
smtc_modem_return_code_t aos_lr1110_gnss_get_result(aos_lr1110_gnss_result_t** scan_result);

/*!
 * \brief Perform a GNSS query (almanac)
 *
 * \param request Query request
 *
 * \note asynchronous request
 */
smtc_modem_return_code_t aos_lr1110_gnss_query(aos_lr1110_gnss_query_request_t* request);

/*!
 * \brief Get the result of the last query
 *
 * \return The query result
 */
const aos_lr1110_gnss_query_result_t* aos_lr1110_gnss_get_query_result(void);

/*!
 * \brief Get the general information
 *
 * \param info Location where the data will be copied
 *
 *
 * \return The operation result
 */
smtc_modem_return_code_t aos_lr1110_gnss_get_info(aos_lr1110_gnss_info_t* info);

/*!
 * \brief Get and clear the events
 *
 * \return The events (bit field)
 */
uint32_t aos_lr1110_gnss_get_and_clear_events(void);

#ifdef __cplusplus
}
#endif


/* --- EOF ------------------------------------------------------------------ */
