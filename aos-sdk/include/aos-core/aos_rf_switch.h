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
 * \enum aos_rf_switch_side_t
 * \brief Define the side of the RF switch
 */
typedef enum {
	aos_rf_switch_side_none = 0,		//!< Nobody own the switch. Free to grant
	aos_rf_switch_side_1,				//!< The RF switch side is associated to GPIO1
	aos_rf_switch_side_2,				//!< The RF switch side is associated to GPIO2
} aos_rf_switch_side_t;

/*!
 * \brief User priority
 */
typedef enum {
	aos_rf_switch_user_priority_no_peempt = 0,	//!< This user uses the entry and cannot be preempted
	aos_rf_switch_user_priority_preemptable,	//!< This user uses the entry and can be preempted
} aos_rf_switch_user_priority_t;

/*!
 * \brief Event sent to the user
 */
typedef enum {
	aos_rf_switch_user_event_acquired = 0,	//!< The user has acquired the antenna
	aos_rf_switch_user_event_waiting_acq,	//!< The user has to wait for acquiring the antenna
	aos_rf_switch_user_event_suspended,		//!< The user has been suspended (no more the antenna)
	aos_rf_switch_user_event_released,		//!< The user has released the antenna
} aos_rf_switch_user_event_t;

/*!
 * \brief User identifier
 */
typedef enum {
	aos_rf_switch_user_id_1 = 0,			//!< RF switch user ID 1
	aos_rf_switch_user_id_2,				//!< RF switch user ID 2
	aos_rf_switch_user_id_3,				//!< RF switch user ID 3
	aos_rf_switch_user_id_4,				//!< RF switch user ID 4
	aos_rf_switch_user_id_count,			//!< Max RF switch user identifier
	aos_rf_switch_user_id_none = aos_rf_switch_user_id_count,	//!< No RF switch user
} aos_rf_switch_user_id_t;

/*!
 * \brief User state
 */
typedef enum {
	aos_rf_switch_user_state_idle,		//!< The user does not use the antenna
	aos_rf_switch_user_state_wait_acq,	//!< The user is waiting for the antenna acquisition
	aos_rf_switch_user_state_suspended,	//!< The user has no more the antenna.
	aos_rf_switch_user_state_active,	//!< The user can use the antenna.
} aos_rf_switch_user_state_t;

/*!
 * \brief User information
 */
typedef struct {
	aos_rf_switch_user_id_t id;			//!< Identifier for this user
	aos_rf_switch_side_t side;			//!< Indicate which side of the RF switch this user is using
	aos_rf_switch_user_state_t state;	//!< State of the user
} aos_rf_switch_user_info_t;

/*!
 * \brief Information related to a RF switch
 */
typedef struct {
	aos_rf_switch_side_t side;										//!< Side of the switch being used
	aos_rf_switch_user_info_t users[aos_rf_switch_user_id_count];	//!< Users for the switch
} aos_rf_switch_info_t;

/*!
 * \brief User callback
 *
 * \param user_arg User argument. Opaque for the driver
 * \param id User identifier
 * \param event RF switch Event
 */
typedef void (aos_rf_switch_user_cb_t)(void* user_arg, aos_rf_switch_user_id_t id, aos_rf_switch_user_event_t event);


/*!
 * \def RF_SWITCH_SIDE_BLE
 * \brief Helper defining the first member of the WIFI/BLE RF switch
 */
#define RF_SWITCH_SIDE_BLE		aos_rf_switch_side_1

/*!
 * \def RF_SWITCH_SIDE_WIFI
 * \brief Helper defining the second member of the WIFI/BLE
 */
#define RF_SWITCH_SIDE_WIFI	aos_rf_switch_side_2

/*!
 * \def RF_SWITCH_SIDE_GNSS_MT
 * \brief Helper defining the first member of the GNSS LR/MT RF switch
 */
#define RF_SWITCH_SIDE_GNSS_MT		aos_rf_switch_side_1

/*!
 * \def RF_SWITCH_SIDE_GNSS_LR
 * \brief Helper defining the second member of the GNSS LR/MT RF switch
 */
#define RF_SWITCH_SIDE_GNSS_LR		aos_rf_switch_side_2


/*
 * \brief User definition for the aos_rf_switch_type_ble_wifi RF switch
 */
#define RF_SWITCH_USER_WIFI			aos_rf_switch_user_id_1		//!< WIFI user
#define RF_SWITCH_USER_BLE_SCAN		aos_rf_switch_user_id_2		//!< BLE scan user
#define RF_SWITCH_USER_BLE_BEACON	aos_rf_switch_user_id_3		//!< BLE beaconing user
#define RF_SWITCH_USER_BLE_CNX		aos_rf_switch_user_id_4		//!< BLE connectivity user


/*
 * \brief User definition for the aos_rf_switch_type_gnss switch
 */
#define RF_SWITCH_USER_GNSS_MT		aos_rf_switch_user_id_1		//!< GNSS MT3333
#define RF_SWITCH_USER_GNSS_LR		aos_rf_switch_user_id_2		//!< GNSS LR1110


/*!
 * \brief Initialize a single RF switch
 *
 * \return Result of the operation
 *
 * \note Called by the system
 */
aos_result_t aos_rf_switch_init(void);


/*!
 * \brief Register a RF switch user
 *
 * \param type RF switch we want to initialize
 * \param user_id User identifier. Refer to the macro helpers
 * \param side RF side the user wants to use. Refer to the macro helpers
 * \param prio Priority for this user
 * \param callback User callback used to received events. Cannot be NULL
 * \param user_arg Opaque pointer passed along to the callback
 *
 * \return Result of the operation
 */
aos_result_t aos_rf_switch_register_client(aos_rf_switch_type_t type,
		aos_rf_switch_user_id_t user_id,
		aos_rf_switch_side_t side,
		aos_rf_switch_user_priority_t prio,
		aos_rf_switch_user_cb_t* callback,
		void* user_arg);

/*!
 * \brief Unregister a RF switch user
 *
 * \param type RF switch we want to address
 * \param user_id User identifier. Refer to the macro helpers
 *
 * \return Result of the operation
 */
aos_result_t aos_rf_switch_unregister_client(aos_rf_switch_type_t type, aos_rf_switch_user_id_t user_id);


/*!
 * \brief Acquire the antenna
 *
 * \param type RF switch we want to address
 * \param user_id User identifier. Refer to the macro helpers
 *
 * \return Result of the operation
 *
 * \note The function sets up the RF switch and powers up the LNA if needed.
 */
aos_result_t aos_rf_switch_acquire_antenna(aos_rf_switch_type_t type, aos_rf_switch_user_id_t user_id);

/*!
 * \brief Release the given antenna by the antenna owner
 *
 * \param type RF switch we want to address
 * \param user_id User identifier. Refer to the macro helpers
 *
 * \return Result of the operation
 *
 * \note The function sets up the RF switch and powers down the LNA if needed.
 */
aos_result_t aos_rf_switch_release_antenna(aos_rf_switch_type_t type, aos_rf_switch_user_id_t user_id);

/*!
 * \brief Retrieve the current active side of the switch
 *
 * \param type RF switch we want to address
 *
 * \return The active side of the switch
 *
 */
aos_rf_switch_side_t aos_rf_switch_get_active_side(aos_rf_switch_type_t type);

/*!
 * \brief Convert the event to a displayable ASCII string
 * \param event User event to convert
 *
 * \return The associated string
 *
 */
const char* aos_rf_switch_event_to_str(aos_rf_switch_user_event_t event);

/*!
 * \brief Convert the state to a displayable ASCII string
 * \param state State to convert
 *
 * \return The associated string
 */
const char* aos_rf_switch_user_state_to_str(aos_rf_switch_user_state_t state);

/*!
 * \brief Retrieve the information of a switch
 * \param type RF switch we want to address
 * \param info Storage area where to place the result.
 *
 * \return The result of the operation
 */
aos_result_t aos_rf_switch_get_info(aos_rf_switch_type_t type, aos_rf_switch_info_t* info);

/*! @}*/
#ifdef __cplusplus
}
#endif
