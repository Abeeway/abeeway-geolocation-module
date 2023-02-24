/**
 ******************************************************************************
 * @file    app_custom_srvc.c
 * @author  Hamza
 * @brief   BLE Custom Services Application Management
 *          This file define for a custom service :
 *          - Event handler function
 *          - Function to set the configuration of a service
 *          - Function to initialize the characteristics value
 *          - Update characteristic value API
 *          In this file we implemented two custom services as example,
 *          those services could be removed and replaced by customer services
 *****************************************************************************
 * @attention
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 ******************************************************************************
 */


#include "common_blesvc.h"
#include "aos_ble_common.h"
#include "app_custom_srvc.h"



/*!
 * \struct app_custom_char_handle_t
 *
 * \brief This struct define a pointer for each custom characteristic created
 */
typedef struct {
	uint16_t *cts_current_time_char_handle;       //!< Pointer to the current time characteristic handler
	uint16_t *hts_temperature_meas_char_handle;   //!< Pointer to the temperature measurement characteristic handler
} app_custom_char_handle_t;

app_custom_char_handle_t app_custom_ctx = {0};


/***** Event Handler Functions *****/

/*!
 * \fn SVCCTL_EvtAckStatus_t cts_event_handler(void *event)
 *
 * \brief HCI Event Handler for Current Time service event
 *
 * \param  event: Address of the buffer holding the Event
 *
 * \return Ack: Return whether the Event has been managed or not
 */
SVCCTL_EvtAckStatus_t cts_event_handler(void *event)
{
	SVCCTL_EvtAckStatus_t ret = 0;
	hci_event_pckt *event_pckt;
	evt_blecore_aci *blecore_evt;
	aci_gatt_attribute_modified_event_rp0 *attribute_modified;
	aci_gatt_read_permit_req_event_rp0 *attribute_read;

	ret = SVCCTL_EvtNotAck;
	event_pckt = (hci_event_pckt *)(((hci_uart_pckt*)event)->data);

	switch(event_pckt->evt) {
	case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE:
	{
		blecore_evt = (evt_blecore_aci*)event_pckt->data;
		switch(blecore_evt->ecode) {
		case ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE:
			attribute_modified = (aci_gatt_attribute_modified_event_rp0*)blecore_evt->data;
			if (attribute_modified->Attr_Handle == (*(app_custom_ctx.cts_current_time_char_handle) + CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET)) {
				ret = SVCCTL_EvtAckFlowEnable;
				if (attribute_modified->Attr_Data[0] & COMSVC_Notification) {
					APP_DBG_MSG("CURRENT TIME NOTIFICATION Enabled\n");
				} else {
					APP_DBG_MSG("CURRENT TIME NOTIFICATION Disabled\n");
				}
			}
			break;

		case ACI_GATT_READ_PERMIT_REQ_VSEVT_CODE:
			attribute_read = (aci_gatt_read_permit_req_event_rp0*)blecore_evt->data;
			if (attribute_read->Attribute_Handle == (*(app_custom_ctx.cts_current_time_char_handle) + CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET)) {
				ret = SVCCTL_EvtAckFlowEnable;
				aci_gatt_allow_read(attribute_read->Connection_Handle);
				APP_DBG_MSG("CURRENT TIME NOTIFICATION READ REQ\n");
			}
			break;

		case ACI_GATT_WRITE_PERMIT_REQ_VSEVT_CODE:
			break;

		default:
			break;
		}
	}
	break;

	default:
		break;
	}

	return(ret);
}

/*!
 * \fn SVCCTL_EvtAckStatus_t hts_event_handler(void *event)
 *
 * \brief HCI Event Handler for Health Thermometer service event
 *
 * \param  event: Address of the buffer holding the Event
 *
 * \return Ack: Return whether the Event has been managed or not
 */
SVCCTL_EvtAckStatus_t hts_event_handler(void *event)
{
	SVCCTL_EvtAckStatus_t ret = 0;
	hci_event_pckt *event_pckt;
	evt_blecore_aci *blecore_evt;
	aci_gatt_attribute_modified_event_rp0 *attribute_modified;

	ret = SVCCTL_EvtNotAck;
	event_pckt = (hci_event_pckt *)(((hci_uart_pckt*)event)->data);

	switch(event_pckt->evt) {
	case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE:
		blecore_evt = (evt_blecore_aci*)event_pckt->data;
		switch(blecore_evt->ecode) {
		case ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE:
			attribute_modified = (aci_gatt_attribute_modified_event_rp0*)blecore_evt->data;
			if (attribute_modified->Attr_Handle == (*(app_custom_ctx.hts_temperature_meas_char_handle) + CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET)) {
				ret = SVCCTL_EvtAckFlowEnable;
				if (attribute_modified->Attr_Data[0] & COMSVC_Indication) {
					APP_DBG_MSG("HTS_MEASUREMENT_IND_ENABLED_EVT\n");
				} else {
					APP_DBG_MSG("HTS_MEASUREMENT_IND_DISABLED_EVT\n");
				}
			}
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return(ret);
}

/***** Services Configuration Functions *****/
/**
 *          My_Custom_Server
 *
 * Max_Attribute_Records = 1 + 2*no_of_char + 1*no_of_char_with_notify_or_indicate_property + 1*no_of_char_with_broadcast_property
 * Example: service_max_attribute_record =
 *                                1 for HTS_Server +
 *                                2 for Temperature_Char +
 *                                1 for Temperature_Char configuration descriptor (indication)
 *                              = 4
 */

/*!
 * \fn static void _cts_srvc_conf_set(ble_custom_srvc_init_data_t* custom_srvc_init_data)
 *
 * \brief CTS service configuration
 *
 * \param  cts_srvc_init_data: pointer to CTS services data structure
 *
 * \return Ack: Return whether the Event has been managed or not
 */
static void _cts_srvc_conf_set(ble_custom_srvc_init_data_t* cts_srvc_init_data)
{
	/**
	 *  Current Time Service
	 *
	 * service_max_attribute_record = 1 for health thermometer service +
	 *                                2 for temperature measurement +
	 *                                1 for temperature measurement notification
	 */
	cts_srvc_init_data->service.service_uuid.uuid_type = UUID_TYPE_16;
	cts_srvc_init_data->service.service_uuid.uuid.uuid_16 = CURRENT_TIME_SERVICE_UUID;
	cts_srvc_init_data->service.service_type = PRIMARY_SERVICE;
	cts_srvc_init_data->service.num_attr_rec = 4;

	cts_srvc_init_data->event_handler = cts_event_handler;

	cts_srvc_init_data->char_count = 1; // Char count could not exceed MAX_CHAR_COUNT

	cts_srvc_init_data->characteristic[cts_current_time_char].char_uuid.uuid_type = UUID_TYPE_16;
	cts_srvc_init_data->characteristic[cts_current_time_char].char_uuid.uuid.uuid_16 = CURRENT_TIME_CHAR_UUID;
	cts_srvc_init_data->characteristic[cts_current_time_char].char_value_length = sizeof(cts_exact_time_256_t);
	cts_srvc_init_data->characteristic[cts_current_time_char].char_properties = CHAR_PROP_READ | CHAR_PROP_NOTIFY;
	cts_srvc_init_data->characteristic[cts_current_time_char].security_permissions = ATTR_PERMISSION_ENCRY_READ | ATTR_PERMISSION_ENCRY_WRITE;
	cts_srvc_init_data->characteristic[cts_current_time_char].gatt_evt_mask = GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP;
	cts_srvc_init_data->characteristic[cts_current_time_char].enc_key_size = 10;
	cts_srvc_init_data->characteristic[cts_current_time_char].is_variable = CHAR_VALUE_LEN_CONSTANT;

	app_custom_ctx.cts_current_time_char_handle = &cts_srvc_init_data->characteristic[cts_current_time_char].char_handle;
}

/*!
 * \fn static void _hts_srvc_conf_set(ble_custom_srvc_init_data_t* custom_srvc_init_data)
 *
 * \brief HTS service configuration
 *
 * \param  hts_srvc_init_data: pointer to HTS service data structure
 *
 * \return Ack: Return whether the Event has been managed or not
 */
static void _hts_srvc_conf_set(ble_custom_srvc_init_data_t* hts_srvc_init_data)
{
	/**
	 *  Add Health Thermometer Service
	 *
	 * service_max_attribute_record = 1 for health thermometer service +
	 *                                3 for temperature measurement (2 + 1 desc(indication))
	 */
	hts_srvc_init_data->service.service_uuid.uuid_type = UUID_TYPE_16;
	hts_srvc_init_data->service.service_uuid.uuid.uuid_16 = HEALTH_THERMOMETER_SERVICE_UUID;
	hts_srvc_init_data->service.service_type = PRIMARY_SERVICE;
	hts_srvc_init_data->service.num_attr_rec = 4;
	hts_srvc_init_data->event_handler = hts_event_handler;

	hts_srvc_init_data->char_count = 1; // Char count could not exceed MAX_CHAR_COUNT

	hts_srvc_init_data->characteristic[hts_temperature_meas_char].char_uuid.uuid_type = UUID_TYPE_16;
	hts_srvc_init_data->characteristic[hts_temperature_meas_char].char_uuid.uuid.uuid_16 = TEMPERATURE_MEASUREMENT_CHAR_UUID;
	hts_srvc_init_data->characteristic[hts_temperature_meas_char].char_value_length = 1 + 1 + 4;       /**< flags + Temp value*/
	hts_srvc_init_data->characteristic[hts_temperature_meas_char].char_properties = CHAR_PROP_INDICATE;
	hts_srvc_init_data->characteristic[hts_temperature_meas_char].security_permissions = ATTR_PERMISSION_NONE;
	hts_srvc_init_data->characteristic[hts_temperature_meas_char].gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS;
	hts_srvc_init_data->characteristic[hts_temperature_meas_char].enc_key_size = 10;
	hts_srvc_init_data->characteristic[hts_temperature_meas_char].is_variable = CHAR_VALUE_LEN_VARIABLE;

	app_custom_ctx.hts_temperature_meas_char_handle = &hts_srvc_init_data->characteristic[hts_temperature_meas_char].char_handle;
}

/*!
 * \fn void app_custom_srvc_conf_set(ble_custom_srvc_init_data_t* custom_srvc_init_data)
 *
 * \brief Set configuration for all services
 *
 * \param  custom_srvc_init_data: pointer to the services data structure
 */
void app_custom_srvc_conf_set(ble_custom_srvc_init_data_t* custom_srvc_init_data)
{
	_cts_srvc_conf_set(&custom_srvc_init_data[custom_service_cts]);

	_hts_srvc_conf_set(&custom_srvc_init_data[custom_service_hts]);
}

/***** Characteristics Value Initialization Functions *****/

/*!
 * \fn static void _cts_char_data_init(ble_custom_srvc_init_data_t* srvc_info)
 *
 * \brief Initialize CTS service characteristics
 *
 * \param  srvc_info: pointer to CTS service data structure
 */
static void _cts_char_data_init(ble_custom_srvc_init_data_t* srvc_info)
{
	cts_exact_time_256_t time = {
			.year = 2022,
			.month = 10,
			.day = 17,
			.hours = 16,
			.minutes = 23,
			.seconds = 00,
			.day_of_week = 1,
			.fractions256 = 250
	};
	ble_char_update_data_t char_data;
	char_data.service_handle = srvc_info->service.service_handle;
	char_data.char_handle = srvc_info->characteristic[cts_current_time_char].char_handle;
	char_data.value_offset = 0;
	char_data.char_value_length = sizeof(cts_exact_time_256_t);
	char_data.char_value = (uint8_t*)&time;

	custom_srvc_update_char(&char_data);
}

/*!
 * \fn static void _hts_char_data_init(ble_custom_srvc_init_data_t* srvc_info)
 *
 * \brief Initialize HTS service characteristics
 *
 * \param  srvc_info: pointer to HTS service data structure
 */
static void _hts_char_data_init(ble_custom_srvc_init_data_t* srvc_info)
{
	/*
	 * The Temperature measurement char is in indication mode only,
	 * there is no need to initialize the char data
	 * A BLE CLI command added to test the temperature indication
	 * > ble service temperature_ind
	 */
}

/*!
 * \fn void app_custom_srvc_data_init(ble_custom_srvc_init_data_t* srvc_info)
 *
 * \brief Initialize services characteristics value
 *
 * \param  srvc_info: pointer to the services data structure
 */
void app_custom_srvc_data_init(ble_custom_srvc_init_data_t* srvc_info)
{
	_cts_char_data_init(&srvc_info[custom_service_cts]);
	_hts_char_data_init(&srvc_info[custom_service_hts]);
}

/***** Update characteristic value API *****/

/*!
 * \fn void app_custom_srvc_update_char(ble_char_update_data_t *char_data)
 *
 * \brief Update services characteristics value
 *
 * \param  srvc_info: pointer to the characteristic update data structure
 */
void app_custom_srvc_update_char(ble_char_update_data_t *char_data)
{
	custom_srvc_update_char(char_data);
}
