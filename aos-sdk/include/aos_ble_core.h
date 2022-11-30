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
#include "ble_types.h"
#include "stdbool.h"
#include "aos_common.h"
#include "aos_ble_common.h"
#include "aos_lpm.h"


/*!
 * \defgroup aos_ble_core BLE core
 *
 * \addtogroup aos_ble_core
 * @{
 */

#define AOS_BLE_CORE_TX_POWER_LEVEL_MIN 0			//!< Min index of tx power level
#define AOS_BLE_CORE_TX_POWER_LEVEL_MAX 31			//!< Max index of tx power level

#define RESTART_ADV     true
#define STOP_ADV        false

//TODO find a better place for this:
#define ABS(x) ((x)>0? (x):-(x))		            //!< Macro to get the absolute value of the input parameter

/* Exported types ------------------------------------------------------------*/

/*!
 * \enum aos_ble_core_conn_status_t
 *
 * \brief BLE connection state.
 */
typedef enum {
	aos_ble_core_idle,              //!< BLE in idle state
	aos_ble_core_fast_adv,          //!< BLE in fast advertisement state
	aos_ble_core_lp_adv,            //!< BLE in slow advertisement state
	aos_ble_core_scan,              //!< BLE in scan state
	aos_ble_core_lp_connecting,     //!< BLE is connecting as client
	aos_ble_core_connected_server,  //!< BLE connected as server
	aos_ble_core_connected_client,  //!< BLE connected as client
	aos_ble_core_connected_bonded   //!< BLE connected as client
} aos_ble_core_conn_status_t;

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
 * \fn void aos_ble_core_app_init(uint8_t role)
 *
 * \brief BLE Initialization API
 *
 * \param app_info BLE application initialization data
 */
void aos_ble_core_app_init(aos_ble_app_data_t *app_info);

/*!
 * \fn aos_ble_core_conn_status_t aos_ble_core_get_connection_status(void);
 *
 * \brief Get BLE connection status
 *
 * \return BLE connection status
 */
aos_ble_core_conn_status_t aos_ble_core_get_connection_status(void);

/*!
 * \fn const uint8_t* aos_ble_core_get_bd_address( void )
 *
 * \brief Get BLE Device address
 *
 * \return pointer to the BLE MAC address
 */
const uint8_t* aos_ble_core_get_bd_address(void);


/*!
 * \fn void aos_ble_core_advertise(aos_ble_core_conn_status_t New_Status)
 *
 * \brief Start connectivity advertisement
 *
 * \param New_Status Start fast/slow advertisement
 */
void aos_ble_core_advertise(aos_ble_core_conn_status_t new_status);

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

/*!
 * \fn void hci_notify_asynch_evt(void* pdata)
 *
 * \brief  This callback is called from either
 *          - IPCC RX interrupt context
 *          - hci_user_evt_proc() context.
 *          - hci_resume_flow() context
 *         It requests hci_user_evt_proc() to be executed.
 *
 * \param  pdata Packet or event pointer
 *
 * \return None
 */
void hci_notify_asynch_evt(void* pdata);

/*!
 * \fn void hci_cmd_resp_release(uint32_t flag)
 *
 * \brief  This function is called when an ACI/HCI command response is received from the CPU2.
 *         A weak implementation is available in hci_tl.c based on polling mechanism
 *         The user may re-implement this function in the application to improve performance :
 *         - It may use UTIL_SEQ_SetEvt() API when using the Sequencer
 *         - It may use a semaphore when using cmsis_os interface
 *
 * \param  flag: Release flag
 *
 * \return None
 */
void hci_cmd_resp_release(uint32_t flag);

/*!
 * \fn void hci_cmd_resp_wait(uint32_t timeout)
 *
 * \brief  This function is called when an ACI/HCI command is sent to the CPU2 and the response is waited.
 *         It is called from the same context the HCI command has been sent.
 *         It shall not return until the command response notified by hci_cmd_resp_release() is received.
 *         A weak implementation is available in hci_tl.c based on polling mechanism
 *         The user may re-implement this function in the application to improve performance :
 *         - It may use UTIL_SEQ_WaitEvt() API when using the Sequencer
 *         - It may use a semaphore when using cmsis_os interface
 *
 * \param  timeout: Waiting timeout
 *
 * \return None
 */
void hci_cmd_resp_wait(uint32_t timeout);

/*!
 * \fn bool aos_ble_core_get_adv_status(void)
 *
 * \brief  This function return the advertisement status (ON/OFF)
 *
 * \return bool return the advertisement status (advertising or no)
 */
bool aos_ble_core_get_adv_status(void);

/*!
 * \fn bool aos_ble_core_remove_bond(void)
 *
 * \brief  This function remove all bonded devices
 *
 * \return bool return success/fail
 */
bool aos_ble_core_remove_bond(void);

/*!
 * \fn bool aos_ble_core_stop_connectivity(bool restart_adv)
 *
 * \brief This function stop connectivity, stop advertisement
 *
 * \param restart_adv restart or not the advertisement after disconnection
 *
 * \return bool return success/fail
 */
bool aos_ble_core_stop_connectivity(bool restart_adv);

/*!
 * \fn void aos_ble_core_get_firmware_version(aos_ble_core_fw_version_t *ble_version)
 *
 * \brief get stack and FUS firmware version,
 *
 * \param ble_version output the stack and fus version
 *
 */
void aos_ble_core_get_firmware_version(aos_ble_core_fw_version_t *ble_version);

/*!
 * \fn void aos_ble_core_set_lpm_mode(aos_lpm_mode_t mode, bool delayed)
 *
 * \brief Disable/enable the low power mode of the BLE requester, the request
 *  could be delayed by 100ms if the param "delayed" is set to true
 *
 * \param mode The desired LPM mode
 *
 * \param delayed delay the request if set to True
 *
 */
void aos_ble_core_set_lpm_mode(aos_lpm_mode_t mode, bool delayed);

/* USER CODE END EF */

/*! @}*/
#ifdef __cplusplus
}
#endif
