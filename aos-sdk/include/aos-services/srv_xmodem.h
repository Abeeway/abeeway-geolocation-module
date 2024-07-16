/*!
 * \file srv_xmodem.h
 *
 * \brief The xmodem service manages file transfer (reception only) using the Xmodem protocol
 *
 * \copyright 2023, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \defgroup xmodem_srv xModem service
 *
 * \addtogroup xmodem_srv
 * @{
 */

#define XMODEM_DATA_SIZE   128        //!< Data size of a chunk sent by xmodem


/*!
 * \enum srv_xmodem_operation_t
 *
 * \brief operation to send to the application
 */
typedef enum {
	srv_xmodem_operation_write,       //!< Write command, data is sent to the app to be processed
	srv_xmodem_operation_end,         //!< xmodem transfer ended
	srv_xmodem_operation_error        //!< xmodem transfer error
} srv_xmodem_operation_t;

/*!
 * \enum srv_xmodem_app_result_t
 *
 * \brief Application processing result
 */
typedef enum {
	srv_xmodem_app_result_ok,         //!< Application processing success
	srv_xmodem_app_result_wait,       //!< Application processing waited
	srv_xmodem_app_result_error       //!< Application processing error returned
} srv_xmodem_app_result_t;

/*!
 * \brief Application user callback
 *
 * \param operation operation to send to be managed by the application.
 * \param offset file offset.
 * \param buffer Buffer where the bytes to be sent reside.
 * \param length Number of bytes to transmit.
 *
 * \return The result of the application processing
 */
typedef srv_xmodem_app_result_t (*srv_xmodem_user_cb_t)(srv_xmodem_operation_t operation, const uint32_t offset, uint32_t* buffer, const uint8_t length);

/*!
 * \brief Initialize xmodem and wait for firmware transfer
 *
 * \param uart_type Serial port on which the firmware will be transferred.
 * \param speed baudrate speed to be applied.
 * \param user_cb user callback function.
 *
 * \return The result of the operation
 */
aos_result_t srv_xmodem_start(aos_uart_type_t uart_type, aos_uart_speed_t speed, srv_xmodem_user_cb_t user_cb);

/*!
 * \brief Close xmodem for firmware transfer
 *
 * \return The result of the operation
 */
aos_result_t srv_xmodem_close(void);

/*!
 * \brief Acknowledge last chunk received and continue reception
 */
void srv_xmodem_continue(void);

/*! @}*/
#ifdef __cplusplus
}
#endif
