/*!
 * \file aos_gnss_driver.h
 *
 * \brief Top level API for the GNSS drivers.
 *
 *
 * Copyright (C) 2022, Abeeway (http://www.abeeway.com/)
 *
 *
 */
#pragma once
#include "aos_gnss_common.h"



#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_gnss_driver GNSS driver
 *
 * \addtogroup aos_gnss_driver
 * @{
 */


/*!
 * \struct aos_gnss_driver_t
 *
 * \brief Driver access definition
 */
typedef struct {
	/*!
	 * \fn void (*init)(void)
	 *
	 * \brief Initialize the driver
	 */
	void (*init)(void);

	/*!
	 * \fn aos_result_t (*open)(aos_gnss_callback_t cbfn, void *cbarg)
	 *
	 * \brief Power on and initialize the GNSS.
	 *
	 * \param type Type of GNSS chip
	 * \param cbfn User callback function to be invoked for GNSS events.
	 * \param cbarg User argument to be passed to the GNSS event function.
	 * \param msg_Mask Default message mask.
	 *
	 * \return The return value is a aos_result_t indicating success or failure.
	 */
	aos_result_t (*open)(aos_gnss_callback_t cbfn, void *cbarg) __attribute__((warn_unused_result));

	/*!
	 * \fn aos_result_t (*close)(void)
	 *
	 * \brief Close the GNSS
	 * This function closes the GNSS driver
	 *
	 * \return The return value is a aos_result_t indicating success or failure.
	 */
	aos_result_t (*close)(void);

	/*!
	 * \fn aos_result_t send_msg(const uint8_t* msg, uint16_t length, aos_gnss_rqst_type_t type, uint32_t answer_id)
	 *
	 * \brief Sends a message to the GNSS chip. It up to the caller to use the correct data.
	 * The NMEA encapsulation will be added by the driver.
	 *
	 * \param msg Message/query to send
	 * \param length Message length
	 * \param type Request type. Refer the enumerated definition.
	 * \param answer_id Expected identifier for the answer. Must be populated (not null) if the answer is not carried thru an ACK
	 *
	 * \return The return value is a aos_result_t indicating success or failure.
	 */
	aos_result_t (*send_msg)(const uint8_t* msg, uint16_t length, aos_gnss_rqst_type_t type, uint32_t answer_id);

	/*!
	 * \fn void (*delayed_rx_process)(void)
	 *
	 * \brief Delayed_RX processing.
	 * This function is called by the GNSS thread for further RX processing
	 *
	 * \warning It should not be used by the application user.
	 */
	void (*delayed_rx_process)(void);

	/*!
	 * \fn aos_result_t (*ioctl)(aos_gnss_ioctl_t *request, aos_gnss_callback_t cbfn, void *cbarg)
	 *
	 * \brief IO request.
	 * This function allows controlling GNSS parameters. Refer to
	 * the aos_gnss_ioctl_req_t enum value for details on the different actions.
	 *
	 * \param request The IOCTL request to be passed to the GNSS driver.
	 * \param cbfn  User callback function to be invoked for GNSS events.
	 * \param cbarg User argument to be passed to the GNSS event function.
	 *
	 *
	 * \return The return value is a aos_result_t indicating success or failure
	 * of the requested action.
	 *
	 * \note The callback function is optional. If not provided and driver open,
	 *then the initial callback function (passed via the open function) is called.
	 *If provided and driver already opened, the initial callback function is
	 *overwritten by the new one.
	 */
	aos_result_t (*ioctl)(aos_gnss_ioctl_t *request, aos_gnss_callback_t cbfn, void *cbarg);

} aos_gnss_driver_t;

/*! @}*/
#ifdef __cplusplus
}
#endif
