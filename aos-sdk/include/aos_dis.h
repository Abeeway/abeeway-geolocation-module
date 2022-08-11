
/*!
 * \file aos_dis.h
 *
 * \brief Device Information Service
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup aos_dis Command Line Interface service (CLI)
 *
 * \addtogroup aos_dis
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include "stdint.h"
#include "aos_common.h"

/* Exported macros -----------------------------------------------------------*/
#define DISAPP_MANUFACTURER_NAME              "MURATA"    //!< Device Manufacturer name
#define DISAPP_MODEL_NUMBER                   "Module"    //!< Device model name
#define DISAPP_HARDWARE_REVISION_NUMBER       "1.0"       //!< Device hardware revision number
#define DISAPP_FIRMWARE_REVISION_NUMBER       "1.0"       //!< Device BLE firmware version
#define DISAPP_SOFTWARE_REVISION_NUMBER       "1.0"       //!< Device application firmware version
#define DISAPP_OUI                            0x123456    //!< Device system ID information part 1
#define DISAPP_MANUFACTURER_ID                0x9ABCDE    //!< Device system ID information part 2

/* Exported functions ------------------------------------------------------- */

/*!
 * \fn void DISAPP_Init(void)
 *
 * \brief Initialize DIS service
 *
 */
void DISAPP_Init(void);

/*!
 * \fn void aos_dis_set_dev_eui(uint8_t value[AOS_PROVISIONING_EUI_SIZE])
 *
 * \brief Set the device EUI needed for SERIAL_NUMBER_UUID characteristic
 *
 * \param value The device EUI
 *
 */
void aos_dis_set_dev_eui(uint8_t value[AOS_PROVISIONING_EUI_SIZE]);


/*! @}*/
#ifdef __cplusplus
}
#endif
