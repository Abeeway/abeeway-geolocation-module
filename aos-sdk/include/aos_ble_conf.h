/*!
 * \file aos_ble_conf.h
 *
 * \brief Configuration file for BLE Middleware
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once

#include "app_conf.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup aos_ble_conf BLE Middleware configuration
 *
 * \addtogroup aos_ble_conf
 * @{
 */

/******************************************************************************
 *
 * BLE SERVICES CONFIGURATION
 * blesvc
 *
 ******************************************************************************/

 /**
 * This setting shall be set to '1' if the device needs to support the Peripheral Role
 * In the MS configuration, both BLE_CFG_PERIPHERAL and BLE_CFG_CENTRAL shall be set to '1'
 */
#define BLE_CFG_PERIPHERAL                                                     1   //!< Peripheral Role supported

/**
 * This setting shall be set to '1' if the device needs to support the Central Role
 * In the MS configuration, both BLE_CFG_PERIPHERAL and BLE_CFG_CENTRAL shall be set to '1'
 */
#define BLE_CFG_CENTRAL                                                        1   //!< Central Role supported

/**
 * There is one handler per service enabled
 * Note: There is no handler for the Device Information Service
 *
 * This shall take into account all registered handlers
 * (from either the provided services or the custom services)
 */
#define BLE_CFG_SVC_MAX_NBR_CB                                                 7   //!< Max number of services Handler to register

#define BLE_CFG_CLT_MAX_NBR_CB                                                 0   //!< Max number of client Handler to register

/******************************************************************************
 * Device Information Service (DIS)
 ******************************************************************************/
/**< Options: Supported(1) or Not Supported(0) */
#define BLE_CFG_DIS_MANUFACTURER_NAME_STRING                                   1   //!< DIS_MANUFACTURER_NAME_STRING support
#define BLE_CFG_DIS_MODEL_NUMBER_STRING                                        1   //!< DIS_MODEL_NUMBER_STRING support
#define BLE_CFG_DIS_SERIAL_NUMBER_STRING                                       1   //!< DIS_SERIAL_NUMBER_STRING support
#define BLE_CFG_DIS_HARDWARE_REVISION_STRING                                   0   //!< DIS_HARDWARE_REVISION_STRING support
#define BLE_CFG_DIS_FIRMWARE_REVISION_STRING                                   1   //!< DIS_FIRMWARE_REVISION_STRING support
#define BLE_CFG_DIS_SOFTWARE_REVISION_STRING                                   1   //!< DIS_SOFTWARE_REVISION_STRING support
#define BLE_CFG_DIS_SYSTEM_ID                                                  0   //!< DIS_SYSTEM_ID support
#define BLE_CFG_DIS_IEEE_CERTIFICATION                                         0   //!< DIS_IEEE_CERTIFICATION support
#define BLE_CFG_DIS_PNP_ID                                                     0   //!< DIS_PNP_ID support

/**
 * device information service characteristic lengths
 */
#define BLE_CFG_DIS_SYSTEM_ID_LEN_MAX                                        (8)   //!< DIS device system ID max length
#define BLE_CFG_DIS_MODEL_NUMBER_STRING_LEN_MAX                              (32)  //!< DIS model number string max length
#define BLE_CFG_DIS_SERIAL_NUMBER_STRING_LEN_MAX                             (32)  //!< DIS serial number string max length
#define BLE_CFG_DIS_FIRMWARE_REVISION_STRING_LEN_MAX                         (32)  //!< DIS firmware revision string max length
#define BLE_CFG_DIS_HARDWARE_REVISION_STRING_LEN_MAX                         (32)  //!< DIS hardware revision string max length
#define BLE_CFG_DIS_SOFTWARE_REVISION_STRING_LEN_MAX                         (32)  //!< DIS software revision string max length
#define BLE_CFG_DIS_MANUFACTURER_NAME_STRING_LEN_MAX                         (32)  //!< DIS manufacturer name string max length
#define BLE_CFG_DIS_IEEE_CERTIFICATION_LEN_MAX                               (32)  //!< DIS IEEE certification ID information max length
#define BLE_CFG_DIS_PNP_ID_LEN_MAX                                           (7)   //!< DIS PNP ID information max length

/******************************************************************************
 * GAP Service - Appearance
 ******************************************************************************/

#define BLE_CFG_UNKNOWN_APPEARANCE                  (0)                                //!< GAP unknown appearance ID
#define BLE_APPEARANCE_GENERIC_KEYRING              (576)                              //!< GAP generic keyring appearance ID
#define BLE_CFG_GAP_APPEARANCE                      (BLE_APPEARANCE_GENERIC_KEYRING)   //!< Default GAP appearance

/******************************************************************************
 * Over The Air Feature (OTA) - STM Proprietary
 ******************************************************************************/
#define BLE_CFG_OTA_REBOOT_CHAR         0   //!<  REBOOT OTA MODE CHARACTERISTIC

/*! @}*/
#ifdef __cplusplus
}
#endif
