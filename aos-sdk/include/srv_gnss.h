/*!
 * \file srv_gnss.h
 *
 * \brief GNSS service
 *
 * The GNSS service  manages the different GPS supported by AOS.
 * An IOCTL API is provided for controlling specific parameters
 *
 */
#pragma once

#include "aos_system.h"
#include "aos_gnss_common.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup gnss_srv GNSS service
 *
 * \addtogroup gnss_srv
 * @{
 */

/*!
 * \enum aos_gnss_type_t
 *
 * \brief Type of GPS driver to be used.
 *
 * \note Only one driver can be used at a time.
 */
typedef enum {
    aos_gnss_type_mt3333,			    //!< Use the Mediatek MT3333
	aos_gnss_type_last					//!< Last driver in the list
} aos_gnss_type_t;

/*!
 * \struct aos_gnss_configuration_t
 *
 * \brief Basic GNSS configuration
 */
typedef struct {
	aos_gnss_cfg_constellation_t constellations;	//!< Constellations to be used
	aos_gnss_callback_t cbfn;						//!< User Callback function
	void* cbarg;									//!< user callback argument
	uint32_t filter_mask;							//!< Type of expected messages. Refer to AOS_GNSS_MSG_MSK_xxx definitions
	uint8_t fix_period;								//!< Define the period at which the chip should reports the fix data.
	uint8_t track_period;							//!< Define the period at which the chip should reports the tracking data.
	aos_gnss_cfg_prn_filter_t prn_filter;			//!< Pseudo range Filters. Required only if AGPS used.
} aos_gnss_configuration_t;


/*!
 * \fn aos_result_t srv_gnss_init(void)
 *
 * \brief Initialize the GNSS service
 *
 * \return Success, failure
 *
 * \note Should called only once at the initialization time.
 */
aos_result_t srv_gnss_init(void);

/*!
 * \fn aos_result_t srv_gnss_open(aos_gnss_type_t gtype, const aos_gnss_configuration_t* config)
 *
 * \brief Open a GNSS device service
 *
 * \param gtype GNSS driver type to be used
 * \param config: Basic configuration
 *
 * \return  Success, failure
 */
aos_result_t srv_gnss_open(aos_gnss_type_t gtype, const aos_gnss_configuration_t* config);

/*!
 * \fn aos_result_t srv_gnss_close(void)
 *
 * \brief Close a GNSS device service
 *
 * \return Success, failure
 *
 * \note The call to this function requires having the service opened.
 *
 */
aos_result_t srv_gnss_close(void);


/*!
 * \fn aos_result_t srv_gnss_send_msg(const uint8_t* msg, uint16_t length, aos_gnss_rqst_type_t mtype)
 *
 * \brief srv_gnss_send_msg() - send a message to the GNSS device
 *
 * \param msg Message to send. Must comply with the GNSS device protocol
 * \param length message length
 * \param mtype Type of the request. Refer the associated enumerated except aos_gnss_rqst_type_query.
 *        If you wish to send a query, use the function aos_gnss_send_query instead.
 *
 * \return   success , failure or tx_busy.
 *
 * \note The call to this function requires having the service opened.
 *
 */
aos_result_t srv_gnss_send_msg(const uint8_t* msg, uint16_t length, aos_gnss_rqst_type_t mtype);


/*!
 * \fn srv_gnss_send_query() - send a query
 *
 * \param guery Query to send
 * \param length Query length
 * \param answer_id Expected identifier for the answer. Must be populated (not null) if the answer is not carried thru an ack
 * \return   success , failure or tx_busy.
 *
 * \note
 *    The NMEA parser is able to process once request at a time. So the user has to wait for the event
 *     aos_gnss_event_req_status before sending another command. If a new command is issued while the previous
 *     one is not complete, the function will return busy.
 *
 */
aos_result_t srv_gnss_send_query(const uint8_t* query, uint16_t length, uint32_t answer_id);


/*!
 * \fn aos_result_t srv_gnss_set_power(aos_gnss_type_t gtype, aos_gnss_power_t power)
 *
 * \brief Set the GNSS power state
 *
 * \param gtype - GNSS driver type
 * \param power - power mode to set.
 *
 * \return Success or failure.
 *
 * \note The call to this function does not require having the service opened.
 *
 */
aos_result_t srv_gnss_set_power(aos_gnss_type_t gtype, aos_gnss_power_t power);

/*!
 * \fn aos_result_t srv_gnss_get_power(aos_gnss_type_t gtype, aos_gnss_power_t* power)
 *
 * \brief Get the GNSS power state
 *
 * \param gtype GNSS driver type
 * \param power Pointer to the returned power value
 *
 * \return Success or failure.
 * \note The call to this function does not require having the service opened.
 *
 */
aos_result_t srv_gnss_get_power(aos_gnss_type_t gtype, aos_gnss_power_t* power);


/*!
 * \fn aos_result_t srv_gnss_ioctl(aos_gnss_type_t gtype, aos_gnss_ioctl_t* req)
 *
 * \brief Send an IO control to the service
 *
 * \param gtype GNSS driver type
 * \param req IO control request. Refer to the associated structure
 *
 * \return Success or failure.
 *
 * \note The call to this function does not require having the service opened.
 */
aos_result_t srv_gnss_ioctl(aos_gnss_type_t gtype, aos_gnss_ioctl_t* req);


/*! @}*/
#ifdef __cplusplus
}
#endif


