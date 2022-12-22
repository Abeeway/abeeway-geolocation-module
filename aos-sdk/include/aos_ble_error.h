/*
 * aos_ble_error.h
 *
 *  Created on: Apr 6, 2022
 *      Author: hamza
 */

#pragma once


/*!
 * \enum ble_error
 *
 * \brief BLE application errors
 */
enum ble_error {
	BLE_SUCCESS = 0, 		/*!< The operation completed successfully */
	BLE_ERR_INIT,			/*!< Initialization failed */
	BLE_ERR_ALREADY,		/*!< Already initialized */
	BLE_ERR_INVALID_STATE,	/*!< Invalid state*/
	BLE_ERR_NOT_ENABLED,	/*!< Corresponding service not been initialized*/
	BLE_ERR_INVALID_ARG,	/*!< The parameter contains invalid data */
	BLE_ERR_NULL_ARG,		/*!< The parameter is NULL. */
	BLE_ERR_NOT_FOUND, 		/*!< Not found */
	BLE_ERR_STORAGE_WRITE,	/*!< Writing to persistent storage failed */
	BLE_ERR_STORAGE_DELETE,	/*!< Deleting data in persistent storage failed */
	BLE_ERR_STORAGE_READ,	/*!< Reading from persistent storage failed */
	BLE_ERR_UNALIGNED,		/*!< Address is not word align */
	BLE_ERR_MAX_REACHED,	/*!< Maximum count exceeded for this operation*/
	BLE_ERR_BUSY,			/*!< Busy */
	BLE_ERR_TIMEOUT,		/*!< Operation timed-out */
	BLE_ERR_INTERNAL,		/*!< Internal error */
};

