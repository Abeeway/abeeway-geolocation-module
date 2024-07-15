/*!
 * \file cli-cmd-system.c
 *
 * \brief System CLI commands
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>


#include "stm32wbxx.h"	// NVIC_SystemReset()

#include "FreeRTOS.h"
#include "task.h"	// FreeRTOS, task info
#include "srv_cli.h"
#include "aos_log.h"
#include "aos_gpio.h"
#include "aos-sdk-version.h"
#include "aos_rtc.h"
#include "aos_error.h"

#include "accelero.h"


/*
 * ----------------------------------------------------------------------------
 * Error commands
 * ----------------------------------------------------------------------------
 */
static const char* _cli_system_error_code_to_str(aos_error_code_t code)
{
	const char* _error_code_to_str[AOS_ERROR_SW_APP_START] = {
		[AOS_ERROR_NONE] = "No error",
		[AOS_ERROR_HW_NMI] = "NMI",
		[AOS_ERROR_HW_FAULT] = "HW fault",
		[AOS_ERROR_HW_MPU] = "MPU fault",
		[AOS_ERROR_HW_BUS] = "BUS error",
		[AOS_ERROR_HW_USAGE] = "Usage fault",
		[AOS_ERROR_HW_IRQ] = "Unexpected IRQ",
		[AOS_ERROR_SW_ST_HAL_ERROR] = "ST HAL error",
		[AOS_ERROR_SW_FREERTOS_ASSERT] = "FreeRTOS assert",
		[AOS_ERROR_SW_FREERTOS_TASK_OVF] = "Task stack overflow",
		[AOS_ERROR_SW_RTC_FAIL] = "RTC init fails"
	};

	if (code >= AOS_ERROR_SW_APP_START) {
		return "Unknown";
	}

	return _error_code_to_str[code];
}

static cli_parser_status_t _cli_system_error_get(void *arg, int argc, char **argv)
{
	aos_error_info_t* error;

	error = aos_error_get();

	cli_printf("Error\n");
	cli_printf(" Code: %d (%s)\n", error->code, _cli_system_error_code_to_str(error->code));
	if (error->code == AOS_ERROR_NONE) {
		return cli_parser_status_ok;
	}

	if (error->code < AOS_ERROR_SW_BASE) {
		// Registers info available
		if (error->code == AOS_ERROR_HW_IRQ) {
			cli_printf(" IRQ  : %d\n", error->reg_info.irq);
		}
		cli_printf(" XPSR : 0x%08lx\n", error->reg_info.xpsr);
		cli_printf(" PC   : 0x%08lx\n", error->reg_info.pc);
		cli_printf(" LR   : 0x%08lx\n", error->reg_info.lr);
		cli_printf(" R0   : 0x%08lx\n", error->reg_info.r0);
		cli_printf(" R1   : 0x%08lx\n", error->reg_info.r1);
		cli_printf(" R2   : 0x%08lx\n", error->reg_info.r2);
		cli_printf(" R3   : 0x%08lx\n", error->reg_info.r3);
		cli_printf(" CSFR : 0x%08lx\n", error->reg_info.csfr);
		cli_printf(" DSFR : 0x%08lx\n", error->reg_info.dsfr);
		cli_printf(" ASFR : 0x%08lx\n", error->reg_info.asfr);
		cli_printf(" HSFR : 0x%08lx\n", error->reg_info.hsfr);
		if (error->reg_info.csfr & 0x0080) {
			cli_printf(" MMFAR: 0x%08lx\n", error->reg_info.extra.mmfar);
		}
		if (error->reg_info.csfr & 0x8000) {
			cli_printf(" BFAR: 0x%08lx\n", error->reg_info.extra.bfar);
		}
		return cli_parser_status_ok;
	}

	if (error->code == AOS_ERROR_SW_FREERTOS_TASK_OVF) {
		cli_printf(" Task: %s\n", error->file[0]?error->file:"No provided");
		return cli_parser_status_ok;
	}

	// Others
	cli_printf(" File: %s\n", error->file[0]?error->file:"No provided");
	cli_printf(" Line: %d\n", error->line);
	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_system_error_clear(void *arg, int argc, char **argv)
{
	aos_error_clear();
	cli_printf("Error cleared\n");
	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _system_error_cmd_table[] = {
		PARSER_CMD_FUNC("get", "Get error", _cli_system_error_get, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("clear", "Clear error", _cli_system_error_clear, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};


/*
 * ----------------------------------------------------------------------------
 * log command
 * ----------------------------------------------------------------------------
 */
static void _cli_log_display_info(void)
{
	uint8_t ii;
	aos_log_level_t level;

	cli_printf("Log info\n");
	cli_printf(" State: %s\n", aos_log_is_enabled()?"enabled":"disabled");

	cli_printf(" Module state\n");
	cli_printf("   %-14s%-10s%s\n", "Name", "ID", "Level");
	for (ii=0; ii < aos_log_module_last; ii++) {
		level = aos_log_get_module_level(ii);
		cli_printf("   %-14s%-10d%-10s\n", aos_log_get_module_str(ii), ii, aos_log_get_level_str(level));
	}
}

static void _cli_log_display_help(void)
{
	uint8_t ii;

	cli_printf("Modules\n");
	for (ii=0; ii < aos_log_module_last; ii++) {
		cli_printf(" %-14s%d\n",  aos_log_get_module_str(ii), ii);
	}

	cli_printf("Levels\n");
	for (ii=0; ii < aos_log_level_last; ii++) {
		cli_printf(" %-14s%d\n",  aos_log_get_level_str(ii), ii);
	}
}

static cli_parser_status_t _cli_log_enable(void *arg, int argc, char **argv)
{
	aos_log_enable_disable(true);
	cli_printf("logs enabled\n");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_log_disable(void *arg, int argc, char **argv)
{
	aos_log_enable_disable(false);
	cli_printf("logs disabled\n");
	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_log_module(void *arg, int argc, char **argv)
{
	aos_log_module_id_t mid;
	aos_log_level_t level;

	if (argc < 3) {
		cli_print_missing_argument();
		_cli_log_display_help();
		return cli_parser_status_error;
	}

	// Read the module name
	mid = aos_log_str_to_module_id(argv[1]);
	if (mid == aos_log_module_last) {
		cli_printf("Module %s unknown\n", argv[1]);
		_cli_log_display_help();
		return cli_parser_status_error;
	}

	level = aos_log_str_to_level_id(argv[2]);
	if (level == aos_log_level_last) {
		cli_printf("Level %s unknown\n", argv[2]);
		_cli_log_display_help();
		return cli_parser_status_error;
	}

	aos_log_set_module_level(mid, level);
	cli_printf("logs for module %s set to %s\n", aos_log_get_module_str(mid), aos_log_get_level_str(level));
	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_log_info(void *arg, int argc, char **argv)
{
	_cli_log_display_info();
	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _system_log_cmd_table[] = {
		PARSER_CMD_FUNC("on", "Enable", _cli_log_enable, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("off", "Disable", _cli_log_disable, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("module <m> <l>", "Set the log level l to the module m", _cli_log_module, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("info", "Display log information", _cli_log_info, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};

/*
 * ----------------------------------------------------------------------------
 * Thread command
 * ----------------------------------------------------------------------------
 */
static const char* _cli_thread_state_to_str(eTaskState state)
{
	switch (state) {
	case eRunning:
		return "running";
	case eReady:
		return "ready";
	case eBlocked:
		return "blocked";
	case eSuspended:
		return "suspended";
	case eDeleted:
		return "deleted";
	case eInvalid:
		break;
	}
	return "invalid";
}

static cli_parser_status_t _cli_thread_details(char* tname)
{
	TaskStatus_t tdetails;
	TaskHandle_t task;

	// Read the task
	task = xTaskGetHandle(tname);

    if (!task) {
    	cli_printf("Task '%s' not found\n", tname);
    	return cli_parser_status_error;
    }

    /* Use the handle to obtain further information about the task. */
    vTaskGetInfo(task, &tdetails, pdTRUE,eInvalid);

    cli_printf("Task '%s'\n", tname);
    cli_printf(" Number: %d\n", tdetails.xTaskNumber);
    cli_printf(" State: %s\n", _cli_thread_state_to_str(tdetails.eCurrentState));
    cli_printf(" Base priority: %d\n", tdetails.uxBasePriority);
    cli_printf(" Current priority: %d\n", tdetails.uxCurrentPriority);
    cli_printf(" Runtime: %d\n", tdetails.ulRunTimeCounter);
    cli_printf(" Stack free space: %d bytes\n", tdetails.usStackHighWaterMark * sizeof(StackType_t));

	return cli_parser_status_ok;
}

#define CLI_MAX_TASK_LIST_DISPLAYED		20
TaskStatus_t task_status_list[CLI_MAX_TASK_LIST_DISPLAYED];
static cli_parser_status_t _cli_thread_info(void *arg, int argc, char **argv)
{
	uint32_t tot_runtime, runtime_percent;
	int nb_entry;

	if (argc >= 2) {
		return _cli_thread_details(cli_remove_str_quotes(argv[1]));
	}

	// Take a snapshot of the number of tasks in case it changes while this
	// function is executing.
	nb_entry = uxTaskGetNumberOfTasks();
	if (nb_entry > CLI_MAX_TASK_LIST_DISPLAYED) {
		nb_entry = CLI_MAX_TASK_LIST_DISPLAYED;
	}


	// Generate raw status information about each task. Total
	nb_entry = uxTaskGetSystemState( task_status_list, nb_entry, &tot_runtime );

	// For percentage calculations.
	tot_runtime /= 100UL;

	// Avoid divide by zero errors.
	if( tot_runtime > 0 ) {
		// For each populated position in the task_status_list array,
		// format the raw data as human readable ASCII data
		cli_printf(" %-20s%-10s %5s %9s %6s%% %14s\n","Name", "State", "Prio", "Runtime", "CPU", "Stack free");
		for (unsigned ii=0; ii< nb_entry; ii++) {
			TaskStatus_t *task = &task_status_list[ii];

			// Integer calculations x 100 to display pseudo-floating CPU percentage

			runtime_percent = task->ulRunTimeCounter * 100 / tot_runtime;
			if (runtime_percent > 10000) {
				runtime_percent = 10000;
			}
			cli_printf(" %-20s%-10s %5u %9u %3u.%02u%% %14u\n", task->pcTaskName,
					_cli_thread_state_to_str(task->eCurrentState),
					task->uxCurrentPriority,
					task->ulRunTimeCounter,
					runtime_percent / 100,
					runtime_percent % 100,
					task->usStackHighWaterMark * sizeof(StackType_t));
		}
	}
   	return cli_parser_status_ok;
}

/*
 * ----------------------------------------------------------------------------
 * System info, reset and bootloader commands
 * ----------------------------------------------------------------------------
 */
static void _system_display_version(void)
{
	cli_printf(" AOS: %d.%d-%d. Built on: %s\n",
			AOS_SDK_VERSION_MAJOR, AOS_SDK_VERSION_MINOR, AOS_SDK_VERSION_PATCH, aos_system_get_build_date());
}

static void _system_display_uptime(void)
{
	TickType_t now;
	uint8_t s, m, h;

	now = xTaskGetTickCount()/pdMS_TO_TICKS(1000);	// Now in ms

	// Remove ms
	s = now % 60;
	now /= 60;
	m = now % 60;
	now /= 60;
	h = now % 24;
	now /= 24;
	cli_printf(" Uptime: %lud,%02u:%02u:%02u\n", (int32_t)(now % 0xffffffff), h, m, s);
}

static cli_parser_status_t _cli_system_info(void *arg, int argc, char **argv)
{
	uint16_t battery;
	int16_t temperature;
	aos_rtc_systime_t systime;

	cli_printf("System\n");

	cli_printf(" Battery level: ");
	if (aos_gpio_read_battery_voltage(&battery) != aos_result_success) {
		cli_printf("failure\n");
	} else {
		cli_printf("%d mV\n", battery);
	}
	cli_printf(" CPU temperature: ");
	if (aos_system_read_cpu_temperature(&temperature) != aos_result_success) {
		cli_printf("failure\n");
	} else {
		cli_printf("%d degree C\n", temperature);
	}

	systime = aos_rtc_systime_get();
	cli_print_systime(systime.seconds);
	_system_display_uptime();

	cli_printf("Version\n");
	_system_display_version();
   	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_system_reset(void *arg, int argc, char **argv)
{
	cli_printf("Resetting...\n");
	aos_error_clear();
	vTaskDelay(pdMS_TO_TICKS(500));
	NVIC_SystemReset();
   	return cli_parser_status_error;
}

static cli_parser_status_t _cli_system_bootloader(void *arg, int argc, char **argv)
{
	cli_printf("Bootloader entrance set\n");
	aos_rtc_backup_write(aos_rtc_backup_register_bootloader, aos_rtc_bootloader_rtc_cmd_enter);
	return _cli_system_reset(arg, argc, argv);
}

/*
 * ----------------------------------------------------------------------------
 * Accelerometer info command
 * ----------------------------------------------------------------------------
 */
static cli_parser_status_t _cli_accelero_info(void *arg, int argc, char **argv)
{
	if (accelero_cli_show_info()) {
		return cli_parser_status_ok;
	}

	return cli_parser_status_error;
}

static cli_parser_status_t _cli_accelero_read_reg(void *arg, int argc, char *argv[])
{
	int32_t value;
	board_accelero_reg_item_t reg_item;
	board_accelero_result_t result;
	uint8_t ii;
	uint8_t count = 1;

	if (argc < 2) {
		count = 0xFF; // Set the max value
		reg_item.reg = 0;
	} else {
		if (!cli_parse_int(argv[1], &value)) {
			return cli_parser_status_error;
		}
		reg_item.reg = value;
		if (argc >= 3) {
			if (!cli_parse_int(argv[2], &value)) {
				return cli_parser_status_error;
			}
			count = value;
		}
	}

	for (ii=0; ii < count; ii ++, reg_item.reg ++) {
		result = accelero_read_reg(&reg_item);
		if (result == board_accelero_result_bad_params) {
			// We are done
			break;
		}
		if (result != board_accelero_result_success) {
			cli_printf("Fail to read the register %d\n", reg_item.reg);
			return cli_parser_status_error;
		}
		cli_printf("Register 0x%02x: 0x%02x\n", reg_item.reg, reg_item.data);
	}

	return cli_parser_status_ok;
}

static cli_parser_status_t _cli_accelero_write_reg(void *arg, int argc, char *argv[])
{
	board_accelero_reg_item_t reg_item;
	int32_t value;

	if (argc < 3) {
		cli_print_missing_argument();
		return cli_parser_status_error;
	}
	if (!cli_parse_int(argv[1], &value)) {
		return cli_parser_status_error;
	}
	reg_item.reg = value;

	if (!cli_parse_int(argv[2], &value)) {
		return cli_parser_status_error;
	}
	if (value > 0xFF) {
		cli_printf("Value too large. Max 0xFF\n");
		return cli_parser_status_error;
	}

	reg_item.data = value;
	if (!accelero_write_reg(&reg_item)) {
		cli_printf("Fail to write register\n");
		return cli_parser_status_error;
	}

	// Read the value back
	if (accelero_read_reg(&reg_item) != board_accelero_result_success) {
		cli_printf("Fail to read back the value\n");
		return cli_parser_status_error;
	}

	cli_printf("Register 0x%02x write attempt with 0x%02x, actual 0x%02x\n", reg_item.reg , value, reg_item.data);
	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _accelero_cmd_table[] = {
		PARSER_CMD_FUNC("info", "Display information and accelerations", _cli_accelero_info, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("rr [reg] [count]", "Show register(s)", _cli_accelero_read_reg, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("wr <reg> <value>", "Write a register", _cli_accelero_write_reg, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};


// Main sub-commands definition
static const cli_parser_cmd_t _system_cmd_table[] = {
		PARSER_CMD_FUNC("info", "System information", _cli_system_info, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("log", "Manage the logs", _system_log_cmd_table, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("reset", "System reset", _cli_system_reset, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("bootloader", "Restart on bootloader mode", _cli_system_bootloader, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("thread [name]", "Thread information", _cli_thread_info, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("error", "Error information", _system_error_cmd_table, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("accelero", "Accelerometer information", _accelero_cmd_table, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};


/*
 * ----------------------------------------------------------------------------
 * Help commands and logout
 * ----------------------------------------------------------------------------
 */
static  cli_parser_status_t _cli_cmd_logout(void *arg, int argc, char *argv[])
{
	cli_printf("Logged out\n");
	srv_cli_set_access_mask(cli_access_mask_none);
	return cli_parser_status_void;
}


static  cli_parser_status_t _cli_cmd_help(void *arg, int argc, char *argv[])
{
	srv_cli_show_help_ext(argc, argv);
	return cli_parser_status_void;
}

static  cli_parser_status_t _cli_cmd_help_long(void *arg, int argc, char *argv[])
{
	srv_cli_show_help();
	return cli_parser_status_void;
}


/*
 * ----------------------------------------------------------------------------
 * Main Commands registration
 * ----------------------------------------------------------------------------
 */

// Register main commands against the CLI parser
CLI_COMMAND_TAB_REGISTER(system,"System commands", _system_cmd_table, CLI_ACCESS_ALL_LEVELS );

/*
 * The "?" command needs manual declaration as the macros cannot paste non-alphanumerics.
 */
__attribute__((section(".commands.qmark"),used))
static const cli_parser_cmd_t __cmdfun_qmark = {
		.command = "?",
		.help = "Display all helps",
		{ .func = _cli_cmd_help_long },
		.action = cli_parser_action_execute,
		.access = CLI_ACCESS_ALL_LEVELS };

// Register help and logout commands
CLI_COMMAND_FUNC_REGISTER(help, "<cmd> Display help information", _cli_cmd_help, CLI_ACCESS_ALL_LEVELS);
CLI_COMMAND_FUNC_REGISTER(logout, "Disconnect the CLI", _cli_cmd_logout, CLI_ACCESS_ALL_LEVELS);

