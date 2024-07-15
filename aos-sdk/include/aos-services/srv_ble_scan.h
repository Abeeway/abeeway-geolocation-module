/*!
 * \file srv_ble_scan.h
 *
 * \brief BLE scan service
 *
 * \details This service performs BLE beacon scans.<BR>
 *
 * \warning Before using this service, the BLE driver should be opened as observer only or observer and peripheral.
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 * <BR>
 *
 * <H3>Service usage</H3>
 * <DIV>
 * Basically the service performs a beacon scan based on filters. Once the scan is done, a user callback is triggered
 * with the result and a report.<BR>
 * Both the result and the reports contain the list of observed beacons matching the filtering criteria. The difference
 * comes from the provided data:
 * <UL>
 *  <LI>
 *   The result handles the information contained in the beacon advertisement part according to the beacon type.
 *  </LI>
 *  <LI>
 *   The report contains only information requested by the user. For example, the user can be interested by the beacon identifiers<BR>
 *   (carried inside the advertised data) instead of the MAC addresses.<BR>
 *   Refer to the srv_ble_scan_report_type_t structure for more details.
 *  </LI>
 * </UL>
 * The scan operation is started via the call to the srv_ble_scan_start function, for which a user callback should be provided.<BR>
 * The user argument is optional.<BR>
 * The scan operation can be aborted at any time by calling the function srv_ble_scan_stop.<BR>
 * </DIV>
 *
 * <H3>Filtering</H3>
 * <DIV>
 * Before starting a scan, it is recommended to configure it. This is done by calling the function srv_ble_scan_get_params and modify the parameters. <BR>
 * SRV uses an enhanced filtering scheme depending on the beacons type to filter. The beacon type, listed by the enumerated srv_ble_scan_beacon_type_t, can be:
 * <UL>
 *  <LI>
 *    srv_ble_scan_beacon_type_all: All beacon types are accepted. The filters cannot be applied and the report can handle only MAC addresses.
 *  </LI>
 *  <LI>
 *   srv_ble_scan_beacon_type_eddy_uid: Only Eddystone UUID beacons are accepted. The filters can be applied and the report can handle any type of information (beacon identifier or MAC addresses)
 *  </LI>
 *  <LI>
 *   srv_ble_scan_beacon_type_eddy_url: Only Eddystone URL beacons are accepted. The filters can be applied and the report can handle any type of information (beacon identifier or MAC addresses)
 *  </LI>
 *  <LI>
 *   srv_ble_scan_beacon_type_eddy_all: All Eddystone beacons (UUID/URL/TLM/EID) are accepted. The filters cannot be applied and the report can handle only MAC addresses.
 *  </LI>
 *  <LI>
 *   srv_ble_scan_beacon_type_ibeacon: Only iBeacons are accepted. The filters can be applied and the report can handle any type of information (beacon identifier or MAC addresses)
 *  </LI>
 *  <LI>
 *   srv_ble_scan_beacon_type_altbeacon: Only altBeacons are accepted. The filters can be applied and the report can handle any type of information (beacon identifier or MAC addresses)
 *  </LI>
 *  <LI>
 *   srv_ble_scan_beacon_type_custom: The filters must be configured. Beacons matching the filter are accepted. The report can handle any type of information (beacon identifier or MAC addresses)
 *  </LI>
 *  <LI>
 *   srv_ble_scan_beacon_type_exposure: Only exposure advertisements are accepted. The filters cannot be applied and the report can handle only MAC addresses.
 *  </LI>
 * </UL>
 * The scan configuration contains two filters. Each filter contains a 10 bytes mask and value. It also defines a start offset from which the filter should applied.<BR>
 * The filtering principle is straight forward: The filter mask is applied (logical AND) on the advertised data and compared to the value.<BR>
 * If the filter matches (comparison success), the beacon is kept otherwise it is discarded.<BR>
 * The start offset is the offset starting from the beginning of the advertisement data part.
 * </DIV>
 *
 * <H3>Result vs report</H3>
 * <DIV>
 * Once the scan is done, two structures are provided. Both contains beacons entries stored after filtering:
 * <UL>
 *  <LI>
 *  srv_ble_scan_result_t: Each entry contains the full beacon information. The entry format is based on the beacon type. The structure content is fixed and cannot be tuned.
 *  </LI>
 *  <LI>
 *  srv_ble_scan_report_t: Each entry contains the relevant part of the beacon information. It is customizable via the srv_ble_scan_param_report_t structure. <BR>
 *  you can decide about the number of entry you want and the type of identifier:
 *  <UL>
 *   <LI>
 *    srv_ble_scan_report_type_mac_address: The entries contain the MAC address in the identifier field. The short form is used in this case and the start_o
 *   </LI>
 *   <LI>
 *   srv_ble_scan_report_type_short_id: The identifier field of an entry contains 6 bytes starting from the configured start_id_offset.
 *   </LI>
 *   <LI>
 *   srv_ble_scan_report_type_long_id: The identifier field of an entry contains 16 bytes starting from the configured start_id_offset.<BR>
 *   Such a type is usually configured to do BLE finger printing.
 *   </LI>
 *  </UL>
 *  </LI>
 * </UL>
 * Note that the start_id_offset does not start from the beginning of the advertisement frame. Instead it is related to the type of beacons:
 * <UL>
 *  <LI>
 *  Eddystone beacons: start_id_offset = 0 locates the data part of the advertisement frame.
 *  </LI>
 *  <LI>
 *  iBeacons: start_id_offset = 0 locates the manufacturing UUID field of the advertisement frame.
 *  </LI>
 *  <LI>
 *  altBeacons: start_id_offset = 0 locates the beacon ID field of the advertisement frame.
 *  </LI>
 *  <LI>
 *  exposure: start_id_offset = 0 locates the RPI field of the advertisement frame.
 *  </LI>
 *  <LI>
 *  custom: start_id_offset = 0 locates the beginning of the advertisement frame.
 *  </LI>
 * </UL>
 * </DIV>
 *
 * <H3>Scan configuration</H3>
 * <DIV>
 * The scan is configured using the structure srv_ble_scan_param_t. This structure should be retrieved and manipulated before starting a scan.<BR>
 * It is strongly discouraged to change the structure while a scan is in progress. <BR>
 * The overall scan duration is configured via the scan_duration parameter. At the end of this duration, the user callback will be triggered.<BR>
 * The scan_window and scan_interval reflects the standard BLE parameters:
 * <UL>
 *  <LI>
 *   The window defines the actual scan duration for a given channel.
 *  </LI>
 *  <LI>
 *   The interval defines the actual scan duration for given channel and the delay to switch to the next channel. It is always greater than window.
 *  </LI>
 * </UL>
 * The repeat_delay parameter defines the period of complete scans. If the user expects a single scan, this parameter should be set to 0.<BR>
 * If the user expects periodic scans, this parameter should contains the period (delay between each scans).<BR>
 * The configuration structure contains the filter, the report parameters and the type of beacons we consider.<BR>
 * The field rssi_threshold provides an extra filtering based on the RSSI level. Beacons with a RSSI below this threshold are discarded.<BR>
 * Finally, the configuration a fine tuning of the advertised channels via the parameter adv_compensation. It may be used in the case where the<BR>
 * BLE antenna has not the same gain on all advertisement channel frequencies.
 * </DIV>
 */

#pragma once

#include "aos_common.h"
#include "aos_ble_common.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup srv_ble_scan BLE scan service
 *
 * \addtogroup srv_ble_scan
 * @{
 */


/*
 * ***********************************************************************
 * General definitions
 * ***********************************************************************
 */

/*!
 * \brief Define constants related to BLE scan module.
 *
 * \note They should not be modified
 */
#define SRV_BLE_MAX_SCAN_RESULT                     20	//!< Maximum scan entries in the results
#define SRV_BLE_SCAN_FILTER_MAX_SIZE                10	//!< Size of a scan filter
#define SRV_BLE_SCAN_NB_FILTER_MAX                  2	//!< Number of scan filters
#define SRV_BLE_MAX_SHORT_BEACON_ID_SIZE            6	//!< Size of a beacon identifier in the report. Short format.
#define SRV_BLE_MAX_LONG_BEACON_ID_SIZE             16	//!< Size of a beacon identifier in the report. Long format.
#define SRV_BLE_SCAN_SHORT_NB_ENTRY	              	20	//!< Maximum number of entries in the report when the short format is used.
#define SRV_BLE_SCAN_LONG_NB_ENTRY              	10	//!< Maximum number of entries in the report when the long format is used.

#define SRV_BLE_SCAN_MAX_RAW_DATA                   24	//!< Maximum data size when the custom beacon format is used.
#define SRV_BLE_NB_ADV_CHANNELS                     3	//!< Number of BLE advertisement channels (standard)


/*!
 * \enum srv_ble_scan_eddystone_type_t
 *
 * \brief Eddystone beacon types
 *
 * \note This is not a bitmap
 */
typedef enum {
	srv_ble_scan_eddystone_type_uid     = 0x00,     //!< Eddystone type UID
	srv_ble_scan_eddystone_type_url     = 0x10,     //!< Eddystone type URL
	srv_ble_scan_eddystone_type_tlm     = 0x20,     //!< Eddystone type TLM
	srv_ble_scan_eddystone_type_eid     = 0x30,     //!< Eddystone type EID
	srv_ble_scan_eddystone_type_unknown = 0xFF      //!< Unknown Eddystone
} srv_ble_scan_eddystone_type_t;

/*!
 * \enum srv_ble_scan_beacon_type_t
 *
 * \brief Type of beacons to filter-out or type of beacons in the result structure
 */
typedef enum {
	srv_ble_scan_beacon_type_all		= 0, //!< All advertisement frames are accepted.
	srv_ble_scan_beacon_type_eddy_uid   = 1, //!< Only Eddystone UID advertisement frames are accepted.
	srv_ble_scan_beacon_type_eddy_url   = 2, //!< Only Eddystone URL advertisement frames are accepted.
	srv_ble_scan_beacon_type_eddy_all   = 3, //!< All eddystone beacon adverstisements are accepted.
	srv_ble_scan_beacon_type_ibeacon    = 4, //!< Only iBeacon advertisement frames are accepted.
	srv_ble_scan_beacon_type_altbeacon  = 5, //!< Only altBeacon advertisement frames are accepted.
	srv_ble_scan_beacon_type_custom     = 6, //!< Only advertisement frames matching the custome filter are accepted.
	srv_ble_scan_beacon_type_exposure   = 7	 //!< Only exposure advertisement frames are accepted.
} srv_ble_scan_beacon_type_t;


/*!
 * \enum srv_ble_scan_report_type_t
 *
 * \brief Beacon address/identifier type to be reported
 */
typedef enum {
	srv_ble_scan_report_type_mac_address,		//!< MAC addresses are reported
	srv_ble_scan_report_type_short_id,			//!< Beacon ID -short format- are reported
	srv_ble_scan_report_type_long_id			//!< Beacons ID -long format- are reported
} srv_ble_scan_report_type_t;

/*
 * ***********************************************************************
 * Result definitions
 * ***********************************************************************
 */
#define SRV_BLE_EXPOSURE_RPI_LEN                    16	//!< Length of the RPI field (exposure beacons)
#define SRV_BLE_EXPOSURE_METADATA_LEN               4	//!< Length of the metadata field (exposure beacons)

#define SRV_BLE_SCAN_IBEACON_PROXIMITY_UUID_LEN		16  //!< Length of the company identifier (iBeacons beacons)

#define SRV_BLE_SCAN_ALTBEACON_MANUF_ID_LEN			2	//!< Manufacturer Identifier length
#define SRV_BLE_SCAN_ALTBEACON_BEACON_CODE_LEN		2	//!< Beacon code length
#define SRV_BLE_SCAN_ALTBEACON_BEACON_ID_LEN		20	//!< Beacon identifier


#define SRV_BLE_SCAN_EDDYSTONE_RAW_DATA_LEN			20	//!< Eddystone information length in raw format
#define SRV_BLE_SCAN_EDDYSTONE_UUID_NAME_SPACE_LEN	10	//!< Eddystone UUID. name space field length
#define SRV_BLE_SCAN_EDDYSTONE_UUID_INSTANCE_LEN	6	//!< Eddystone UUID.instance field length
#define SRV_BLE_SCAN_EDDYSTONE_URL_ENC_LEN			17	//!< Eddystone URL. Encoded URL length


#ifndef __packed
#define __packed __attribute__((__packed__))
#endif

/*!
 * \struct srv_ble_scan_exposure_info_t
 *
 * \brief exposure beacon information fields
 */
typedef struct  {
	uint8_t rpi[SRV_BLE_EXPOSURE_RPI_LEN];				//!< Random public identifier
	uint8_t meta_data[SRV_BLE_EXPOSURE_METADATA_LEN];	//!< Meta data
} srv_ble_scan_exposure_info_t;

/*!
 * \struct srv_ble_scan_ibeacon_info_t
 *
 * \brief Ibeacon beacon information fields
 */
typedef struct {
	uint8_t proximity_uuid[SRV_BLE_SCAN_IBEACON_PROXIMITY_UUID_LEN];	//!< Unique identifier
	uint16_t major;														//!< Major number
	uint16_t minor;														//!< Minor number
} __packed  srv_ble_scan_ibeacon_info_t;

/*!
 * \struct srv_ble_scan_altbeacon_info_t
 *
 * \brief Altbeacon beacon information fields
 */
typedef struct {
	uint8_t manufacturer_id[SRV_BLE_SCAN_ALTBEACON_MANUF_ID_LEN];	//!< Manufacturer identifier
	uint8_t beacon_code[SRV_BLE_SCAN_ALTBEACON_BEACON_CODE_LEN];	//!< Beacon code
	uint8_t beacon_id[SRV_BLE_SCAN_ALTBEACON_BEACON_ID_LEN];		//!< Beacon identifier field
} __packed srv_ble_scan_altbeacon_info_t;

/*!
 * \struct srv_ble_scan_eddystone_info_t
 *
 * \brief Eddystone beacon information fields
 */
typedef struct {
	srv_ble_scan_eddystone_type_t type; 										//!< eddystone beacon type
	uint8_t data_len;															//!< data length
	union {
		uint8_t data[SRV_BLE_SCAN_EDDYSTONE_RAW_DATA_LEN];						//!< Data in raw format
		struct {
			// Refer to https://github.com/google/eddystone/tree/master/eddystone-uid
			uint8_t name_space[SRV_BLE_SCAN_EDDYSTONE_UUID_NAME_SPACE_LEN];		//!< Name space
			uint8_t instance[SRV_BLE_SCAN_EDDYSTONE_UUID_INSTANCE_LEN];			//!< Instance
			uint8_t reserved[2];												//!< Unused
		} uid ;																	//!< UUID format
		struct {
			///Refer to https://github.com/google/eddystone/tree/master/eddystone-url
			uint8_t url_scheme;													//!< URL scheme
			uint8_t url_encoded[SRV_BLE_SCAN_EDDYSTONE_URL_ENC_LEN];			//!< Encoded URL
		} url;																	//!< URL format
		struct {
			/// Refer to https://github.com/google/eddystone/blob/master/eddystone-tlm/tlm-plain.md
			uint8_t version; 													//!< Version: 0x00 for encrypted; 0x01 for encrypted */
			union {
				struct {
					uint8_t vbatt[2];											//!< Battery voltage. Little endian encoding
					uint8_t temp[2];											//!< Temperature. Little endian encoding
					uint8_t adv[4];												//!< Advertising PDU count.  Little endian encoding
					uint8_t time[4];											//!< Time since power-on or reboot. Little endian encoding
				} unencrypted ;
				struct {
					uint8_t data[12];											//!< Encrypted TLM data
					uint8_t salt[2];											//!< Encryption salt
					uint8_t mic[2];												//!< Message Integrety check
				} encrypted ;
			};
		} tlm;																	//!< TLM format
		struct {
			// Refer to https://github.com/google/eddystone/tree/master/eddystone-eid
			uint8_t ephemeral_id[8];											//!< Ephemeral identifier
		} eid;
	};
} __packed srv_ble_scan_eddystone_info_t;

/*!
 * \struct srv_ble_scan_custom_info_t
 *
 * \brief Custom beacon information field
 */
typedef struct {
	uint8_t data[SRV_BLE_SCAN_MAX_RAW_DATA];	//!< Custom raw data
} srv_ble_scan_custom_info_t;


/*!
 * \struct srv_ble_scan_beacon_info_t
 *
 * \brief Scanned beacon informations
 */
typedef struct {
	int8_t tx_power;										//!< Advertised TX power at 1 meter (dB)
	srv_ble_scan_beacon_type_t type;						//!< Type of beacon.
	union {
		srv_ble_scan_ibeacon_info_t ibeacon;				//!< ibeacon information (type = srv_ble_scan_beacon_type_ibeacon)
		srv_ble_scan_altbeacon_info_t abeacon;				//!< altbeacon information (type = srv_ble_scan_beacon_type_altbeacon
		srv_ble_scan_eddystone_info_t ebeacon;				//!< Eddystone information (type = srv_ble_scan_beacon_type_eddy_xx, xx can be uid, tlm or all)
		srv_ble_scan_exposure_info_t exposure_beacon;		//!< exposure beacon
		srv_ble_scan_custom_info_t custom;					//!< custom format
	} info;													//!< beacon information
} srv_ble_scan_beacon_info_t;

/*!
 * \struct srv_ble_scan_result_entry_t
 *
 * \brief Single result entry
 */
typedef struct {
	struct {
		uint8_t addr[BLE_MAC_ADDR_SIZE];						//!< GAP address
		int8_t rssi;									//!< Receive Signal Strength Information (dB)
	} header;											//!< Header
	srv_ble_scan_beacon_info_t binfo;					//!< Beacon data information
} srv_ble_scan_result_entry_t;

/*!
 * \struct srv_ble_scan_result_t
 *
 * \brief Complete scan result
 */
typedef struct {
	uint32_t timestamp;												//!< Date (System-time in seconds) at which the scan process ended. Set by the service
	uint8_t scan_count;												//!< Number of scan entries
	srv_ble_scan_result_entry_t entries[SRV_BLE_MAX_SCAN_RESULT];	//!< Array of scanned entries
} srv_ble_scan_result_t;


/*
 * ***********************************************************************
 * Report definitions
 * ***********************************************************************
 */
/*!
 * \struct srv_ble_scan_short_beacon_info_t
 *
 * \brief Short beacon id information
 */
typedef struct {
	uint8_t identifier[SRV_BLE_MAX_SHORT_BEACON_ID_SIZE];	//!< Beacon identifier. Can be either the BLE MAC address or a part of the data
	int8_t rssi;											//!< Receive Signal Strength Indication (dB)
	int8_t tx_power;										//!< Advertised TX power at 1 meter (dB)
} srv_ble_scan_short_beacon_info_t;

/*!
 * \struct srv_ble_scan_long_beacon_info_t
 *
 * \brief Long beacon id information
 */
typedef struct {
	uint8_t identifier[SRV_BLE_MAX_LONG_BEACON_ID_SIZE];	//!< Beacon identifier. Extracted from a part of the data
	int8_t rssi;											//!< Receive Signal Strength Indication (dB)
	int8_t tx_power;										//!< Advertised TX power at 1 meter (dB)
} srv_ble_scan_long_beacon_info_t;

/*!
 * \struct srv_ble_scan_report_t
 *
 * \brief Scan report. Built from requested filtering (refer to srv_ble_scan_param_report_t)
 */
typedef struct {
	uint8_t entry_cnt;							//!< Number of entries in the report
	uint32_t timestamp;							//!< Date (System-time in seconds) at which the PRN process ended. Set by the service
	srv_ble_scan_report_type_t report_type;		//!< Type of beacon ID reported.
	union {
		srv_ble_scan_short_beacon_info_t short_beacons[SRV_BLE_SCAN_SHORT_NB_ENTRY]; //!< Short ID beacons
		srv_ble_scan_long_beacon_info_t long_beacons[SRV_BLE_SCAN_LONG_NB_ENTRY];	 //!< Long entry beacons
	};
} srv_ble_scan_report_t;


/*
 * ***********************************************************************
 *  Report parameters and Filtering
 * ***********************************************************************
 */
/*!
 * \struct srv_ble_scan_param_report_t
 *
 * \brief Describe what we expect in the report
 *
 * \note The beacon ID size is always 6 bytes.
 */
typedef struct {
	uint8_t nb_beacons;							//!< nb_beacons:  Max number of beacons to report
	srv_ble_scan_report_type_t beacon_id_type;	//!< Type of beacon ID to report. 0: MAC addresses, 1: beacons IDs
	uint8_t start_id_offset;					//!< Offset from which we should read the beacon ID (applicable only if the type=1)
} srv_ble_scan_param_report_t;


/*!
 * \struct srv_ble_scan_param_filter_t
 *
 * \brief Filter to be applied on scanned beacons
 */
/*
* @brief Scan param filter
*/
typedef struct {
	uint8_t start_offset;                     		//!< Offset at which the filter application start. Depends on the filter type
	uint8_t mask[SRV_BLE_SCAN_FILTER_MAX_SIZE];		//!< filter mask
	uint8_t value[SRV_BLE_SCAN_FILTER_MAX_SIZE];	//!< filter value
} srv_ble_scan_param_filter_t;

/*!
 * \struct srv_ble_scan_param_t
 *
 * \brief BLE scan parameters
 */
typedef struct {
	uint16_t scan_duration;									//!< Scan duration (milliseconds)
	uint16_t scan_window;									//!< Scan window (in step of 0.625 ms)
	uint16_t scan_interval;									//!< Scan interval (in step of 0.625 ms)
	uint16_t repeat_delay;									//!< Continuous scanning: Duration (in seconds) between 2 scans. Value of 0 indicate single shot scan
	srv_ble_scan_beacon_type_t ble_scan_type;				//!< Type of beacons we consider.
	srv_ble_scan_param_filter_t filters[SRV_BLE_SCAN_NB_FILTER_MAX];	//!< Filters to be applied
	srv_ble_scan_param_report_t report;						//!< Report configuration
	int8_t rssi_threshold;									//!< Minimum RSSI to consider a beacon
	int8_t adv_compensation[SRV_BLE_NB_ADV_CHANNELS];		//!< Advertised channel compensation
} srv_ble_scan_param_t;



/*
 * ***********************************************************************
 *  BLE scan service API
 * ***********************************************************************
 */

/*!
 * \fn void srv_ble_scan_callback_t(void* user_arg, srv_ble_scan_result_t *result, srv_ble_scan_report_t* report)
 *
 * \brief User callback called at the end of each scans.
 *
 * \param user_arg User argument
 * \param result Scan result in the general format
 * \param report Scan result formatted as configured via the srv_ble_scan_param_report_t parameter
 */
typedef void srv_ble_scan_callback_t(void* user_arg, srv_ble_scan_result_t *result, srv_ble_scan_report_t* report);

/*!
 * \fn bool srv_ble_scan_is_active(void)
 *
 * \brief Indicate whether the BLE scan is active
 *
 * \return true if the BLE scan is active
 */
bool srv_ble_scan_is_active(void);

/*!
 * \fn uint8_t srv_ble_scan_start(srv_ble_scan_callback_t* callback, void* arg)
 *
 * \brief start a scan based on the configured parameters
 *
 * \param callback User callback triggered once the scan is done,
 * \param arg Opaque user argument passed along to the callback function.
 *
 * \return Result of the operation
 *
 * \note The scan operation should be configured before starting. This should be done via
 * the function srv_ble_scan_get_params to retrieve the configuration parameter and modify
 * them according to your need.
 */
uint8_t srv_ble_scan_start(srv_ble_scan_callback_t* callback, void* arg);

/*!
 * \fn void srv_ble_scan_stop(void)
 *
 * \brief Stop scan procedure
 */
void srv_ble_scan_stop(void);

/*!
 * \fn srv_ble_scan_param_t srv_ble_scan_get_params(void)
 *
 * \brief Retrieve the scan parameters
 *
 * \return Pointer to the scan parameters
 */
srv_ble_scan_param_t* srv_ble_scan_get_params(void);

/*!
 * \fn srv_ble_scan_result_t* srv_ble_scan_get_result(void)
 *
 * \brief retrieve the scan parameters
 *
 * \return Pointer to the scan results
 */
srv_ble_scan_result_t* srv_ble_scan_get_result(void);

/*!
 * \fn srv_ble_scan_result_t* srv_ble_scan_get_report(void)
 *
 * \brief retrieve the scan report
 *
 * \return Pointer to the scan report
 */
srv_ble_scan_report_t* srv_ble_scan_get_report(void);

/*!
 * \fn const char * srv_ble_scan_beacon_type_to_str(srv_ble_scan_beacon_type_t type)
 *
 * \brief return beacon type name in string format
 *
 * \param type the beacon to scan type
 *
 * \return the beacon type name in string format
 */
const char * srv_ble_scan_beacon_type_to_str(srv_ble_scan_beacon_type_t type);

/*!
 * \fn const char* srv_ble_scan_report_type_to_str(srv_ble_scan_report_type_t type)
 *
 * \brief return scan report type name in string format
 *
 * \param type the scan report type
 *
 * \return the report type name in string format
 */
char* srv_ble_scan_report_type_to_str(srv_ble_scan_report_type_t type);

/*!
 * \fn uint64_t srv_ble_scan_get_power_consumption(void)
 *
 * \brief return total BLE scan power consumption
 *
 * \return total scan power consumption in uAh
 */
uint64_t srv_ble_scan_get_power_consumption(void);

/*!
 * \fn void srv_ble_scan_clear_consumption(void)
 *
 * \brief reset BLE scan power consumption
 */
void srv_ble_scan_clear_consumption(void);

/*! @}*/
#ifdef __cplusplus
}
#endif

