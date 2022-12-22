/*!
 * \file aos_ble_services.h
 *
 * \brief BLE services related API
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once



#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "aos_common.h"
#include "aos_ble_common.h"
/*!
 * \defgroup aos_ble_services BLE core service
 *
 * \addtogroup aos_ble_services
 * @{
 */

/* Exported macros -----------------------------------------------------------*/
#define DISAPP_MANUFACTURER_NAME              "MURATA"    //!< Device Manufacturer name
#define DISAPP_MODEL_NUMBER                   "Module"    //!< Device model name
#define DISAPP_HARDWARE_REVISION_NUMBER       "1.0"       //!< Device hardware revision number
#define DISAPP_FIRMWARE_REVISION_NUMBER       "1.0"       //!< Device BLE firmware version
#define DISAPP_OUI                            0x123456    //!< Device system ID information part 1
#define DISAPP_MANUFACTURER_ID                0x9ABCDE    //!< Device system ID information part 2

/* Exported functions ------------------------------------------------------- */

/*!
 * \fn void dis_char_value_init(void)
 *
 * \brief Initialize Device Information Service
 *
 * \param app_info provide device information data characteristic data
 *
 */
void dis_char_value_init(aos_ble_app_data_t *app_info);

/*!
 * \fn void ias_char_value_init(void)
 *
 * \brief Initialize Immediate Alert Service characteristic data
 *
 */
void ias_char_value_init(void);

/*!
 * \fn void lls_char_value_init(void)
 *
 * \brief Initialize Link Loss Service characteristic data
 *
 */
void lls_char_value_init(void);

/*!
 * \fn void tps_char_value_init(void)
 *
 * \brief Initialize Tx Power Service characteristic data
 *
 * \param tx_power BLE tx power to initialize
 *
 */
void tps_char_value_init(int8_t tx_power);

/*!
 * \fn void bas_char_value_init(void)
 *
 * \brief Initialize Battery Service characteristic data
 *
 */
void bas_char_value_init(void);

/*!
 * \fn void ess_char_value_init(void)
 *
 * \brief Initialize Environmental Sensing Service characteristic data
 *
 */
void ess_char_value_init(void);


/**
 * @brief  Event handler
 * @param  Event: Address of the buffer holding the Event
 * @retval Ack: Return whether the Event has been managed or not
 */
SVCCTL_EvtAckStatus_t custom_event_handler(void *Event);

/*! @}*/
#ifdef __cplusplus
}
#endif
