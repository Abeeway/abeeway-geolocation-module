/*
 * \file board_drv_accelero.h
 *
 * \brief Generic accelerometer definitions
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "fix16.h"
#include "aos_log.h"
#include "aos_gpio.h"

/*
 * \enum board_accelero_state_t
 * \brief Accelerometer state.
 * \note Do not change the order
 */
typedef enum {
	board_accelero_state_min = 0,								//!< First state
	board_accelero_state_standby = board_accelero_state_min,	//!< Accelero in standby mode
	board_accelero_state_wake,									//!< Accelero in wake state
	board_accelero_state_sleep,									//!< Accelero in sleep state
	board_accelero_state_starting,								//!< Accelero is statrting
	board_accelero_state_power_off,								//!< Accelero is in deep sleep mode (ODR=0) or is powered off
	board_accelero_state_last									//!< Last State in the enum
} board_accelero_state_t;

/*
 * \enum board_accelero_result_t
 *
 * \brief Accelerometer API result
 */
typedef enum {
	board_accelero_result_success = 0,							//!< Success
	board_accelero_result_done,									//!< Request done
	board_accelero_result_no_init,								//!< Failure. Driver not initialized
	board_accelero_result_not_open,								//!< Failure. Driver not open
	board_accelero_result_not_supported,						//!< Failure. ioctl not supported
	board_accelero_result_bad_params,							//!< Failure. Bad calling parameters
	board_accelero_result_data_not_ready,						//!< Failure. Data not yet ready
	board_accelero_result_chip_not_found,						//!< Failure. No matching accelerometer found on the I2C
	board_accelero_result_other_error							//!< Failure. Catch all
} board_accelero_result_t;


/*
 * \enum board_accelero_odr_type_t
 *
 * \brief Output data rate (ODR)
 */
typedef enum {
	board_accelero_odr_type_12_5HZ = 0,		//!< ODR=12.5Hz
	board_accelero_odr_type_25HZ,			//!< ODR=25Hz
	board_accelero_odr_type_50HZ,			//!< ODR=50Hz
	board_accelero_odr_type_100HZ,			//!< ODR=100Hz
	board_accelero_odr_type_200HZ,			//!< ODR=200Hz
} board_accelero_odr_type_t;

/*
 * \enum board_accelero_fs_type_t
 *
 * \brief Full scale
 */
typedef enum {
	board_accelero_fs_type_2G = 0,			//!<  Full-scale=2G
	board_accelero_fs_type_4G,				//!<  Full-scale=4G
	board_accelero_fs_type_8G,				//!<  Full-scale=8G
	board_accelero_fs_type_16G,				//!<  Full-scale=16G
} board_accelero_fs_type_t;


/*
 * \enum board_accelero_odr_type_t
 *
 * \brief Notification type sent along the user callback
 */
typedef enum {
	board_accelero_notif_type_sleep,		//!< Accelero is ready and in sleep state (motion off)
	board_accelero_notif_type_wake,			//!< Accelero is ready and in wake state (motion on)
	board_accelero_notif_type_shock,		//!< A shock has been detected
	board_accelero_notif_type_failure,		//!< A failure occurred
	board_accelero_notif_type_last			//!< Last type
} board_accelero_notif_type_t;

/*
 * \struct board_accelero_notif_info_t
 *
 * \brief Notification data sent along the user callback
 */
typedef struct {
	fix16_vector_t vector;				//!< Acceleration vector
	uint32_t gadd_index;				//!< Gadd index (shock only)
} board_accelero_notif_info_t;


/*
 * \struct board_accelero_reg_item_t
 *
 * \brief Accelero register item
 */
typedef struct {
	uint8_t reg;					//!< Register address
	uint8_t data;					//!< Value
} board_accelero_reg_item_t;

/*!
 * \fn void (*board_accelero_user_callback_t)(board_accelero_notif_type_t type, board_accelero_notif_info_t* info, void* arg)
 *
 * \brief User callback function definition for ISR to background process deferring
 *
 * \param type Notification type
 * \param info Notification information
 * \param arg: User argument
 */
typedef void (*board_accelero_user_callback_t)(board_accelero_notif_type_t type, board_accelero_notif_info_t* info, void* arg);

/*
 * \struct board_accelero_config_t
 *
 * \brief Accelerometer configuration
 */
typedef struct {
	uint8_t motion_sensi;		//!< Motion sensitivity. Step: 0.063g regardless FS.
	uint8_t motion_debounce;	//!< Motion debounce. step 1/ODR. Max: 3*1/ODR
	uint8_t shock_threshold;	//!< Shock intensity threshold. FS/64.
	uint32_t wake_duration;		//!< Time in millisecond that the component should wait after the last option trigger to move to the sleep state (ASLP).
	board_accelero_odr_type_t odr;	//!< Output data rate
	board_accelero_fs_type_t fs;	//!< Full scale selection
	board_accelero_user_callback_t callback;	//!< user callback called for events
	void* user_arg;								//!< User argument
} board_accelero_config_t;

/*
 * \enum board_accelero_ioctl_type_t
 *
 * \brief Supported IO control types
 *
 * \note To send ioctl, the driver should be opened
 */
typedef enum {
	board_accelero_ioctl_type_reconfigure,		//!< Reconfigure the accelerometer
	board_accelero_ioctl_type_get_state,		//!< Get the accelerometer state
	board_accelero_ioctl_type_get_consumption,	//!< Get the consumption
	board_accelero_ioctl_type_clear_info,		//!< Clear the consumption and the usage
	board_accelero_ioctl_type_get_info,			//!< Retrieve driver information
	board_accelero_ioctl_type_read_reg, 		//!< Read a register (debug purpose)
	board_accelero_ioctl_type_write_reg, 		//!< Write a register (debug purpose)
} board_accelero_ioctl_type_t;

/*
 * \enum board_accelero_usage_duration_t
 *
 * \brief Usage durations in tenth of ms
 *
 * \note To send ioctl, the driver should be opened
 */
typedef struct {
	uint64_t active;				//!< Duration when the accelero is in active state (step 100us)
	uint64_t sleep;					//!< Duration when the accelero is in active state (step 100us)
} board_accelero_usage_duration_t;

/*
 * \struct board_accelero_info_t
 *
 * \brief Data returned along to the configuration information request ioctl
 */
typedef struct {
	board_accelero_odr_type_t odr;			//!< Output data rate
	board_accelero_fs_type_t fs;			//!< Full-scale
	uint32_t wake_time;						//!< Actual wake time used (in milli-sec)
	uint32_t poll_timeout;					//!< Polling time in wake state
	board_accelero_usage_duration_t usage;	//!< Durations sleep/wake
} board_accelero_info_t;


/*
 * \struct board_accelero_ioctl_t
 *
 * \brief IO control data passed along to the ioctl
 */
typedef struct {
	board_accelero_ioctl_type_t type;		//!< IO control type
	union {
		board_accelero_config_t* config;	//!< New configuration. Follow board_accelero_ioctl_type_reconfigure
		bool enable_disable;				//!< Activate/deactivate the accelero. Follow board_accelero_ioctl_type_enable
		uint64_t consumption_uah;			//!< Current in uAh. Follow board_accelero_ioctl_type_get_consumption
		board_accelero_info_t info;			//!< Configuration. Follow board_accelero_ioctl_type_get_info
		board_accelero_state_t state;		//!< State of the accelero. Follow board_accelero_ioctl_type_get_state
		board_accelero_reg_item_t* reg_item;	//!< Register access. Follow board_accelero_ioctl_type_read_reg and board_accelero_ioctl_type_write_reg
	};
} board_accelero_ioctl_t;

/*
 * \struct board_accelero_init_info_t
 *
 * \brief Initialization structure
 */
typedef struct {
	aos_gpio_id_t	power_gpio;			//<! GPIO managing the power. Must be supplied.
	aos_gpio_id_t	interrupt_gpio;		//<! GPIO connected to the accelerometer interruption. Must be supplied.
} board_accelero_init_info_t;

/*
 * board_accelero_drv_t
 *
 * \brief Accelerometer driver structure
 *
 * open: Open the driver (and power on if needed) and configure the mems. Driver is not started
 * close: Power off the driver
 * read_data: Read the accelerometer data
 *  notification type and information
 * ioctl: IO control. Control the mems.
 */
typedef struct {
	board_accelero_result_t (*init)(board_accelero_init_info_t* init_info);	//!< Initialize the driver and return if the chip exits.
	board_accelero_result_t (*open)(board_accelero_config_t* config);		//!< Open the accelerometer. You must activate it using IOCTL
	board_accelero_result_t (*close)(void);									//!< Close the driver and power off the chip.
	board_accelero_result_t (*read_data)(fix16_vector_t *);					//!< Read the accelerometer data (Acceleration vector)
	board_accelero_result_t (*ioctl)(board_accelero_ioctl_t*);				//!< Control the accelerometer
} board_accelero_drv_t;

/*!
 * \fn int16_t board_accelero_fix16_to_mg(fix16_t f16)
 *
 * \brief Generic function to convert a fix16 number to milli-G
 *
 * \param f16 Fix16 to convert
 *
 * \return Value in milli-G
 */
int16_t board_accelero_fix16_to_mg(fix16_t f16);

