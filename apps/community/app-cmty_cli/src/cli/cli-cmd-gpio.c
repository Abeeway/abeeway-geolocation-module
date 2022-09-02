/*
 * \file cli-cmd-gpio.c
 *
 * \brief This file demonstrates the use of GPIO
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
// #include <stdio.h>

#include "aos_gpio.h"
#include "srv_cli.h"

#define GPIO_CMD_ACCESS (CLI_ACCESS_ALL_LEVELS)

static const cli_cmd_option_t _gpio_name_map[] = {
		{ "vbat-sense", aos_gpio_id_vbat_sense },
		{ "gpio-01", aos_gpio_id_1 },
		{ "gpio-02", aos_gpio_id_2 },
		{ "gpio-03", aos_gpio_id_3 },
		{ "gpio-04", aos_gpio_id_4 },
		{ "gpio-05", aos_gpio_id_5 },
		{ "gpio-06", aos_gpio_id_6 },
		{ "gpio-07", aos_gpio_id_7 },
		{ "gpio-08", aos_gpio_id_8 },
		{ "gpio-09", aos_gpio_id_9 },
		{ "gpio-10", aos_gpio_id_10 },
		{ "lpuart-rx", aos_gpio_id_lpuart_rx },
		{ "lpuart-tx", aos_gpio_id_lpuart_tx },
		{ "lpuart-cts", aos_gpio_id_lpuart_cts },
		{ "lpuart-rts", aos_gpio_id_lpuart_rts },
		{ "usart-rx", aos_gpio_id_usart_rx },
		{ "usart-tx", aos_gpio_id_usart_tx },
		{ "i2c-power", aos_gpio_id_i2c_power },
		{ "spi-cs", aos_gpio_id_spi_cs },
		{ "gnss-main", aos_gpio_id_mt3333_main_power },
		{ "gnss-backup", aos_gpio_id_mt3333_backup_power },
		{ "user-adc1", aos_gpio_id_user_adc1 },
		{ "i2c-int1", aos_gpio_id_i2c_int1 },
		{ "i2c-int2", aos_gpio_id_i2c_int2 },
		{ "pwm-ctrl", aos_gpio_id_pwm_ctrl },
};
static const unsigned _gpio_name_map_count = sizeof(_gpio_name_map) / sizeof(*_gpio_name_map);

static aos_gpio_id_t _get_gpio_id(int argc, char *argv[])
{
	if (argc < 1) {
		cli_printf("Missing GPIO name\n");
		return aos_gpio_id_last;
	}

	int option;

	option = cli_get_option_index(_gpio_name_map, _gpio_name_map_count, argv[1]);

	if (option >= 0) {
		return option;
	}

	cli_list_options("Missing or invalid GPIO. Valid GPIOs are :", _gpio_name_map, _gpio_name_map_count);

	return aos_gpio_id_last;
}

static void _gpio_isr_handler(void* arg)
{
	uint32_t gpio = (uint32_t)arg;

	cli_printf("ISR (INT) activated for GPIO ID: %d\n", gpio);
}

static void _gpio_thread_handler(uint8_t user_id, void* arg)
{
	uint32_t gpio = (uint32_t)arg;

	cli_printf("ISR (thread) activated for GPIO ID: %d\n", gpio);
}

static aos_gpio_mode_t _gpio_parse_mode(const char* mode_str)
{
	enum {opt_analog, opt_output, opt_input, opt_help, opt_count };
	static const cli_cmd_option_t options[opt_count] = {
			{ "analog", opt_analog },
			{ "output", opt_output },
			{ "input", opt_input },
			{ "help", opt_help },
	};


	switch(cli_get_option_index(options, opt_count, mode_str)) {
	case cli_cmd_option_index_not_found:
		cli_printf("Unknown mode: %s\n", mode_str);
		return aos_gpio_mode_last;

	case cli_cmd_option_index_ambiguous:
		cli_printf("Ambiguous mode: %s\n", mode_str);
		return aos_gpio_mode_last;

	case opt_analog:
		return aos_gpio_mode_analog;

	case opt_output:
		return aos_gpio_mode_output;

	case opt_input:
		return aos_gpio_mode_input;

	case opt_help:
		cli_printf("values:\n");
		cli_printf("    analog         	Analog GPIO\n");
		cli_printf("    output       	Digital output GPIO\n");
		cli_printf("    input       	Digital input GPIO\n");
		return aos_gpio_mode_last;

	default:
		break; // Should not happen, really, unless there is a program error.

	}
	cli_printf("Invalid option '%s'\n", mode_str);
	return aos_gpio_mode_last;
}

static bool _gpio_parse_config(int argc, char *argv[], aos_gpio_id_t id, aos_gpio_mode_t mode, aos_gpio_config_t* cfg)
{
	enum {opt_pno, opt_pup, opt_pdown, opt_out_pp, opt_out_ood, opt_ine, opt_ire, opt_ife,opt_ibe,
		opt_ip0, opt_ip1, opt_ip2, opt_ip3, opt_ip4, opt_isr, opt_ith,opt_help,  opt_count };
	static const cli_cmd_option_t options[] = {
			{ "pno", opt_pno },		// no pull
			{ "pup", opt_pup },		// pull up
			{ "pdn", opt_pdown },	// pull down
			{ "opp", opt_out_pp },	// output push pull
			{ "ood", opt_out_ood },	// output open drain
			{ "ine", opt_ine }, // No IRQ
			{ "ire", opt_ire }, // IRQ rising edge
			{ "ife", opt_ife }, // IRQ falling edge
			{ "ibe", opt_ibe }, // IRQ both edges
			{ "ip0", opt_ip0 }, // IRQ priority very low
			{ "ip1", opt_ip1 }, // IRQ priority low
			{ "ip2", opt_ip2 }, // IRQ priority medium
			{ "ip3", opt_ip3 }, // IRQ priority high
			{ "ip4", opt_ip4 }, // IRQ priority very high
			{ "isr", opt_isr },	// IRQ service ISR
			{ "ith", opt_ith },	// IRQ service thread
			{ "help", opt_help },
	};

	// Setup the defaults
	cfg->mode = mode;
	cfg->pull = aos_gpio_pull_type_none;
	cfg->output_type = aos_gpio_output_type_push_pull;
	cfg->irq_mode = aos_gpio_irq_mode_none;
	cfg->irq_prio = aos_gpio_irq_priority_very_low;
	cfg->irq_servicing = aos_gpio_irq_service_type_thread;
	cfg->irq_handler.sys_cb = NULL;
	cfg->user_arg = NULL;

	// Parse options
	for (int argn = 0; argn < argc; ++argn) {
		char *option = argv[argn];

		switch(cli_get_option_index(options, opt_count, option)) {
		case cli_cmd_option_index_not_found:
			cli_printf("Unknown option: %s\n", option);
			return false;

		case cli_cmd_option_index_ambiguous:
			cli_printf("Ambiguous option: %s\n", option);
			return false;

		case opt_pno:
			cfg->pull = aos_gpio_pull_type_none;
			break;

		case opt_pup:
			cfg->pull = aos_gpio_pull_type_pullup;
			break;

		case opt_pdown:
			cfg->pull = aos_gpio_pull_type_pulldown;
			break;

		case opt_out_pp:
			cfg->output_type= aos_gpio_output_type_push_pull;
			break;

		case opt_out_ood:
			cfg->output_type= aos_gpio_output_type_push_pull;
			break;

		case opt_ine:
			cfg->irq_mode = aos_gpio_irq_mode_none;
			break;

		case opt_ire:
				cfg->irq_mode = aos_gpio_irq_mode_rising_edge;
				break;

		case opt_ife:
			cfg->irq_mode = aos_gpio_irq_mode_falling_edge;
			break;

		case opt_ibe:
			cfg->irq_mode = aos_gpio_irq_mode_both_edges;
			break;

		case opt_ip0:
			cfg->irq_prio = aos_gpio_irq_priority_very_low;
			break;

		case opt_ip1:
			cfg->irq_prio = aos_gpio_irq_priority_low;
			break;

		case opt_ip2:
			cfg->irq_prio = aos_gpio_irq_priority_medium;
			break;

		case opt_ip3:
			cfg->irq_prio = aos_gpio_irq_priority_high;
			break;

		case opt_ip4:
			cfg->irq_prio = aos_gpio_irq_priority_very_high;
			break;

		case opt_isr:
			cfg->irq_servicing = aos_gpio_irq_service_type_int;
			break;

		case opt_ith:
			cfg->irq_servicing = aos_gpio_irq_service_type_thread;
			break;

		case opt_help:
			cli_printf("Options:\n");
			cli_printf(" Pullup selection\n");
			cli_printf("    pno         No pullup/pulldown (default)\n");
			cli_printf("    pup       	Pullup selection\n");
			cli_printf("    pdn       	Pulldown selection\n");
			cli_printf(" Output selection\n");
			cli_printf("    opp       	Output in push pull (default)\n");
			cli_printf("    ood       	Output in open drain\n");
			cli_printf(" Interrupt mode selection\n");
			cli_printf("    ine       	No interrupt (default)\n");
			cli_printf("    ire       	Interrupt on rising edge\n");
			cli_printf("    ife       	Interrupt on falling edge\n");
			cli_printf("    ibe       	Interrupt on both edges\n");
			cli_printf(" Interrupt priority selection\n");
			cli_printf("    ip0       	Interrupt priority very low (default)\n");
			cli_printf("    ip1       	Interrupt priority low\n");
			cli_printf("    ip2       	Interrupt priority medium\n");
			cli_printf("    ip3       	Interrupt priority high\n");
			cli_printf("    ip4       	Interrupt priority very high\n");
			cli_printf(" Interrupt service selection\n");
			cli_printf("    isr       	Interrupt serviced under interrupt context\n");
			cli_printf("    ith       	Interrupt serviced under thread context (default)\n");
			return cli_parser_status_ok;

		default:	// Should not happen, really, unless there is a program error.
			cli_printf("Invalid option '%s'\n", option);
			return false;
		}
	}

	if (cfg->irq_mode != aos_gpio_irq_mode_none) {
		if (mode != aos_gpio_mode_input) {
			cli_printf("Interrupts can be enabled only with output GPIOs\n");
			return false;
		}
		if (cfg->irq_servicing == aos_gpio_irq_service_type_int) {
			cfg->irq_handler.isr_cb = _gpio_isr_handler;
		} else {
			cfg->irq_handler.sys_cb = _gpio_thread_handler;
		}
		cfg->user_arg = (void*)id;
	}
	return true;
}


static cli_parser_status_t _cmd_gpio_display(void *arg, int argc, char *argv[])
{
	uint8_t ii;

	cli_printf(" %-8s%-35s%s\n","ID", "Name", "Mode");
	for (ii=0; ii < _gpio_name_map_count; ii ++) {
		cli_printf(" %-8d%-35s%s\n",ii, _gpio_name_map[ii].name, aos_gpio_get_mode_str(_gpio_name_map[ii].id));
	}
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_gpio_close(void *arg, int argc, char *argv[])
{
	aos_gpio_id_t gpio;
	aos_result_t rc;

	gpio = _get_gpio_id(argc, argv);

	if (gpio == aos_gpio_id_last) {
		return cli_parser_status_error;
	}

	rc = aos_gpio_close(gpio);
	if (rc == aos_result_success) {
		return cli_parser_status_ok;
	}
	cli_print_aos_result(rc);
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_gpio_read(void *arg, int argc, char *argv[])
{
	aos_gpio_id_t gpio;
	aos_result_t rc;
	uint16_t value;

	gpio = _get_gpio_id(argc, argv);
	if (gpio == aos_gpio_id_last) {
		return cli_parser_status_error;
	}

	rc = aos_gpio_read(gpio, &value);
	if (rc == aos_result_success) {
		cli_printf("Value: %d\n", value);
		return cli_parser_status_ok;
	}
	cli_print_aos_result(rc);
	return cli_parser_status_error;
}

static cli_parser_status_t _cmd_gpio_write(void *arg, int argc, char *argv[])
{
	aos_gpio_id_t gpio;
	aos_result_t rc;
	int32_t value;

	gpio = _get_gpio_id(argc, argv);
	if (gpio == aos_gpio_id_last) {
		return cli_parser_status_error;
	}

	if (argc < 3) {
		cli_print_missing_argument();
		return cli_parser_status_error;
	}
	if(!cli_parse_int(argv[2], &value)) {
		cli_printf("Invalid value\n");
		return aos_gpio_id_last;
	}

	rc = aos_gpio_write(gpio, value);
	if (rc == aos_result_success) {
		return cli_parser_status_ok;
	}
	cli_print_aos_result(rc);
	return cli_parser_status_error;
}


static cli_parser_status_t _cli_gpio_open(void *arg, int argc, char *argv[])
{
	aos_gpio_id_t gpio;
	aos_gpio_config_t gpio_cfg;
	aos_gpio_mode_t mode;
	aos_result_t rc;

	gpio = _get_gpio_id(argc, argv);
	if (gpio == aos_gpio_id_last) {
		return cli_parser_status_error;
	}

	// Retrieve the mode
	if (argc < 3) {
		cli_printf("The mode should be provided. Enter help to see the acceptable value\n");
		cli_print_missing_argument();
		return cli_parser_status_error;
	}
	mode = _gpio_parse_mode(argv[2]);
	if (mode == aos_gpio_mode_last) {
		return cli_parser_status_error;
	}

	if (argc== 3) {
		// Open the GPIO using the simple method
		rc = aos_gpio_open(gpio, mode);
	} else {
		//Parse the configuration
		if (!_gpio_parse_config(argc - 3, &argv[3], gpio, mode, &gpio_cfg)) {
			return false;
		}
		rc = aos_gpio_open_ext(gpio, &gpio_cfg);
	}

	if (rc == aos_result_success) {
		return cli_parser_status_ok;
	}
	cli_print_aos_result(rc);
	return cli_parser_status_error;
}


static const cli_parser_cmd_t _cli_gpio_command_table[] = {
	PARSER_CMD_FUNC("display", "Display all GPIOs", _cmd_gpio_display, GPIO_CMD_ACCESS),
	PARSER_CMD_FUNC("open <name> <mode> [config]", "Open and configure a GPIO", _cli_gpio_open, GPIO_CMD_ACCESS),
	PARSER_CMD_FUNC("close <name>", "Close GPIO", _cmd_gpio_close, GPIO_CMD_ACCESS),
	PARSER_CMD_FUNC("read <name>", "Read a GPIO", _cmd_gpio_read, GPIO_CMD_ACCESS),
	PARSER_CMD_FUNC("write <name> <value>", "Write a GPIO", _cmd_gpio_write, GPIO_CMD_ACCESS),
	PARSER_CMD_END
};

CLI_COMMAND_TAB_REGISTER(gpio, "GPIO related commands", _cli_gpio_command_table, GPIO_CMD_ACCESS);
