/*!
 * \file aos_ob.h
 *
 * \brief Flash Option Byte access (Flash Option register)
 *
 * This source component allows to configure the option bytes (OB)
 * to set a specific Brown Out Reset (BOR) level and change the independent
 *  watch-dog timer activation.
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

/*!
 * \enum aos_ob_bor_level_t
 *
 * \brief Brown Out Reset (BOR) levels
 */
typedef enum {
	aos_ob_bor_level_0 = 0,					//!< BOR Level 0. Reset level threshold is ~ 1.7 V
	aos_ob_bor_level_1,						//!< BOR Level 1. Reset level threshold is ~ 2.0 V
	aos_ob_bor_level_2,						//!< BOR Level 2. Reset level threshold is ~ 2.2 V
	aos_ob_bor_level_3,						//!< BOR Level 3. Reset level threshold is ~ 2.5 V
	aos_ob_bor_level_4,						//!< BOR Level 4. Reset level threshold is ~ 2.8 V
	aos_ob_bor_level_count,
} aos_ob_bor_level_t;

/*!
 * \enum aos_iwdg_activation_t
 *
 * \brief Independent watch-dog timer activation
 */
typedef enum {
	aos_iwdg_activation_customer = 0,		//!< Activation for costumer  : enable the HW watch-dog timer
	aos_iwdg_activation_developer = 1,		//!< Activation for developer : enable the SW watch-dog timer
	aos_iwdg_activation_default = 7,		//!< Default activation
	aos_iwdg_activation_count,				//!< WDG activation count
} aos_iwdg_activation_t;

/*!
 * \fn aos_result_t aos_ob_bor_level_set(aos_ob_bor_level_t bor_level)
 *
 * \brief Set the Brown Out Reset (BOR) level
 *
 * \param bor_level BOR level to be configured
 *
 * \return Result of the operation
 */
aos_result_t aos_ob_bor_level_set(aos_ob_bor_level_t bor_level);

/*!
 * \fn aos_result_t aos_ob_apply(void)
 *
 * \brief Apply the current OB configuration
 *
 * \return Result of the operation
 *
 *  * \note, The function will cause the platform to reset
 *
 */
aos_result_t aos_ob_apply(void);

/*!
 * \fn aos_ob_bor_level_t aos_ob_bor_level_get(void)
 *
 * \brief Get the current BOR level
 *
 *
 * \return The BOR current level
 */
aos_ob_bor_level_t aos_ob_bor_level_get(void);


/*!
 * \fn aos_result_t aos_ob_independent_wdg_set_activation(aos_iwdg_activation_t aos_iwdg_activation)
 *
 * \brief Set the independent watch-dog activation
 *
 * \param aos_iwdg_activation activation of independent watch-dog timer : developer or customer
 *
 * \return Result of the operation
 */
aos_result_t aos_ob_independent_wdg_set_activation(aos_iwdg_activation_t aos_iwdg_activation);

/*!
 * \fn aos_ob_independent_wdg_get_activation(void)
 *
 * \brief Get the current independent watch-dog timer activation
 *
 * \return The current independent watch-dog timer activation
 */
aos_iwdg_activation_t aos_ob_independent_wdg_get_activation(void);
