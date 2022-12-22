/*!
 * \file app_custom_srvc.h
 *
 * @brief   BLE Custom Services Application Management
 *          This file define for a custom service :
 *          - Event handler function
 *          - Function to set the configuration of a service
 *          - Function to initialize the characteristics value
 *          - Update characteristic value API
 *          In this file we implemented two custom services as example,
 *          those services could be removed and replaced by customer services
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


#include "svc_ctl.h"

/*!
 * \defgroup app_custom_srvc Custom BLE services application management
 *
 * \addtogroup app_custom_srvc
 * @{
 */


/*!
 * \enum BLE_custom_services
 *
 * \brief Implemented BLE custom services
 */
enum {
	custom_service_cts = 0,              //!< Current Time Service correspond to CUSTOM_1_SERVICE in ble_services_bit_mask
	custom_service_hts,                  //!< Health Thermometer Service correspond to CUSTOM_2_SERVICE in ble_services_bit_mask
	custom_service_count,                //!< max number of services should not exceed MAX_CUSTOM_SERVICES_COUNT
};

/*!
 * \enum cts_char
 *
 * \brief Implemented BLE CTS characteristics
 */
enum {
	cts_current_time_char = 0,           //!< Current time characteristic
	cts_char_count                       //!< Max number of characteristics should not exceed MAX_CHAR_COUNT
};


/*!
 * \enum hts_char
 *
 * \brief Implemented BLE HTS characteristics
 */
enum {
	hts_temperature_meas_char = 0,       //!< Temperature measurement characteristic
	hts_char_count                       //!< Max number of characteristics should not exceed MAX_CHAR_COUNT
};

/*!
 * \struct cts_exact_time_256_t
 *
 * \brief Value type of the characteristic cts_current_time_char
 */
typedef struct {
	uint16_t year;
	uint8_t  month;
	uint8_t  day;
	uint8_t  hours;
	uint8_t  minutes;
	uint8_t  seconds;
	uint8_t  day_of_week;
	uint8_t  fractions256;
} cts_exact_time_256_t;

/*!
 * \enum hts_tm_flags
 *
 * \brief Temperature measurement flags
 */
enum {
	NO_FLAGS = 0,
	VALUE_UNIT_FAHRENHEIT = (1<<0),
	SENSOR_TIME_STAMP_PRESENT = (1<<1),
	SENSOR_TEMPERATURE_TYPE_PRESENT = (1<<2),
};

/*!
 * \enum hts_temperature_type
 *
 * \brief Measured Temperature Type
 */
enum {
	tt_armpit = 1,
	tt_body = 2,
	tt_ear = 3,
	tt_finger = 4,
	tt_gastro_intestinal_Tract = 5,
	tt_mouth = 6,
	tt_rectum = 7,
	tt_toe = 8,
	tt_tympanum = 9
};

/*!
 * \fn void app_custom_srvc_conf_set(ble_custom_srvc_init_data_t* custom_srvc_init_data)
 *
 * \brief Set configuration for all services
 *
 * \param  custom_srvc_init_data: pointer to the services data structure
 */
void app_custom_srvc_conf_set(ble_custom_srvc_init_data_t* custom_srvc_init_data);

/*!
 * \fn void app_custom_srvc_data_init(ble_custom_srvc_init_data_t* srvc_info)
 *
 * \brief Initialize services characteristics value
 *
 * \param  srvc_info: pointer to the services data structure
 */
void app_custom_srvc_data_init(ble_custom_srvc_init_data_t *app_info);

/*!
 * \fn void app_custom_srvc_update_char(ble_char_update_data_t *char_data)
 *
 * \brief Update services characteristics value
 *
 * \param  srvc_info: pointer to the characteristic update data structure
 */
void app_custom_srvc_update_char(ble_char_update_data_t *char_data);

/*! @}*/
#ifdef __cplusplus
}
#endif
