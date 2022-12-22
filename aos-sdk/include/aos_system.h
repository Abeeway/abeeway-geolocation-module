/*!
 * \file aos_system.h
 *
 * \brief System management and ISR to background process deferring.
 *
 * \details The ISR to background process deferring consists of deferring the ISR processing
 * in a usual RTOS thread. The system thread used for this purpose has the highest priority (7).
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "aos_common.h"


/*!
 * \defgroup aos_system System facility
 *
 * \addtogroup aos_system
 * @{
 */



/*!
 * \def AOS_SYSTEM_MAX_USER_CALLBACK_ENTRY
 *
 * \brief Maximum number of user than can be registered for ISR to background process deferring
 */
#define AOS_SYSTEM_MAX_USER_CALLBACK_ENTRY 32

/*!
 * \enum aos_system_isr_to_bkg_users_t
 *
 * \brief Users that can be reserved for ISR -> background process deferring
 */
typedef enum {
	aos_system_isr_to_bkg_lpuart1,			//!< Reserved for the LPUART1
	aos_system_isr_to_bkg_usart1,			//!< Reserved for the USART1
	aos_system_isr_to_bkg_usb_cdc,			//!< Reserved for the USB CDC
	aos_system_isr_to_bkg_radio,			//!< Reserved for the radio driver (XXX remove?)
	aos_system_isr_to_bkg_lorawan,			//!< Reserved for LoRaWAN MAC (XXX remove?)
	aos_system_isr_to_bkg_mac_timer,		//!< Reserved for MAC timers (XXX replaces radio and lorawan timers?)
	aos_system_isr_to_bkg_first_free, 		//!< First free for user purpose
	aos_system_isr_to_bkg_last_free = AOS_SYSTEM_MAX_USER_CALLBACK_ENTRY -1, //!< Last free value
} aos_system_isr_to_bkg_users_t;



/*!
 * \fn void (*aos_system_user_callback_t)(uint8_t user_id, void* arg)
 *
 * \brief User callback function definition for ISR to background process deferring
 *
 * \param user_id User identifier
 * \param arg User argument
 */
typedef void (*aos_system_user_callback_t)(uint8_t user_id, void* arg);


/*!
 * \fn aos_result_t aos_system_init(void)
 * \brief Initialize the system.
 * This function initializes the AOS system. It also create the system thread.
 *
 * \return the success/error
 */
aos_result_t aos_system_init(void);


/*!
 * \fn aos_result_t aos_system_register_user_callback(uint8_t* user_id, aos_system_user_callback_t callback, void* user_arg)
 *
 * \brief Register a user callback.
 * Allocate a user identifier and register the callback to run under the system thread. This is mainly used to move
 * from interruption context to task context.
 * The system retrieves a free user ID and returns it via the user_id parameter
 *
 * \param user_id output value containing the allocated user identifier.
 * \param callback User callback
 * \param user_arg User argument (usually the context of the user).
 *
 * \return the success/error
 */
aos_result_t aos_system_register_user_callback(uint8_t* user_id, aos_system_user_callback_t callback, void* user_arg);

/*!
 * \fn aos_result_t aos_system_register_callback(uint8_t user_id, aos_system_user_callback_t callback, void* user_arg)
 *
 * \brief Register a user callback.
 * Register a user callback with a known user identifier. to run under the system thread. This is mainly used to move
 * from interruption context to task context.
 * This function is generally used by the system itself and used the reserved user identifier.
 *
 * \param user_id User identifier
 * \param callback User callback
 * \param user_arg User argument (usually the context of the user).
 *
 * \return the success/error
 */
aos_result_t aos_system_register_callback(uint8_t user_id, aos_system_user_callback_t callback, void* user_arg);

/*!
 * \fn aos_result_t aos_system_unregister_callback(uint8_t user_id)
 *
 * \brief Unregister a user callback
 *
 * \param user_id User identifier
 *
 * \return the success/error
 */
aos_result_t aos_system_unregister_callback(uint8_t user_id);

/*!
 * \fn aos_result_t aos_system_trigger_user_callback(uint8_t user_id)
 *
 * \brief Trigger the system thread for a given user
 *
 * \param user_id User identifier
 *
 * \return the success/error
 *
 * \note  This function is usually called under interruption to trigger the system thread.
 * Once the system thread executes, the user callback will be called.
 */
aos_result_t aos_system_trigger_user_callback(uint8_t user_id);

/*!
 * \fn aos_result_t aos_system_read_cpu_temperature(int16_t* temperature)
 *
 * \brief Read the CPU temperature
 *
 *\param temperature Output value containing the temperature level. The value is in degree.
 *
 * \return the success/error
 */
aos_result_t aos_system_read_cpu_temperature(int16_t* temperature);

/*!
 * \fn aos_result_t aos_system_clock_resume(void)
 *
 * \brief Restart the main clocks after wake up.
 *
 * \return the success/error
 *
 * \warning Intended to be used internally only.
 */
aos_result_t aos_system_clock_resume(void);

/*! @}*/
#ifdef __cplusplus
}
#endif
