/*!
 * \file srv_ble_connectivity.h
 *
 * \brief BLE connectivity service
 */
#pragma once

/*!
 * \defgroup srv_ble_cnx BLE connectivity service
 *
 * \addtogroup srv_ble_cnx
 * @{
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "aos_ble_common.h"

#define CONNECTION_HANDLE_NOT_IN_CONNECTION      0xFFFF    //!< Define value of connection handle when not connected
#define LOCAL_NAME_MAX_SIZE                      (27)      //!< Max size of local name in advertising packet(31-3 for flag - 1 for local name len)
#define SCAN_RESP_DATA_MAX_SIZE                  (31)      //!< Max size of scan response data

/*!
 * \enum srv_ble_conn_event_t
 *
 * \brief BLE connection events.
 */
typedef enum {
	srv_ble_conn_event_idle,              //!< BLE connectivity in idle state
	srv_ble_conn_event_advertise,         //!< BLE connectivity in fast advertisement state
	srv_ble_conn_event_connection,        //!< BLE connected to a central device
	srv_ble_conn_event_bonded,            //!< BLE connected and bonded to a central device
} srv_ble_conn_event_t;

/*!
 * \enum srv_ble_connectivity_status_t
 *
 * \brief BLE connection state.
 */
typedef enum {
	srv_ble_connectivity_idle,              //!< BLE in idle state
	srv_ble_connectivity_advertising,       //!< BLE in fast advertisement state
	srv_ble_connectivity_connected_server,  //!< BLE connected as server
	srv_ble_connectivity_connected_bonded   //!< BLE connected as client
} srv_ble_connectivity_status_t;

/*!
 * \struct srv_ble_connectivity_conn_params_t
 *
 * \brief Connection parameters structure.
 */
typedef struct {
  uint16_t min_conn_interval;         //!< Minimum Connection Interval in 1.25 ms units
  uint16_t max_conn_interval;         //!< Maximum Connection Interval in 1.25 ms units
  uint16_t slave_latency;             //!< Slave Latency in number of connection events
  uint16_t conn_sup_timeout;          //!< Connection Supervision Timeout in 10 ms units
} srv_ble_connectivity_conn_params_t;

/*!
 * \struct srv_ble_conn_adv_config_t
 *
 * \brief Advertising configuration structure.
 */
typedef struct {
	uint16_t min_interval;                            //!< Minimum connectivity advertising interval
	uint16_t max_interval;                            //!< Maximum connectivity advertising interval
	uint8_t own_address_type;                         //!< Own address type
	uint8_t filter_policy;                            //!< Whitelist used or not
	uint8_t local_name[LOCAL_NAME_MAX_SIZE];          //!< Local name advertised
	uint8_t local_name_size;                          //!< Local name size
	uint8_t scan_resp_data[SCAN_RESP_DATA_MAX_SIZE];  //!< Scan response data, 31 Bytes of data formatted as defined in [Vol 3] Part C, Section 11 of the BLE conn spec.
	uint8_t advt_serv_uuid_len;                       //!< Length of the UUID list to be used while advertising
	uint8_t advt_serv_uuid[100];                      //!< The UUID list to be used while advertising
} srv_ble_conn_adv_config_t;

/*!
 * \fn aos_result_t srv_ble_connectivity_start(srv_ble_conn_adv_config_t *conf)
 *
 * \brief Start the advertisement for connectivity
 *
 * \param conf advertising configuration
 *
 * \return The status of the operation
 */
aos_result_t srv_ble_connectivity_start(srv_ble_conn_adv_config_t *conf);

/*!
 * \fn aos_result_t srv_ble_connectivity_stop(void)
 *
 * \brief Stop the advertisement for connectivity
 *
 * \return The status of the operation
 */
aos_result_t srv_ble_connectivity_stop(void);

/*!
 * \fn uint64_t srv_ble_connectivity_get_power_consumption(void)
 *
 * \brief return connectivity power consumption.
 *
 * \return value of the power consumption in uAh
 */
uint64_t srv_ble_connectivity_get_power_consumption(void);

/*!
 * \fn void srv_ble_connectivity_clear_consumption(void)
 *
 * \brief reset the connectivity power consumption.
 */
void srv_ble_connectivity_clear_consumption(void);

/*!
 * \fn bool srv_ble_connectivity_do_restart(void)
 *
 * \brief Check if we have to restart connectivity. To completely
 * stop connectivity the user should call srv_ble_connectivity_stop(),
 * but if connectivity is stopped because of an error (like when getting
 * error when restarting connectivity advertising while beaconing is ON)
 * the application should call this API to check if the connectivity
 * advertising should be restarted
 *
 * \return True: restart adv, False: Do not restart Adv
 */
bool srv_ble_connectivity_do_restart(void);

/*!
 * \fn aos_result_t srv_ble_connectivity_disconnect(uint8_t reason)
 *
 * \brief Disconnect connection identified with connection_handle and send
 * the disconnection reason as specified in reason.
 *
 * \param reason The disconnection reason
 *
 * \return The status result of the operation
 */
aos_result_t srv_ble_connectivity_disconnect(uint8_t reason);

/*!
 * \fn aos_result_t srv_ble_connectivity_stop_advertising(void)
 *
 * \brief Stop connectivity advertising.
 *
 * \return The status result of the operation
 */
aos_result_t srv_ble_connectivity_stop_advertising(void);

/*!
 * \fn bool srv_ble_connectivity_check_device_in_whitelist(uint8_t *bond_count, uint8_t mac_addr[BLE_MAC_ADDR_SIZE], uint8_t addr_type)
 *
 * \brief Check if the device with BLE address and addr_type are in the white list,
 *
 * \param bond_count output the number of bonded devices
 * \param mac_addr the MAC address to check
 * \param addr_type the address type of mac_addr
 *
 * \return true if in the white list, false if not
 */
bool srv_ble_connectivity_check_device_in_whitelist(uint8_t *bond_count, uint8_t mac_addr[BLE_MAC_ADDR_SIZE], uint8_t addr_type);

/*!
 * \fn bool srv_ble_connectivity_add_bonded_to_whitelist(void)
 *
 * \brief Add the device actually connected and bonded to the whitelist,
 *
 * \return true if success, false if fail
 */
bool srv_ble_connectivity_add_bonded_to_whitelist(void);

/*!
 * \fn bool srv_ble_connectivity_remove_bond(void)
 *
 * \brief  This function remove all bonded devices,
 * it doesn't disconnect the device if connected
 *
 * \return bool return success/failure
 */
bool srv_ble_connectivity_remove_bond(void);

/*!
 * \fn aos_result_t srv_ble_connectivity_conn_parameters_update_req(srv_ble_connectivity_conn_params_t new_conn_params)
 *
 * \brief Update BLE connection parameters,
 *
 * \param new_conn_params connection parameters to set
 *
 * \return aos_result_t error status
 */
aos_result_t srv_ble_connectivity_conn_parameters_update_req(srv_ble_connectivity_conn_params_t new_conn_params);

/*!
 * \fn uint8_t srv_ble_connectivity_get_bond_count(void)
 *
 * \brief return number of client bonded to the device,
 *
 * \return Bond count
 */
uint8_t srv_ble_connectivity_get_bond_count(void);

/*!
 * \fn srv_ble_connectivity_status_t srv_ble_connectivity_get_connection_status(void);
 *
 * \brief Get BLE connection status
 *
 * \return BLE connection status
 */
srv_ble_connectivity_status_t srv_ble_connectivity_get_connection_status(void);

/*!
 * \fn uint16_t srv_ble_connectivity_get_connection_handle(void)
 *
 * \brief Return connection handle for the actual connection,
 *
 * \return uint16_t connection handle
 */
uint16_t srv_ble_connectivity_get_connection_handle(void);

/*!
 * \fn void srv_ble_connectivity_init_ble_services(aos_ble_app_data_t *app_info)
 *
 * \brief Initialize BLE services (standard and custom BLE services),
 *
 * \param app_info data needed for initialization (services and characteristics to init and characteristics value)
 */
void srv_ble_connectivity_init_ble_services(aos_ble_app_data_t *app_info);

/*! @}*/
#ifdef __cplusplus
}
#endif
