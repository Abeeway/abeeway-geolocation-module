/*!
 * \file aos_ble_common.h
 *
 * \brief common types and const for BLE module
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BIT
#define BIT(n) (1UL << (n))
#endif

#include "aos_common.h"

/*!
 * \defgroup aos_ble_common BLE core common defines
 *
 * \addtogroup aos_ble_common
 * @{
 */

#define MAX_CUSTOM_SERVICES_COUNT      (10)       //!< We choose to offer max services count of 16, 6 are standard services already defined, and 10 kept for customer to define
#define MAX_CHAR_COUNT                 (10)       //!< The max number of characteristic for a custom service
#define CUSTOM_SERVICE_BITMASK         (0xFFC0)   //!< Custom services are in bit 6 to 15 (refer to BLE_services_bit_mask_t)

#define BLE_GAP_ADDR_LEN               (6)        //!< BLE address length
#define EDDYSTONE_SERVCE_UUID          0xFEAA     //!< Eddystone beacon service UUID
#define IBEACON_COMPANY_IDENTIFIER     0x004C     //!< Ibeacon company identifier
#define ALTBEACON_CODE                 0xBEAC     //!< Altbeacon code
#define EXPOSURE_SERVCE_UUID           0xFD6F     //!< Exposure beacon service UUID

#define EXP_NOTIF_SERVICE_VERSION      0x00       //!< Exposure beacon notification service version
#define SERIAL_NUMBER_ID_LEN           AOS_PROVISIONING_EUI_SIZE    //!< The actual size of the field serial number string in DIS service
/*!
 * \enum BLE_services_bit_mask_t
 *
 * \brief BLE services bit mask
 */
enum {
	TX_POWER_SERVICE              = BIT(0),       //!< TX power service bit-mask
	DEVICE_INFORMATION_SERVICE    = BIT(1),       //!< Device information service bit-mask
	IMMEDIATE_ALERT_SERVICE       = BIT(2),       //!< Immediate alert service bit-mask
	LINK_LOSS_SERVICE             = BIT(3),       //!< Link loss service bit-mask
	BATTERY_SERVICE               = BIT(4),       //!< Battery service bit-mask
	ENVIRONMENTAL_SENSING_SERVICE = BIT(5),       //!< Environmental sensing service bit-mask
	CUSTOM_1_SERVICE              = BIT(6),       //!< Custom 1 service bit-mask
	CUSTOM_2_SERVICE              = BIT(7),       //!< Custom 2 service bit-mask
	CUSTOM_3_SERVICE              = BIT(8),       //!< Custom 3 service bit-mask
	CUSTOM_4_SERVICE              = BIT(9),       //!< Custom 4 service bit-mask
	CUSTOM_5_SERVICE              = BIT(10),      //!< Custom 5 service bit-mask
	CUSTOM_6_SERVICE              = BIT(11),      //!< Custom 6 service bit-mask
	CUSTOM_7_SERVICE              = BIT(12),      //!< Custom 7 service bit-mask
	CUSTOM_8_SERVICE              = BIT(13),      //!< Custom 8 service bit-mask
	CUSTOM_9_SERVICE              = BIT(14),      //!< Custom 9 service bit-mask
	CUSTOM_10_SERVICE             = BIT(15)       //!< Custom 10 service bit-mask
};

/*!
 * \struct aos_ble_core_fw_version_t
 *
 * \brief BLE stack and FUS versions
 */
typedef struct {
	// Wireless Info
	uint8_t stack_major;               //!< Major part of the BLE stack version
	uint8_t stack_minor;               //!< Minor part of the BLE stack version
	uint8_t stack_sub;                 //!< SUB   part of the BLE stack version
	// Fus Info
	uint8_t fus_major;                 //!< Major part of the BLE FUS version
	uint8_t fus_minor;                 //!< Minor part of the BLE FUS version
	uint8_t fus_sub;                   //!< SUB   part of the BLE FUS version
} aos_ble_core_fw_version_t;

/*!
 * \union uuid_t
 *
 * \brief Definition of uuid_t struct, to hold UUID on 16 Bits or 128 bits
 */
typedef union {
  uint16_t uuid_16;                    //!< 16-bit UUID
  uint8_t  uuid_128[16];               //!<< 128-bit UUID
} __attribute__((packed)) uuid_t;

/*!
 * \struct uuid_type_t
 *
 * \brief Definition of uuid_type_t struct
 */
typedef struct {
	uint8_t  uuid_type;                //!< UUID Type 16 Bits or 128 Bits
	uuid_t   uuid;                     //!< UUID value
} uuid_type_t;

/*!
 * \struct ble_service_init_data_t
 *
 * \brief BLE custom service initialization data,
 *        data needed to initialize/configure a custom service
 */
typedef struct {
	uint16_t    service_handle;        //!< Service Handle
	uuid_type_t service_uuid;          //!< Service UUID
	uint8_t     service_type;          //!< Service Type (Primary or secondary service)
	uint8_t     num_attr_rec;          //!< number off attribute record
} ble_service_init_data_t;

/*!
 * \struct ble_char_init_data_t
 *
 * \brief BLE Characteristic initialization data,
 *        data needed to initialize/configure a characteristic
 */
typedef struct {
	uint16_t    char_handle;           //!< Characteristic handle
	uuid_type_t char_uuid;             //!< Characteristic UUID and UUID Type
	uint16_t    char_value_length;     //!< Maximum length of the characteristic value
	uint8_t     char_properties;       //!< Characteristic properties, Characteristic Properties (Volume 3, Part G, section 3.3.1.1 of Bluetooth Core Specification)
	uint8_t     security_permissions;  //!< Security permission flags
	uint8_t     gatt_evt_mask;         //!< GATT event mask
	uint8_t     enc_key_size;          //!< Minimum encryption key size required to read the characteristic
	uint8_t     is_variable;           //!< Specify if the characteristic value has a fixed length or a variable length
} ble_char_init_data_t;

/*!
 * \struct ble_char_update_data_t
 *
 * \brief BLE Characteristic update data,
 *        data needed to update a characteristic value
 */
typedef struct {
	uint16_t service_handle;           //!< Service handle
	uint16_t char_handle;              //!< Characteristic handle
	uint16_t char_uuid;                //!< Characteristic UUID
	uint8_t  value_offset;             //!< Offset value
	uint8_t  char_value_length;        //!< Maximum length of the characteristic value
	uint8_t  *char_value;              //!< Characteristic value
} ble_char_update_data_t;

/*!
 * \struct ble_custom_srvc_init_data_t
 *
 * \brief BLE custom services and characteristics initialization data,
 *        data needed to update a custom service and its characteristics
 */
typedef struct {
	ble_service_init_data_t service;   //!< BLE custom service initialization data
	void* event_handler;               //!< Event handler function
	uint8_t char_count;                //!< characteristics count
	ble_char_init_data_t characteristic[MAX_CHAR_COUNT];   //!< BLE Characteristic initialization data
} ble_custom_srvc_init_data_t;


/*!
 * \typedef void custom_srvc_char_value_init_cb_t(ble_custom_srvc_init_data_t* app_info)
 *
 * \brief Call back function for custom characteristics value initialization.
 *        In the AOS this function is called, when the BLE is initialized,
 *        To set the initialized characteristics value.
 *
 * \param app_info BLE custom services and characteristics data to be initialized
 */
typedef void custom_srvc_char_value_init_cb_t(ble_custom_srvc_init_data_t* app_info);

/*!
 * \struct aos_ble_app_data_t
 *
 * \brief Application data to send to the BLE to update
 *        characteristic data
 */
typedef struct {
	uint8_t ble_role;                                                                //!< BLE GAP role : Peripheral, central, observer, broadcaster ...
	buffer_t serial_number;                                                          //!< Device Serial number to set in DIS service
	uint32_t app_version;                                                            //!< Application version (MFG or demo app ...)
	aos_ble_core_fw_version_t ble_version;                                           //!< BLE fw version (stack and FUS versions)
	uint16_t ble_srvc_mask;                                                          //!< BLE services bitmask, enable/disable a service, refer to BLE_services_bit_mask_t */
	uint8_t custom_srvc_count;                                                       //!< Number of custom services to add, should not exceed MAX_CUSTOM_SERVICES_COUNT
	ble_custom_srvc_init_data_t custom_srvc_init_data[MAX_CUSTOM_SERVICES_COUNT];    //!< BLE custom services and characteristics initialization data
	custom_srvc_char_value_init_cb_t *app_init_char_cb;                              //!< Callback function to register, used at the BLE initialization to initialize characteristics value
} aos_ble_app_data_t;

/*! @}*/
#ifdef __cplusplus
}
#endif
