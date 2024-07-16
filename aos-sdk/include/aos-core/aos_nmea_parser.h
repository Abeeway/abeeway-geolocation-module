/*!
 * \file aos_nmea_parser.h
 *
 * @\brief NMEA parser/driver
 *
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "aos_system.h"
#include "aos_gnss_common.h"
#include "aos_nmea_parser.h"

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_nmea_parser NMEA parser
 *
 * \addtogroup aos_nmea_parser
 * @{
 */

/*
 *************************************************
 * NMEA message processor
 *************************************************
 */

/*!
 * \def NMEA_MAX_SAT_FOR_FIX
 *
 * \brief Maximum number of satellites supported by the NMA parser
 */
#define NMEA_MAX_SAT_FOR_FIX 12

/*!
 * \enum nmea_msg_type_t
 *
 * \brief Usual NME Message type
 */
typedef enum {
    nmea_type_gga = 0,		//!< GGA message type
    nmea_type_gsa,			//!< GSA message type
    nmea_type_gsv,			//!< GSV message type
    nmea_type_rmc,			//!< RMS message type
    nmea_type_accuracy,		//!< Accuracy message type
    nmea_type_vtg,			//!< VTG message type
    nmea_type_prn,			//!< PRN message type
    nmea_type_prn_done,		//!< End of PRN message
    nmea_type_ack,			//!< ACK message
    nmea_type_vendor,		//!< Vendor specific message
    nmea_type_unknown,		//!< Unknown message
} nmea_msg_type_t;

/*!
 * \enum nmea_parsing_res_t
 *
 * \brief NMEA parser result
 */
typedef enum {
	nmea_parsing_res_success,		//!< Success
	nmea_parsing_res_discard,		//!< Not processed sentence
	nmea_parsing_res_error,			//!< Error
	nmea_parsing_res_not_populated,	//!< An cycle has not been fully populated.
	nmea_parsing_ack_base_status,								//<! Base for NMEA statuses
	nmea_parsing_ack_cmd_error = nmea_parsing_ack_base_status,	//<! Invalid command (0)
	nmea_parsing_ack_cmd_unknown,								//<! unsupported command
	nmea_parsing_ack_cmd_ok_no_action,							//!< Command success but no action taken (or no answer)
	nmea_parsing_ack_cmd_ok_action_ok,							//!< Command success and action taken
	nmea_parsing_ack_cmd_last									//<! last status of the ack
} nmea_parsing_res_t;


/*!
 * \struct nmea_parsing_cb_param_ack_t
 *
 * \brief Callback parameter used once the NMEA ACK has been received
 */
typedef struct {
	uint16_t tx_len;		//!< Transmit length
	uint8_t* tx_buf;		//!< Transmit data
} nmea_parsing_cb_param_ack_t;

/*!
 * \struct nmea_parsing_cb_param_t
 *
 * \brief callback information used for the parsing
 */
typedef struct {
	union {
		nmea_parsing_cb_param_ack_t p_ack;				//!< Ack parameter
		aos_gnss_satellite_prn_report_t* prn_report;	//!< Pseudo range report
	};
} nmea_parsing_cb_param_t;

/*!
 * \brief generic parsing callback function
 *
 * \param info pre-parsed message
 * \param callback information
 */
typedef nmea_parsing_res_t (*nmea_parsing_cb_t)(nmea_parse_msg_t* info, nmea_parsing_cb_param_t* param);

/*!
 * \struct nmea_dictionary_word_t
 *
 * \brief Word definition entry used by the NMEA dictionary
 *
 * \remark: last field must have a null name field.
 */
typedef struct {
	const char *name;			//!< Message type in string format
	nmea_msg_type_t type;		//!< Message type: nmea_type_vendor if the NMEA processor is not expected to process it
	nmea_parsing_cb_t fnct;		//!< Function to call: NULL if the NMEA processor is aware of the message
} nmea_dictionary_word_t;


/*!
 * \struct nmea_dictionary_t
 *
 * \brief Dictionary to parse the address field.
 *
 */
typedef struct {
	const char *talker_id;						//!< talker_id Talker identifier, eg PMTK
	const nmea_dictionary_word_t* dictionary;	//!< dictionary List of dictionary words.
} nmea_dictionary_t;


//  API
/*!
 * \fn char* nmea_extract_hex(char* str, int8_t nb_digits, uint32_t* value)
 *
 * \brief Extracts a hexadecimal integer
 *
 * \param str - Input string (null terminated)
 * \param nb_digits: Number of digits to read. Passing -1 forces the function to read all digits
 * \param value: output value
 *
 * \return The position of the first unread bytes. NULL if error.
 *
 * \note: lower/upper cases hexa supported.
 */
char* nmea_extract_hex(char* str, int8_t nb_digits, uint32_t* value);

/*!
 * \fn char* nmea_extract_uint(char* str, int8_t nb_digits, uint32_t* value)
 *
 * \brief Extract an unsigned decimal integer
 *
 * \param str - Input string (null terminated)
 * \param nb_digits Number of digits to read. Passing -1 forces the function to read all digits
 * \param value Output value
 *
 * \return The position of the first unread bytes. NULL if error.
 */
char* nmea_extract_uint(char* str, int8_t nb_digits, uint32_t* value);

/*!
 * \fn char* nmea_extract_int(char* str, int8_t nb_digits, int32_t* value)
 *
 * \brief Extract a signed decimal integer
 *
 *
 * \param str Input string (null terminated)
 * \param nb_digits Number of digits to read. Passing -1 forces the function to read all digits.
 * \param value  Output value
 *
 * \return The position of the first unread bytes. NULL if error.
 */
char* nmea_extract_int(char* str, int8_t nb_digits, int32_t* value);

/*!
 * \fn int64_t nmea_extract_float_as_int(char* str, uint8_t nb_dec_digits)
 *
 * \brief Extract a float number and convert it as an integer
 *
 * \param str Input string (null terminated)
 * \param nb_dec_digits Number of decimal digits we want to extract
 *
 * \return the converted float
 *
 * \remark The output value is multiplied by 10^nb_dec_digits.
 *         Example1: value 123.1, nb_dec_digits = 1, gives a value of 1231
 *                   Value is expressed in 1/10.
 *
 *         Example2: value 123.45, nb_dec_digits = 2, gives a value of 12345
 *                   Value is expressed in 1/100.
 *
 *         Example3: value 123.45, nb_dec_digits = 1, gives a value of 1234
 *                   Value is expressed in 1/10.
 *
 *         Example4: value 123.4, nb_dec_digits = 2, gives a value of 12340
 *                   Value is expressed in 1/100.
 *
 *         Example5: value 123, nb_dec_digits = 2, gives a value of 12300
 *                   Value is expressed in 1/100.
 *
 */
int64_t nmea_extract_float_as_int(char* str, uint8_t nb_dec_digits);


/*!
 * \fn char* nmea_get_field(nmea_parse_msg_t* info, uint8_t field_idx)
 *
 * \brief Extract a NMEA field according to the provided index.
 *
 * \param info Parsed message information
 * \param field_idx Field index to be extracted
 *
 * \return The null terminated string of the matched field. NULL if error
 *
 * \remark The index starts at 0, which is the message type (eg: GPGSA)
 */
char* nmea_get_field(nmea_parse_msg_t* info, uint8_t field_idx);


/*!
 * \fn aos_gnss_counters_t* nmea_get_counters(void)
 *
 * \brief Return the pointer to the NMEA counters (statistics)
 *
 * \return The pointer to the counters.
 */
aos_gnss_counters_t* nmea_get_counters(void);

/*!
 * \fn void nmea_clear_counters(void)
 *
 * \brief Clear the NMEA counters
 */
void nmea_clear_counters(void);


/*!
 * \fn void nmea_delayed_rx_process(void);
 *
 * \brief Callback called by the GNSS managed from its own thread to perform further RX process.
 * The NMEA parser had previously triggered the GNSS manager to be called back
 */
void nmea_delayed_rx_process(void);


/*!
 * \fn nmea_msg_type_t nmea_get_msg_type(nmea_parse_msg_t* info, aos_gnss_constellation_t *constellation)
 *
 * \brief Return the type of the NMEA sentence and the type of constellation
 *
 * \param info Parsed message information
 * \param constellation Type of constellation.
 *
 * \return the NMEA sentence type
 */
nmea_msg_type_t nmea_get_msg_type(nmea_parse_msg_t* info, aos_gnss_constellation_t *constellation);


/*!
 * \fn  bool nmea_get_hex(nmea_parse_msg_t* info, uint8_t field_idx, uint32_t* value)
 *
 * \brief extract an unsigned hexadecimal number at given field index
 *
 * \param info - parsed message information
 * \param field_idx - field index
 * \param value: output value
 *
 * \return true if success
 */
bool nmea_get_hex(nmea_parse_msg_t* info, uint8_t field_idx, uint32_t* value);

/*!
 * \fn bool nmea_get_uint32(nmea_parse_msg_t* info, uint8_t field_idx, uint32_t* value)
 *
 * \brief Extract an unsigned decimal integer at given field index
 *
 * \param info Parsed message information
 * \param field_idx Field index
 * \param value Output value
 *
 * \return True if success, false otherwise.
 */
bool nmea_get_uint32(nmea_parse_msg_t* info, uint8_t field_idx, uint32_t* value);


/*!
 * \fn bool nmea_get_float_as_int(nmea_parse_msg_t* info, uint8_t field_idx,
 *			   uint8_t nb_dec_digits, int64_t* value);
 *
 * \brief  Extract a signed float at given field index
 *
 * \param info Parsed message information
 * \param field_idx Field index
 * \param nb_dec_digits Number of decimal digits we want to extract
 * \param value Output value
 *
 * \return true if success, false otherwise.
 *
 * \remark: The output value is multiplied by 10^nb_dec_digits.
 * Refer to the function nmea_extract_float_as_int for examples.
 */
bool nmea_get_float_as_int(nmea_parse_msg_t* info, uint8_t field_idx,
			   uint8_t nb_dec_digits, int64_t* value);

/*!
 * \fn bool nmea_get_coordinate(nmea_parse_msg_t* info, uint8_t field_idx,  bool latitude, int32_t *coordinate)
 *
 * \brief Extract the latitude or longitude in units of 0.0000001 degree.
 *
 * \param info Parsed message information
 * \param field_idx Field index
 * \param latitude True if latitude, false if longitude
 * \param coordinate Coordinate extracted in units of 0.0000001 degree
 *
 * \return true if success, false otherwise.
 */
bool nmea_get_coordinate(nmea_parse_msg_t* info, uint8_t field_idx,  bool latitude, int32_t *coordinate);

/*!
 * \fn bool nmea_get_xdop(nmea_parse_msg_t* info, uint8_t field_idx, uint16_t *xdop)
 *
 * \brief Extract PDOP, VDOP, HDOP
 *
 * \param info Parsed message information
 * \param field_idx Field index
 * \param xdop Extracted value. Unit: 0.01
 *
 * \return true if success, false otherwise.
 */
bool nmea_get_xdop(nmea_parse_msg_t* info, uint8_t field_idx, uint16_t *xdop);


/*!
 * \fn bool nmea_get_time(nmea_parse_msg_t* info, uint8_t field_idx, aos_gnss_utc_time_t *utc)
 *
 * \brief Extract the UTC time
 *
 * \param info Parsed message information
 * \param field_idx Field index
 * \param utc Extracted UTC time
 *
 * \return true if success, false otherwise.
 */
bool nmea_get_time(nmea_parse_msg_t* info, uint8_t field_idx, aos_gnss_utc_time_t *utc);


/*!
 * \fn bool nmea_get_date(nmea_parse_msg_t* info, uint8_t field_idx, aos_gnss_utc_time_t *utc)
 *
 * \brief Extract the UTC data
 *
 * \param info Parsed message information
 * \param field_idx Field index
 * \param utc: Extracted UTC date
 *
 * \return true if success, false otherwise.
 */
bool nmea_get_date(nmea_parse_msg_t* info, uint8_t field_idx, aos_gnss_utc_time_t *utc);


/*!
 * \fn char* nmea_dump_raw(nmea_parse_msg_t* info, char* buffer, uint16_t buf_len)
 *
 * \brief Restore the receive message in raw format as it was originally.
 *
 * \param info - parsed message information
 * \param buffer - buffer where to put the result
 * \param buf_len - Buffer length
 *
 * \return the buffer pointer if success, NULL otherwise.
 */
char* nmea_dump_raw(nmea_parse_msg_t* info, char* buffer, uint16_t buf_len);


/*
 *************************************************
 * NMEA Generic driver/NMEA Driver API
 *************************************************
 */
/*!
 * \fn aos_result_t nmea_send(const uint8_t *nmea, uint16_t len, aos_gnss_rqst_type_t type)
 *
 * \brief Send a NMEA message
 *
 * \param nmea NMEA message to send
 * \param len  NMEA message Length
 * \param type Request type
 *
 * \return Success or failure
 */
aos_result_t nmea_send(const uint8_t *nmea, uint16_t len, aos_gnss_rqst_type_t type);

/*!
 * \fn aos_result_t nmea_close(void)
 *
 * \brief Close the NMEA driver
 *
 * \return Success or failure
 */
aos_result_t nmea_close(void);

/*!
 * \brief Open the NMEA driver
 *
 * \param cb_fn User callback to receive event
 * \param cb_arg User argument to pass along the user callback. Opaque for the driver.
 * \param msgbuf Message buffer to be used by the NMEA driver
 * \param msgbuf_len Size of the message buffer
 * \param vendor_dict Vendor specific dictionary
 *
 * \return Success or failure
 */
aos_result_t nmea_open(aos_gnss_callback_t cb_fn, void *cb_arg, uint8_t *msgbuf, uint16_t msgbuf_len, const nmea_dictionary_t* vendor_dict);

/*!
 * \fn aos_result_t nmea_change_callback(aos_gnss_callback_t cb_fn, void *cb_arg);
 *
 * \brief Change the user callback and argument
 *
 * \param cb_fn New user callback to receive event
 * \param cb_arg New user argument to pass along the user callback. Opaque for the driver.
 *
 * \return Success or failure
 */
aos_result_t nmea_change_callback(aos_gnss_callback_t cb_fn, void *cb_arg);

/*!
 * \fn aos_result_t nmea_set_msg_mask(uint8_t msg_mask);
 *
 * \brief Set the message filtering mask
 *
 * \param msg_mask Message filter mask
 *
 * \return Success or failure
 */
aos_result_t nmea_set_msg_mask(uint8_t msg_mask);

/*!
 * \fn aos_result_t nmea_set_constellation(uint32_t constellation)
 *
 * \brief Set the constellation usable by the GNSS driver
 *
 * \param constellation Usable constellation bit_map (refer to AOS_GNSS_CFG_ENABLE_xxx)
 *
 * \return Success or failure
 */
aos_result_t nmea_set_constellation(uint32_t constellation);

/*!
 * \fn void nmea_delayed_rx_process(void);
 *
 * \brief Callback called by the GNSS managed from its own thread to perform further RX process.
 * The NMEA parser had previously triggered the GNSS manager to be called back.
 *
 * \warning USED ONLY BY THE GNSS SERVICE.
 */
void nmea_delayed_rx_process(void);


/*! @}*/
#ifdef __cplusplus
}
#endif
