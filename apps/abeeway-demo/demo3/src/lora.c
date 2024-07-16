/*!
 * \file lora.c
 *
 * \brief Manage LoRa class A
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

#include <string.h>
#include <stdint.h>


#include "aos_log.h"

#include "srv_lora.h"

#define LORA_STATUS(...)	do { aos_log_status(aos_log_module_lora, true, __VA_ARGS__);  } while (0)

// Local context
typedef struct {
	bool tx_allowed;		// True if TX allowed, false otherwise
} lora_ctx_t;

static lora_ctx_t _lora_ctx;


// DR selection for join. The LBM may choose itself the actual DR
static const uint8_t _non_us_join_dr_cycle[AOS_LR1110_LORA_MAX_DR_DISTRIBUTION] = {
		aos_lr1110_mgr_lora_dr0,
		aos_lr1110_mgr_lora_dr1,
		aos_lr1110_mgr_lora_dr2,
		aos_lr1110_mgr_lora_dr0,
		aos_lr1110_mgr_lora_dr1,
		aos_lr1110_mgr_lora_dr2,
		aos_lr1110_mgr_lora_dr0,
		aos_lr1110_mgr_lora_dr1,
		aos_lr1110_mgr_lora_dr2,
		aos_lr1110_mgr_lora_dr0,
		aos_lr1110_mgr_lora_dr1,
		aos_lr1110_mgr_lora_dr2,
		aos_lr1110_mgr_lora_dr0,
		aos_lr1110_mgr_lora_dr1,
		aos_lr1110_mgr_lora_dr2,
		aos_lr1110_mgr_lora_dr0
};

// Convert the eventy type in string
static const char* _event_to_str(srv_lora_user_event_t event)
{
	const char* evt_name[srv_lora_user_event_count] = {
			[srv_lora_user_event_join_success] = "join success",
			[srv_lora_user_event_join_failure] = "join failure",
			[srv_lora_user_event_leave_success] = "leave success",
			[srv_lora_user_event_leave_failure] = "leave failure",
			[srv_lora_user_event_tx_success] = "TX success",
			[srv_lora_user_event_tx_failure] = "TX failure",
			[srv_lora_user_event_tx_request_failure] = "TX request failure",
			[srv_lora_user_event_rx] = "DL available",
			[srv_lora_user_event_time_upd_success] = "RX time update success",
			[srv_lora_user_event_time_upd_failure] = "RX time update failure",
			[srv_lora_user_event_link_check_success] = "Link check success",
			[srv_lora_user_event_link_check_failure] = "Link check failure",
	};

	if (event >= srv_lora_user_event_count) {
		return  "unknown";
	}
	return evt_name[event];
}

// LoRa class A callback
void _lora_user_callback_t(void* context, srv_lora_user_event_t event, aos_lr1110_mgr_info_lora_t* rx_data)
{
	lora_ctx_t* ctx = context;

	LORA_STATUS("Got event: %s\n", _event_to_str(event));

	switch(event) {
	case srv_lora_user_event_join_success:		//!< Join network success
		ctx->tx_allowed = true;					//!< Transmission of the next frame is allowed
		break;

	case srv_lora_user_event_join_failure:		//!< Join network failure
	case srv_lora_user_event_leave_success:		//!< Leave network success
	case srv_lora_user_event_leave_failure:		//!< Leave failure
		ctx->tx_allowed = false;				//!< Transmission of the next frame is allowed
		break;

	case srv_lora_user_event_tx_success:		//!< Actual transmission success
	case srv_lora_user_event_tx_failure:		//!< Actual transmission fails
	case srv_lora_user_event_tx_request_failure:	//!< Request to transmit fails
		ctx->tx_allowed = true;					//!< Transmission of the next frame is allowed
		break;

	case srv_lora_user_event_rx:					//!< Downlink available
	case srv_lora_user_event_rx_pending:			//!< Downlink available with frame pending bit set.
		// Do reception
		LORA_STATUS("RX (Port: %d): ", rx_data->rx_data.port);
		aos_log_dump_hex(aos_log_module_lora, aos_log_level_status, rx_data->rx_data.payload, rx_data->rx_data.payload_size, true);
		break;

	case srv_lora_user_event_time_upd_success:	//!< Time update success. System time updated.
	case srv_lora_user_event_time_upd_failure:	//!< Time update failure
		break;

	case srv_lora_user_event_link_check_success:	//!< Link check success
	case srv_lora_user_event_link_check_failure:	//!< Link check failure
		break;
	case srv_lora_user_event_count:				//!< Number of events
		break;
	}
}


/*!
  * \brief Initialization
  */
void lora_init(void)
{
	memset(&_lora_ctx, 0, sizeof(_lora_ctx));

	if (srv_lora_init(_lora_user_callback_t, &_lora_ctx) == aos_lr1110_mgr_status_success) {
		if (srv_lora_join(_non_us_join_dr_cycle) == aos_lr1110_mgr_status_success) {
			LORA_STATUS("Joining the network\n");
			return;
		}
	}
	LORA_STATUS("Init failure\n");
}


bool lora_send(aos_lr1110_mgr_lora_dr_t dr, uint8_t port, uint8_t data_len, uint8_t*data)
{
	if (!_lora_ctx.tx_allowed) {
		return false;
	}
	LORA_STATUS("TX (Port: %d): ", port);
	aos_log_dump_hex(aos_log_module_lora, aos_log_level_status, data, data_len, true);

	return (srv_lora_tx(dr, 0, port, data_len, data) == aos_lr1110_mgr_status_success);
}
