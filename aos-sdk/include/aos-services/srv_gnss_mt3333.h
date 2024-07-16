/*!
 * \file srv_gnss_mt3333.h
 *
 * \brief GNSS service using the MT3333
 *
 * \details
 * The MT3333 GNSS service provides the following facilities:
 * - Fully configurable GNSS position acquisition
 * - Read the MT3333 FW version
 * - Read GPS and BEIDOU almanacs
 * - Write the GPS almanac.
 *
 * Position acquisition
 * The position acquisition starts with the API function srv_gnss_mt3333_start. The structure
 * srv_gnss_mt3333_configuration_t, containing the configuration and the user callback, should be
 * filled.
 * The user callback function is triggered with an event type srv_gnss_mt3333_event_type_t, which
 * indicates the end of the position acquisition.
 * During the position acquisition, status messages are logged indicating the progress of the
 * acquisition (tracking data, fix and pseudo-range). If such messages are not expected, the
 * user can prevent them by setting the appropriate log level of the GNSS traces.
 * Once the position acquisition ends up, the GNSS chip is set to either power dow or in standby. This
 * power selection is done via the standby_timeout configuration parameter: When set to 0, the power
 * is set to off after the acquisition. If non null, the power is set to standby until the standby_timeout
 * is reached, after this delay, it is powered off.
 *
 * Querying
 * A query can be done at any time and does not require the GNSS to be powered on before:
 * Before sending the query, the service checks the power state and power on the chip if needed.
 * A specific query callback should be provided along to a query request. This allows separates
 * callbacks between the acquisitions and the queries.
 * The callback should return the value true, if the user wishes to keep the GNSS powered up (to
 * send another request for example) or false to stop the GNSS.
 * Once the query is complete (answered or got a status), the service will act as follow:
 *  1. Let the GNSS powered up if the query callback returns true.
 *  2. Let the GNSS powered up, if an acquisition is still in progress.
 *  3. Put the GNSS power to standby, if the standby timer did not expire.
 *  4. Put the GNSS power off otherwise.
 * Notes:
 *  - the service supports only one query at a time. The previous query must be ended before
 *    doing another one.
 *  - Setting a GPS almanac entry will restart the standby timer.
 */
#pragma once

#include "aos_system.h"
#include "aos_gnss_common.h"
#include "aos_rtc.h"
#include "srv_gnss.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup gnss_srv_mt3333 GNSS MT3333 service
 *
 * \addtogroup gnss_srv_mt3333
 * @{
 */

/*!
 * \enum srv_gnss_mt3333_event_type_t
 *
 * \brief Event type sent to the user
 */
typedef enum {
	srv_gnss_mt3333_event_error,				//!< GNSS start error
	srv_gnss_mt3333_event_fix_success,			//!< GNSS fix success
	srv_gnss_mt3333_event_agps_success,			//!< AGPS complete (enough data for resolution)
    srv_gnss_mt3333_event_no_sat_timeout,		//!< T0 timer (not satellite in view after T0), GNSS fix and AGPS
    srv_gnss_mt3333_event_fix_acq_timeout,		//!< No GNSS fix after acquisition timeout (fix_acq_timeout reached)
    srv_gnss_mt3333_event_fix_t1_timeout,		//!< No GNSS fix after t1_timeout
	srv_gnss_mt3333_event_abort,				//!< GNSS aborted by user
	srv_gnss_mt3333_event_last					//!< Last driver in the list
} srv_gnss_mt3333_event_type_t;


/*!
 * \enum srv_gnss_mt3333_state_t
 *
 * \brief State of the service
 */
typedef enum {
	srv_gnss_mt3333_state_none = 0,		//!< Not initialized
	srv_gnss_mt3333_state_ready,		//!< Service is initialized and is waiting for a start
	srv_gnss_mt3333_state_starting,		//!< Service is starting
	srv_gnss_mt3333_state_configuring,	//!< Service has started and the chip is being configured
	srv_gnss_mt3333_state_updating,		//!< Service has started and is configured. Local information are being sent to the chip
	srv_gnss_mt3333_state_running,		//!< Service is running
	srv_gnss_mt3333_state_standby,		//!< Chip is in standby (has been started at least once)
	srv_gnss_mt3333_state_hold_on,		//!< Chip is in hold_on mode (has been started at least once)
	srv_gnss_mt3333_state_stopping,		//!< Service is stopping
	srv_gnss_mt3333_state_last			//!< Last state
} srv_gnss_mt3333_state_t;

/*!
 * \struct srv_gnss_mt3333_usage_duration_t
 *
 * \brief GNSS usage duration. To be used for power consumption estimation
 */
typedef struct {
	uint64_t running;							//!< Duration when GNSS running (step 100us)
	uint64_t standby;							//!< Duration when GNSS in standby mode (step 100 us)
} srv_gnss_mt3333_usage_duration_t;

/*!
 * \struct srv_gnss_mt3333_sat_track_info_t
 *
 * \brief Data for one satellite upon timeout
 *
 *\note Tracking data from aos_gnss_track_data_t are too large. They are reduced by using
 * this structure
 */
typedef struct {
	aos_gnss_constellation_t constellation;	//!< Constellation
	uint8_t	svid;							//!< Satellite identifier
	uint8_t cn0;							//!< C/N0
} srv_gnss_mt3333_sat_track_info_t;

/*!
 * \struct srv_gnss_mt3333_track_info_t
 *
 * \brief Data for fix timeout
 */
typedef struct {
	uint8_t	nb_sat;														//!< Number of satellite seen
	aos_rtc_systime_t systime; 											//!< RTC date/time
	srv_gnss_mt3333_sat_track_info_t sat_info[AOS_GNSS_MAX_SATELLITES];	//!< Satellite list
} srv_gnss_mt3333_track_info_t;

/*!
 * \struct srv_gnss_mt3333_event_info_t
 *
 * \brief Event sent to the user
 */
typedef struct {
	srv_gnss_mt3333_event_type_t event;				//!< Event type
	union {
		aos_gnss_fix_info_t * fix_info;				//!< Fix information. Belongs to event srv_gnss_mt3333_event_fix_success
		srv_gnss_mt3333_track_info_t* track_info;	//!< Tracking information. Belongs to event srv_gnss_mt3333_event_fix_timeout
		aos_gnss_satellite_prn_report_t* prn_report;	//!< PRN report (LP-GPS). Belongs to event srv_gnss_mt3333_event_agps_success
	};
} srv_gnss_mt3333_event_info_t;


/*!
 * \fn void srv_gnss_mt3333_user_callback_t(void* user_arg, srv_gnss_mt3333_event_info_t* info)
 *
 * \brief: User callback definition
 *
 * \param user_arg User argument (opaque for the service)
 * \param info Pointer to the event information
 *
 * \note info remains valid until the next request is started.
 */
typedef void (* srv_gnss_mt3333_user_callback_t)(void* user_arg, srv_gnss_mt3333_event_info_t* info);

/*!
 * \enum srv_gnss_mt3333_local_info_type_t
 *
 * \brief Local information type that we can send to the GNSS chip to speedup the first fix
 */
typedef enum {
	srv_gnss_mt3333_loc_info_type_none = 0,			//!< No local information is provided
	srv_gnss_mt3333_loc_info_type_time_only,		//!< Only the time information (from the RTC) is provided.
	srv_gnss_mt3333_loc_info_type_time_local_pos,	//!< The time information (from the RTC) and the last GNSS position are provided
	srv_gnss_mt3333_loc_info_type_time_user_pos		//!< The time information (from the RTC) and a user position are provided
} srv_gnss_mt3333_local_info_type_t;

/*!
 * \struct srv_gnss_mt3333_cfg_local_info_t
 *
 * \brief Local information to be sent to the GNSS chip to speedup the first fix.
 *
 * \note user provided parameters are required only if the type is srv_gnss_mt3333_loc_info_type_time_user_pos.
 */
typedef struct {
	srv_gnss_mt3333_local_info_type_t type;	//!< Type of provided information
	int32_t lat;						//!< User provided latitude in 1E-7 degree
	int32_t lon;						//!< User provided longitude in 1E-7 degree
	int32_t alt;						//!< User provided  altitude in centimeter
	uint32_t ehpe;             			//!< User provided estimated horizontal position in centimeter
} srv_gnss_mt3333_cfg_local_info_t;


/*!
 * \struct srv_gnss_mt3333_configuration_t
 *
 * \brief Basic GNSS configuration
 */
typedef struct {
	srv_gnss_mode_xgnss_t mode;						//!< Mode for this driver
	aos_gnss_cfg_constellation_t constellations;	//!< Constellations to be used
	uint16_t fix_acq_timeout;						//!< Max acquisition time in seconds to get a GNSS fix
	uint16_t agnss_acq_timeout;						//!< Max acquisition time in seconds to get a valid aided GNSS
	uint16_t t0_timeout;							//!< Max time in seconds to see at least one satellite. 0 to disable the check
	uint16_t t1_timeout;							//!< Time in seconds to expect a fix. If null, the time is extended to the max GNSS acquisition timeout.
	uint16_t ehpe;									//!< Estimated horizontal position error in meters.
	uint16_t convergence_timeout;					//!< Max time in seconds from TTFF to converge to the given EHPE
	uint32_t standby_timeout;						//!< Duration for which we let the Chip in standby mode. 0 to disable the standby mode.
	aos_gnss_cfg_prn_t prn_cfg;						//!< Pseudo range configuration. Required only if AGPS used.
	bool update_systime_on_fix;						//!< If set to true, the system time will be updated upon a fix reception. False: No system time update
	bool hold_gnss_on;								//!< If set to true, the GNSS is kept on and keep the RF switch
	srv_gnss_mt3333_cfg_local_info_t local_info;	//!< Local information (time/position) to provide to the chip.
} srv_gnss_mt3333_configuration_t;


/*!
 * \enum srv_gnss_mt3333_query_status_t
 *
 * \brief Status of a query
 */
typedef enum {
	srv_gnss_mt3333_query_status_success,		//!< Query is successful
	srv_gnss_mt3333_query_status_failure,		//!< Query has failed
} srv_gnss_mt3333_query_status_t;

/*!
 * \enum srv_gnss_mt3333_query_type_t
 *
 * \brief Type of a query
 */
typedef enum {
	srv_gnss_mt3333_query_type_get_version,				//!< Request the version
	srv_gnss_mt3333_query_type_get_almanac_gps,			//!< Request the GPS almanac entry for a given satellite
	srv_gnss_mt3333_query_type_set_almanac_gps,			//!< Set a GPS almanac entry for a given satellite
	srv_gnss_mt3333_query_type_get_almanac_beidou,		//!< Request the BEIDOU almanac entry for a given satellite
} srv_gnss_mt3333_query_type_t;

/*!
 * \struct srv_gnss_mt3333_query_info_t
 *
 * \brief Data belonging to the answer of a query
 */
typedef struct {
	srv_gnss_mt3333_query_type_t type;					//!< Type of the query
	srv_gnss_mt3333_query_status_t status;				//!< Status of a query
	union {
		char* version;									//!< GNSS FW Version
		aos_gnss_almanac_standard_entry_t* almanac_entry;	//!< Almanac entry
	};
} srv_gnss_mt3333_query_info_t;


/*!
 *
 * \brief: User callback used for query
 *
 * \param user_arg User argument (opaque for the service)
 * \param query_info Information related to the query
 *
 * \return True if another query will follow (this prevent shutting down the driver between queries).
 * False if no subsequent requests.
 *
 * \note Data remain valid until the next request is performed
 *
 */
typedef bool (*srv_gnss_mt3333_query_callback_t)(void* user_arg, srv_gnss_mt3333_query_info_t* query_info);


/*!
 * \fn aos_result_t srv_gnss_mt3333_init(void)
 *
 * \brief Initialize the GNSS service
 *
 * \return Success, failure
 *
 * \note Should called only once at the initialization time.
 */
aos_result_t srv_gnss_mt3333_init(void);

/*!
 * \fn aos_result_t srv_gnss_mt3333_start(srv_gnss_mt3333_user_callback_t callback, void* user_arg, const srv_gnss_mt3333_configuration_t* config)
 *
 * \brief Start a GPS/AGPS acquisition.
 *
 * \param callback User Callback function
 * \param user_arg User argument. Opaque for the service
 * \param config Basic configuration
 *
 * \return  Success, failure
 *
 * \note Upon start success, the service will do the acquisition based on the
 * provided configuration. Once complete, the user callback is triggered with
 * an event structure containing the result.
 * The srv_gnss_mt3333_event_error will not contain any data.
 * Once ended, the service will put the GNSS in standby mode for the duration of standby_timeout.
 * Once this time elapses, the GNSS chip is powered off by the service.
 */
aos_result_t srv_gnss_mt3333_start(srv_gnss_mt3333_user_callback_t callback, void* user_arg, const srv_gnss_mt3333_configuration_t* config);

/*!
 * \fn aos_result_t srv_gnss_mt3333_stop(void)
 *
 * \brief Stop a GNSS device service and put it in power off state
 *
 * \return Success, failure
 *
 * \note: The GNSS device can be stopped in the states: starting, running and standby
 */
aos_result_t srv_gnss_mt3333_stop(void);

/*!
 * \fn aos_result_t aos_result_t srv_gnss_mt3333_hold_on_stop(void)
 *
 * \brief Stop the GNSS hold_on mode
 *
 * \return None
 *
 * \note: If the standby timeout is configured, the device  is moved to the standby state
 * otherwise the device is powered off. The RF switch is also released.
 */
void srv_gnss_mt3333_hold_on_stop(void);

/*!
 * \fn aos_result_t srv_gnss_mt3333_abort_acquisition(void)
 *
 * \brief Abort the current acquisition (fix or PRN).
 *
 * \return Success, failure
 *
 * \note: The GNSS device will move in standby or off depending on the configuration
 */
aos_result_t srv_gnss_mt3333_abort_acquisition(void);

/*!
 * \fn aos_result_t srv_gnss_mt3333_get_prn_report(aos_gnss_satellite_prn_report_t ** prn_report)
 *
 * \brief Retrieve the last Pseudo-range report
 *
 * \param prn_report Pointer where the data are located.
 * \return   success , failure.
 *
 * \note The call to this function does not require having the GNSS opened.
 *
 */
aos_result_t srv_gnss_mt3333_get_prn_report(aos_gnss_satellite_prn_report_t** prn_report);

/*!
 * \fn aos_result_t srv_gnss_mt3333_get_fix(aos_gnss_fix_info_t ** fix_info)
 *
 * \brief Retrieve the last fix. MT3333 only
 *
 * \param fix_info Pointer where the data are located.
 * \return   success , failure.
 *
 * \note MT3333 only. The call to this function does not require having the GNSS opened.
 *
 */
aos_result_t srv_gnss_mt3333_get_fix(aos_gnss_fix_info_t ** fix_info);


/*!
 * \fn aos_result_t srv_gnss_mt3333_get_tracking_data(srv_gnss_mt3333_track_info_t** tracking)
 *
 * \brief Retrieve the last tracking information
 *
 * \param tracking Pointer where the data are located.
 *
 * \return   success , failure.
 *
 * \note MT3333 only. The call to this function does not require having the GNSS opened.
 *
 */
aos_result_t srv_gnss_mt3333_get_tracking_data(srv_gnss_mt3333_track_info_t** tracking);

/*!
 * \fn srv_gnss_mt3333_state_t srv_gnss_mt3333_get_state(void)
 *
 * \brief Return the state of the GNSS service
 *
 * \return The state of the service
 *
 */
srv_gnss_mt3333_state_t srv_gnss_mt3333_get_state(void);

/*!
 * \fn aos_result_t srv_gnss_mt3333_get_version(srv_gnss_mt3333_query_callback_t callback, void* user_arg)
 *
 * \brief Request the MT3333 FW version
 *
 * \param callback User callback to receive the answer
 * \param user_arg User argument passed along to the callback. Opaque for the service.
 *
 * \return The status of the operation
 *
 */
aos_result_t srv_gnss_mt3333_get_version(srv_gnss_mt3333_query_callback_t callback, void* user_arg);

/*!
 * \fn bool srv_gnss_mt3333_is_acq_in_progress(void)
 *
 * \brief Check if the MT3333 is acquising a position
 *
 * \return True is a position is being acquired, false otherwise
 *
 */
bool srv_gnss_mt3333_is_acq_in_progress(void);

/*!
 * \fn aos_result_t srv_gnss_mt3333_get_almanac_gps(srv_gnss_mt3333_query_callback_t callback, void* user_arg, uint16_t svid)
 *
 * \brief Request a GPS Almanac entry
 *
 * \param callback User callback to receive the answer
 * \param user_arg User argument passed along to the callback. Opaque for the service.
 * \param svid Satellite identifier for which we want the Almanac entry. Start at 1.
 *
 * \return The status of the operation
 *
 */
aos_result_t srv_gnss_mt3333_get_almanac_gps(srv_gnss_mt3333_query_callback_t callback, void* user_arg, uint16_t svid);

/*!
 * \fn aos_result_t srv_gnss_mt3333_get_almanac_beidou(srv_gnss_mt3333_query_callback_t callback, void* user_arg, uint16_t svid)
 *
 * \brief Request a GPS Almanac entry
 *
 * \param callback User callback to receive the answer
 * \param user_arg User argument passed along to the callback. Opaque for the service.
 * \param svid Satellite identifier for which we want the Almanac entry.Start at 1.
 *
 * \return The status of the operation
 *
 */
aos_result_t srv_gnss_mt3333_get_almanac_beidou(srv_gnss_mt3333_query_callback_t callback, void* user_arg, uint16_t svid);

/*!
 * \fn aos_result_t srv_gnss_mt3333_set_almanac_gps(srv_gnss_mt3333_query_callback_t callback, void* user_arg, aos_gnss_almanac_entry_t* entry)
 *
 * \brief Set a GPS Almanac entry
 *
 * \param callback User callback to receive the answer
 * \param user_arg User argument passed along to the callback. Opaque for the service.
 * \param entry Almanac entry. 8 words of 24 bits should be set. Start at 1.
 *
 * \return The status of the operation
 *
 */
aos_result_t srv_gnss_mt3333_set_almanac_gps(srv_gnss_mt3333_query_callback_t callback, void* user_arg, aos_gnss_almanac_entry_t* entry);

/*!
 * \fn aos_result_t srv_gnss_mt3333_convert_almanac_gps_entry(aos_gnss_almanac_entry_t* entry)
 *
 * \brief Convert a standard GPS Almanac entry to a reduced one
 *
 * \param entry Almanac generic entry. In input, it contains the standard entry. In output, it contains the reduced entry.
 *
 * \return The status of the operation
 *
 */
aos_result_t srv_gnss_mt3333_convert_almanac_gps_entry(aos_gnss_almanac_entry_t* entry);

/*!
 * \fn aos_gnss_power_t srv_gnss_mt3333_get_power(void)
 *
 * \brief Return the current power state
 *
 * \return The power
 */
aos_gnss_power_t srv_gnss_mt3333_get_power(void);

/*!
 * \fn aos_result_t srv_gnss_mt3333_set_power(aos_gnss_power_t power)
 *
 * \brief Set the power
 * \param power Power to set
 *
 * \return The power
 */
aos_result_t srv_gnss_mt3333_set_power(aos_gnss_power_t power);

/*!
 * \fn  aos_gnss_counters_t* srv_gnss_mt3333_get_stats(void)
 *
 * \brief Retrieve the GNSS statistics
 *
 * \return The counters
 *
 */
const aos_gnss_counters_t* srv_gnss_mt3333_get_stats(void);


/*!
 * \fn aos_result_t srv_gnss_mt3333_clear_stats(void)
 *
 * \brief Clear the GNSS statistics
 *
 * \return The status of the operation
 *
 */
aos_result_t srv_gnss_mt3333_clear_stats(void);

/*!
 * \fn srv_gnss_mt3333_usage_duration_t srv_gnss_mt3333_get_usage_duration(void)
 *
 * \brief Retrieve the GNSS usage duration.
 *
 * \return The usage duration counters (step 100us)
 *
 */
const srv_gnss_mt3333_usage_duration_t* srv_gnss_mt3333_get_usage_duration(void);

/*!
 * \fn void srv_gnss_mt3333_clear_usage_duration(void)
 *
 * \brief Clear the GNSS usage duration.
 *
 */
void srv_gnss_mt3333_clear_usage_duration(void);

/*!
 * \fn uint64_t srv_gnss_mt3333_get_consumption_uah(void)
 *
 * \brief Retrieve the MT3333 energy consumption
 *
 * \return MT3333 energy consumption
 */
uint64_t srv_gnss_mt3333_get_consumption_uah(void);

/*! @}*/
#ifdef __cplusplus
}
#endif


