/*!
 * \file aos_common.h
 *
 * \brief Common definitions
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/*!
 * \defgroup aos_common Common definitions
 *
 * \addtogroup aos_system
 * @{
 */

#define AOS_PROVISIONING_EUI_SIZE 8    //!< Size in byte of the LoRa device unique Identifier

/*!
 * \enum aos_result_t
 *
 * \brief Result status of AOS functions
 */
typedef enum {
	aos_result_success = 0,				//!< Success
	aos_result_param_error,				//!< Parameter error
	aos_result_not_supported,			//!< Action not supported
	aos_result_not_init,				//!< Resource not initialized
	aos_result_not_open,				//!< Driver not open
	aos_result_mem_failure,				//!< Memory allocation failure
	aos_result_busy,					//!< Resource busy
	aos_result_not_found,				//!< Resource not found
	aos_result_ioctl_req_unknown,		//!< IO request type not supported
	aos_result_ioctl_not_supported,		//!< The driver does not support IO control
	aos_result_lock_fail,				//!< Resource lock failure
	aos_result_unlock_fail,				//!< Resource unlock failure
	aos_result_ownership_error,			//!< Action refused due to an ownership issue.
	aos_result_internal_failure,		//!< Other errors
	aos_result_last						//<! Last item in the enum. Not an actual error
} aos_result_t;

/*!
 * \struct buffer_t
 *
 * \brief Buffer data type
 */
typedef struct {
	uint8_t len;                        //!< Buffer length
	uint8_t *data;                      //!< Buffer address
} buffer_t;

/*! @}*/
#ifdef __cplusplus
}
#endif
