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
#define DISAPP_MANUFACTURER_NAME              "ABEEWAY"   //!< Device Manufacturer name
#define DISAPP_MODEL_NUMBER                   "Module"    //!< Device model name
#define DISAPP_HARDWARE_REVISION_NUMBER       "1.0"       //!< Device hardware revision number
#define DISAPP_FIRMWARE_REVISION_NUMBER       "1.0"       //!< Device BLE firmware version
#define DISAPP_OUI                            0x123456    //!< Device system ID information part 1
#define DISAPP_MANUFACTURER_ID                0x9ABCDE    //!< Device system ID information part 2

/* Exported functions ------------------------------------------------------- */

/*!
 * \fn void aos_ble_services_data_init(aos_ble_app_data_t *app_info)
 *
 * \brief Set all characteristics value for all enabled services
 *
 * \param app_info: BLE application information needed to initialization
 *
 */
void aos_ble_services_data_init(aos_ble_app_data_t *app_info);

/*! @}*/
#ifdef __cplusplus
}
#endif
