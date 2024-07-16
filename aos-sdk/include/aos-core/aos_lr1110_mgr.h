/*!
 * \file aos_lr1110_mgr.h
 *
 * \brief LR1110 manager
 *
 * \details
 * This manager centralizes the LR1110 accesses to available feature set: LoRa class A and B, WIFI sniffing
 * and GPS sniffing.
 * It provides a client/server scheme, where it acts as the server. The clients of the manager should
 * register against it.
 * The registration requires a client callback function, which will be triggered each time a significant
 * event is received from the lower layers.
 *
 * The manager runs its own thread under which the client callback is called. The manager sits at the top
 * of the Semtech LBM (LoRa Basic Modem) stack. It uses the smtc_modem_api functionalities as well as
 * the lorawan_api (when functions are not exported by the modem API).
 * The manager also relies on the Semtech radio planer, which schedules the different LR1110 tasks such
 * as Join, LoRa TX, LoRa RX, LoRa class B, WIFI and GPS sniffing.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "aos_wifi.h"
#include "aos_lr1110_gnss.h"
#include "aos_gpio.h"

/*!
 * \defgroup lr1110_mgr LR1110 manager
 *
 * \addtogroup lr1110_mgr
 * @{
 */

/*!
 * \enum aos_lr1110_mgr_status_t
 *
 * \brief Statuses returned by the manager
 */
typedef enum {
	aos_lr1110_mgr_status_success = 0,		//!< Command success
	aos_lr1110_mgr_status_rx_data,			//!< RX data success
	aos_lr1110_mgr_status_tx_success,		//!< Non confirmed TX success
	aos_lr1110_mgr_status_tx_conf_success,	//!< Confirmed TX success
	aos_lr1110_mgr_status_tx_failure,		//!< TX failure
	aos_lr1110_mgr_status_error_no_provisioning,	//!< Device not provisioned
	aos_lr1110_mgr_status_error_not_join,	//!< Command fails because the device has not join
	aos_lr1110_mgr_status_error_not_init,	//!< Command fails because the modem is not initialized.
	aos_lr1110_mgr_status_error_param,		//!< Command fails because the calling parameter are incorrect
	aos_lr1110_mgr_status_error_busy,		//!< Command fails because the modem is busy
	aos_lr1110_mgr_status_error_size,		//!< Size error
	aos_lr1110_mgr_status_error_no_time,	//!< Timing error
	aos_lr1110_mgr_status_error_not_registered,		//!< Client not registered
	aos_lr1110_mgr_status_error_time_not_sync,	// Time is not in sync with LoRa
	aos_lr1110_mgr_status_error_cancelled,	//!< Action has been cancelled by the user
	aos_lr1110_mgr_status_error_start_scan,	//!< GNSS start scan or WIFI start scan failure
	aos_lr1110_mgr_status_error_cancel_scan,//!< GNSS cancel scan or WIFI cancel scan failure
	aos_lr1110_mgr_status_error_other		//!< Catch all
} aos_lr1110_mgr_status_t;


/*!
 * \enum aos_lr1110_mgr_client_type_t
 *
 * \brief Clients that can be registered agaist the LR1110 manager
 */
typedef enum {
	aos_lr1110_mgr_client_lora_class_a,			//!< LoRa class A client (use it also to join the network)
	aos_lr1110_mgr_client_lora_class_b,			//!< LoRa class B client
	aos_lr1110_mgr_client_wifi,					//!< WIFI scan client
	aos_lr1110_mgr_client_gnss_scan,			//!< GNSS scan client
	aos_lr1110_mgr_client_gnss_query,			//!< GNSS query (almanac) client
	aos_lr1110_mgr_client_count					//!< Number of clients
} aos_lr1110_mgr_client_type_t;

/*!
 * \enum aos_lr1110_mgr_event_type_t
 *
 * \brief Event types reported to client.
 */
typedef enum {
	aos_lr1110_mgr_event_type_lora_join,		//!< Join network success/Success
	aos_lr1110_mgr_event_type_lora_leave,		//!< Leave network success/Success
	aos_lr1110_mgr_event_type_lora_tx_done,		//!< TX success/failure
	aos_lr1110_mgr_event_type_lora_rx,			//!< RX downlink
	aos_lr1110_mgr_event_type_lora_rx_pending,	//!< RX downlink with the frame pending flag set (gateway has more data to send).
	aos_lr1110_mgr_event_type_lora_time,		//!< LoRa time request/answer
	aos_lr1110_mgr_event_type_link_check,		//!< LoRa link check response
	aos_lr1110_mgr_event_type_wifi, 			//!< WIFI success/failure
	aos_lr1110_mgr_event_type_gnss_scan,		//!< GNSS scan success/failure
	aos_lr1110_mgr_event_type_gnss_query,		//!< GNSS query success/failure
	aos_lr1110_mgr_event_type_count,			//!< Number of event types
} aos_lr1110_mgr_event_type_t;

/*!
 * \struct aos_lr1110_mgr_info_lora_t
 *
 * \brief Information belonging to LoRa events
 */
typedef struct {
	union {
		struct {
			uint8_t port;					//!< Port number on which the payload has been received
			uint16_t payload_size;			//!< RX payload size
			uint8_t* payload;				//!< Payload received
		} rx_data;							//!< Data received
	};
} aos_lr1110_mgr_info_lora_t;


/*!
 * \struct aos_lr1110_mgr_client_event_t
 *
 * \brief Event sent to the registered client
 */
typedef struct {
	aos_lr1110_mgr_event_type_t type;						//!< Type of event
	aos_lr1110_mgr_status_t status;							//!< Status of the event
	union {
		aos_lr1110_mgr_info_lora_t 	lora;					//!< LoRa specific data
		aos_wifi_scan_result_data_t* wifi;					//!< WIFI specific data
		aos_lr1110_gnss_result_t* gnss_scan;				//!< GNSS scan specific data
		const aos_lr1110_gnss_query_result_t* gnss_query;	//!< GNSS query specific data
	};
}aos_lr1110_mgr_client_event_t;


#define AOS_LR1110_LORA_MAX_PAYLOAD_SIZE	252			//!< Max LoRa payload
#define AOS_LR1110_LORA_MAX_DR_DISTRIBUTION	16			//!< Max Data rate distribution for the join

/*!
 * \brief Values for the flags
 */
#define AOS_LR1110_LORA_FLAG_CONFIRMED		(1<<0)		//!< Indicate whether the payload is confirmed
#define AOS_LR1110_LORA_FLAG_ALLOW_UP_DR	(1<<1)		//!< If the payload size does not fit the current DR one, allow to use a greater DR


/*!
 * \enum aos_lr1110_mgr_lora_dr_t
 *
 * \brief Supported LoRa dara-rate.
 * \note  Some regions does not support all of them.
 */
typedef enum {
    aos_lr1110_mgr_lora_dr0 = 0,				//!< Data rate 0
    aos_lr1110_mgr_lora_dr1,					//!< Data rate 1
    aos_lr1110_mgr_lora_dr2,					//!< Data rate 2
    aos_lr1110_mgr_lora_dr3,					//!< Data rate 3
    aos_lr1110_mgr_lora_dr4,					//!< Data rate 4
    aos_lr1110_mgr_lora_dr5,					//!< Data rate 5
    aos_lr1110_mgr_lora_dr6,					//!< Data rate 6
    aos_lr1110_mgr_lora_dr7,					//!< Data rate 7
    aos_lr1110_mgr_lora_dr8,					//!< Data rate 8
    aos_lr1110_mgr_lora_dr9,					//!< Data rate 9
    aos_lr1110_mgr_lora_dr10,					//!< Data rate 10
    aos_lr1110_mgr_lora_dr11,					//!< Data rate 11
    aos_lr1110_mgr_lora_dr12,					//!< Data rate 12
    aos_lr1110_mgr_lora_dr13,					//!< Data rate 13
    aos_lr1110_mgr_lora_dr14,					//!< Data rate 14
    aos_lr1110_mgr_lora_dr15,					//!< Data rate 15
    aos_lr1110_mgr_lora_dr_adr,					//!< Network managed (ADR)
    aos_lr1110_mgr_lora_dr_count,				//!< Number of DR
} aos_lr1110_mgr_lora_dr_t;

/*!
 * \struct aos_lr1110_mgr_lora_payload_t
 *
 * \brief LoRa payload definition
 */
typedef struct {
	uint8_t dr;			//!< Data rate. If ADR is expected, field unused but the associated flag should be set
	uint8_t flags;		//!< Flag. See definitions above.
	uint8_t size;		//!< Payload size
	uint8_t fport;		//!< LoRa port
	uint8_t data[AOS_LR1110_LORA_MAX_PAYLOAD_SIZE]; //!< Payload data
} aos_lr1110_mgr_lora_payload_t;

/*!
 * \brief   Client callback function definition
 * \details Client function called when events are available
 * \param   context Client context. Opaque for the manager
 * \param   event Event passed to the client
 * \return  None
 */
typedef void (* aos_lr1110_mgr_client_callback_t)(void* context, aos_lr1110_mgr_client_event_t* event);

/*!
 * \brief   Manager initialization
 * \details Must be called at the system start time before using the manager
 * \param   ext_lora_rf_switch GPIO identifier, which manages an external RF switch. Use aos_gpio_id_last if no RF switch
 *
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_init(aos_gpio_id_t ext_lora_rf_switch);

/*!
 * \brief   Client registration function
 * \details Client wishing to receive events should register using this function
 * \param   type Client type
 * \param   cb Client callback function
 * \param	user_arg Client context that will be provided along to the callback function
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_register_client(aos_lr1110_mgr_client_type_t type, aos_lr1110_mgr_client_callback_t cb, void* user_arg);

/*!
 * \brief   Client un-registration function
 * \details Remove the client registration
 * \param   type Client type
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_unregister_client(aos_lr1110_mgr_client_type_t type);

/*!
 * \brief   Return the hardware information of the LR1110 component
 * \details Client wishing to receive events should register using this function
 * \param hw Hardware version
 * \param type Hardware type
 * \param fw Firmware version running on the LR1110
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_get_hw_version(uint8_t *hw, uint8_t* type, uint16_t* fw);

/*!
 * \brief  Set the board delay in ms
 * \details At the start time, the board delay is fixed to 10 ms. You can customize it via this function
 * \param delay_ms: Delay in ms
 */
void aos_lr1110_mgr_set_board_delay(int8_t delay_ms);

/*!
 * \brief  Set the antenna compensation in dBm
 * \details Set the antenna compensation, which is the loss or the gain provided by the external LoRa antenna.
 * \param compensation Antenna compensation in dBm
 */
void aos_lr1110_mgr_set_lora_antenna_compensation(int8_t compensation);

/*!
 * \brief  Manage the LoRa duty cycle
 * \details Enable/disable the LoRa duty cycle (DTC)
 * \param enable True to enable the DTC, false to disable it
 */
void aos_lr1110_mgr_lora_set_duty_cycle(bool enable);

/*!
 * \brief  Retrieve the LoRa duty cycle state
 * \details Retrieve the LoRa duty cycle state (DTC)
 * \return True if the DTC is enabled, False if disabled.
 */
bool aos_lr1110_mgr_lora_get_duty_cycle(void);

/*!
 * \brief   Request a LoRa network join
 * \details Start the LoRa network joing process
 * \param dr_distribution Datarate distribution for the join
 * \return  The status of the operation
 * \note 	To use this function, The Network should not be joined.
 * 			If you want to force a rejoin, then the network should be leaved before.
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_lora_join(const aos_lr1110_mgr_lora_dr_t dr_distribution[AOS_LR1110_LORA_MAX_DR_DISTRIBUTION]);

/*!
 * \brief   Leave the LoRa network
 * \details Reinitialize the LoRa network part
 * \return  The status of the operation
 * \note If you want to force a rejoin, then the network should be leaved before.
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_lora_leave(void);

/*!
 * \brief   Set the LoRa DevNonce value
 * \details This value is used during the LoRa join procedure.
 * \param	devnonce LoRa DevNonce value
 * \return  The status of the operation
 * \note This value is permanently stored.
 * \warning Setting an incorrect value may prevent a successful join.
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_set_devnonce(uint16_t devnonce);

/*!
 * \brief   Get the last LoRa DevNonce value used.
 * \details This value is used during the LoRa join procedure.
 * \param[out]	devnonce LoRa DevNonce value
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_get_devnonce(uint16_t *devnonce);

/*!
 * \brief Send a LoRa payload in class A
 * \details The payload will be scheduled for transmission.
 * \param dr Data rate at which the payload should be sent
 * \param flags LoRa flags. Refer to AOS_LR1110_LORA_FLAG_xxx
 * \param port Uplink port number
 * \param data_len Payload size
 * \param data Actual payload
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_lora_tx(aos_lr1110_mgr_lora_dr_t dr, uint8_t flags, uint8_t port, uint8_t data_len, uint8_t* data);

/*!
 * \brief Send an empty LoRa payload in class A
 * \details The payload will be scheduled for transmission.
 * \param dr Data rate at which the payload should be sent
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_lora_tx_empty(aos_lr1110_mgr_lora_dr_t dr);

/*!
 * \brief   Send a device time request to LoRa to update the system time
 * \details The request status is received via the usual callback.
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_lora_request_time(void);

/*!
 * \brief   Send a link check request
 * \details The request status is received via the usual callback.
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_lora_link_check(void);

/*!
 * \brief   Manage the LCTT certification
 * \details Enable/Disable the certification mode
 * \param	enable True to enable the certification mode, false to disable it
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_lora_enable_certif(bool enable);

/*!
 * \brief   Start a WIFI scan
 * \details WIFI scan
 * \param	settings WIFI scan configuration
 * \return  The status of the operation
 *
 * \note The WIFI client must be registered
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_wifi_start_scan(aos_wifi_settings_t* settings);

/*!
 * \brief Stop a WIFI scan
 *
 * \details. If a scan was actually in progress, it is aborted and the user callback
 * will be triggered with an abort status. Otherwise the user callback is not triggered,
 *
 * \return The operation result:
 * aos_lr1110_mgr_status_success if the scan can be aborted,
 * aos_lr1110_mgr_status_error_other otherwise
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_wifi_stop_scan(void);

/*!
 * \brief   Start a GNSS scan
 * \details GNSS scan
 * \param	settings GNSS scan configuration
 * \return  The status of the operation
 *
 * \note The GNSS client must be registered
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_gnss_start(aos_lr1110_gnss_settings_t* settings);

/*!
 * \brief Stop a GNSS scan
 *
 * \details. If a scan was actually in progress, it is aborted and the user callback
 * will be triggered with an abort status. Otherwise the user callback is not triggered,
 * \return The operation result:
 * aos_lr1110_mgr_status_success if the scan can be aborted,
 * aos_lr1110_mgr_status_error_other otherwise
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_gnss_stop(void);

/*!
 * \brief   Perform a GNSS query
 * \details GNSS almanac query
 * \param	query Query description
 * \return  The status of the operation
 *
 * \note The GNSS client does not need to be registered
 */
aos_lr1110_mgr_status_t aos_lr1110_mgr_gnss_query(aos_lr1110_gnss_query_request_t* query);

/*!
 * \brief  Convert the LR1110 manager status to a displayable string
 * \param mgr_status Status to convert
 * \return The string
 */
const char *aos_lr1110_mgr_status_to_str(aos_lr1110_mgr_status_t mgr_status);

/*!
 * \brief   Trigger a run of the modem (LBM) task
 * \details trigger a run of the modem task
 * \return  True if success
 *
 * \note Used internally upon WIFI/GNSS scan done event
 */
bool aos_lr1110_trigger_modem_task(void);
/*! @}*/
#ifdef __cplusplus
}
#endif


