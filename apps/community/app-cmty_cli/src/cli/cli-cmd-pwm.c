/*
 * \file cli-cmd-pwm.c
 *
 * \brief Demostrate the PWM driver
 */

#include <stdlib.h> // strtoul
#include "aos_pwm.h"
#include "srv_cli.h"



static void _display_start_usage(void)
{
	cli_printf("Usage\n");
	cli_printf(" start <freq> <dc>\n");
	cli_printf("  freq: Frequency\n");
	cli_printf("  dc: Duty cycle\n");
}


static cli_parser_status_t _cmd_pwm_start(void *arg, int argc, char *argv[])
{
	int32_t value;
	uint32_t frequency;
	uint8_t dc;
	aos_result_t result;

	if (argc < 3) {
		cli_print_missing_argument();
		_display_start_usage();
		return cli_parser_status_error;
	}

	if(!cli_parse_int(argv[1], &value)) {
		cli_printf("Invalid value\n");
		return cli_parser_status_error;
	}
	frequency = value;

	if(!cli_parse_int(argv[2], &value)) {
		cli_printf("Invalid value\n");
		return cli_parser_status_error;
	}
	if ((value < 1) || (value > 99)) {
		cli_printf("Duty cycle should range in [1..99]\n");
		return cli_parser_status_error;
	}
	dc = value;

	result = aos_pwm_start(frequency, dc);
	if (result != aos_result_success) {
		cli_print_aos_result(result);
		return cli_parser_status_ok;
	}
	return cli_parser_status_ok;
}


static cli_parser_status_t _cmd_pwm_stop(void *arg, int argc, char *argv[])
{
	aos_result_t result;

	result = aos_pwm_stop();
	if (result != aos_result_success) {
		cli_print_aos_result(result);
		return cli_parser_status_ok;
	}
	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _cli_cmd_pwm_table[] = {
	PARSER_CMD_FUNC("start", "<freq> <dc>. Start the PWM", _cmd_pwm_start, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_FUNC("stop", "Stop the PWM", _cmd_pwm_stop, CLI_ACCESS_ALL_LEVELS),
	PARSER_CMD_END
};

CLI_COMMAND_TAB_REGISTER(pwm, "PWM related commands", _cli_cmd_pwm_table, CLI_ACCESS_ALL_LEVELS);
