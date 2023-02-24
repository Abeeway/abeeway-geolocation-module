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
#include "aos_uart.h"
#include "aos_adc.h"
#include "aos_error.h"
#include "srv_cli.h"
#include "aos_log.h"
#include "systime.h"
#include "aos-core_release.h"
#include "aos-services_release.h"
#include "aos-boards_release.h"

/*
 * ----------------------------------------------------------------------------
 * error commands
 *
 * ----------------------------------------------------------------------------
 */
// Set to 1 if required, otherwise 0
#define ERROR_EMULATION 1

#if ERROR_EMULATION
#include "stm32wbxx_hal.h"

extern void Error_Handler(void);

#define TRIGGER_ERROR_ILLEGAL_INST 		0
#define TRIGGER_ERROR_ILLEGAL_DATA 		1
#define TRIGGER_ERROR_ILLEGAL_DIV_0 	2
#define TRIGGER_ERROR_ILLEGAL_IRQ		3
#define TRIGGER_ERROR_ST_HAL_ASSERT		4
#define TRIGGER_ERROR_ST_HAL_ERROR		5
#define TRIGGER_ERROR_FREERTOS_ASSERT	6
#define TRIGGER_FREERTOS_TASK_OVF		7


#define ERROR_OVF_TASK_SIZE_BYTE			2000
#define ERROR_OVF_TASK_SIZE					(ERROR_OVF_TASK_SIZE_BYTE/sizeof(StackType_t))

typedef struct {
	StackType_t stack[ERROR_OVF_TASK_SIZE];
	TaskHandle_t task;
	StaticTask_t info;
} test_task_ovf_ctx_t;

static test_task_ovf_ctx_t _test_task_ovf_ctx;

static void _tst_task_thread(void *argument)
{
	char str[ERROR_OVF_TASK_SIZE_BYTE - 50]; // Generate a stack overflow

	memset(str, 'a' , sizeof(str));
	strcpy(str, "TST ovf");
	for ( ; ; ) {
		cli_printf("%s\n", str);
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

static cli_parser_status_t _cli_system_error_trigger(void *arg, int argc, char **argv)
{
	int32_t test_id = 0;

	if (argc >= 2 ) {
		if (!cli_parse_int(argv[1], &test_id)) {
			return cli_parser_status_error;
		}
	}

	cli_printf("Start test %d\n", test_id);

	switch (test_id) {
	case TRIGGER_ERROR_ILLEGAL_INST:
		{
			void (*fp)(void) = (void (*)(void))(0x00000000);
			fp();
		}
		break;

	case TRIGGER_ERROR_ILLEGAL_DATA:
		{
			uint32_t* dptr = (uint32_t*)(0xFFFFFFFC);
			*dptr = 0x10;
		}
		break;

	case TRIGGER_ERROR_ILLEGAL_DIV_0:
		{
			uint32_t divider = 0;
			test_id /= divider;
			// Line below required to avoid optimization. Not displayed
			cli_printf("Res: %d\n", test_id);
		}
		break;

	case TRIGGER_ERROR_ILLEGAL_IRQ:
		// COMP_IRQn is unused by AOS
		NVIC_EnableIRQ(COMP_IRQn);
		NVIC_SetPendingIRQ(COMP_IRQn);
		break;

	case TRIGGER_ERROR_ST_HAL_ASSERT:
		assert_param(0);
		break;

	case TRIGGER_ERROR_ST_HAL_ERROR:
		Error_Handler();
		break;

	case TRIGGER_ERROR_FREERTOS_ASSERT:
		configASSERT(0);
		break;

	case TRIGGER_FREERTOS_TASK_OVF:
		// Create the task.
		 _test_task_ovf_ctx.task = xTaskCreateStatic(_tst_task_thread, "TEST", ERROR_OVF_TASK_SIZE, NULL,
				0, _test_task_ovf_ctx.stack, &_test_task_ovf_ctx.info);
		break;

	default:
		cli_printf("Test ID %d unknown\n", test_id);
		break;
	}

	return cli_parser_status_ok;
}
#endif


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
	aos_error_into_t* error;

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
		cli_printf(" Task: %s\n", error->file?error->file:"No provided");
		return cli_parser_status_ok;
	}

	// Others
	cli_printf(" File: %s\n", error->file?error->file:"No provided");
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
#if ERROR_EMULATION
		PARSER_CMD_FUNC("trigger <id>", "Trigger the error matching the id", _cli_system_error_trigger, cli_access_mask_super),
#endif
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
	aos_log_level_t level;

	cli_printf("Modules\n");
	for (ii=0; ii < aos_log_module_last; ii++) {
		cli_printf(" %-14s%d\n",  aos_log_get_module_str(ii), ii);
	}

	cli_printf("Levels\n");
	for (ii=0; ii < aos_log_level_last; ii++) {
		level = aos_log_get_module_level(ii);
		cli_printf(" %-14s%d\n",  aos_log_get_level_str(level), ii);
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
    cli_printf(" Stack high water mark: %d\n", tdetails.usStackHighWaterMark);

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
		cli_printf(" %-20s%-10s %5s %9s %6s%% %10s\n","Name", "State", "Prio", "Runtime", "CPU", "Highwater");
		for (unsigned ii=0; ii< nb_entry; ii++) {
			TaskStatus_t *task = &task_status_list[ii];

			// Integer calculations x 100 to display pseudo-floating CPU percentage

			runtime_percent = task->ulRunTimeCounter * 100 / tot_runtime;
			if (runtime_percent > 10000) {
				runtime_percent = 10000;
			}
			cli_printf(" %-20s%-10s %5u %9u %3u.%02u%% %10u\n", task->pcTaskName,
					_cli_thread_state_to_str(task->eCurrentState),
					task->uxCurrentPriority,
					task->ulRunTimeCounter,
					runtime_percent / 100,
					runtime_percent % 100,
					task->usStackHighWaterMark);
		}
	}
   	return cli_parser_status_ok;
}



/*
 * ----------------------------------------------------------------------------
 * Info command
 * ----------------------------------------------------------------------------
 */
static const char *_day_name(int weekday)
{
	static const char *days[] = {
			"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	const unsigned day_count = sizeof(days) / sizeof(*days);
	return (weekday < day_count) ? days[weekday] : "???";
}

static const char *_month_name(int month)
{
	static const char *months[] = {
			"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	const unsigned month_count = sizeof(months) / sizeof(*months);
	return (month < month_count) ? months[month] : "???";
}

static void _system_display_date(void)
{
	//
	// Display the date like the unix 'date' command. Since we don't know our
	// time zone, display UTC, just like the 'TZ=UTC date' command under unix.
	//
	// Date and time is updated by the response to a MAC device time request.
	//
	SysTime_t t;
	struct tm tm;

	t = SysTimeGet();
	SysTimeLocalTime(t.Seconds, &tm);

	cli_printf("%s %2u-%s %02u:%02u:%02u UTC %u\n",
			_day_name(tm.tm_wday), tm.tm_mday, _month_name(tm.tm_mon),
			tm.tm_hour, tm.tm_min, tm.tm_sec, 1900+tm.tm_year);

}

static cli_parser_status_t _cli_system_info(void *arg, int argc, char **argv)
{
	uint16_t battery;
	int16_t temperature;

	cli_printf("Battery level: ");
	if (aos_gpio_read_battery_voltage(&battery) != aos_result_success) {
		cli_printf("failure\n");
	} else {
		cli_printf("%d mV\n", battery);
	}
	cli_printf("CPU temperature: ");
	if (aos_system_read_cpu_temperature(&temperature) != aos_result_success) {
		cli_printf("failure\n");
	} else {
		cli_printf("%d degree C\n", temperature);
	}
	cli_printf("Date: ");
	_system_display_date();
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

static cli_parser_status_t _cmd_system_version(void *arg, int argc, char *argv[])
{
	static const char *fmtstr = "%25s : %s\n";
	cli_printf(fmtstr, "AOS Core version", AOS_CORE_RELEASE_GITVERSION);
	cli_printf(fmtstr, "AOS Services version", AOS_SERVICES_RELEASE_GITVERSION);
	cli_printf(fmtstr, "AOS Boards version", AOS_BOARDS_RELEASE_GITVERSION);
	return cli_parser_status_ok;
}

// Main sub-commands definition
static const cli_parser_cmd_t _system_cmd_table[] = {

		PARSER_CMD_FUNC("info", "System information", _cli_system_info, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("log", "Manage the logs", _system_log_cmd_table, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("reset", "System reset", _cli_system_reset, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("thread [name]", "Thread information", _cli_thread_info, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_TAB("error", "Error information", _system_error_cmd_table, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("version", "Display system version", _cmd_system_version, CLI_ACCESS_ALL_LEVELS),
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
 * An alternative would be to change this to e.g. "help -l" - but that means doc updates.
 */
__attribute__((section(".commands.qmark"),used))
static const cli_parser_cmd_t __cmdfun_qmark = {
		.command = "?",
		.help = "Display all helps",
		{ .func = _cli_cmd_help_long },
		.action = cli_parser_action_execute,
		.access = CLI_ACCESS_ALL_LEVELS };

CLI_COMMAND_FUNC_REGISTER(help, "<cmd> Display help information", _cli_cmd_help, CLI_ACCESS_ALL_LEVELS);
CLI_COMMAND_FUNC_REGISTER(logout, "Disconnect the CLI", _cli_cmd_logout, CLI_ACCESS_ALL_LEVELS);

