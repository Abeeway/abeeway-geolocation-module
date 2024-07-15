/*!
 *
 * \file   srv_geoloc_basic.h
 *
 * \brief  Geolocation engine using the fallback method. Geolocation technologies are
 * scheduled and results are kept based on what is configured.
 *
 * Copyright (C) 2023, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "srv_geoloc_common.h"

/*!
 * \defgroup srv_geoloc_basic Geolocation basic engine
 *
 * \addtogroup srv_geoloc_basic
 * @{
 *
 */

/*!
 * \brief Action that can be done on a technology
 *
 *\note It is recommended to have the always_acquire actions at the end of the scheduling list.
 */
typedef enum {
	srv_geoloc_basic_action_none = 0,			//!< No action. Technology unused
	srv_geoloc_basic_action_always_acquire,		//!< Always do the position acquisition regardless previous successes
	srv_geoloc_basic_action_skip_if_success,	//!< If there is a previous success do not schedule this techno
	srv_geoloc_basic_action_count				//!< Number of actions
}srv_geoloc_basic_action_t;

/*!
 * \brief Settings per technology
 */
typedef struct {
	srv_geolocation_type_t type;				//!< Type of geolocation
	srv_geoloc_basic_action_t action;			//!< Action
	srv_geolocation_techno_cfg_t cfg;			//!< Technology configuration
} srv_geoloc_basic_cfg_per_techno_t;

/*!
 * \brief Configuration of the basic geolocation engine
 */
typedef struct {
	uint8_t nb_techno;															//!< Number of technologies in the list
	srv_geoloc_basic_cfg_per_techno_t scheduling[srv_geolocation_type_count];	//!< Scheduling order of the technologies.
} srv_geoloc_basic_configuration_t;

/*!
 * \brief Service initialization
 *
 * \return the status of the operation
 *
 * \note Should be called at the board init
 */
aos_result_t srv_geoloc_basic_init(void);

/*!
 * \brief Start the basic geolocation engine
 *
 * \param user_callback User callback
 * \param user_arg User argument (opaque for the service)
 * \param settings Configuration of this geolocation run.
 *
 * \return the status of the operation
 *
 * \note The user callback is called under the basic geolocation thread
 */
aos_result_t srv_geoloc_basic_start(srv_geolocation_callback_t user_callback, void* user_arg, srv_geoloc_basic_configuration_t* settings);

/*!
 * \brief Abort the geolocation
 *
 * \return the status of the operation
 *
 * \note The user callback is called under the basic geolocation thread
 */
aos_result_t srv_geoloc_basic_abort(void);


/*!
 * \brief Get the geolocation results.
 *
 * \param results Handle where to store the pointer to the results.
 *
 * \return the status of the operation
 */
aos_result_t srv_geoloc_basic_get_results(const srv_geolocation_result_t** results);

/*! @}*/
#ifdef __cplusplus
}
#endif

