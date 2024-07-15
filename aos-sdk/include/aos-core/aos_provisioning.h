/*!
 * \file aos_provisioning.h
 *
 * \brief Module provisioning facility
 *
 * This source component allows to store and retrieve all the
 * persistent module parameters, notably all the LoRaWAN parameters but
 * also a number of board- and application-specific parameters that are
 * set during module provisioning (manufacturing) and will not change
 * over the lifetime of the device.
 *
 * It relies on the LR1110 crypto engine for storing keys and parameters.
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

 #pragma once

#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup aos_provisioning provisioning
 *
 * \addtogroup aos_provisioning
 * @{
 */
#define PROVISIONING_EUI_SIZE 8				//!< LoRa Unique device identifier (devEUI) size
#define PROVISIONING_KEY_SIZE 16			//!< Size of the cryptographic key

typedef uint8_t aos_provisioning_eui_t[PROVISIONING_EUI_SIZE];	//!< Provisioned devEUI type
typedef uint8_t aos_provisioning_key_t[PROVISIONING_KEY_SIZE];	//!< Provisioned rcyptographic key type

/*!
 * \enum aos_provisioning_status_t
 *
 * \brief Operation result
 */
typedef enum {
	aos_provisioning_status_success = 0,	//!< Operation success
	aos_provisioning_status_failure,		//!< Generic failure
	aos_provisioning_status_bad_parameter,	//!< Invalid parameter passed to API function
	aos_provisioning_status_crypto_failure,	//!< Crypto engine failure
	aos_provisioning_status_no_data_found,	//!< No provisioning data in flash
} aos_provisioning_status_t;

/*!
 * \enum aos_provisioning_parameter_t
 *
 * \brief User parameters
 * \note Flash storage is inside the LR1110
 */
typedef enum {
	aos_provisioning_parameter_0,			//!< User parameter 0. Free for use
	aos_provisioning_parameter_1,			//!< User parameter 1. Free for use
	aos_provisioning_parameter_2,			//!< User parameter 2. Free for use
	aos_provisioning_parameter_3,			//!< User parameter 3. Free for use
	aos_provisioning_parameter_4,			//!< User parameter 4. Free for use
	aos_provisioning_parameter_5,			//!< User parameter 5. Free for use
	aos_provisioning_parameter_6,			//!< User parameter 6. Free for use
	aos_provisioning_parameter_7,			//!< User parameter 7. Free for use
	aos_provisioning_parameter_8,			//!< User parameter 8. Free for use
	aos_provisioning_parameter_9,			//!< User parameter 9. Free for use
	aos_provisioning_parameter_10,			//!< User parameter 10. Free for use
	aos_provisioning_parameter_11,			//!< User parameter 11. Free for use
	aos_provisioning_parameter_12,			//!< User parameter 12. Free for use
	aos_provisioning_parameter_13,			//!< User parameter 13. Free for use
	aos_provisioning_parameter_14,			//!< User parameter 14. Free for use
	aos_provisioning_parameter_devnonce,	//!< User parameter 15. Used to store the LoRa devNonce
	aos_provisioning_parameter_count		//!< Number of parameters
} aos_provisioning_parameter_t;

/*!
 * \enum aos_provisioning_data_state_t
 *
 * \brief State of the provisioning.
 * \note Flash storage is inside the LR1110
 */
typedef enum {
	aos_provisioning_data_state_invalid = 0,	//!< No valid provisioning data in flash or ram
	aos_provisioning_data_state_updated,		//!< RAM settings updated (needs to be saved)
	aos_provisioning_data_state_saved,			//!< RAM settings match those saved in flash.
} aos_provisioning_data_state_t;

/*!
 * \enum aos_provisioning_mac_region_t
 *
 * \brief LoRa regions definitions
 */
typedef enum {
	aos_provisioning_mac_region_eu868      =  0,	//!< LoRa region EU868 (Europe)
	aos_provisioning_mac_region_us915      =  1,	//!< LoRa regions US915 (USA)
	aos_provisioning_mac_region_ru864      =  2,	//!< LoRa region RU864 (Russio)
	aos_provisioning_mac_region_as923_1    =  3,	//!< LoRa region AS923-1 (Asia)
	aos_provisioning_mac_region_au915      =  4,	//!< LoRa region AU915 (Australia)
	aos_provisioning_mac_region_kr920      =  5,	//!< LoRa region KR915 (korea)
	aos_provisioning_mac_region_in865      =  6,	//!< LoRa region IN865 (India)
	aos_provisioning_mac_region_as923_1_jp =  7,	//!< LoRa region AS923_-11_jp (Japan)
	aos_provisioning_mac_region_as923_2    =  8,	//!< LoRa region AS923-2 (Asia)
	aos_provisioning_mac_region_as923_3    =  9,	//!< LoRa region AS923-3 (Asia)
	aos_provisioning_mac_region_as923_4    = 10,	//!< LoRa region AS923-4 (Asia)
	aos_provisioning_mac_region_count				//!< Number of regions
} aos_provisioning_mac_region_t;

/*!
 * \enum aos_provisioning_activation_t
 *
 * \brief LoRa activation type
 */
typedef enum {
	aos_provisioning_activation_otaa = 0,			//!< OTAA activation
	aos_provisioning_activation_abp,				//!< ABP activation. Not supported by the SDK.
	aos_provisioning_activation_count				//!< Number of activations
} aos_provisioning_activation_t;

/*!
 * \enum aos_provisioning_key_id_t
 *
 * \brief LoRa cryptographic key identifier
 *
 * \note For security reasons, keys can only be written, but not read back. To
 * determine whether a particular key has been stored, a bitmask is used.
 */
typedef enum {
	aos_provisioning_key_id_app_s_key = 0,		//!< Used by ABP
	aos_provisioning_key_id_nwk_s_key,			//!< Used by ABP
	aos_provisioning_key_id_app_key,			//!< Used by OTAA
	aos_provisioning_key_id_nwk_key,			//!< Used by OTAA
	aos_provisioning_key_id_count				//!< Number of key identifiers
} aos_provisioning_key_id_t;

/*!
 * \brief Write a user parameter (in the RAM cache).
 *
 * \param id Parameter identifier
 * \param value Value to write
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_set_parameter(aos_provisioning_parameter_t id, uint32_t value);

/*!
 * \brief Read a user parameter (from the RAM cache).
 *
 * \param id Parameter identifier
 * \param value Returned value
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_get_parameter(aos_provisioning_parameter_t id, uint32_t *value);


/*!
 * \brief Read the state of the provisioning
 *
 * \return The provisioning state
 */
aos_provisioning_data_state_t aos_provisioning_data_state(void);

/*!
 * \brief Initialize with the default values
 *
 * \param region LoRa region
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_factory_settings(aos_provisioning_mac_region_t region);

/*!
 * \brief Write a key in the crypto engine
 *
 * \param id Key identifier
 * \param key Key to be written
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_set_key(aos_provisioning_key_id_t id, const aos_provisioning_key_t key);

/*!
 * \brief Erase a key in the crypto engine
 *
 * \param id Key identifier
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_erase_key(aos_provisioning_key_id_t id);

/*!
 * \brief Check whether a key is set
 *
 * \param k Key identifier
 *
 * \return True if the key is set, false otherwise
 */
bool aos_provisioning_key_is_set(aos_provisioning_key_id_t k);

/*!
 * \brief Erase the provisioning
 *
 * \return The operation status
 *
 * \note The keys are also erased
 */
aos_provisioning_status_t aos_provisioning_erase(void);

/*!
 * \brief Read the provisioning from the flash and update the RAM cache
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_read(void);

/*!
 * \brief Save the provisioning from the RAM cache to the flash
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_save(void);

/*!
 * \brief Read the LoRa device unique identifier from the RAM cache
 *
 * \param ret_eui Pointer where the devEUI will be copied
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_get_lora_device_eui(aos_provisioning_eui_t ret_eui);

/*!
 * \brief Write the LoRa device unique identifier to the RAM cache
 *
 * \param eui DevEUI to be written
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_set_lora_device_eui(aos_provisioning_eui_t eui);

/*!
 * \brief Read the Join unique identifier from the RAM cache
 *
 * \param ret_eui Pointer where the join EUI will be copied
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_get_lora_join_eui(aos_provisioning_eui_t ret_eui);

/*!
 * \brief Write the LoRa join identifier to the RAM cache
 *
 * \param eui Join identifier to be written
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_set_lora_join_eui(aos_provisioning_eui_t eui);

/*!
 * \brief Read the LoRa device identifier from the RAM cache (ABP)
 *
 * \param devaddr Pointer where the device address will be copied
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_get_lora_devaddr(uint32_t *devaddr);

/*!
 * \brief Write the LoRa device identifier to the RAM cache
 *
 * \param devaddr device identifier to be written
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_set_lora_devaddr(uint32_t devaddr);

/*!
 * \brief Read the LoRa activation From the RAM cache
 *
 * \param type LoRa activation to be written
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_get_lora_activation(aos_provisioning_activation_t *type);

/*!
 * \brief Write the LoRa activation to the RAM cache
 *
 * \param type Pointer where the activation will be copied
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_set_lora_activation(aos_provisioning_activation_t type);

/*!
 * \brief Read the LoRa region from the RAM cache
 *
 * \param region Pointer where the region will be copied
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_get_lora_mac_region(aos_provisioning_mac_region_t *region);

/*!
 * \brief Write the LoRa region to the RAM cache
 *
 * \param region Region to be written
 *
 * \return The operation status
 */
aos_provisioning_status_t aos_provisioning_set_lora_mac_region(aos_provisioning_mac_region_t region);


/*! @}*/
#ifdef __cplusplus
}
#endif

