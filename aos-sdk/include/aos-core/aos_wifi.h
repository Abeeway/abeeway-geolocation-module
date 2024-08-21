/**
 * \file      aos_wifi.h
 *
 * \brief     Basic LR1110 WIFI scan driver
 *
 * \details	  This file is inspired from the Semtech wifi_middleware. It interfaces AOS
 * with the Radio planner. Note that the wifi_middleware is too restrictive in term of
 * WIFI configuration and the number of reported Access points. That's the reason why
 * it is overloaded by this driver,
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "lr11xx_wifi_types.h"
#include "smtc_modem_api.h"
#include "ralf.h"

/*!
 * \defgroup aos_wifi GNSS MT3333 service
 *
 * \addtogroup aos_wifi
 * @{
 */

#define AOS_WIFI_MAC_ADDRESS_SIZE		6							//!< Size of a MAC address of a WIFI Access point

#define AOS_WIFI_MAX_RESULTS			LR11XX_WIFI_MAX_RESULTS		//!< Max number of WIFI scans

/*!
 * \brief WIFI event sent to the LR1110 manager
 */
typedef enum {
	aos_wifi_scan_event_success     = (1 << 0),  //!< The scan operation has been successfully completed
	aos_wifi_scan_event_error  		= (1 << 1),  //!< The scan operation has failed for an unknown reason
	aos_wifi_scan_event_cancel		= (1 << 2),  //!< The scan operation has been aborted by the user
	aos_wifi_scan_event_abort		= (1 << 3),  //!< The scan operation has been aborted by the radio planner
} aos_wifi_scan_event_type_t;

/*!
 * \brief WIFI channels used for the mask
 */
typedef enum {
	aos_wifi_scan_channel_mask_channel_1 = (1<<0),		//!< Channel 1
	aos_wifi_scan_channel_mask_channel_2 = (1<<1),		//!< Channel 2
	aos_wifi_scan_channel_mask_channel_3 = (1<<2),		//!< Channel 3
	aos_wifi_scan_channel_mask_channel_4 = (1<<3),		//!< Channel 4
	aos_wifi_scan_channel_mask_channel_5 = (1<<4),		//!< Channel 5
	aos_wifi_scan_channel_mask_channel_6 = (1<<5),		//!< Channel 6
	aos_wifi_scan_channel_mask_channel_7 = (1<<6),		//!< Channel 7
	aos_wifi_scan_channel_mask_channel_8 = (1<<7),		//!< Channel 8
	aos_wifi_scan_channel_mask_channel_9 = (1<<8),		//!< Channel 9
	aos_wifi_scan_channel_mask_channel_10 = (1<<9),	//!< Channel 10
	aos_wifi_scan_channel_mask_channel_11 = (1<<10),	//!< Channel 11
	aos_wifi_scan_channel_mask_channel_12 = (1<<11),	//!< Channel 12
	aos_wifi_scan_channel_mask_channel_13 = (1<<12),	//!< Channel 13
	aos_wifi_scan_channel_mask_channel_14 = (1<<13),	//!< Channel 14
	aos_wifi_scan_channel_mask_channel_all = 0x1FFF,	//!< All channels
}aos_wifi_scan_channel_mask_type_t;

/*!
 * \brief Wi-Fi signal type for passive scanning configuration
 *
 * \note It is not possible to configure the WiFi passive scanning to search Wi-Fi type N GreenField. Only Wi-Fi type N
 * Mixed Mode can be scanned by LR11XX.
 *
 * \warning LR11XX_WIFI_TYPE_SCAN_G and LR11XX_WIFI_TYPE_SCAN_N configurations are implemented the same way, and
 * both will scan Wi-Fi type G AND Wi-Fi type N.
 */
typedef enum {
	aos_wifi_scan_signal_type_scan_b     = 0x01,  //!< Wi-Fi B
	aos_wifi_scan_signal_type_scan_g     = 0x02,  //!< Wi-Fi G
	aos_wifi_scan_signal_type_scan_n     = 0x03,  //!< Wi-Fi N
	aos_wifi_scan_signal_type_scan_b_g_n = 0x04,  //!< Wi-Fi B and Wi-Fi G/N
} aos_wifi_scan_signal_type_scan_t;

/*!
 * \brief WiFi theoretical Datarates
 *
 * \note Mapped on lr11xx_wifi_datarate_info_byte_t
 */
typedef enum {
    aos_wifi_scan_datarate_1_MBPS    = 1,			//<! 1 MBPS
    aos_wifi_scan_datarate_2_MBPS    = 2,			//<! 2 MBPS
    aos_wifi_scan_datarate_6_MBPS    = 3,			//<! 6 MBPS
    aos_wifi_scan_datarate_9_MBPS    = 4,			//<! 9 MBPS
    aos_wifi_scan_datarate_12_MBPS   = 5,			//<! 12 MBPS
    aos_wifi_scan_datarate_18_MBPS   = 6,			//<! 18 MBPS
    aos_wifi_scan_datarate_24_MBPS   = 7,			//<! 24 MBPS
    aos_wifi_scan_datarate_36_MBPS   = 8,			//<! 36 MBPS
    aos_wifi_scan_datarate_48_MBPS   = 9,			//<! 48 MBPS
    aos_wifi_scan_datarate_54_MBPS   = 10,			//<! 54 MBPS
    aos_wifi_scan_datarate_6_5_MBPS  = 11,			//<! 6.5 MBPS
    aos_wifi_scan_datarate_13_MBPS   = 12,			//<! 13 MBPS
    aos_wifi_scan_datarate_19_5_MBPS = 13,			//<! 19.5 MBPS
    aos_wifi_scan_datarate_26_MBPS   = 14,			//<! 26 MBPS
    aos_wifi_scan_datarate_39_MBPS   = 15,			//<! 39 MBPS
    aos_wifi_scan_datarate_52_MBPS   = 16,			//<! 52 MBPS
    aos_wifi_scan_datarate_58_MBPS   = 17,			//<! 58 MBPS
    aos_wifi_scan_datarate_65_MBPS   = 18,			//<! 65 MBPS
    aos_wifi_scan_datarate_7_2_MBPS  = 19,			//<! 7.2 MBPS
    aos_wifi_scan_datarate_14_4_MBPS = 20,			//<! 14.4 MBPS
    aos_wifi_scan_datarate_21_7_MBPS = 21,			//<! 21.7 MBPS
    aos_wifi_scan_datarate_28_9_MBPS = 22,			//<! 28.9 MBPS
    aos_wifi_scan_datarate_43_3_MBPS = 23,			//<! 43.3 MBPS
    aos_wifi_scan_datarate_57_8_MBPS = 24,			//<! 57.8 MBPS
    aos_wifi_scan_datarate_65_2_MBPS = 25,			//<! 65.2 MBPS
    aos_wifi_scan_datarate_72_2_MBPS = 26,			//<! 72.2 MBPS
} aos_wifi_scan_datarate_type_t;

/*!
 * \brief Origin of the BSSID
 */
typedef enum {
	aos_wifi_scan_origin_gateway = 1,			//!< MAC Address from a gateway
	aos_wifi_scan_origin_mobile = 2,			//!< MAC Address from a mobile
	aos_wifi_scan_origin_unknown,				//!< Undetermined
} aos_wifi_scan_origin_type_t;

/*!
 * \brief Validation
 */
typedef enum {
	aos_wifi_scan_valid_ap = 0,					//!< MAC Address from an access point
	aos_wifi_scan_valid_device = 1,				//!< MAC Address from a device
} aos_wifi_scan_valid_type_t;


#define AOS_WIFI_SCAN_GET_WIFI_TYPE(data_rate_info_byte) (data_rate_info_byte & 0x3)	//!< Retrieve the WIFI type A/G/N. Refer to aos_wifi_scan_signal_type_scan_t
#define AOS_WIFI_SCAN_GET_DATARATE(data_rate_info_byte)	(data_rate_info_byte >> 2)		//!< Retrieve the datarate. Refer to aos_wifi_scan_datarate_type_t
#define AOS_WIFI_SCAN_GET_CHANNEL_ID(channel_info_byte)	(channel_info_byte & 0xF)		//!< Retrieve the channel identifier.
#define AOS_WIFI_SCAN_GET_MAC_ORIGIN(channel_info_byte)	((channel_info_byte >> 4) & 0x3)	//!< Retrieve the MAC origin. Refer to aos_wifi_scan_origin_type_t
#define AOS_WIFI_SCAN_GET_AP_VS_DEVICE(channel_info_byte)	((channel_info_byte >> 6) & 0x1)	//!< Retrieve the MAC validation. Refer to aos_wifi_scan_valid_type_t


/*!
 * \brief Basic MAC, type, channel result structure
 * \note comes from ( lr11xx_wifi_basic_mac_type_channel_result_t)
 */
typedef struct {
    uint8_t data_rate_info_byte;						//!< Info byte: use the macro AOS_WIFI_SCAN_GET_WIFI_TYPE and AOS_WIFI_SCAN_GET_DATARATE
    uint8_t channel_info_byte; 							//!< Channel info.
    int8_t rssi;										//!< RSSI
    uint8_t mac_address[AOS_WIFI_MAC_ADDRESS_SIZE];		//!< MAC address of the scanned device
} aos_wifi_scan_channel_result_t;

/*!
 * \brief WIFI data available to the user
 */
typedef struct {
	uint32_t timestamp;							//!< Date (System-time in seconds) at which the scan process ended. Set by the service
	uint64_t power_consumption_uah;				//!< Consumed power during the scan
	uint8_t nb_scan_results;					//!< Number of scan results
	aos_wifi_scan_channel_result_t* data;		//!< pointer to an array of scanned data
} aos_wifi_scan_result_data_t;

/*!
 * \brief WIFI scan configuration
 */
typedef struct {
    uint16_t  channels_mask;           		//!< A mask of the channels to be scanned (combination of aos_wifi_scan_channel_mask_type_t)
    aos_wifi_scan_signal_type_scan_t types; //!< WiFi types to be scanned
    uint8_t  max_results; 					//!< The maximum number of results expected for a scan (np more than
    uint16_t timeout_per_channel;  			//!< The max time to spend scanning one channel, in ms.
    uint16_t timeout_per_scan;				//!< The maximal time to spend in preamble detection for each single scan, in ms.
} aos_wifi_settings_t;


/*!
 * \brief Initialize the driver
 *
 * \param modem_radio The interface of the modem
 *
 * \return The operation result
 */
smtc_modem_return_code_t aos_wifi_init(ralf_t* modem_radio);

/*!
 * \brief Start a scan
 *
 * \param settings Configuration of the scan
 *
 * \return The operation result
 */
smtc_modem_return_code_t aos_wifi_scan_start(aos_wifi_settings_t* settings);

/*!
 * \brief Abort a scan
 *
 * \return The operation result
 */
smtc_modem_return_code_t aos_wifi_scan_stop(void);

/*!
 * \brief Get and clear the WIFI events
 *
 *
 * \return The events (bit mask)
 */
uint32_t aos_wifi_get_and_clear_events(void);

/*!
 * \brief Retrieve the scan results.
 *
 * \param scan_result Results
 *
 * \return The operation status
 */
smtc_modem_return_code_t aos_wifi_get_scan_result(aos_wifi_scan_result_data_t* scan_result);

/*! @}*/

#ifdef __cplusplus
}
#endif

/* --- EOF ------------------------------------------------------------------ */
