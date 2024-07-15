/*!
 *
 * \file    srv_config_param.h
 *
 * \brief  Configuration parameter management
 *
 * \details This module provides a configuration parameter management usually stored in a flash
 * The module uses a full flash page to permanently store the parameters. It also uses a RAM cache
 * limiting the number of the flash accesses. Most of the operations are realized in the cache.
 *
 * The manager support up to 5 parameter types.
 * - deprecated: Means that the parameter is nor more used by the system.
 * - integer: Signed integer value on 32 bit
 * - float: Floating point single precision.
 * - string: Ascii string, NULL terminated
 * - byte-array: Array of hexadecimal bytes.
 *
 * Each configuration parameter is stored in a descriptor typed srv_config_param_descriptor_t.
 * While all basic types (deprecated, integer and float) have their value directly stored in the descriptor,
 * the strings and byte-arrays have their value in another area pointed by the field value.ascii or value.barray.
 * The max size of strings (including the trailer NULL byte) and byte arrays is 32.
 *
 * The function srv_config_param_parse_value is able to parse an ascii string and determine the type based on
 * the following syntax:
 * - A value starting with '"' is detected as an ASII string.
 * - A value starting with '{' is considered a a byte array. Each values must be in hexadecimal (without the 0x prefix) and separated by a comma
 * - A value containing '.' is considered as a float. Scientist syntax (eg 1.45E-2) is accepted.
 * - Values not matching the above criteria are considered as integer. Note that the 0x prefix can be used to express hexadecimal numbers.
 *
 * \note    Write operations should be limited to the strict minimum to avoid deteriorating the underlying storage medium, notably FLASH memory
 *          which is only guaranteed for 10000 erase/write cycles.
 *
 * Copyright (C) 2023, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>


/*!
 * \defgroup srv_cfg_param Configuration parameter management service
 *
 * \addtogroup srv_cfg_param
 * @{
 *
 */

#define SRV_CONFIG_MAX_NUMBER_OF_PARAMETERS	300	//!< Max number of parameters
#define SRV_CONFIG_STR_BYTE_ARRAY_MAX_SIZE  32	//!< Max size of byte array and Ascii string

/*!
 * \brief Returned result of the API
 */
typedef enum {
	srv_config_result_success,      //!< All good
	srv_config_result_invalid_page_addr,	//!< The flash page provided in the init function is not 4Ko aligned.
	srv_config_result_not_formated,	//!< The flash page is not formated.
	srv_config_result_error,   		//!< Generic error
	srv_config_result_not_init,		//!< Not initialized
	srv_config_result_bad_pointer,   //!< Pointer is incorrect for a string or a b-array.
	srv_config_result_too_much_parameters,   //!< Too much parameters
	srv_config_result_string_barray_area_overflow,     //!< too much String or bytes array. Storage overflow
	srv_config_result_param_too_long, //!< String or bytes array length exceeds the max.
	srv_config_result_param_type_mismatch, //!< Type has changed via the write_param command
	srv_config_result_param_not_found,    //!< Parameter not found
	srv_config_result_param_invalid_value, //!< Value not valid
	srv_config_result_flash_write_error, //!< Failure in writing to flash
	srv_config_result_count,		 //!< Number of results
} srv_config_result_t;

/*!
 * \brief Type of managed parameters
 */
typedef enum {
	srv_config_param_type_deprecated = 0,	//!< This parameter has been deprecated
	srv_config_param_type_int32,			//!< Signed integer on 32 bits
	srv_config_param_type_float,			//!< Single precision float number (size 4 bytes)
	srv_config_param_type_str,			//!< ASCII string null terminated (max 32 bytes including the NULL)
	srv_config_param_type_byte_array		//!< Stream of bytes (max 32 bytes)
} srv_config_param_type_t;

/*!
 * \brief Value of a parameter
 */
typedef union {
    int32_t integer;				//!< Signed integer
    float decimal;					//!< Floating point
    char* ascii;					//!< Pointer to an ASII char (NULL terminated)
    uint8_t* barray;				//!< Pointer to an array of bytes
} srv_config_param_value_t;

/*!
 * \brief Parameter descriptor
 *
 * \warning Do not change the order
 */
typedef struct {
	struct {
		uint16_t identifier;    //!< Unique parameter identifier
		uint8_t type;           //!< Value type refer to srv_config_param_type_t
		uint8_t length;         //!< string length of the barray value
	} descriptor;			//!< Descriptor associated to the parameter
    srv_config_param_value_t value;  //!< Parameter value
} __attribute__((__packed__)) srv_config_param_descriptor_t;


/*!
 * \brief Version of the parameters stored in the flash
 */
typedef struct {
	uint8_t major;			//!< Major number
	uint8_t minor;			//!< Minor number
	uint8_t iteration;		//!< Iteration number
	uint8_t user;			//!< Free for use at the user side
} srv_config_flash_version_t;

/*!
 * \brief State of the non-volatile memory
 */
typedef enum {
	srv_config_flash_state_unknown = 0,	//!< The flash is not formated. It could be erased
	srv_config_flash_state_erased,			//!< The flash is erased
	srv_config_flash_state_formated,		//!< The flash is formated
	srv_config_flash_state_count			//!< Number of states
} srv_config_flash_state_t;

/*!
 * \brief general Information structure
 */
typedef struct {
	srv_config_flash_version_t version;		//!< Parameter set version identifier
	uint32_t crc;							//!< CRC of the whole parameters
	uint32_t nb_params;						//!< Number of parameters
	srv_config_flash_state_t state;			//!< State of the non-volatile memory
	uint8_t* base_address;					//!< Base address of the flash
	uint32_t str_barray_used_size;			//!< Used size for strings and byte-arrays
	uint32_t str_barray_free_size;			//!< Free size for strings and byte-arrays
	bool	in_sync;						//!< Indicate whether the FLASH and the cache are synchronized.
} srv_config_flash_info_t;


/*!
 * \fn srv_config_result_t srv_config_init(uint8_t* flash_config_page_addr)
 *
 * \brief Initialize the manager
 *
 * \param flash_config_page_addr Base address of the flash page used for the manager
 *
 * \return  The operation status
 *
 * \note Usually the flash_config_page_addr is provided by the linker script. It must
 * be aligned on a flash page (4 k bytes)
 */
srv_config_result_t srv_config_init(uint8_t* flash_config_page_addr);

/*!
 * \fn srv_config_result_t srv_config_get_info(srv_config_flash_info_t* info)
 *
 * \brief Retrieve the general information
 *
 * \param info Pointer to the area where the information will be copied
 *
 * \return  The operation status
 */
srv_config_result_t srv_config_get_info(srv_config_flash_info_t* info);

/*!
 * \fn srv_config_result_t srv_config_erase_all(void)
 *
 * \brief Erase the configuration. The flash will not be formated.
 *
 * \return  The operation status
 */
srv_config_result_t srv_config_erase_all(void);

/*!
 * \fn srv_config_result_t srv_config_check_and_do_crc(const srv_config_param_descriptor_t* param_list, uint16_t nb_param, uint32_t* crc)
 *
 * \brief Check the configuration and calculate its CRC
 *
 * \param param_list List of parameters to be be checked.
 * \param nb_param Number of parameters in the list
 * \param crc Calculated CRC
 *
 * \return  The operation status
 */
srv_config_result_t srv_config_check_and_do_crc(const srv_config_param_descriptor_t* param_list, uint16_t nb_param, uint32_t* crc);

/*!
 * \fn srv_config_result_t srv_config_format_and_init(const srv_config_param_descriptor_t* param_list, uint16_t nb_param, srv_config_flash_version_t* version, bool keep_deprecated)
 *
 * \brief Format the flash and write the parameters in both cache and flash
 *
 * \param param_list List of parameters to be be checked.
 * \param nb_param Number of parameters in the list
 * \param version Version of the parameter set
 * \param keep_deprecated  True if the deprecated parameters should be kept.
 *
 * \return The operation status
 */
srv_config_result_t srv_config_format_and_init(const srv_config_param_descriptor_t* param_list, uint16_t nb_param, srv_config_flash_version_t* version, bool keep_deprecated);

/*!
 * \brief Dump all parameters either in flash or in cache
 *
 * \param param_list List of parameters inside the flash
 * \param nb_param Number of parameters in the list inside the flash
 * \param version Version of the parameters inside the flash
 * \param crc CRC of the parameter list
 * \param cfg_in_flash True if the expected dump is the flashed one. False if it should come from the RAM cache
 *
 * \return The operation status
 */
srv_config_result_t srv_config_dump_all_params(const srv_config_param_descriptor_t** param_list, uint16_t* nb_param, uint32_t* crc, const srv_config_flash_version_t** version, bool cfg_in_flash);

/*!
 * \fn srv_config_result_t srv_config_save(srv_config_flash_version_t* version, bool keep_deprecated)
 *
 * \brief Save the configuration in flash
 *
 * \param version The version of the new configuration. If NULL, the old version is kept.
 * \param keep_deprecated Indicate whether the deprecated (deleted) parameters should be kept. Note that
 *        in the case where a deprecated parameter was a string or a byte-array, its associated value storage
 *        is freed regardless the value of keep_deprecated.
 * \return The operation status
 *
 * \warning Saving the configuration to often may damage the flash. It is recommended to fully complete the
 * 			configuration in the cache, then flash (save) at then end.
 */
srv_config_result_t srv_config_save(srv_config_flash_version_t* version, bool keep_deprecated);

/*!
 * \fn srv_config_result_t srv_config_param_get(uint16_t identifier, const srv_config_param_descriptor_t** param)
 *
 * \brief Get (Read) a configuration parameter.
 *
 * \details The function returns the matching cache entry. If you write any field in the descriptor, you have
 *          to actually write it using the srv_config_param_set function. In any case you must not change the
 *          type of the value.
 *
 * \param identifier The parameter identifier
 * \param param Handle to retrieve the internal copy.
 *
 * \return  The operation status
 */
srv_config_result_t srv_config_param_get(uint16_t identifier, const srv_config_param_descriptor_t** param);

/*!
 * \fn srv_config_result_t srv_config_param_set(uint16_t identifier, const srv_config_param_descriptor_t* param)
 *
 * \brief Set (Write) a configuration parameter.
 *
 * \details The cache will be updated while the flash won't be. To flash the new configuration, use the srv_config_save
 *          function.
 *
 * \param[in] 	identifier The parameter identifier
 * \param[out]	param Point to the parameter to be written. All fields must be filled.
 *
 * \return  The operation status
 */
srv_config_result_t srv_config_param_set(uint16_t identifier, const srv_config_param_descriptor_t* param);

/*!
 * \fn srv_config_result_t  srv_config_param_delete(uint16_t identifier)
 *
 * \brief Remove (delete) a configuration parameter.
 *
 * \details Only the cache will be affected. The parameter type will be set to deprecated. To fully remove
 *          the parameter you should save the configuration with the keep_deprecated set to false.
 *
 * \param identifier The parameter identifier
 *
 * \return The operation status
 */
srv_config_result_t srv_config_param_delete(uint16_t identifier);

/*!
 * \fn srv_config_result_t srv_config_param_new(uint16_t identifier, const srv_config_param_descriptor_t* param, bool override_deprecated)
 *
 * \brief Create a new configuration parameter.
 *
 * \details Only the cache will be affected. If override_deprecated is set, a previously deprecated parameter
 * 			will be used (if any). Otherwise, a new parameter is actually created. To update also the flash,
 * 			the configuration should be saved.
 *
 * \param identifier The parameter identifier
 * \param param The parameter descriptor filled. Note that identifier field in this structure is not required
 * 		  as it will be overwritten by the identifier calling parameter.
 * \param override_deprecated Boolean indicating whether the deprecated parameter descriptor can be reused to
 * 		   this one. A True value enable the reuse.
 *
 * \return The operation status
 */
srv_config_result_t srv_config_param_new(uint16_t identifier, const srv_config_param_descriptor_t* param, bool override_deprecated);

/*!
 * \fn srv_config_result_t srv_config_param_parse_value(char* input_str, const srv_config_param_descriptor_t **param)
 *
 * \brief Parse an ASCII string and format the provided parameter descriptor accordingly.
 *
 * \details The function determine the type and the value of the parameter.
 *
 * \param input_str ASCII string containing the value
 * \param param Handle on the internal descriptor used to store the internal descriptor copy provided by the function.
 * 		  Only the value, the type and the length of the descriptor are set by the function.
 *
 * \return The operation status
 *
 * \note This function does not feed the identifier field of the descriptor. It does not actually create a descriptor
 * 		 in the cache. It's just a copy.
 */
srv_config_result_t srv_config_param_parse_value(char* input_str, const srv_config_param_descriptor_t **param);

/*!
 * \fn const char* srv_config_param_type_to_str(srv_config_param_type_t type)
 *
 * \brief Helper that converts a type in its associated string equivalent
 *
 * \param type Parameter type for which the ASCII string equivalent is expected.
 *
 * \return The ASCII string equivalent
 */
const char* srv_config_param_type_to_str(srv_config_param_type_t type);

/*!
 * \fn const char* srv_config_result_to_str(srv_config_result_t result)
 *
 * \brief Helper that converts a result in its associated string equivalent
 *
 * \param result Function result for which the ASCII string equivalent is expected.
 *
 * \return The ASCII string equivalent
 */
const char* srv_config_result_to_str(srv_config_result_t result);

/*! @}*/
#ifdef __cplusplus
}
#endif

