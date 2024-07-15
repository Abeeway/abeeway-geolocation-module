/*!
 * \file srv_wifi_scan.h
 *
 * \brief WIFI scan service
 *
 * \details This service relies on the LR1110 manager. It sits on to of it.
 * 			Events are sent (via callback) to the service's user.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "aos_lr1110_mgr.h"
#include "aos_wifi.h"


/*!
 * \defgroup srv_wifi WIFI scan service
 *
 * \addtogroup srv_wifi
 * @{
 */

/*!
 * \brief   User callback function definition
 * \param   context User context. Opaque for the service
 * \param   event Event passed to the user
 * \note    User function called when events are available
 */
typedef void (* srv_wifi_scan_user_callback_t)(void* context, aos_lr1110_mgr_client_event_t* event);

/*!
 * \brief   Service initialization function
 *
 * \return  The status of the operation
 * \note Should be called at the start time if you wish to use this service
 */
aos_lr1110_mgr_status_t srv_wifi_scan_init(void);

/*!
 * \brief   Start a WIFI scan
 *
 * \param   callback user callback function
 * \param	context user context that will be provided along to the callback function
 * \param	settings WIFI scan configuration
 *
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t srv_wifi_scan_start(srv_wifi_scan_user_callback_t callback, void* context, aos_wifi_settings_t* settings);

/*!
 * \brief   Abort a WIFI scan
 *
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t srv_wifi_scan_stop(void);

/*!
 * \brief   Return the WIFI consumption in uAh
 *
 * \return  The accumulated consumption
 */
uint64_t srv_wifi_get_consumption_uah(void);

/*!
 * \brief  Clear the WIFI consumption
 *
 */
void srv_wifi_clear_consumption(void);

/*! @}*/
#ifdef __cplusplus
}
#endif


