/*!
 * \file gm02s.h
 *
 * \brief Sequans GM02S driver
 *
 * \copyright 2023, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once

#include <stdio.h>
#include <stdarg.h>
#include "aos_uart.h"
#include "aos_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * **********************************************************************************
 * General definitions
 * **********************************************************************************
 */
#define GM02S_IP_ADDR_URL_MAX_SIZE	32		//!< Max string size of an IPv4, IPv6 or URL in string format
#define GM02S_DATA_BUFFER_LEN 		255		//!< Max data size for TX/RX data message over a socket
#define GM02S_ICCID_LEN				20		//!< Max ICCID and EUICCID string length including the NULL
#define GM02S_EUICCID_LEN			33		//!< Max ICCID and EUICCID string length including the NULL
#define GM02S_IMSI_LEN				16		//!< Max ICCID and EUICCID string length including the NULL
#define GM02S_APN_MAX_SIZE			32		//!< Max string size for the Access Point Name
#define GM02S_OPERATOR_MAX_SIZE		32		//!< Max string size for Operator name
#define GM02S_FIRMWARE_VERSION_LEN	16		//!< Max string size for FW version
/*
 * **********************************************************************************
 * PSM and eDRX bit mapping
 * **********************************************************************************
 */

/**
 * \brief Active time conversion (B7,B6, B5 bits of the TAU value) in seconds
 */
static const uint32_t _cell_psm_active_time_units[] = {
		2,			//!< 0 0 0 => 2 seconds
		60,			//!< 0 0 1 => 1 minute
		6*60		//!< 0 1 0 => 1 deci-hour (6 minutes)
};

/**
 * \brief TAU time conversion (B7,B6, B5 bits of the TAU value) in seconds
 */
static const uint32_t _cell_psm_tau_units[] = {
		10*60,		//!< 0 0 0 => 10 minutes
		60*60,		//!< 0 0 1 => 1 hour
		10*60*60,	//!< 0 1 0 => 10 hours
		2,			//!< 0 1 1 => 2 seconds
		30,			//!< 1 0 0 => 30 seconds
		60,			//!< 1 0 1 => 1 minute
		320*60*60,	//!< 1 1 0 => 320 hours
		0			//!< 1 1 1 => deactivated
};

/*
 * **********************************************************************************
 * Socket management
 * **********************************************************************************
 */
/**
 * \brief Sockets identifier
 */
typedef enum {
	gm02s_socket_id_none = 0,		//!< Not use
	gm02s_socket_id_1,				//!< Socket identifier 1
	gm02s_socket_id_2,				//!< Socket identifier 2
	gm02s_socket_id_3,				//!< Socket identifier 3
	gm02s_socket_id_4,				//!< Socket identifier 4
	gm02s_socket_id_5,				//!< Socket identifier 5
	gm02s_socket_id_6,				//!< Socket identifier 6
	gm02s_socket_id_last,			//!< Number of Socket identifiers
} gm02s_socket_id_t;

/**
 * \brief Socket types
 */
typedef enum {
	gm02s_socket_type_tcp = 0,		//!< TCP transport
	gm02s_socket_type_udp = 1		//!< UDP transport
} gm02s_socket_type_t;

/**
 * \brief Socket configuration
 */
typedef struct {
	gm02s_socket_id_t sid;							//!< Socket identifier
	gm02s_socket_type_t protocol;					//!< Protocol running on the socket
	char ip_url_addr[GM02S_IP_ADDR_URL_MAX_SIZE];	//!< Destination IP address (IPv4, IPv6 or URL)
	uint16_t dst_ip_port;							//!< Destination port
	uint16_t src_ip_port;							//!< Source port
} gm02s_socket_cfg_t ;

/**
 * \brief Socket buffer
 */
typedef struct {
	gm02s_socket_id_t sid;					//!< Socket on which the buffer should be sent
	uint16_t len;							//!< Data length to transmit
	uint8_t data[GM02S_DATA_BUFFER_LEN];	//!< Data
} gm02s_socket_buffer_t;


/*
 * **********************************************************************************
 * Commands and requests
 * **********************************************************************************
 */
/**
 * \brief AT command request type
 */
typedef enum {
	gm02s_at_request_type_none,			//!< No request
	gm02s_at_request_type_set,			//!< Set command. Expect only a OK as answer
	gm02s_at_request_type_get_set,		//!< Use the set format (=) but expect an answer
	gm02s_at_request_type_get_single,	//!< Get command. Only the first reported line is sent along the user request callback.
										//!< The OK may be present or not
	gm02s_at_request_type_get_multiple,	//!< Get command. All reported lines are sent along the user request callback.
										//!< The end of the command must be OK
} gm02s_at_request_type_t;


// Antenna tunning
typedef enum {
	gm02s_band_prefix_0 = 0,
	gm02s_band_prefix_1,
	gm02s_band_prefix_2,
	gm02s_band_prefix_3,
	gm02s_band_prefix_none,
} gm02s_band_prefix_t;

#define GM02S_MAX_NB_BANDS	3
#define GM02S_MIN_FREQUENCY 700
#define GM02S_MAX_FREQUENCY 1200

typedef struct {
	gm02s_band_prefix_t	prefix;
	uint16_t min_freq;
	uint16_t max_freq;
} gm02s_antenna_tuning_band_t;

typedef struct {
	gm02s_band_prefix_t default_prefix;
	gm02s_antenna_tuning_band_t bands[GM02S_MAX_NB_BANDS];
} gm02s_hw_antenna_tuning_t;

// Active mode
typedef enum {
	gm02s_active_mode_lte = 1,		//!< LTE-M mode
	gm02s_active_mode_nb_iot = 2,	//!< LTE NB-IOT
} gm02s_active_mode_t;

// eDrx ACT type
typedef enum {
	gm02s_edrx_act_type_none = 0,		//!< Not using eDRX
	gm02s_edrx_act_type_ec_gsm_iot = 1, //!< EC-GSM-IoT (A/Gb mode)
	gm02s_edrx_act_type_gsm = 2,		//!< GSM (A/Gb mode)
	gm02s_edrx_act_type_utran = 3,		//!< UTRAN (Iu mode)
	gm02s_edrx_act_type_eutran_wb = 4,	//!< E-UTRAN (WB-S1 mode)
	gm02s_edrx_act_type_eutran_nb = 5 	//!< E-UTRAN (NB-S1 mode)
}gm02s_edrx_act_type_t;

typedef enum {
	gm02s_edrx_mode_disabled = 0,		//!< Disable the use of eDRX
	gm02s_edrx_mode_enabled = 2 		//!< Enable the use of eDRX + unsolicited eDRX changes
} gm02s_edrx_mode_t;

typedef struct {
	gm02s_edrx_mode_t mode;				//!< eDRX mode
	gm02s_edrx_act_type_t act_type;		//!< ACT type
	uint8_t	requested_edrx_value;		//!< User Requested eDRX config (bitmap on a nibble) (see sub-clause 10.5.5.32 of 3GPP TS 24.008 [8])
	uint8_t	requested_ptw;				//!< paging time window (see sub-clause 10.5.5.32 of 3GPP TS 24.008 [8])
} gm02s_edrx_cfg_t;

typedef enum {
	gm02s_psm_mode_disabled = 0,		//!< Disable the use of PSM
	gm02s_psm_mode_enabled = 1 			//!< Enable the use of PSM
} gm02s_psm_mode_t;

typedef struct {
	gm02s_psm_mode_t mode;				//!< PSM mode
	uint8_t tau;						//!< bit-field: see sub-clause 10.5.5.32 of 3GPP TS 24.008 [8])
	uint8_t active_time;				//!< bit-field: see sub-clause 10.5.5.32 of 3GPP TS 24.008 [8])
} gm02s_psm_cfg_t;

typedef struct {
	uint32_t earfcn;					//!< TX/RX: An E-UTRA Absolute Radio Frequency Channel
	bool tx_enable;						//!< TX. Enable the TX RF wave
	int32_t tx_level;						//!< TX. Level in hundred of dBm
} gm02s_rf_test_t;

typedef enum {
	gm02s_nw_connection_detach = 0,			//!< Detach from the network
	gm02s_nw_connection_attach = 1,			//!< Attach to the network
} gm02s_nw_connection_t;

/*
 * \enum gm02s_request_data_t
 *
 * Request data passed along to a request or a command
 */

typedef struct {
	uint32_t rx_time;					//!< Number of mills spent in reception
	uint32_t tx_time;					//!< Number of mills spent in transmission (sum of all transmission times)
	uint32_t tx_time23;					//!< Number of mills spent in transmission with power > 22 dBm
	uint32_t tx_time22;					//!< Number of mills spent in transmission with  21 dBm < power <= 22 dBm
	uint32_t tx_time21;					//!< Number of mills spent in transmission with  20 dBm < power <= 21 dBm
	uint32_t tx_time20;					//!< Number of mills spent in transmission with  19 dBm < power <= 20 dBm
	uint32_t tx_time19;					//!< Number of mills spent in transmission with  18 dBm < power <= 19 dBm
	uint32_t tx_time18;					//!< Number of mills spent in transmission with  17 dBm < power <= 18 dBm
	uint32_t tx_time17;					//!< Number of mills spent in transmission with  16 dBm < power <= 17 dBm
	uint32_t tx_time16;					//!< Number of mills spent in transmission with  15 dBm < power <= 16 dBm
	uint32_t tx_time15;					//!< Number of mills spent in transmission with  12 dBm < power <= 15 dBm
	uint32_t tx_time12;					//!< Number of mills spent in transmission with   9 dBm < power <= 12 dBm
	uint32_t tx_time9;					//!< Number of mills spent in transmission with   6 dBm < power <= 9  dBm
	uint32_t tx_time6;					//!< Number of mills spent in transmission with   3 dBm < power <= 6  dBm
	uint32_t tx_time3;					//!< Number of mills spent in transmission with   0 dBm < power <= 3  dBm
	uint32_t tx_time0;					//!< Number of mills spent in transmission with power <= 0 dBm
}gm02s_rxtx_stats_t;

typedef union {
	gm02s_socket_cfg_t socket_cfg;			//!< Socket configuration
	gm02s_active_mode_t active_mode;		//!< Active mode
	gm02s_edrx_cfg_t edrx_cfg;				//!< eDRX configuration
	gm02s_psm_cfg_t psm_cfg;				//!< PSM configuration
	gm02s_rf_test_t rf_test;				//!< RF test (generate a wave or listen a RF channel)
	char* user_cmd;							//!< User command
	gm02s_nw_connection_t nw_connection;	//!< Attach/Detach the network
} gm02s_request_data_t;

/*
 * **********************************************************************************
 * Unsolicited notification
 * **********************************************************************************
 */
typedef enum {
	gm02s_nw_status_none = 0,			//!< Unregistered and no search for attachment
	gm02s_nw_status_home = 1,			//!< Registered against a home network
	gm02s_nw_status_searching = 2,		//!< Unregistered but searching an operator to register to.
	gm02s_nw_status_denied = 3,			//!< Registration denied
	gm02s_nw_status_unknown = 4,		//!< Unknown or out of coverage
	gm02s_nw_status_roaming = 5,		//!< Registered, in roaming
	gm02s_nw_status_sms_home = 6,		//!< Registered for SMS only against home network (NB-IoT mode only)
	gm02s_nw_status_sms_roaming = 7,	//!< Registered for SMS only, in roaming (NB-IoT mode only)
	gm02s_nw_status_emergency = 8,		//!< Attach for emergency bearer service only
	gm02s_nw_status_lost = 80,			//!< Sequans's proprietary notification. registered but temporary connection lost
	gm02s_nw_status_suspended,			//!< Abeeway specific: Airplane mode entered
} gm02s_nw_status_t;

typedef enum {
	gm02s_notif_act_type_gsm = 0,			//!< GSM (not applicable)
	gm02s_notif_act_type_gsm_compact=1,		//!< GSM Compact (not applicable)
	gm02s_notif_act_type_utran = 2,			//!< UTRAN (not applicable)
	gm02s_notif_act_type_gsm_egprs = 3,		//!< GSM w/EGPRS (see NOTE 3) (not applicable)
	gm02s_notif_act_type_utran_w_hsdpa = 4, //!< UTRAN w/HSDPA (see NOTE 4) (not applicable)
	gm02s_notif_act_type_utran_w_hsupa = 5, //!< UTRAN w/HSUPA (see NOTE 4) (not applicable)
	gm02s_notif_act_type_utran_both = 6,	//!< UTRAN w/HSDPA and HSUPA (see NOTE 4) (not applicable)
	gm02s_notif_act_type_eutran = 7, 		//!< E-UTRAN
	gm02s_notif_act_type_ec_gsm = 8,		//!< EC-GSM
	gm02s_notif_act_type_eutran_nb = 9 		//!< E-UTRAN NB
} gm02s_notif_act_type_t;

typedef enum {
	gm02s_cause_code_type_3gpp = 0,			//!< reject_cause is 3GPP reject cause (3GPP TS 24.301 [83] Annex A)
	gm02s_cause_code_type_manuf,			//!< reject_cause is manufacturer specific
	gm02s_cause_code_type_none,				//!< reject_cause is none (No error)
} gm02s_cause_code_type_t;

// Unsolicited CEREG data
typedef struct {
	gm02s_nw_status_t status;
	gm02s_notif_act_type_t act_type;	//!< ACT type
	uint16_t tac;						//!< Tracking area code
	uint32_t ci;						//!< EUTRAN cell-ID
	uint8_t psm_active_time; 			//!< PSM active time
	uint8_t psm_tau; 					//!< PSM TAU
	gm02s_cause_code_type_t cause_type;	//!< False means that the reject_cause is 3GPP reject cause (3GPP TS 24.301 [83] Annex A). True: Manufacturer specific
	uint32_t reject_cause;				//!< Reject cause
} gm02s_nw_status_info_t;

// Unsolicited EDRX data
typedef struct {
	gm02s_edrx_act_type_t act_type;		//!< Negotiated ACT type
	uint8_t	requested_cfg;				//!< User Requested eDRX config (bitmap on a nibble) (see sub-clause 10.5.5.32 of 3GPP TS 24.008 [8])
	uint8_t	provided_cfg;				//!< Network provided eDRX config (bitmap on a nibble) (see sub-clause 10.5.5.32 of 3GPP TS 24.008 [8])
	uint8_t	ptw;						//!< paging time window (see sub-clause 10.5.5.32 of 3GPP TS 24.008 [8])
} gm02s_edrx_info_t;

typedef struct {
	uint16_t year;						//!< Year (full format)
	uint8_t month;						//!< Month  [1..12]
	uint8_t day;						//!< day 	[1..31]
	uint8_t hour;						//!< Hour 	[0..23]
	uint8_t min;						//!< minute [0..59]
	uint8_t sec;						//!< Second [0..59]
} gm02s_utc_time_info_t;

typedef enum {
	gm02s_modem_status_active = 0,		//!< Modem is active
	gm02s_modem_status_sleeping,		//!< Modem is sleeping
}gm02s_modem_status_t;

typedef enum {
	gm02s_notif_type_open_done = 0,   	//!< GM02S configuration done.
	gm02s_notif_type_close_done,   		//!< GM02S close done. (modem is shutdown)
	gm02s_notif_type_modem_status, 		//!< GM02S Modem status
	gm02s_notif_type_nw_status,  		//!< GSM02 network status
	gm02s_notif_type_sim_status,  		//!< GSM02 SIM status
	gm02s_notif_type_edrx_change,   	//!< eDRX change
	gm02s_notif_type_socket_open,		//!< Socket opening result
	gm02s_notif_type_socket_close,		//!< Socket close result
	gm02s_notif_type_socket_rx_data,	//!< Socket Data received
	gm02s_notif_type_socket_tx_done,	//!< Socket transmit data complete
	gm02s_notif_type_utc_time,			//!< UTC Time information
	gm02s_notif_type_upgrade_start,		//!< SFU upgrade status start
	gm02s_notif_type_upgrade_success,	//!< SFU upgrade status success
	gm02s_notif_type_upgrade_failure,	//!< SFU upgrade status fail
	gm02s_notif_type_modem_crash		//!< Modem crash
} gm02s_notif_type_t;

typedef struct {
	gm02s_socket_id_t sid;				//!< Socket identifier
	uint16_t len;						//!< Data length
	uint8_t* data;						//!< Data in raw format
} gm02s_notif_socket_rx_data_t;

typedef enum {
	gm02s_notif_socket_result_success = 0,	//!< Socket open/close success
	gm02s_notif_socket_result_in_progress,	//!< Socket open/close request accepted by the modem. Wait for the success/failure
	gm02s_notif_socket_result_rqst_error,	//!< Socket open/close failure. Modem rejected the request.
	gm02s_notif_socket_result_timeout,		//!< Socket open/close timeout. Max retry to send the command reached,
	gm02s_notif_socket_result_no_carrier,	//!< Failure. Reason: No carrier
	gm02s_notif_socket_result_unknown,		//!< Failure. Reason: Unknown
	gm02s_notif_socket_result_refused,		//!< Failure. Reason: Refused by the peer
	gm02s_notif_socket_result_auth_reject,	//!< Failure. Reason: Authentication rejected
	gm02s_notif_socket_result_tls_error,	//!< Failure. Reason: TLS error
} gm02s_notif_socket_result_t;

/**
 * \brief SIM  status
 */
typedef enum {
	gm02s_sim_status_no_sim,				//!< No SIM card
	gm02s_sim_status_sim_under_init,		//!< SIM under initialization
	gm02s_sim_status_sim_locked,			//!< SIM locked (PIN/PUK) required
	gm02s_sim_status_sim_invalid,			//! SIM invalid
	gm02s_sim_status_sim_failure,			//!< SIM card failure
	gm02s_sim_status_sim_ready,				//!< SIM card ready
	gm02s_sim_status_sim_ph_net_pin,		//!< PH-NET pin required
	gm02s_sim_status_sim_ph_sim_pin,		//!< Phone-to-SIM password required
	gm02s_sim_status_invalid_ps_domain,		//!< Invalid SIM card in PS domain
	gm02s_sim_status_invalid_ps_cs_domain,	//!< Invalid SIM card in PS and CS domain
	gm02s_sim_status_invalid_cs_domain,		//!< Invalid SIM card in CS domain
}gm02s_sim_status_t;

typedef struct {
	gm02s_socket_id_t sid;				//!< Socket identifier
	gm02s_notif_socket_result_t result;	//!< Status of the operation
} gm02s_notif_socket_mgmt_data_t;

typedef union {
	gm02s_edrx_info_t edrx_info;			//!< Belong to gm02s_notif_type_edrx_change
	gm02s_notif_socket_rx_data_t rx_data;	//!< Belong to gm02s_notif_type_socket_rx_data
	gm02s_notif_socket_mgmt_data_t sock_mgmt;//!< Belong to gm02s_notif_type_socket_open, gm02s_notif_type_socket_close
	gm02s_nw_status_info_t nw_info;			//!< Belong to gm02s_notif_type_nw_status
	gm02s_utc_time_info_t utc_time;			//!< Belong to gm02s_notif_type_utc_time
	gm02s_sim_status_t sim_status;			//!< Belong to gm02s_notif_type_sim_status
	gm02s_modem_status_t modem_status;		//!< Belong to gm02s_notif_type_modem_status
	bool connected;							//!< Belong to gm02s_notif_type_connected. true: connected, false: disconnected
	bool power;								//!< Belong to gm02s_notif_type_power_state. true: power on, false: power off
	bool config_success;					//!< Belong to gm02s_notif_type_config_done. true: power on, false: power off
} gm02s_notif_data_t;

typedef void (*gm02s_notif_callback_t)(gm02s_notif_type_t type, gm02s_notif_data_t* notif_data, void* arg);

/*
 * **********************************************************************************
 * Response of a request
 * **********************************************************************************
 */
typedef enum {
	gm02s_response_status_success = 0,	//!< Answer correct
	gm02s_response_status_partial,		//!< Answer has partial result. Some fields may be incorrect
	gm02s_response_status_timeout,		//!< No response
	gm02s_response_status_error,		//!< Answer error
	gm02s_response_status_last			//!< Last status
} gm02s_response_status_t;

typedef struct {
	gm02s_socket_id_t sid;				//!< Socket identifier
	uint8_t ring_mode;					//!< Ring mode
	uint8_t rx_mode;					//!< RX mode
	uint32_t keep_alive;				//!< keep alive time
} gm02s_socket_cfg_info_t;

typedef struct {
	char sim_iccid[GM02S_ICCID_LEN];			//!< SIM ICCID
	char sim_euiccid[GM02S_EUICCID_LEN];		//!< SIM embedded universal ICCID (for e-sim only)
	char sim_imsi[GM02S_IMSI_LEN];				//!< SIM International Mobile Subscriber Identifier
} gm02s_sim_id_t;

typedef union {
	gm02s_socket_cfg_info_t socket_cfg_info;	//!< Socket configuration
	gm02s_active_mode_t active_mode;			//!< Active mode
	gm02s_edrx_cfg_t edrx_cfg;					//!< eDRX configuration
	int32_t rssi_rf_test;						//!< Value of rssi (in rx rf test) in hundreds of dBm
	int8_t rssi;								//!< Signal quality indication [-113 .. -57] dBm. Value 0: Indicates Not know or undetectable
	gm02s_nw_status_info_t nw_info;				//!< Network status. Same information than the notification
	gm02s_rxtx_stats_t stats;					//!< Modem RXTX statistics
	gm02s_sim_id_t sim_id;						//!< ICCID, EUICCID and IMSI of the SIM in use
} gm02s_response_data_t;

typedef struct {
	gm02s_response_status_t status;		//!< answer status
	char* raw_data;						//!< Unparsed data
	gm02s_response_data_t data;			//!< Parsed data
} gm02s_response_t;

// Called once a request has been serviced
typedef void (*gm02s_request_callback_t)(gm02s_response_t* response, void* arg);


/*
 * **********************************************************************************
 * Configuration
 * **********************************************************************************
 */
/**
 * \brief SIM  slots
 */
typedef enum {
	gm02s_sim_slot_interface_0,
	gm02s_sim_slot_interface_1,
	gm02s_sim_slot_interface_count,
}gm02s_sim_slot_interface_t;

typedef struct {
	aos_gpio_id_t power_load_switch;	//!< Load switch powering the modem
	aos_gpio_id_t reset_pin;			//!< Reset
	aos_gpio_id_t rts_pin;				//!< RTS of the UART. Managed manually
	aos_gpio_id_t cts_pin;				//!< CTS of the UART. Managed manually
	aos_gpio_id_t ring_pin;				//!< Ring pin
	aos_gpio_id_t status_pin;			//!< Status pin
} gm02s_hw_init_t;

typedef enum {
	gm02s_open_mode_off = 0,	//!< Modem shutdown
	gm02s_open_mode_normal,		//!< Normal mode
	gm02s_open_mode_airplane,	//!< Airplane mode
	gm02s_open_mode_manuf,		//!< Manufacturing mode
	gm02s_open_mode_manuf_ccid,	//!< Airplane mode to get the CCID in manuf mode
} gm02s_open_mode_t;

typedef struct {
	gm02s_active_mode_t network_type;					//!< Active mode
	gm02s_edrx_cfg_t edrx;								//!< eDRX configuration
	gm02s_psm_cfg_t psm;								//!< PSM configuration
	gm02s_sim_slot_interface_t sim_interface;			//!< SIM interface used
	char access_point_name[GM02S_APN_MAX_SIZE];			//!< Access point name string.
	char operator_sim_slot_0[GM02S_OPERATOR_MAX_SIZE];	//!< Operator forced on SIM inserted on slot 0.
	char operator_sim_slot_1[GM02S_OPERATOR_MAX_SIZE];	//!< Operator forced on SIM inserted on slot 1.
	gm02s_notif_callback_t callback;					//!< user callback called for notification and RX data
	void* user_arg;										//!< User argument
} gm02s_modem_config_t;


typedef enum{
	gm02s_config_save = 0,				//!< manufacturing config save
	gm02s_config_restore,				//!< manufacturing config restore
}gm02s_modem_config_save_restore_t;

typedef enum{
	gm02s_modem_sim1_gpio_nrst = 25,				//!< sim1 interface nRST pin
	gm02s_modem_sim1_gpio_clk,						//!< sim1 interface CLK pin
	gm02s_modem_sim1_gpio_io,						//!< sim1 interface IO pin
}gm02s_modem_sim1_gpio_t;


/*
 * **********************************************************************************
 * Information
 * **********************************************************************************
 */
/**
 * \brief FSM  states
 */
typedef enum {
	gm02s_state_off = 0,			// Modem is sleeping and not configured (off)
	gm02s_state_probing,			// Probe the UART speed
	gm02s_state_initializing,		// Modem is being initialized
	gm02s_state_shutdown,			// Modem is initialized and has been shutdown
	gm02s_state_opening,			// Modem is opening
	gm02s_state_sleeping,			// Modem is sleeping
	gm02s_state_active,				// Modem active, Free to send or receive data
	gm02s_state_wait_wakeup,		// Modem was sleeping and we wait for its wake-up
	gm02s_state_closing,			// Modem is in the process of stopping
	gm02s_state_upgrading,			// Modem is in the process of stopping
	gm02s_state_last				// Number of states
} gm02s_state_t;

typedef struct {
	gm02s_sim_slot_interface_t sim_interface;	//!< SIM interface used
	gm02s_sim_status_t sim_status;				//!< SIM status
	char sim_iccid[GM02S_ICCID_LEN];			//!< SIM ICCID
	char sim_euiccid[GM02S_EUICCID_LEN];		//!< SIM embedded universal ICCID (for e-sim only)
	char sim_imsi[GM02S_IMSI_LEN];				//!< SIM ICCID
	bool iccid_available;						//!< SIM ICCID availability
	bool imsi_available;						//!< SIM IMSI availability
} gm02s_sim_info_t;

/**
 * \brief Modem usage and consumption
 */
typedef struct {
	uint64_t usage_active;				//!< Total amount of time the modem is active in ms
	uint64_t usage_nwk;					//!< Total amount of time the modem is communicating with the network (RX and TX only) in ms
	uint64_t usage_deepsleep;			//!< Total amount of time the modem is in deeep-sleep in ms
	uint64_t nwk_conso;					//!< Power estimation during networking (predicted by the model)
	uint64_t total_conso;				//!< Total power estimation of the modem in uAh
	uint32_t seq_count;					//!< Total count of up-link sequences
} gm02s_modem_usage_t;

/**
 * \brief Modem firmware version
 */
typedef struct {
	uint8_t fw_branch;					//!< Firmware branch
	uint8_t fw_mode;					//!< Firmware mode : single mode or dual mode
	uint8_t fw_image;					//<! Firmware image
	uint8_t delivery_number;			//<! Firmware official delivery number
	uint16_t release_number;			//<! Firmware release number
} gm02s_firmware_version_t;

typedef struct {
	gm02s_state_t state;							//!< Driver state
	bool cts_on;									//!< True if CTS asserted (TX enabled)
	bool rts_on;									//!< True if RTS asserted (RX enabled)
	bool modem_on;									//!< True if modem is on
	bool uart_speed_115k;							//!< True if the UART speed is 115200, false if UART speed is 57600 (low power)
	gm02s_open_mode_t open_mode;					//!< Opened mode
	gm02s_psm_mode_t psm_mode;						//!< PSM mode used
	gm02s_nw_status_info_t nw_info;					//!< Network status
	gm02s_edrx_info_t edrx_info;					//!< EDRX changes updated by the network
	int last_rx_err;								//!< Last received error
	uint16_t curr_rx_len;							//!< Current number of bytes in the RX buffer
	bool tx_buf_used;								//!< True if the buffer is currently filled, false otherwise
	uint32_t modem_active_time;						//!< Time the modem was active
	gm02s_sim_info_t sim_info;						//!< SIM status
	gm02s_modem_usage_t usage;						//!< Modem power usage
	gm02s_firmware_version_t fw_version;			//!< Modem firmware version
} gm02s_drv_info_t;

/*
 * **********************************************************************************
 * Initialization
 * **********************************************************************************
 */
typedef enum {
	gm02s_init_success,			//!< Modem initialized  success
	gm02s_init_failure,			//!< Modem not responding
	gm02s_init_partial,			//!< Modem partially initialized : unable to retrieve SIM info and/or firmware version
} gm02s_init_status_t;

typedef struct {
	gm02s_init_status_t status;		//!< init status
	char* raw_data;					//!< Unparsed data for future use
} gm02s_init_result_t;

typedef void (*gm02s_init_callback_t)(gm02s_init_result_t* response, void* arg);


/*
 * ***************************************************************************
 * Service API
 * ***************************************************************************
 */

/**
 *
 * \fn aos_result_t gm02s_drv_pre_init(gm02s_hw_init_t* hw_cfg)
 *
 * \brief Initialize and turn off the load switch during system startup
 *
 * \param hw_cfg Hardware configuration of the modem
 *
 * \return The status of the operation
 *
 * \note At the end of the initialization, the modem is turned off using the load switch
 */
aos_result_t gm02s_drv_pre_init(gm02s_hw_init_t* hw_cfg);

/**
 *
 * \fn aos_result_t gm02s_drv_init(gm02s_init_callback_t init_cb)
 *
 * \brief Initialize the gm02s driver
 *
 * \param init_cb User callback to receive the result status and data. May be NULL.
 *
 * \return The status of the operation
 *
 * \note The init takes long. At the end, the modem is turned off using the load switch and the init callback is called.
 */
aos_result_t gm02s_drv_init(gm02s_init_callback_t init_cb);

/**
 *
 * \fn aos_result_t gm02s_set_power(bool state)
 *
 * \brief Control the power state of the modem
 *
 * \param state Power state of the modem
 *
 */
aos_result_t gm02s_drv_set_power(bool state);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_open(gm02s_open_mode_t open_mode, gm02s_modem_config_t* modem_cfg)
 *
 * \brief Open the modem and attach to the network
 *
 * \param open_mode Main power mode in which the driver will operate.
 * \param modem_cfg Modem configuration
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_open(gm02s_open_mode_t open_mode, gm02s_modem_config_t* modem_cfg);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_close(void)
 *
 * \brief Close the modem
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_close(void);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_suspend(bool suspend)
 *
 * \brief Suspend or resume the operational mode of the modem
 * \param suspend True if the modem should be suspended. False if the modem should resume
 *
 * \return The status of the operation
 *
 * \Note The modem should be opened in normal mode and connected before being suspended. When suspended,
 * the modem moves to the air-plaine mode. When resumed, it moves back to the operational mode.
 */
aos_result_t gm02s_drv_modem_suspend(bool suspend);

/**
 *
 * \fn gm02s_open_mode_t gm02s_drv_get_open_mode(void)
 *
 * \brief Return the opening mode
 *
 * \return The mode
 */
gm02s_open_mode_t gm02s_drv_get_open_mode(void);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_reset_and_restart(void)
 *
 * \brief Reset the modem (HW reset) and restart
 * \param modem_cfg New configuration if any. NULL if the previous config should be used.
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_reset_and_restart(gm02s_modem_config_t* modem_cfg);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_attach(bool attach, gm02s_request_callback_t user_cb, void* user_arg)
 *
 * \brief Attach/detach to/from the network
 *
 * \param attach true to attach, false to detach
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_attach(bool attach, gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_get_utc_time(void)
 *
 * \brief  Request the UTC time to a well known NTP server
 *
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 *
 * \comment: NTP server used are: time.nist.gov and time.google.com. The NTP result is
 * passed to the user via the usual notification callback
 */
aos_result_t gm02s_drv_modem_get_utc_time(void);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_get_rssi(gm02s_request_callback_t user_cb, void* user_arg)
 *
 * \brief Request the RSSI
 *
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 *
 */
aos_result_t gm02s_drv_modem_get_rssi(gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn bool gm02s_drv_is_busy(void)
 *
 * \brief Check whether the driver is busy
 *
 * \return True is busy, false otherwise
 */
bool gm02s_drv_is_busy(void);

/**
 *
 * \fn aos_result_t gm02s_drv_socket_open(gm02s_socket_cfg_t* socket_cfg)
 *
 * \brief open a TCP or UDP socket
 *
 * \param socket_cfg: Socket configuration
 *
 * \return The status of the operation
 *
 * \note Socket notifications are passed to the user via the user callback provided in the modem_cfg.
 */
aos_result_t gm02s_drv_socket_open(gm02s_socket_cfg_t* socket_cfg);

/**
 *
 * \fn aos_result_t gm02s_drv_socket_close(gm02s_socket_id_t sid)
 *
 * \brief close a TCP or UDP socket
 *
 * \param sid Socket identifier that has been provided to the open function via the socket_cfg parameter
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_socket_close(gm02s_socket_id_t sid);

/**
 *
 * \fn aos_result_t gm02s_drv_socket_send(gm02s_socket_buffer_t* buffer)
 *
 * \brief Send over a TCP or UDP socket
 *
 * \param sid Socket identifier
 * \param data_len Number of bytes to send
 * \param data Pointer to the data to send.
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_socket_send(gm02s_socket_id_t sid, uint8_t data_len, uint8_t* data);

/**
 *
 * \fn aos_result_t aos_result_t gm02s_drv_send_user_at_cmd(char* user_cmd, gm02s_at_request_type_t rqst_type , gm02s_request_callback_t user_cb, void* user_arg);
 *
 * \brief Send a user made AT command
 *
 * \param user_cmd User made AT command to send
 * \param rqst_type Request type
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_send_user_at_cmd(char* user_cmd, gm02s_at_request_type_t rqst_type , gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn aos_result_t gm02s_drv_get_info(gm02s_drv_info_t *info)
 *
 * \brief Get driver information
 *
 * \param info Pointer where to put the information
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_get_info(gm02s_drv_info_t *info);

/**
 *
 * \fn void gm02s_drv_enable_ps_status_log(bool enable)
 *
 * \brief Enable/Disable the tracing of the PS_status line.
 *
 * \param enable True to enable the trace. False to disable
 *
 * \return The status of the operation
 */
void gm02s_drv_enable_ps_status_log(bool enable);

/**
 *
 * \fn const char* gm02s_drv_state_to_str(gm02s_state_t state)
 *
 * \brief Convert the driver state to a displayable C-string
 *
 * \param state Driver state to convert
 *
 * \return The c-string
 */
const char* gm02s_drv_state_to_str(gm02s_state_t state);

/**
 *
 * \fn const char* gm02s_drv_nw_status_to_str(gm02s_nw_status_t status)
 *
 * \brief Convert the network status to a displayable C-string
 *
 * \param status network status
 *
 * \return The c-string
 */
const char* gm02s_drv_nw_status_to_str(gm02s_nw_status_t status);

/**
 *
 * \fn const char* gm02s_drv_notif_type_to_str(gm02s_notif_type_t type)
 *
 * \brief Convert the notification type to a displayable C-string
 *
 * \param type Notification type
 *
 * \return The c-string
 */
const char* gm02s_drv_notif_type_to_str(gm02s_notif_type_t type);

/**
 *
 * \fn const char* gm02s_drv_socket_result_to_str(gm02s_notif_socket_result_t result)
 *
 * \brief Convert the socket result to a displayable C-string
 *
 * \param result Socket result
 *
 * \return The c-string
 */
const char* gm02s_drv_socket_result_to_str(gm02s_notif_socket_result_t result);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_tx_rf_test(bool tx_enable, int32_t earfcn, int32_t tx_level)
 *
 * \brief Start an uplink continuous wave
 *
 * \param modem_rf_test_cfg rf test configuration
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_tx_rf_test(gm02s_rf_test_t* modem_rf_test_cfg, gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_rx_rf_test(int32_t earfcn)
 *
 * \brief Start a continuous wave RX rssi measurement
 *
 * \param modem_rf_test_cfg rf test configuration
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_rx_rf_test(gm02s_rf_test_t* modem_rf_test_cfg, gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_hwcfg_uart0(gm02s_request_callback_t user_cb, void* user_arg)
 *
 * \brief Configure UART0 with default configuration
 *
 * \param high_speed boolean flag indicating UART speed true : 115200, false:57600
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \note using baud-rate 115200 will cause the lpuart to consume an extra 200µA compared to low speed baud-rate (57600).
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_hwcfg_uart0(bool high_speed, gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_hwcfg_uart1(bool uart1_mode_at,gm02s_request_callback_t user_cb, void* user_arg)
 *
 * \brief Configure UART1 with default configuration
 *
 * \param uart1_mode_at boolean flag indicating UART1 mode ("at" or "dcp")
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_hwcfg_uart1(bool uart1_mode_at, gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_config_save_restore(gm02s_modem_config_config_restore_t gm02s_config_action)
 *
 * \brief Save current configuration / restore saved configuration
 *
 * \param gm02s_config_action Action can be save or restore configuration
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_config_save_restore(gm02s_modem_config_save_restore_t gm02s_config_action, gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn aos_result_t gm02s_drv_modem_antenna_tuning(gm02s_hw_antenna_tuning_t* gm02s_antenna_tuning)
 *
 * \brief Tuning the antenna
 *
 * \param gm02s_antenna_tuning Configuration of bands
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_antenna_tuning(gm02s_hw_antenna_tuning_t* gm02s_antenna_tuning, gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn aos_result_t gm02s_drv_rxtx_stats(bool get_stats, bool enable, bool reset, gm02s_request_callback_t user_cb, void* user_arg)
 *
 * \brief Get RXTX statistics
 *
 * \param get_stats  Get current RXTX statistics
 * \param enable Enable the statistics acquisition
 * \param reset  Reset statistics values
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 *
 * \note get_stats is exclusive with other parameters : enable and reset are allowed only when get_stats is false.
 */
aos_result_t gm02s_drv_rxtx_stats(bool get_stats, bool enable, bool reset, gm02s_request_callback_t user_cb, void* user_arg);
/*! @}*/

/**
 *
 * \fn aos_result_t gm02s_drv_modem_sim1_enable(bool enable, gm02s_request_callback_t user_cb, void* user_arg)
 *
 * \brief Enable/Disable the SIM0/SIM1 interfaces.
 *
 * \param sim_interface SIM interface : SIM0 or SIM1
 * \param enable SIM interface activation : enable or disable
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_modem_sim_enable(gm02s_sim_slot_interface_t sim_interface, bool enable, gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn const char* gm02s_drv_sim_status_to_str(gm02s_sim_status_t sim_status)
 *
 * \brief Convert the sim status to a displayable C-string
 *
 * \param sim_status SIM status
 *
 * \return The c-string
 */
const char* gm02s_drv_sim_status_to_str(gm02s_sim_status_t sim_status);

/**
 *
 * \fn aos_result_t gm02s_drv_get_sim_info(gm02s_request_callback_t user_cb, void* user_arg);
 *
 * \brief Get the ICCID, EUICCID and IMSI of the SIM in use.
 *
 * \param user_cb User callback to receive the command result status and data. May be NULL
 * \param user_arg User argument passed along to the user callback. Opaque for the driver.
 *
 * \return The status of the operation
 */
aos_result_t gm02s_drv_get_sim_info(gm02s_request_callback_t user_cb, void* user_arg);

/**
 *
 * \fn uint32_t gm02s_drv_get_psm_tau_in_sec(uint8_t atime);
 *
 * \brief Convert the TAU period to seconds
 *
 * \param tau Bit map of TAU period applied by the network
 *
 * \return The TAU period time in seconds
 */
uint32_t gm02s_drv_get_psm_tau_in_sec(uint8_t tau);

/**
 *
 * \fn uint32_t gm02s_drv_get_psm_active_time_in_sec(uint8_t atime);
 *
 * \brief Convert the PSM active time to seconds
 *
 * \param atime Bit map of PSM active time applied by the network
 *
 * \return The PSM active time in seconds
 */
uint32_t gm02s_drv_get_psm_active_time_in_sec(uint8_t atime);

/**
 * \fn void gm02s_drv_clear_consumption(void)
 *
 * \brief Clear the GM02S power consumption.
 *
 */
void gm02s_drv_clear_consumption(void);

/**
 * \fn uint64_t gm02s_drv_get_consumption_uah(void)
 *
 * \brief Retrieve the GM02S energy consumption in uAh
 *
 * \return GM02S energy consumption
 */
uint64_t gm02s_drv_get_consumption_uah(void);

#ifdef __cplusplus
}
#endif




