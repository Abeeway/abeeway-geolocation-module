/*!
 * \file aos_rf_switch.h
 * \brief RF switch driver.
 *
 * \details
 * Manage the following RF switches:
 *   BLE - WIFI
 *   LR-GNSS - MT-GNSS *
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "aos_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_rf_switch RF switch driver
 *
 * \addtogroup aos_rf_switch
 * @{
 */

/*!
 * \enum aos_rf_switch_type_t
 * \brief RF switch types
 */
typedef enum {
	aos_rf_switch_type_ble_wifi,		//!< BLE/WIFI shared antenna
	aos_rf_switch_type_gnss,			//!< LR1110 AGPS/MT3333 shared GNSS antenna
	aos_rf_switch_type_last				//!< Last entry in the enumerated
}aos_rf_switch_type_t;

/*!
 * \enum aos_rf_switch_owner_t
 * \brief Define who owns the antenna
 */
typedef enum {
	aos_rf_switch_owner_none = 0,		//!< Nobody own the switch. Free to grant
	aos_rf_switch_owner_1,				//!< The first member owns the antenna. Associated to GPIO1
	aos_rf_switch_owner_2,				//!< The second member owns the antenna Associated to GPIO2
} aos_rf_switch_owner_t;

/*!
 * \def RF_SWITCH_OWNER_BLE
 * \brief Helper defining the first member of the WIFI/BLE RF switch
 */
#define RF_SWITCH_OWNER_BLE		aos_rf_switch_owner_1
/*!
 * \def RF_SWITCH_OWNER_WIFI
 * \brief Helper defining the second member of the WIFI/BLE
 */
#define RF_SWITCH_OWNER_WIFI	aos_rf_switch_owner_2

/*!
 * \def RF_SWITCH_OWNER_GNSS_MT
 * \brief Helper defining the first member of the GNSS LR/MT RF switch
 */
#define RF_SWITCH_OWNER_GNSS_MT		aos_rf_switch_owner_1

/*!
 * \def RF_SWITCH_OWNER_GNSS_LR
 * \brief Helper defining the second member of the GNSS LR/MT RF switch
 */
#define RF_SWITCH_OWNER_GNSS_LR		aos_rf_switch_owner_2

/*!
 * \fn aos_result_t aos_rf_switch_init(aos_rf_switch_type_t type)
 *
 * \brief Initialize a single RF switch
 *
 * \param type RF switch we want to initialize
 *
 * \return Result of the operation
 */
aos_result_t aos_rf_switch_init(aos_rf_switch_type_t type);

/*!
 * \fn aos_result_t aos_rf_switch_acquire_antenna(aos_rf_switch_type_t type, aos_rf_switch_owner_t requester)
 *
 * \brief Acquire a given antenna for a given user
 *
 * \param type RF switch we want to address
 * \param requester Antenna requester that wishes to own the antenna. Use the correct
 * definitions
 *
 * \return Result of the operation
 *
 * \note The function sets up the RF switch and powers up an eventual LNA as needed.
 */
aos_result_t aos_rf_switch_acquire_antenna(aos_rf_switch_type_t type, aos_rf_switch_owner_t requester);

/*!
 * \fn aos_result_t aos_rf_switch_release_antenna(aos_rf_switch_type_t type, aos_rf_switch_owner_t owner)
 *
 * \brief Release the given antenna by the antenna owner
 *
 * \param type RF switch we want to address
 * \param owner Antenna owner that wishes to release the antenna
 *
 * \return Result of the operation
 *
 * \note The function sets up the RF switch and powers down an eventual LNA as needed.
 */
aos_result_t aos_rf_switch_release_antenna(aos_rf_switch_type_t type, aos_rf_switch_owner_t owner);


/*!
 * \fn aos_rf_switch_owner_t aos_rf_switch_get_owner(aos_rf_switch_type_t type)
 *
 * \brief Retrieve the switch owner
 *
 * \param type RF switch we want the owner
 *
 * \return The owner of the switch
 *
 */
aos_rf_switch_owner_t aos_rf_switch_get_owner(aos_rf_switch_type_t type);

/*! @}*/
#ifdef __cplusplus
}
#endif
