/*!
 * \file accelero.c
 *
 * \brief Manage the accelerometer
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 * \note The accelerometer may detect a motion if the buzzer is active. The event manager
 * pays attention to this. The accelerometer is stopped while the buzzer plays and
 * restarted when the buzzer stop.
 *
 * The LIS2DW12 does not support all possible configuration for the wakeup duration. That's
 * the reason why a local timer has been added. When this timer elapses, the motion stop
 * event is sent to the event manager.
 */

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"


#include "aos_system.h"
#include "aos_log.h"
#include "srv_cli.h"

#include "board_EVK_ES2.h"
#include "board_drv_accelero.h"

#include "events.h"
#include "accelero.h"


// Import the driver
extern const board_accelero_drv_t board_drv_lis2dw12;

// Tracing facility
#define ACC_TRACE(...)	do { aos_log_status(aos_log_module_accelero, true, __VA_ARGS__);  } while (0)


// Default Wake duration in milliseconds: 10 seconds
#define APP_ACCELERO_WAKE_DURATION 		10000

// Default motion duration in ms: 60 seconds
#define APP_ACCELERO_MOTION_DURATION	60000

// Local context
typedef struct {
	TimerHandle_t timer_hdl;			// Timer for shock data acquisition
	StaticTimer_t timer_local_data;		// Timer internal data
	UBaseType_t timer_count;			// Timer count
	uint32_t wakeup_duration;			// Actual wakeup duration
	uint32_t motion_duration;			// Applicative motion duration
	board_accelero_state_t state;		// Actual accelero state
	board_accelero_config_t cfg;		// Configuration
} app_accelero_ctx_t;

static app_accelero_ctx_t _accelero_ctx;

static const char* _accelero_state_to_str(board_accelero_state_t state)
{
	switch (state) {
	case board_accelero_state_standby:
		return "standby";
	case board_accelero_state_sleep:
		return "sleep";
	case board_accelero_state_wake:
		return "wake";
	case board_accelero_state_starting:
		return "starting";
	case board_accelero_state_power_off:
		return "off";
	case board_accelero_state_last:
		break;
	}
	return "unknown";
}

static const char* _accelero_notif_to_str(board_accelero_notif_type_t type)
{
	switch (type) {
	case board_accelero_notif_type_sleep:
		return "sleep";
	case board_accelero_notif_type_wake:
		return "wake";
	case board_accelero_notif_type_shock:
		return "shock";
	case board_accelero_notif_type_failure:
		return "failure";
	case board_accelero_notif_type_last:
		break;
	}
	return "unknown";
}


static bool _accelero_get_info(board_accelero_info_t* info)
{
	board_accelero_ioctl_t ioctl;

	if (!info) {
		return false;
	}
	ioctl.type = board_accelero_ioctl_type_get_info;
	if (board_drv_lis2dw12.ioctl(&ioctl) != board_accelero_result_success)  {
		return false;
	}
	memcpy(info, &ioctl.info, sizeof(board_accelero_info_t));
	return true;
}

static bool _accelero_get_state(board_accelero_state_t* state)
{
	board_accelero_ioctl_t ioctl;

	if (!state) {
		return false;
	}
	ioctl.type = board_accelero_ioctl_type_get_state;
	if (board_drv_lis2dw12.ioctl(&ioctl) != board_accelero_result_success)  {
		return false;
	}
	*state = ioctl.state;
	return true;
}

static void _accelero_user_callback(board_accelero_notif_type_t type, board_accelero_notif_info_t* info, void* arg)
{
	app_event_type_t evt = app_event_count;

	ACC_TRACE("Callback. Notif %s\n", _accelero_notif_to_str(type));

	switch(type) {
	case board_accelero_notif_type_sleep:
		if (_accelero_ctx.state == board_accelero_state_starting) {
			// Startup done.
			_accelero_ctx.state = board_accelero_state_sleep;
			xTimerStop( _accelero_ctx.timer_hdl, 0);
			break;
		}

		if (xTimerIsTimerActive( _accelero_ctx.timer_hdl) != pdFALSE ) {
			//Wait the end of motion duration timer
			break;
		}
		// Motion timer not active. Enforce the sleep state
		_accelero_ctx.state = board_accelero_state_sleep;
		break;

	case board_accelero_notif_type_shock:	// Consider a wake state
	case board_accelero_notif_type_wake:
		if ((_accelero_ctx.state == board_accelero_state_sleep) ||
			(_accelero_ctx.state == board_accelero_state_starting)) {
			// Send a motion start event to the application
			evt = app_event_motion_start;
		}
		// Restart the motion timer
		xTimerChangePeriod(_accelero_ctx.timer_hdl, pdMS_TO_TICKS(_accelero_ctx.motion_duration), 0);
		_accelero_ctx.state = board_accelero_state_wake;
		if (type == board_accelero_notif_type_shock) {
			// Catch the shock if needed and send a new event.
		}
		_accelero_ctx.state = board_accelero_state_wake;
		break;

	default:
		break;
	}

	if (evt != app_event_count) {
		event_send(evt);
	}
}

static void _accelero_timeout( TimerHandle_t xExpiredTimer )
{
	board_accelero_state_t state;
	app_event_type_t evt = app_event_count;

	// Get the state
	if (!_accelero_get_state(&state)) {
		ACC_TRACE("Timeout. Get state failure\n");
		return;
	}

	ACC_TRACE("Timeout. State current: %s, new: %s\n",
			_accelero_state_to_str(_accelero_ctx.state),
			_accelero_state_to_str(state));

	if (state == board_accelero_state_sleep) {
		// Send a motion stop to the application
		evt = app_event_motion_stop;
	} else if (_accelero_ctx.state != board_accelero_state_starting) {
		// Send a motion start event to the application
		evt = app_event_motion_start;
		// And restart the motion timer
		xTimerChangePeriod(_accelero_ctx.timer_hdl, pdMS_TO_TICKS(_accelero_ctx.motion_duration), 0);
	}

	_accelero_ctx.state = state;

	// Send the event if any
	if (evt != app_event_count) {
		event_send(evt);
	}
}

static bool _accelero_open(board_accelero_config_t* cfg)
{
	board_accelero_ioctl_t ioctl;

	if (_accelero_ctx.state != board_accelero_state_power_off) {
		return true;
	}

	if (board_drv_lis2dw12.open(cfg) != board_accelero_result_success) {
		return false;
	}

	ioctl.type = board_accelero_ioctl_type_get_info;
	if (board_drv_lis2dw12.ioctl(&ioctl) == board_accelero_result_success) {
		_accelero_ctx.wakeup_duration = ioctl.info.wake_time;
	}
	ACC_TRACE("Opened. Wakeup duration: %dms\n", _accelero_ctx.wakeup_duration );

	_accelero_ctx.state = board_accelero_state_starting;
	return true;
}

static bool _accelero_close(void)
{
	if (_accelero_ctx.state == board_accelero_state_power_off) {
		// Nothing to do
		return true;
	}
	xTimerStop(_accelero_ctx.timer_hdl, 0);


	_accelero_ctx.state = board_accelero_state_power_off;

	if (board_drv_lis2dw12.close() != board_accelero_result_success) {
		return false;
	}
	return true;
}

/*!
  * \brief Initialization
  */
void accelero_init(void)
{
	board_accelero_init_info_t init_info;
	board_accelero_result_t result;

	// Clear local context
	memset(&_accelero_ctx, 0, sizeof(_accelero_ctx));

	// Build the initialization structure
	init_info.interrupt_gpio = aos_board_pin_accelero_int;
	init_info.power_gpio = aos_board_pin_accelero_power;

	result = board_drv_lis2dw12.init(&init_info);
	if (result!= board_accelero_result_success) {
		ACC_TRACE("Init failure\n");
		return;
	}

	// Create the local timer, which will be used for the applicative motion duration
	_accelero_ctx.timer_hdl =  xTimerCreateStatic("Accelero",
			pdMS_TO_TICKS(1000),			// Don't care
			pdFALSE,
			&_accelero_ctx.timer_count,
			_accelero_timeout,
			&_accelero_ctx.timer_local_data);
	xTimerStop(_accelero_ctx.timer_hdl, 0);
	ACC_TRACE("Init success\n");

	_accelero_ctx.state = board_accelero_state_power_off;
	_accelero_ctx.cfg.wake_duration = APP_ACCELERO_WAKE_DURATION;

	_accelero_ctx.motion_duration = APP_ACCELERO_MOTION_DURATION;
}

/*!
  * \brief start the accelero
  */
bool accelero_start(void)
{
	// Feed the configuration
	_accelero_ctx.cfg.motion_sensi = 16;		// Motion sensitivity: 1G. Step 0.063G
	_accelero_ctx.cfg.motion_debounce = 1;		// Motion debounce. Step 1/ODR. Max: 3*1/ODR
	_accelero_ctx.cfg.shock_threshold = 0;		// No shock detection
	_accelero_ctx.cfg.wake_duration = APP_ACCELERO_WAKE_DURATION;		// Time in millisecond that the component should wait after the last event trigger to move to the sleep state (ASLP).
	_accelero_ctx.cfg.odr = board_accelero_odr_type_12_5HZ;	// Output data rate.
	_accelero_ctx.cfg.fs = board_accelero_fs_type_4G;	// Full scale selection
	_accelero_ctx.cfg.callback = _accelero_user_callback;
	_accelero_ctx.cfg.user_arg = NULL;

	if (!_accelero_open(&_accelero_ctx.cfg)) {
		ACC_TRACE("Open failure\n");
		return false;
	}
	ACC_TRACE("Open success\n");
	return true;
}

/*!
  * \brief Stop the accelero
  */
void accelero_stop(void)
{
	if (!_accelero_close()) {
		ACC_TRACE("Close failure\n");
		return;
	}
	ACC_TRACE("Close success\n");
}


/*!
 * \brief Display the accelerometer information via the CLI.
 *
 * \note Use the system accelero CLI command
 */
bool accelero_cli_show_info(void)
{
	board_accelero_state_t state;
	board_accelero_info_t info;
	board_accelero_result_t result;
	fix16_vector_t vector;
	uint64_t cons;

	if (_accelero_ctx.state == board_accelero_state_power_off) {
		cli_printf("Information\r\n State: Power off\n");
		return true;
	}

	if (!_accelero_get_state(&state)) {
		cli_printf("Fails to get the state\n");
		return false;
	}

	if (!_accelero_get_info(&info)) {
		cli_printf("Fails to get the info\n");
		return false;
	}

	if (!accelero_get_consumption(&cons)) {
		cli_printf("Fails to get the consumption\n");
		return false;
	}

	cli_printf("Information\r\n State: %s\r\n",
			_accelero_state_to_str(state));
	if (info.odr == board_accelero_odr_type_12_5HZ) {
		cli_printf(" ODR: 12.5 Hz\n");
	} else {
		cli_printf(" ODR: %d Hz\n", (1 << (info.odr - 1)) * 25);
	}
	cli_printf(" Full scale: %dG\n", (1 << (info.fs + 1)));

	if (state == board_accelero_state_power_off) {
		// We are done
		return true;
	}
	cli_printf(" Wake time: %d ms\n", info.wake_time);
	cli_printf(" Poll time: %d ms\n", info.poll_timeout);
	cli_printf(" Usage\n");
	cli_printf("  Active: %d.%04d s\n", (uint32_t) (info.usage.active / 10000), (uint32_t) (info.usage.active % 10000));
	cli_printf("  Sleep: %d.%04d s\n", (uint32_t) (info.usage.sleep / 10000), (uint32_t) (info.usage.sleep % 10000));
	cli_printf(" Consumption: %d.%03d mAh\n", (uint32_t) (cons / 1000), (uint32_t) (cons % 1000));


	result = board_drv_lis2dw12.read_data(&vector);
	if (result != board_accelero_result_success) {
		cli_printf("Read failure (%d)\n", result);
		return false;
	}
	cli_printf("Current (mG)\r\n x: %d\r\n y: %d\r\n z: %d\n",
			board_accelero_fix16_to_mg(vector.x),
			board_accelero_fix16_to_mg(vector.y),
			board_accelero_fix16_to_mg(vector.z));
	return true;
}

bool accelero_get_consumption(uint64_t* cons)
{
	board_accelero_ioctl_t ioctl;

	if (!cons) {
		return false;
	}
	ioctl.type = board_accelero_ioctl_type_get_consumption;
	if (board_drv_lis2dw12.ioctl(&ioctl) != board_accelero_result_success)  {
		return false;
	}
	*cons = ioctl.consumption_uah;
	return true;
}

bool accelero_clear_consumption(void)
{
	board_accelero_ioctl_t ioctl;

	ioctl.type = board_accelero_ioctl_type_clear_info;
	if (board_drv_lis2dw12.ioctl(&ioctl) != board_accelero_result_success)  {
		return false;
	}
	return true;
}

board_accelero_result_t accelero_read_reg(board_accelero_reg_item_t* item)
{
	board_accelero_ioctl_t ioctl;

	if (!item) {
		return false;
	}
	ioctl.type = board_accelero_ioctl_type_read_reg;
	ioctl.reg_item = item;
	return board_drv_lis2dw12.ioctl(&ioctl);
}

bool accelero_write_reg(board_accelero_reg_item_t* item)
{
	board_accelero_ioctl_t ioctl;

	if (!item) {
		return false;
	}
	ioctl.type = board_accelero_ioctl_type_write_reg;
	ioctl.reg_item = item;
	return board_drv_lis2dw12.ioctl(&ioctl) == board_accelero_result_success?true:false;
}


