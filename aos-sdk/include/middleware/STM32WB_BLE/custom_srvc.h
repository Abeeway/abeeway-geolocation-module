/*!
 * \file custom_srvc.h
 *
 * \brief BLE Custom Service Manager
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
 * \defgroup custom_srvc BLE custom service management
 *
 * \addtogroup custom_srvc
 * @{
 */


/*!
 * \fn tBleStatus custom_srvc_init(ble_custom_srvc_init_data_t *data)
 *
 * \brief Custom service Initialization
 *
 * \param data service and characteristics data
 *
 * \return tBleStatus status
 */
tBleStatus custom_srvc_init(ble_custom_srvc_init_data_t *data);

/*!
 * \fn tBleStatus custom_srvc_update_char(ble_char_update_data_t *data)
 *
 * \brief Update the data of the custom characteristic
 *
 * \param data service and characteristic id and data to update
 *
 * \return tBleStatus status
 */
tBleStatus custom_srvc_update_char(ble_char_update_data_t *data);

/*!
 * \fn tBleStatus custom_srvc_init_all(aos_ble_app_data_t *app_info)
 *
 * \brief Initialize custom services if enabled in ble_srvc_mask
 *
 * \param app_info app_data informations (include bit mask for enabled services)
 *
 * \return tBleStatus status
 */
tBleStatus custom_srvc_init_all(aos_ble_app_data_t *app_info);



/*! @}*/
#ifdef __cplusplus
}
#endif
