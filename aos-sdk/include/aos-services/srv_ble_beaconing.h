/*!
 * \file srv_ble_beaconing.h
 *
 * \brief BLE beaconing service
 *
 * \details This service performs BLE beacon emulation
 *
 * \warning Before using this service, the BLE driver should be opened as advertiser only or peripheral.
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 * <H3>References</H3>
 * <DIV>
 *  Eddystone beacons specification: https://github.com/google/eddystone/tree/master/eddystone-uid<BR>
 *  iBeacon specification: https://developer.apple.com/ibeacon/Getting-Started-with-iBeacon.pdf
 * </DIV>
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup srv_ble_beaconing Beaconing service
 *
 * \addtogroup srv_ble_beaconing
 * @{
 */

#include <stdbool.h>
#include <stdint.h>

#include "aos_common.h"


#define SRV_BLE_BEACONING_EXPOSURE_RPI_LEN 16				//!< Length of the RPI field (exposure beacons)
#define SRV_BLE_BEACONING_EXPOSURE_METADATA_LEN 4			//!< Length of the meta data
#define SRV_BLE_BEACONING_IBEACON_COMPANY_UUID_LEN		16  //!< Length of the company identifier (iBeacons beacons)
#define SRV_BLE_BEACONING_ALTBEACON_MANUF_ID_LEN		4	//!< Manufacturer Identifier length
#define SRV_BLE_BEACONING_ALTBEACON_BEACON_ID_LEN		20	//!< Beacon identifier
#define SRV_BLE_BEACONING_EDDYSTONE_UUID_NAME_SPACE_LEN	10	//!< Eddystone UUID. name space field length
#define SRV_BLE_BEACONING_EDDYSTONE_UUID_INSTANCE_LEN	6	//!< Eddystone UUID.instance field length
#define SRV_BLE_BEACONING_QUUPPA_ID_LEN					6	//!< QUUPPA identifier length

/*!
 * \enum srv_ble_beacon_type_t
 *
 * \brief Beaconing emulation
 */
typedef enum {
	srv_ble_beacon_type_eddy_uuid,      //!< Eddystone UUID beacon emulation
	srv_ble_beacon_type_ibeacon,        //!< IBeacon emulation
	srv_ble_beacon_type_altbeacon,      //!< AltBeacon emulation
	srv_ble_beacon_type_quuppa,         //!< QUUPPA beacon emulation
	srv_ble_beacon_type_exposure        //!< Exposure beacon emulation
} srv_ble_beacon_type_t;


/*!
 * \struct srv_ble_beaconing_exposure_data_t
 *
 * \brief exposure beacon data
 */
typedef struct  {
	uint8_t rpi[SRV_BLE_BEACONING_EXPOSURE_RPI_LEN];			//!< Random public identifier
	uint8_t meta_data[SRV_BLE_BEACONING_EXPOSURE_METADATA_LEN];	//!< Meta data
} srv_ble_beaconing_exposure_data_t;

/*!
 * \struct srv_ble_beaconing_ibeacon_data_t
 *
 * \brief Ibeacon beacon information data
 */
typedef struct {
	uint8_t company_uuid[SRV_BLE_BEACONING_IBEACON_COMPANY_UUID_LEN];	//!< Company Unique identifier
	uint8_t major[2];													//!< Major number
	uint8_t minor[2];													//!< Minor number
} srv_ble_beaconing_ibeacon_data_t;

/*!
 * \struct srv_ble_beaconing_altbeacon_data_t
 *
 * \brief Altbeacon beacon data fields
 */
typedef struct {
	uint8_t manufacturer_id[SRV_BLE_BEACONING_ALTBEACON_MANUF_ID_LEN];	//!< Manufacturer identifier
	uint8_t beacon_id[SRV_BLE_BEACONING_ALTBEACON_BEACON_ID_LEN];		//!< Beacon identifier
} srv_ble_beaconing_altbeacon_data_t;


/*!
 * \struct srv_ble_beaconing_eddy_uuid_data_t
 *
 * \brief UUID beacon data fields
 */
typedef struct {
	uint8_t name_space[SRV_BLE_BEACONING_EDDYSTONE_UUID_NAME_SPACE_LEN];		//!< Name space
	uint8_t instance[SRV_BLE_BEACONING_EDDYSTONE_UUID_INSTANCE_LEN];			//!< Instance
} srv_ble_beaconing_eddy_uuid_data_t;


/*!
 * \struct srv_ble_beaconing_quuppa_data_t
 *
 * \brief QUUPPA advertisement data
 */
typedef struct {
	int8_t compensated_tx_power;								//!< TX power at 1 meter plus tx power global compensation
	uint8_t identifier[SRV_BLE_BEACONING_QUUPPA_ID_LEN];		//!< Identifier length Usually the 6 lowest by of LoRa deveui
} srv_ble_beaconing_quuppa_data_t;

/*!
 * \struct srv_ble_beaconing_param_t
 *
 * \brief Beaconing configuration
 */
typedef struct {
	uint16_t adv_interval;           //!< Specifies the desired advertising interval [ms].
	uint8_t  calibrated_tx_power;    //!< Specifies the power at 0m or 1m
	uint8_t  tx_level;               //!< Specifies the TX power level for advertisement
	union {
		srv_ble_beaconing_eddy_uuid_data_t eddy_uuid;	//!< Eddystone UUID Data to advertise
		srv_ble_beaconing_ibeacon_data_t ibeacon;		//!< ibeacon Data to advertise
		srv_ble_beaconing_altbeacon_data_t alt_beacon;	//!< altBeacon Data to advertise
		srv_ble_beaconing_quuppa_data_t quuppa;			//!< quuppa data to adverise
		srv_ble_beaconing_exposure_data_t exposure;		//!< Exposure Data to advertise
  };
} srv_ble_beaconing_param_t;

/*!
 * \fn aos_result_t srv_ble_beaconing_start(srv_ble_beacon_type_t type, srv_ble_beaconing_param_t* param)
 *
 * \brief Start beaconing
 *
 * \param type Beacon type to emulate
 * \param param beacon emulation configuration
 *
 * \return result status
 */
aos_result_t srv_ble_beaconing_start(srv_ble_beacon_type_t type, srv_ble_beaconing_param_t* param);

/*!
 * \fn void srv_ble_beaconing_stop(void)
 *
 * \brief Stop the beaconing
 */
void srv_ble_beaconing_stop(void);

/*!
 * \fn bool srv_ble_beaconing_is_active(void)
 *
 * \brief return beaconing state.
 *
 * \return true if beaconing is active
 */
bool srv_ble_beaconing_is_active(void);

/*!
 * \fn uint64_t srv_ble_beaconing_get_power_consumption(void)
 *
 * \brief return beaconing power consumption.
 *
 * \return value of the power consumption in uAh
 */
uint64_t srv_ble_beaconing_get_power_consumption(void);

/*!
 * \fn void srv_ble_beaconing_clear_consumption(void)
 *
 * \brief reset the power consumption.
 */
void srv_ble_beaconing_clear_consumption(void);

/*!
 * \fn void srv_ble_beaconing_tx_power_updated(uint8_t pow_level)
 *
 * \brief Update TX power level used this implies changing beaconing consumption.
 * The user should call this function each time BLE tx power is updated.
 *
 * \param pow_level power level used to process new power consumption value
 */
void srv_ble_beaconing_tx_power_updated(uint8_t pow_level);

/*! @}*/
#ifdef __cplusplus
}
#endif
