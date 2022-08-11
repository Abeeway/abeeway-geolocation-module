/*!
 * \file aos_ble_core.h
 *
 * \brief BLE core related API (connectivity, advertisement, scan ...)
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hci_tl.h"
#include "ble_common.h"
#include "ble_types.h"
#include "stdbool.h"


/*!
 * \defgroup aos_ble_core BLE core
 *
 * \addtogroup aos_ble_core
 * @{
 */

#define AOS_BLE_CORE_TX_POWER_LEVEL_MIN 0			//!< Min index of tx power level
#define AOS_BLE_CORE_TX_POWER_LEVEL_MAX 31			//!< Max index of tx power level

//TODO find a better place for this:
#define ABS(x) ((x)>0? (x):-(x))		//!< Macro to get the absolute value of the input parameter

/* Exported types ------------------------------------------------------------*/

/*!
 * \enum APP_BLE_ConnStatus_t
 *
 * \brief BLE connection state.
 */
typedef enum {
	APP_BLE_IDLE,				//!< BLE in idle state
	APP_BLE_FAST_ADV,			//!< BLE in fast advertisement state
	APP_BLE_LP_ADV,				//!< BLE in slow advertisement state
	APP_BLE_SCAN,				//!< BLE in scan state
	APP_BLE_LP_CONNECTING,		//!< BLE is connecting as client
	APP_BLE_CONNECTED_SERVER,	//!< BLE connected as server
	APP_BLE_CONNECTED_CLIENT	//!< BLE connected as client
} APP_BLE_ConnStatus_t;

/*!
 * \typedef void aos_ble_core_scan_callback_t(const Advertising_Report_t *par)
 *
 * \brief BLE scan call back to call when a scan is done.
 *
 * \param par advertisement report data
 */
typedef void aos_ble_core_scan_callback_t(const Advertising_Report_t *par);

/* Exported functions ---------------------------------------------*/

/*!
 * \fn void APP_BLE_Init(uint8_t role)
 *
 * \brief BLE Initialization API
 *
 * \param role the role to be initialized for (peripheral, central, observer or broadcaster)
 *
 */
void APP_BLE_Init(uint8_t role);

/*!
 * \fn APP_BLE_ConnStatus_t APP_BLE_Get_Server_Connection_Status(void);
 *
 * \brief Get BLE connection status
 *
 * \return BLE connection status
 *
 */
APP_BLE_ConnStatus_t APP_BLE_Get_Server_Connection_Status(void);

/*!
 * \fn const uint8_t* BleGetBdAddress( void )
 *
 * \brief Get BLE MAC address
 *
 * \return pointer to the BLE MAC address
 */
const uint8_t* BleGetBdAddress(void);


/*!
 * \fn void aos_ble_core_avertise(APP_BLE_ConnStatus_t New_Status)
 *
 * \brief Start connectivity advertisement
 *
 * \param New_Status Start fast/slow advertisement
 */
void aos_ble_core_avertise(APP_BLE_ConnStatus_t New_Status);

/*!
 * \fn void aos_ble_core_set_scan_callback(aos_ble_core_scan_callback_t* cb)
 *
 * \brief Set the scan callback function, when HCI_LE_ADVERTISING_REPORT_SUBEVT_CODE
 * event occur call this callback to process data
 */
void aos_ble_core_set_scan_callback(aos_ble_core_scan_callback_t *cb);

/*!
 * \fn bool aos_ble_core_set_tx_power_level(uint8_t tx_power_level)
 *
 * \brief Set TX POWER LEVEL,
 *
 * \param tx_power_level: Power amplifier output level. Output power is indicative and
 *        depends on the PCB layout and associated components. Here the values
 *        are given at the STM32WB output.
 *        Values:
 *        - 0x00: -40 dBm	- 0x01: -20.85 dBm	- 0x02: -19.75 dBm	- 0x03: -18.85 dBm	- 0x04: -17.6 dBm
 *        - 0x05: -16.5 dBm	- 0x06: -15.25 dBm	- 0x07: -14.1 dBm	- 0x08: -13.15 dBm	- 0x09: -12.05 dBm
 *        - 0x0A: -10.9 dBm	- 0x0B: -9.9 dBm	- 0x0C: -8.85 dBm	- 0x0D: -7.8 dBm	- 0x0E: -6.9 dBm
 *        - 0x0F: -5.9 dBm	- 0x10: -4.95 dBm	- 0x11: -4 dBm		- 0x12: -3.15 dBm	- 0x13: -2.45 dBm
 *        - 0x14: -1.8 dBm	- 0x15: -1.3 dBm	- 0x16: -0.85 dBm	- 0x17: -0.5 dBm	- 0x18: -0.15 dBm
 *        - 0x19: 0 dBm		- 0x1A: +1 dBm		- 0x1B: +2 dBm		- 0x1C: +3 dBm		- 0x1D: +4 dBm
 *        - 0x1E: +5 dBm	- 0x1F: +6 dBm
 *
 * \return result status true/false
 */
bool aos_ble_core_set_tx_power_level(uint8_t tx_power_level);

/*!
 * \fn uint8_t aos_ble_core_get_tx_power_level(void)
 *
 * \brief Get TX POWER LEVEL,
 *
 * \return tx power level
 */
uint8_t aos_ble_core_get_tx_power_level(void);

/*!
 * \fn bool aos_ble_core_get_tx_power_dbm(uint8_t pow_level, int16_t * pow_dbm)
 *
 * \brief Convert TX POWER from LEVEL to 0.01dBm unit,
 *
 * \param pow_level the tx power level
 *
 * \param pow_dbm The output value of the power in 0.01dBm
 *
 * \return status success/fail
 */
bool aos_ble_core_get_tx_power_dbm(uint8_t pow_level, int16_t * pow_dbm);

/* USER CODE END EF */

/*! @}*/
#ifdef __cplusplus
}
#endif
