/*!
 * \file srv_lora.h
 *
 * \brief LoRa service
 *
 * \details This service relies on the LR1110 manager. It sits on to of it.
 * 			Events are sent (via callback) to the service's user.
 * 			Note that the reception can be delayed, meaning that the user does not need
 * 			to process the message as soon as the associated event is received.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup srv_lora LoRa service
 *
 * \addtogroup srv_lora
 * @{
 */

#include "aos_lr1110_mgr.h"

#define SRV_LORA_DEVEUI_SIZE		8			//!< Standard size of a LoRa Device Unique Identifier
#define SRV_LORA_JOINEUI_SIZE		8			//!< Standard size of a LoRa Join Unique Identifier

/*!
 * \enum srv_lora_user_event_t
 *
 * \brief Event types reported to the user.
 */
typedef enum {
	srv_lora_user_event_join_success,		//!< Join network success
	srv_lora_user_event_join_failure,		//!< Join network failure
	srv_lora_user_event_leave_success,		//!< Leave network success
	srv_lora_user_event_leave_failure,		//!< Join failure
	srv_lora_user_event_tx_success,			//!< Actual transmission success
	srv_lora_user_event_tx_failure,			//!< Actual transmission fails
	srv_lora_user_event_tx_request_failure,	//!< Request to transmit fails
	srv_lora_user_event_rx,					//!< Downlink available
	srv_lora_user_event_rx_pending,			//!< Downlink available with frame pending bit set.
	srv_lora_user_event_time_upd_success,	//!< Time update success
	srv_lora_user_event_time_upd_failure,	//!< Time update failure
	srv_lora_user_event_link_check_success,	//!< Link check success
	srv_lora_user_event_link_check_failure,	//!< Link check failure
	srv_lora_user_event_count				//!< Number of events
} srv_lora_user_event_t;

/*!
 * \enum srv_lora_tx_strategy_t
 *
 * \brief Transmission strategy returned when information is retrieved
 */
typedef enum {
	srv_lora_tx_strategy_network,				//!< Follow network ADR
	srv_lora_tx_strategy_custom					//!< Custom strategy
} srv_lora_tx_strategy_t;

/*!
 * \enum srv_lora_join_status_t
 *
 * \brief LoRa network connectivity status
 */
typedef enum {
	srv_lora_join_status_no = 0,			//!< Network not joined
	srv_lora_join_status_yes,				//!< Network joined
	srv_lora_join_status_in_progress		//!< Join in progress
} srv_lora_join_status_t;

/*!
 * \enum srv_lora_stack_state_t
 *
 * \brief State of the MAC layer (Semtech LoRa Basic Modem)
 */
typedef enum {
	srv_lora_stack_state_idle,		//!< Do nothing (sleeping)
	srv_lora_stack_state_busy, 		//!< A process is currently running in the stack
	srv_lora_stack_state_tx_wait	//!< The stack is currently waiting before sending a new uplink (for nb trans or
} srv_lora_stack_state_t;

/*!
 * \struct srv_lora_info_t
 *
 * \brief LoRa information
 */
typedef struct {
	struct {
		uint8_t major;     //!< Major value
		uint8_t minor;     //!< Minor value
		uint8_t patch;     //!< Patch value
		uint8_t revision;  //!< Revision value
	} lora_version;			//!< Supported version of the LoRa Standard
	struct {
		uint8_t major;     //!< Major value
		uint8_t minor;     //!< Minor value
		uint8_t patch;     //!< Patch value
		uint8_t revision;  //!< Revision value
	} lora_regional_version;	//!< Supported version of the LoRa regional parameters.
	struct {
		uint8_t major;  //!< Major value
		uint8_t minor;  //!< Minor value
		uint8_t patch;  //!< Patch value
	} modem_version;	//!< Semtech LoRa Basic Modem version
	struct {
		uint8_t hw;		//!< Hardware identifier
		uint8_t type;	//!< Hardware type
		uint16_t fw;	//!< Firmware version
	} chip_version;		//!< Information related to the hardware component
	uint8_t joineui[SRV_LORA_JOINEUI_SIZE]; //!< join unique identifier
	uint8_t deveui[SRV_LORA_DEVEUI_SIZE];	//!< Device unique identifier
	uint16_t devnonce;						//!< Last devnonce used for a successful join.
	uint32_t devaddr;						//!< Device address (provided by the network)
	const char* region_name;				//!< Region name in string format
	srv_lora_tx_strategy_t tx_strategy;		//!< transmit strategy
	uint32_t consumption_mah; 				//!< Consumption in mAh.
	srv_lora_stack_state_t stack_state;		//!< State of the stack
	srv_lora_join_status_t join_state;		//!< LoRa network access state
	bool tx_busy;							//!< True if a TX request in in progress
	uint32_t duty_cycle_status_ms; 			//!< Duty cycle in ms. It is the time to wait before the next TX
	uint32_t frame_up_cnt;					//!< Frame uplink counter
	uint32_t frame_dn_cnt;					//!< Frame downlink counter
	uint8_t cf_list[16];					//!< CFList (channel frequency list) as defined by the LoRaWAN 1.0.4 regional specification
	uint32_t next_join_time;				//!< Time in seconds to wait before sending the next join
} srv_lora_info_t;

/*!
 * \brief   User callback function definition
 *
 * \param   context User context. Opaque for the service
 * \param   event Event passed to the user
 * \param	rx_data RX data. Valid only if srv_lora_user_event_rx
 * \note User function called when events are available
 */
typedef void (* srv_lora_user_callback_t)(void* context, srv_lora_user_event_t event, aos_lr1110_mgr_info_lora_t* rx_data);

/*!
 * \brief   Service initialization function
 *
 * \param   callback User callback function
 * \param	context User context that will be provided along to the callback function
 * \return  The status of the operation
 * \note Should be called at the start time if you wish to use this service.
 */
aos_lr1110_mgr_status_t srv_lora_init(srv_lora_user_callback_t callback, void* context);

/*!
 * \brief   Join the LoRa network
 *
 * \param	dr Data rate distribution at which the join should be done
 * \return  The status of the operation
 * \note AOS does not support the LoRa ABP mode.
 */
aos_lr1110_mgr_status_t srv_lora_join(const aos_lr1110_mgr_lora_dr_t dr[AOS_LR1110_LORA_MAX_DR_DISTRIBUTION]);

/*!
 * \brief   Leave the LoRa network
 *
 * \return  The status of the operation
 * \note AOS does not support the LoRa ABP mode.
 */
aos_lr1110_mgr_status_t srv_lora_leave(void);

/*!
 * \brief   Set the LoRa DevNonce value
 *
 * \param	devnonce LoRa DevNonce value
 * \return  The status of the operation
 * \note This value is permanently stored.
 * \warning Setting an incorrect value may prevent a successful join.
 */
aos_lr1110_mgr_status_t srv_lora_set_devnonce(uint16_t devnonce);

/*!
 * \brief   Send a LoRa payload in class A
 *
 * \param dr Data rate at which the payload should be sent
 * \param flags LoRa flags. Refer to AOS_LR1110_LORA_FLAG_xxx
 * \param port LoRa Uplink port number
 * \param data_len Payload size
 * \param data Actual payload
 * \return  The status of the operation
 * \note The payload will be scheduled for transmission.
 */
aos_lr1110_mgr_status_t srv_lora_tx(aos_lr1110_mgr_lora_dr_t dr, uint8_t flags, uint8_t port, uint8_t data_len, uint8_t* data);

/*!
 * \brief   Send an empty LoRa payload in class A
 *
 * \param dr Data rate at which the payload should be sent
 * \return  The status of the operation
 * \note An empty payload is usually sent to trigger a downlink. Used when a downlink indicates that there is a frame pending and no application uplink needs to be sent,
 */
aos_lr1110_mgr_status_t srv_lora_tx_empty(aos_lr1110_mgr_lora_dr_t dr);

/*!
 * \brief   Retrieve LoRa information
 *
 * \param	info Storage area provided by the caller. The service will feed it.
 * \return  None
 */
void srv_lora_get_info(srv_lora_info_t* info);

/*!
 * \brief   Send a device time request to LoRa to update the system time
 *
 * \return  The status of the operation
 * \note The request status is received via the usual callback.
 */
aos_lr1110_mgr_status_t srv_lora_request_time(void);

/*!
 * \brief   Send a link check request
 *
 * \return  The status of the operation
 * \note The request status is received via the usual callback.
 */
aos_lr1110_mgr_status_t srv_lora_link_check(void);

/*!
 * \brief  Return the delay before being allowed to send.
 *
 * \return  The delay in ms
 * \note  The delay is expressed in ms and is due to the LoRa duty-cycle.
 */
uint32_t srv_lora_get_net_tx_time(void);

/*!
 * \brief   Return the LoRa accumulated consumption (RX +TX)
 *
 * \return  The consumption in mAh
 */
uint32_t srv_lora_get_consumption_mah(void);

/*!
 * \brief   Clear the LoRa consumption
 */
void srv_lora_clear_consumption(void);

/*! @}*/
#ifdef __cplusplus
}
#endif


