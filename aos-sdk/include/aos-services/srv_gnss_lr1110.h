/*!
 * \file srv_gnss_lr1110.h
 *
 * \brief GNSS LR1110 service
 *
 * \details This service relies on the LR1110 manager. It sits on to of it.
 * 			Events are sent (via callback) to the service's user.

 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup srv_gnss_lr1110 LR1110 GNSS service
 *
 * \addtogroup srv_gnss_lr1110
 * @{
 *
 */

#include "aos_lr1110_gnss.h"
#include "aos_lr1110_mgr.h"


/*!
 * \brief   User callback function definition
 * \details User function called when scan events are available
 * \param   context User context. Opaque for the service
 * \param   event Event passed to the user
 * \return  None
 */
typedef void (* srv_gnss_lr1110_scan_user_callback_t)(void* context, aos_lr1110_gnss_result_t* event);

/*!
 * \brief   User callback function definition
 * \details User function called when scan events are available
 * \param   context User context. Opaque for the service
 * \param   event Event passed to the user
 * \return  None
 */
typedef void (* srv_gnss_lr1110_query_user_callback_t)(void* context, const aos_lr1110_gnss_query_result_t* event);

/*!
 * \brief   Service initialization function
 * \details Should be called at the start time if you wish to use this service
 *
 * \return  The status of the operation
 */
aos_lr1110_mgr_status_t srv_gnss_lr1110_init();

/*!
 * \brief   Start a GNSS acquisition
 * \details Start the LR1110 GNSS scanner
 *
 * \param   callback user callback function
 * \param	user_arg user context that will be provided along to the callback function
 * \param	settings GNSS scan settings
 *
 * \return  The status of the operation
 *
 * \note The WIFI client must be registered
 */
aos_lr1110_mgr_status_t srv_gnss_lr1110_start(srv_gnss_lr1110_scan_user_callback_t callback, void* user_arg, aos_lr1110_gnss_settings_t* settings);

/*!
 * \brief Stop a GNSS scan
 *
 * \details. If a scan was actually in progress, it is aborted and the user callback
 * will be triggered with an abort status. Otherwise the user callback is not triggered,
 * \return The operation result:
 * aos_lr1110_mgr_status_success if the scan can be aborted,
 * aos_lr1110_mgr_status_error_other otherwise
 */
aos_lr1110_mgr_status_t srv_gnss_lr1110_stop(void);

/*!
 * \brief Do a GNSS query
 *
 * \details Useful to read/write the Almanac
 *
 * \param   callback user callback function
 * \param	user_arg user context that will be provided along to the callback function
 * \param	query GNSS query parameters
 *
 * \return The operation result
 */
aos_lr1110_mgr_status_t srv_gnss_lr1110_query(srv_gnss_lr1110_query_user_callback_t callback, void* user_arg, aos_lr1110_gnss_query_request_t* query);

/*!
 * \brief Convert the status into a display'able ASCII string
 *
 * \param   status GNSS scan status
 *
 * \return The ASCII string
 */
const char* srv_gnss_lr1110_scan_status_to_str(aos_lr1110_gnss_status_t status);

/*!
 * \brief   Return the GNSS consumption in uAh
 *
 * \return  The accumulated consumption
 *
 * \note The LR1110 consumption related to the queries is not counted (negligible).
 */
uint64_t srv_gnss_lr1110_get_consumption_uah(void);

/*!
 * \brief  Clear the GNSS consumption
 *
 */
void srv_gnss_lr1110_clear_consumption(void);

/*! @}*/
#ifdef __cplusplus
}
#endif


