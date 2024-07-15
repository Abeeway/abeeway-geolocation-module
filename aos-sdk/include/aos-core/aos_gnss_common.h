/*!
 * \file aos_gnss_common.h
 *
 * \brief Common GNSS definition (shared between drivers and application)
 *
 * \copyright 2022, Abeeway. All Rights Reserved.
 *
 */

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup gnss_common GNSS common definitions
 *
 * \addtogroup gnss_common
 * @{
 */

#define AOS_GNSS_NB_GPS_SATS	32	//!< Total Number of satellites in the GPS constellation
#define AOS_GNSS_NB_BEIDOU_SATS	37	//!< Total Number of satellites in the BEIDOU constellation


/*!
 * \def AOS_GNSS_MAX_SATELLITES
 *
 * \brief Define the maximum number of satellites that AOS can handle in a report
 */
#define AOS_GNSS_MAX_SATELLITES    12


/*!
 * \def AOS_GNSS_SAT_SYNC_TIME
 *
 * \brief The satellite seen is synchronized with time.
 * \note Sync time is coded on 1 bit, bit 0.
 */
#define AOS_GNSS_SAT_SYNC_TIME           (1 << 0)

/*!
 * \def AOS_GNSS_SAT_SYNC_BIT
 *
 * \brief Satellite is synchronized at the bit level.
 * \note Sync state is coded on 2 bits
 */
#define AOS_GNSS_SAT_SYNC_BIT            (1)

/*!
 * \def AOS_GNSS_SAT_SYNC_FRAME
 *
 * \brief Satellite is synchronized at the frame level.
 * \note Sync state is coded on 2 bits
 */
#define AOS_GNSS_SAT_SYNC_FRAME            (2)

/*!
 * \def AOS_GNSS_SAT_SYNC_EXACT
 *
 * \brief Satellite is synchronized at the frame level.
 * \note Sync state is coded on 2 bits
 */
#define AOS_GNSS_SAT_SYNC_EXACT          (3)

/*!
 * \def AOS_GNSS_SAT_SYNC_SET_STATE(value)
 *
 * \brief Helper setting the sync state
 * \param value Synchronization state to setup
 */
#define AOS_GNSS_SAT_SYNC_SET_STATE(value) (value << 1)


/*!
 * \def AOS_GNSS_SAT_SYNC_GET_STATE(value)
 *
 * \brief Helper getting the sync state
 * \param value Synchronization value for which we want the state
 */
#define AOS_GNSS_SAT_SYNC_GET_STATE(value) (value >> 1)

/*!
 * \def NMEA_MAX_MSG_LEN
 *
 * \brief Maximum size of a NMEA message
 */
#define NMEA_MAX_MSG_LEN    256

/*!
 * \def NMEA_MAX_NB_FIELD
 *
 * \brief Maximum number of fields carried in a NMEA message. The number of fields retrieved from a NMEA
 * Message is limited to this value.
 */
#define NMEA_MAX_NB_FIELD   22

/*!
 * \enum aos_gnss_constellation_t
 *
 * \brief Supported constellation types
 *
 * \warning DO NOT MODIFY THE ORDER
 */
typedef enum {
	aos_gnss_constellation_unknown = 0,			//!< Unknown constellation
	aos_gnss_constellation_gps,					//!< GPS constellation
	aos_gnss_constellation_glonass,				//!< GLONASS constellation
	aos_gnss_constellation_beidou,				//!< BEIDOU constellation
	aos_gnss_constellation_gnss,				//!< Multi-constellation
	aos_gnss_constellation_galileo,				//!< GALILEO constellation
	aos_gnss_constellation_count				//!< Number of constellation. Must be the last.
} aos_gnss_constellation_t;


/*!
 * \enum aos_gnss_fix_status_t
 *
 * \brief Quality of a GNSS fix.
 */
typedef enum {
	aos_gnss_fix_invalid,		//!< Invalid
	aos_gnss_fix_valid,			//!< Valid but no idea if 2d or 3d
	aos_gnss_fix_valid_2d,		//!< Valid. Fix in 2 dimensions
	aos_gnss_fix_valid_3d		//!< Valid. Fix in 3 dimensions
} aos_gnss_fix_status_t;


/*!
 * \enum aos_gnss_time_type_t
 *
 * \brief Type of GNSS time information
 */
typedef enum {
	aos_gnss_time_type_software,	//!< GPS software time in micro-sec. This is the Time Of Week (TWO), without the week number.
	aos_gnss_time_type_week_tow,	//!< GPS week and the Time Of Week (TOW)
	aos_gnss_time_type_utc			//!< UTC time (Year, month, day, hour, min, sec, msec)
} aos_gnss_time_type_t;

/*!
 * \struct aos_gnss_week_tow_time_info_t
 *
 * \brief GPS time provided in week number and Time Of Week (TOW)
 */
typedef struct {
	uint32_t week;				//!< GPS week number
	uint64_t tow;				//!< GPS time of week (microsecond)
} aos_gnss_week_tow_time_info_t;

/*!
 * \struct aos_gnss_utc_time_t
 *
 * \brief GPS time provided in UTC format
 */
typedef struct {
	uint16_t year;		//!< Year
	uint8_t month;		//!< Month [1..12]
	uint8_t day;		//!< Day [1..31]
	uint8_t	hour;		//!< Hour [0..23]
	uint8_t min;		//!< Minute [0..59]
	uint8_t	sec;		//!< Seconds [0..59]
	uint16_t msec;		//!< Milliseconds
} aos_gnss_utc_time_t;

/*!
 * \struct aos_gnss_time_info_t
 *
 * \brief GNSS time info with multiple formats
 */
typedef struct {
	aos_gnss_time_type_t type;							//!< Type of timing information carrier in the union.
	union {
		aos_gnss_week_tow_time_info_t gnss_week_tow;	// Software time in week and time of week
		uint64_t  time;            						// Software time in microsecond
		aos_gnss_utc_time_t utc;						// Time information in UTC format
	};
} aos_gnss_time_info_t;

/*!
 * \struct nmea_parse_msg_t
 *
 * \brief NMEA pre-parsed message. All fields are separated by a NULL byte.
 */
typedef struct {
	char base[NMEA_MAX_MSG_LEN];		//!< Buffer containing the pre-parsed message
	bool used;							//!< True if used (Used by driver only)
	uint16_t size;						//!< Size of the message
	uint8_t field_cnt;					//!< Number of field including the type (field 0)
	uint8_t checksum;					//!< NMEA checksum received
	uint16_t field_ofs[NMEA_MAX_NB_FIELD + 1]; //!< Offset in the pre-parsed buffer of each fields.
} nmea_parse_msg_t;

/*!
 * \struct aos_gnss_raw_data_t
 *
 * \brief Data in raw format (or pre-parsed) of frames received by the GNSS chip.
 */
typedef struct {
	union {
		nmea_parse_msg_t* parsed_msg;	//!< NMEA pre-parsed packet (NMEA only)
		struct {
			uint8_t* data;				//!< Received data as is (other than NMEA)
		    uint16_t data_len;			//!< Packet size. (Other than NMEA)
		};
	};
} aos_gnss_raw_data_t;

/*!
 * \struct aos_gnss_fix_info_t
 *
 * \brief  GNSS fix information
 */
typedef struct {
	aos_gnss_constellation_t constellation;		//!< Constellation used for the fix
	aos_gnss_fix_status_t  status;				//!< Fix type
	aos_gnss_time_info_t gnss_time;				//!< Time info.
	uint16_t ttff;								//!< Time to first fix in seconds
	uint32_t timestamp;							//!< Date (System-time in seconds) at which the fix process ended. Set by the service
	int32_t lat;								//!< Latitude Unit: degree (scale 1/1000000)
	int32_t lon;								//!< Longitude Unit: degree (scale 1/1000000)
	int32_t alt;								//!< Altitude Unit: centimeter
	uint32_t ehpe;             					//!< Estimated Horizontal position Error. Unit: centimeter
	uint8_t track_satellites;					//!< Number of tracked satellites
	uint8_t fix_satellites;						//!< NNumber of satellites used for the fix
	uint8_t sats_for_fix[AOS_GNSS_MAX_SATELLITES]; //!< Satellites used for the fix
	uint16_t hdop;								//!< Horizontal dilution of precision. scale: 0.2 (no unit)
	uint16_t vdop;								//!< Vertical dilution of precision. Scale: 0.01 (no unit)
	uint16_t pdop;								//!< Position dilution of precision. Scale 0.01 (no unit)
	uint16_t cog;								//!< Course over ground True degree in 1/100 degree
	uint16_t sog;								//!< Speed over ground cm/sec
} aos_gnss_fix_info_t;

/*!
 * \struct aos_gnss_satellite_info_t
 *
 * \brief Satellite information for tracking data
 */
typedef struct {
	uint8_t sv_id;					//!< Satellite identifier
	uint8_t cn0;					//!< Carrier to noise value
	uint8_t elevation;				//!< Elevation in degree
	uint16_t azimuth;				//!< Azimuth in degree
} aos_gnss_satellite_info_t;

/*!
 * \struct aos_gnss_track_data_t
 *
 * \brief Tracking data
 */
typedef struct {
	aos_gnss_constellation_t constellation;               		//!< Constellation
	uint8_t nb_sat;                          					//!< Number of satellites
	aos_gnss_satellite_info_t sat_info[AOS_GNSS_MAX_SATELLITES]; //!< Satellite information
} aos_gnss_track_data_t;

/*!
 * \struct aos_gnss_satellite_prn_t
 *
 * \brief Pseudo range information for a given satellite
 *
 * \note The unit of the pseudo-ranges depends on the hardware component, which provided them:
 * - From the MT3333, the pseudo-ranges are expressed in centimeters if the compressed_prn (refer to
 *   the structure aos_gnss_satellite_prn_report_t) is set to false. Otherwise, it is expressed in
 *   steps of 0.059 nanoseconds modulo 2^24.
 * - From the LR1110 (NAV1 mode), the pseudo-ranges are expressed in nanoseconds modulo 1 millisecond.
 *   In the case where the NAV2 mode has been selected, the PRN report is not provided,
 */
typedef struct {
	aos_gnss_constellation_t  constellation;   			  //!< Constellation of the satellite
	uint8_t              sv_id;                           //!< Satellite identifier
	uint8_t              sync_flags;                      //!< Synchronization flags. Refer to GPS_SAT_SYNC_xxx
	uint8_t              cn0;                             //!< Carrier To Noise value
	uint64_t             pseudo_range;                    //!< Pseudo range refer to the note above.
} aos_gnss_satellite_prn_t;

/*!
 * \struct aos_gnss_satellite_prn_report_t
 *
 * \brief Pseudo range report sent to the application
 */
typedef struct {
	uint32_t timestamp;											//!< Date (System-time in seconds) at which the PRN process ended. Set by the service
	aos_gnss_time_info_t    gnss_time;                  		//!< GPS time information
	uint8_t             	nb_sat;                          	//!< Number of satellites in the report
	bool 					solvable;							//!< True if solvable by the cloud. Set by the service
	bool					compressed_prn;						//!< True if PRN compressed, false otherwise
	aos_gnss_satellite_prn_t sat_info[AOS_GNSS_MAX_SATELLITES]; //!< Array of the satellite information
} aos_gnss_satellite_prn_report_t;


/*!
 * \def AOS_GNSS_CFG_ENABLE_GPS
 *
 * \brief Enable the GPS constellation
 *
 * \note  Do not change this bit mask or change the _gps_nmea_build_and_send_constellation in mtk_gps.c
 */
#define AOS_GNSS_CFG_ENABLE_GPS      (1 << 0)

/*!
 * \def AOS_GNSS_CFG_ENABLE_GLONASS
 *
 * \brief Enable the GLONASS constellation
 *
 * \note  Do not change this bit mask or change the _gps_nmea_build_and_send_constellation in mtk_gps.c
 */
#define AOS_GNSS_CFG_ENABLE_GLONASS  (1 << 1)

/*!
 * \def AOS_GNSS_CFG_ENABLE_GALILEO
 *
 * \brief Enable the GALILEO constellation
 *
 * \note  Do not change this bit mask or change the _gps_nmea_build_and_send_constellation in mtk_gps.c
 */
#define AOS_GNSS_CFG_ENABLE_GALILEO  (1 << 2)

/*!
 * \def AOS_GNSS_CFG_ENABLE_BEIDOU
 *
 * \brief Enable the BEIDOU constellation
 *
 * \note  Do not change this bit mask or change the _gps_nmea_build_and_send_constellation in mtk_gps.c
 */
#define AOS_GNSS_CFG_ENABLE_BEIDOU   (1 << 4)


/*!
 * \enum aos_gnss_cfg_constellation_t
 *
 * \brief List of constellations configuration to use to make a GNSS fix
 */
typedef enum {
    aos_gnss_cfg_constellation_gps_only= 0,				//!< Use only the GPS constellation
    aos_gnss_cfg_constellation_glonass_only,			//!< Use only the GLONASS constellations
    aos_gnss_cfg_constellation_gps_glonass, 			//!< Use the GPS and GLONASS constellations
    aos_gnss_cfg_constellation_gps_galileo, 			//!< Use the GPS and GALILEO constellations
    aos_gnss_cfg_constellation_gps_glonass_galileo, 	//!< Use GPS, GLONASS and GALILEO
    aos_gnss_cfg_constellation_beidou_only,				//!< Use only the BEIDOU constellation
    aos_gnss_cfg_constellation_gps_beidou 				//!< Use GPS and BEIDOU constellations
} aos_gnss_cfg_constellation_t;

/*!
 * \struct aos_gnss_cfg_prn_t
 *
 * \brief Pseudo-range setting
 *
 * \note The LR1110 always compresses the pseudo-ranges. So, the parameter compress_prn is not
 * applicable for it.
 */
typedef struct {
	uint8_t min_stable_sats;		//!< Minimum number of stable satellites needed for an AGPS fix
	uint8_t min_cn;					//!< Minimum carrier to noise (C/N0) for acceptance
	uint8_t min_sync;				//!< Minimum sync for acceptance
	bool compress_prn;				//!< True if the PRN must be compressed, false otherwise. Refer to the note above.
} aos_gnss_cfg_prn_t;

/*!
 * \def AOS_GNSS_PRN_FILTER_MIN_CN
 *
 * \brief Minimum Carrier to noise value for filtering the Pseudo-range
 */
#define AOS_GNSS_PRN_FILTER_MIN_CN      5

/*!
 * \def AOS_GNSS_PRN_FILTER_MIN_SYNC
 *
 * \brief Minimum sync status for filtering the Pseudo-range
 */
#define AOS_GNSS_PRN_FILTER_MIN_SYNC    0x1


/*!
 * \def AOS_GNSS_PRN_MIN_NB_SATS
 *
 * \brief Minimum number of satellites for Aided-GNSS
 */
#define AOS_GNSS_PRN_MIN_NB_SATS    	5


/*!
 * \struct aos_gnss_cfg_period_t
 *
 * \brief Configure the period of the reported data from the chip.
 *
 *\note PRN messages are always sent at a frequency of 1 second
 */
typedef struct {
	uint8_t fix_period;			//!<Period at which the chip will report messages for the fix
	uint8_t track_period;		//!<Period at which the chip will report messages for the tracking data
} aos_gnss_cfg_period_t;


// Min/max Periods in seconds
#define AOS_GNSS_MIN_GPS_FIX_PERIOD  1	//!< Minimum period in seconds for fix messages reporting
#define AOS_GNSS_MIN_TRACKING_PERIOD 1  //!< Minimum period in seconds for tracking messages reporting
#define AOS_GNSS_MAX_GPS_FIX_PERIOD  9	//!< Maximum period in seconds for fix messages reporting
#define AOS_GNSS_MAX_TRACKING_PERIOD 9  //!< Maximum period in seconds for fix messages reporting


/*!
 * \enum aos_gnss_rqst_type_t
 *
 * \brief Type of request when sending a message to the GNSS chip.
 */
typedef enum {
	aos_gnss_rqst_type_msg,          //!< Message: Just send the message. Neither ack, nor answer expected
	aos_gnss_rqst_type_cmd,          //!< Command:  The request will be sent and we wait for an ack
	aos_gnss_rqst_type_query         //!< Query:  The request will be sent and we wait for an eventual ack and and answer
} aos_gnss_rqst_type_t;


/*!
 * \enum aos_gnss_request_status_t
 *
 * \brief Status of a request
 */
typedef enum {
	aos_gnss_request_status_success = 0,	//!< Request is successful
	aos_gnss_request_status_answer,			//!< An answer has been received
	aos_gnss_request_status_nack,			//!< The request has been nack'ed
	aos_gnss_request_status_timeout,		//!< Timeout occurred while waiting for an ack or an answer
	aos_gnss_request_status_failure,			//!< Request failure
	aos_gnss_request_status_ack_base,			//!< Base for NMEA statuses
	aos_gnss_request_status_ack_cmd_error =		//!< Invalid command
			aos_gnss_request_status_ack_base,
	aos_gnss_request_status_ack_cmd_unknown,	//<! unsupported command
	aos_gnss_request_status_ack_ok_no_action,	//<! Command success but no action taken (or no answer)
	aos_gnss_request_status_ack_ok_action_ok,	//<! Command success and action taken
	aos_gnss_request_status_ack_last			//!< last status of the ack} aos_gnss_request_status_t;
} aos_gnss_request_status_t;

/*!
 * \def AOS_GNSS_MSG_MSK_ALLOW_ALL_RAW
 *
 * \brief Mask to enable the monitoring of GNSS messages
 * Allow raw messages even if another known type is received. In this case
 * two events will be received: aos_gnss_event_raw_xxx and the known one
 */
#define AOS_GNSS_MSG_MSK_ALLOW_ALL_RAW              (1 << 0)

/*!
 * \def AOS_GNSS_MSG_MSK_ALLOW_FIX
 *
 * \brief Mask to enable the monitoring of GNSS messages. Allow GPS fix message monitoring.
 */
#define AOS_GNSS_MSG_MSK_ALLOW_FIX                  (1 << 1)

/*!
 * \def AOS_GNSS_MSG_MSK_ALLOW_TRACK
 *
 * \brief Mask to enable the monitoring of GNSS messages. Allow tracking data monitoring.
 */
#define AOS_GNSS_MSG_MSK_ALLOW_TRACK                (1 << 2)

/*!
 * \def AOS_GNSS_MSG_MSK_ALLOW_PSEUDO_RANGE
 *
 * \brief Mask to enable the monitoring of GNSS messages. Allow pseudo-range monitoring.
 */
#define AOS_GNSS_MSG_MSK_ALLOW_PSEUDO_RANGE         (1 << 3)

/*!
 * \def AOS_GNSS_MSG_MSK_ALLOW_UNKNOWN
 *
 * \brief Mask to enable the monitoring of GNSS messages. Allow unknown message monitoring
 */
#define AOS_GNSS_MSG_MSK_ALLOW_UNKNOWN              (1 << 4)

/*!
 * \def AOS_GNSS_MSG_MSK_ALLOW_ALL_MSG
 *
 * \brief Mask to enable all messages to be monitored
 */
#define AOS_GNSS_MSG_MSK_ALLOW_ALL_MSG     (AOS_GNSS_MSG_MSK_ALLOW_ALL_RAW | AOS_GNSS_MSG_MSK_ALLOW_FIX |\
				                       AOS_GNSS_MSG_MSK_ALLOW_TRACK | AOS_GNSS_MSG_MSK_ALLOW_PSEUDO_RANGE | \
				                       AOS_GNSS_MSG_MSK_ALLOW_UNKNOWN)
/*!
 * \def AOS_GNSS_MSG_MSK_ALLOW_FULL_NAV
 *
 * \brief Mask to enable the usual messages to be monitored
 */
#define AOS_GNSS_MSG_MSK_ALLOW_FULL_NAV           (AOS_GNSS_MSG_MSK_ALLOW_FIX | AOS_GNSS_MSG_MSK_ALLOW_TRACK | \
											  AOS_GNSS_MSG_MSK_ALLOW_PSEUDO_RANGE)

/*!
 * \struct aos_gnss_counters_t
 *
 * \brief GNSS Statistics
 */
typedef struct {
    uint32_t msg_valid;				//!< Number of valid messages
    uint32_t msg_too_long;			//!< Number of messages too long
    uint32_t msg_too_much_fields;	//!< Number of messages with too much fields
    uint32_t msg_wrong_end;			//!< Number of messages with invalid termination
    uint32_t msg_crc_err;			//!< Number of messages with a CRC error
    uint32_t msg_parser_err;		//!< Number of messages for which the parser failed
    uint32_t msg_unknown;			//!< Number of unknown messages
    uint32_t msg_fix_cycle_reset;	//!< Number of times the number of expected messages to form a fix
    								//!< has failed.
    uint32_t non_digit_bytes;		//!< Number of non ASCII bytes received
    uint32_t no_buffer;				//!< Number of messages discarded due to a lack of memory
    uint32_t overrun;				//!< Number of of time a reception overrun occurred
} aos_gnss_counters_t;


/*!
 * \enum aos_gnss_power_t
 *
 * \brief GNSS power management
 */
typedef enum {
	aos_gnss_power_off,			//!< GNSS chip powered off
	aos_gnss_power_on,			//!< GNSS powered on
	aos_gnss_power_standby,		//!< GNSS chip in standby mode
	aos_gnss_power_hold			//!< GNSS still powered but driver closed
} aos_gnss_power_t;


/*!
 * \enum aos_gnss_event_t
 *
 * \brief Event sent to the application
 */
typedef enum {
    aos_gnss_event_error = 0,		 	//!< Communication error
    aos_gnss_event_power_on,         	//!< GNSS chip fully powered. Event data: none
    aos_gnss_event_power_off,        	//!< GNSS chip not powered. Event data: None
    aos_gnss_event_power_standby,    	//!< GNSS chip in standby or hold  mode. Event data: None
    aos_gnss_event_power_hold_on,    	//!< GNSS chip is powered but the driver is closed. Event data: None
    aos_gnss_event_ready,            	//!< GNSS is ready. Configuration has been sent and ack'ed. Event data: None
    aos_gnss_event_trigger_rx_delayed,  //!< The driver requests that its delayed rx function should be triggered
    aos_gnss_event_fix,              	//!< GNSS fix received. Event data: aos_gnss_fix_info_t
    aos_gnss_event_track_data,       	//!< GNSS tracking data received. Event data: aos_gnss_raw_data_t
    aos_gnss_event_pseudo_range,     	//!< Pseudo range of a given satellite received. Event data: gps_satellite_pseudo_range_t
    aos_gnss_event_raw_nmea_sentence,	//!< Pre-parsed NMEA message received. Event data: aos_gnss_raw_data_t
    aos_gnss_event_req_status,       	//!< Status of a request. Sent once a req has been Ack'ed, answered, Nacke'd or timeout.
    aos_gnss_event_count             	//!< Must be the last
} aos_gnss_event_t;

/*!
 * \struct aos_gnss_request_info_t
 *
 * \brief Information related to a request
 */
typedef struct {
	aos_gnss_request_status_t status;		//!< Status of the request/query
	aos_gnss_raw_data_t raw;				//!< pre-parsed raw data belonging to the answer
} aos_gnss_request_info_t;


/*!
 * \struct aos_gnss_event_info_t
 *
 * \brief Event structure sent to the application via its callback
 */
typedef struct {
    aos_gnss_event_t event;						//!< Event type
    union {
    	aos_gnss_request_info_t req_info;		//!< Information/status of a request
    	aos_gnss_raw_data_t raw;				//!< Raw message (belong to aos_gnss_event_raw_nmea_sentence)
    	aos_gnss_fix_info_t* fix;				//!< fix information (belong to aos_gnss_event_fix)
    	aos_gnss_track_data_t* track;			//<! tracking information (belong to aos_gnss_event_track_data)
    	aos_gnss_satellite_prn_report_t* prn_report;//<! Pseudo range report (belong to aos_gnss_event_pseudo_range)
    };
} aos_gnss_event_info_t;

/*!
 * \enum aos_gnss_ioctl_req_t
 *
 * \brief IO control request sent to the driver
 */
typedef enum {
    aos_gnss_ioctl_req_set_power,      //!< Set the power mode. Parameter: power
    aos_gnss_ioctl_req_get_power,      //!< Read the power mode. Answer: power
    aos_gnss_ioctl_req_set_mask,       //!< Set the message mask. Parameter: value
    aos_gnss_ioctl_req_get_mask,       //!< Get the message mask Answer: value
    aos_gnss_ioctl_req_set_constel,    //!< Set the constellation to use. Parameter: constellation
    aos_gnss_ioctl_req_get_constel,    //!< Get the constellation used:. Answer: constellation
    aos_gnss_ioctl_req_clr_counters,   //!< Clear all statistics. Parameter: none
    aos_gnss_ioctl_req_get_counters,   //!< Get the constellation used. Answer counters.
    aos_gnss_ioctl_req_set_prn_cfg,    //!< Set the PRN configuration. Parameter: aos_gnss_cfg_prn_t
    aos_gnss_ioctl_req_get_prn_cfg,    //!< Get the PRN configuration. Answer: aos_gnss_cfg_t
 	aos_gnss_ioctl_req_set_period,	   //!< Set the period for tracking and fix message. Can be sent ONLY if the GPS is OFF or in standby.
} aos_gnss_ioctl_req_t;

/*!
 * \struct aos_gnss_ioctl_t
 *
 * \brief IO control information sent to the driver via IO requests
 */
typedef struct {
    aos_gnss_ioctl_req_t req;							//!< Type of IO request
    union {
    	aos_gnss_power_t power;  						//!< Power state. Used by aos_gnss_ioctl_req_set_power and aos_gnss_ioctl_req_get_power
    	aos_gnss_cfg_constellation_t constellation;		//!< Constellation configuration. Used by: aos_gnss_ioctl_req_set_constel    													//!< and aos_gnss_ioctl_req_get_constel
    	aos_gnss_counters_t* counters;					//!< Area where to store the counters. Used by aos_gnss_ioctl_req_get_counters
    	aos_gnss_cfg_prn_t prn_cfg;						//!< Pseudo range configuration. Used by aos_gnss_ioctl_req_set_prn_cfg and aos_gnss_ioctl_req_get_prn_cfg
    	aos_gnss_cfg_period_t	periods;				//!< Fix/Tracking periods
    	uint32_t value;									//!< Generic value. User by: aos_gnss_ioctl_req_set_mask
    	void *ptr;
    };
} aos_gnss_ioctl_t;


/*!
 * \typedef aos_gnss_callback_t
 *
 * \brief Application callback to receive GNSS events
 *
 * \param info Event information data
 * \param user_arg User argument. Opaque for the driver.
 */
typedef void (*aos_gnss_callback_t)(aos_gnss_event_info_t *info, void * user_arg);


/*
 * *****************************************************************************
 * Almanac
 * *****************************************************************************
 */
#define AOS_GNSS_ALMANAC_GPS_NB_WORDS		8		//!< Number of words (32 bits) in the GPS almanac entry
#define AOS_GNSS_ALMANAC_BEIDOU_NB_WORDS	7		//!< Number of words (32 bits) in the BEIDOU almanac entry
#define AOS_GNSS_ALMANAC_REDUCED_SIZE_19	19		//!< Number of byte for a reduced almanac entry on 19 bytes (excluding SV ID and Days)
#define AOS_GNSS_ALMANAC_REDUCED_SIZE_15	15		//!< Number of byte for a reduced almanac entry on 15 bytes (excluding SV ID and Days)

/*!
 * \struct aos_gnss_almanac_standard_entry_t
 *
 * \brief Entry of a standard almanac
 *
 * \note  This almanac entry is generated from the standard almanac. It is suitable for
 * the MT3333 only
 */
typedef struct {
	uint8_t svid;					//!< Satellite identifier
	uint16_t week;					//!< Week number
	uint32_t words[AOS_GNSS_ALMANAC_GPS_NB_WORDS];	//!< Words in the almanac. GPS: 8 words of 24 bits. Beidou: 7 words of 32 bits
} aos_gnss_almanac_standard_entry_t;

/*!
 * \struct aos_gnss_almanac_reduced_entry_t
 *
 * \brief Entry of a reduced almanac
 *
 * \note This almanac entry is generated from the ephemeris. It is suitable for both the LR1110 and the MT3333.
 * The data content contains either:
 * - 19 bytes: almanac entry 15 bytes) plus the CA code generator (2 bytes) plus modulation bitmask (1 byte) plus constellation ID (1 byte).
 * - 15 bytes: Almanac entry.
 * Both reduced almanac can be converted to the standard almanac.
 */
typedef struct {
	uint8_t svid;									//!< Satellite identifier
	uint16_t days;									//!< Number of days since 2019 April 7th
	uint8_t data[AOS_GNSS_ALMANAC_REDUCED_SIZE_19];	//!< Almanac data.
} aos_gnss_almanac_reduced_entry_t;

/*!
 * \enum aos_gnss_almanac_format_t
 *
 * \brief Almanac format
 */
typedef enum {
	aos_gnss_almanac_format_standard = 0,		//!< Standard almanac format
	aos_gnss_almanac_format_reduced_15_bytes,	//!< Reduced almanac format on 15 bytes
	aos_gnss_almanac_format_reduced_19_bytes,	//!< Reduced almanac format on 19 bytes
}aos_gnss_almanac_format_t;

/*!
 * \struct aos_gnss_almanac_entry_t
 *
 * \brief Generic almanac entry
 */
typedef struct {
	aos_gnss_constellation_t constellation;					//!< Constellation
	aos_gnss_almanac_format_t format;						//!< Entry format
	union {
		aos_gnss_almanac_standard_entry_t standard_entry;	//!< Standard almanac entry
		aos_gnss_almanac_reduced_entry_t reduced_entry;		//!< Reduced almanac entry
	};
} aos_gnss_almanac_entry_t;


/*! @}*/
#ifdef __cplusplus
}
#endif
