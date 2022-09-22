/*!
 * \file cli-cmd-uart.c
 *
 * \brief UART commands
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"	// FreeRTOS, task info
#include "aos_board.h"
#include "aos_uart.h"
#include "aos_adc.h"
#include "aos_error.h"
#include "srv_cli.h"

/*
 * ----------------------------------------------------------------------------
 * UART stats command
 * ----------------------------------------------------------------------------
 */
static const cli_cmd_option_t _uart_type_map[] = {
		{ "lpuart1", aos_uart_type_lpuart1 },
		{ "usart1", aos_uart_type_usart1 },
};
static const unsigned _uart_type_map_count = sizeof(_uart_type_map) / sizeof(*_uart_type_map);

aos_uart_type_t _cli_stats_parse_uart_str(char* name)
{
	int choice;

	choice = cli_get_option_index(_uart_type_map, _uart_type_map_count, name);
	if (choice >= 0) {
		return choice;	// Valid selection made
	}

	cli_list_options("Unknown UART type. Accepted types are:", _uart_type_map, _uart_type_map_count);
	return aos_uart_type_last;
}

static cli_parser_status_t _cli_stats_display(void *arg, int argc, char **argv)
{
	aos_uart_type_t type;
	aos_uart_ioctl_info_t req;
	aos_result_t result;

	if (argc < 2) {
		cli_print_missing_argument();
		return cli_parser_status_error;
	}
	type = _cli_stats_parse_uart_str(argv[1]);
	if (type == aos_uart_type_last) {
		return cli_parser_status_error;
	}

	req.type = aos_uart_ioctl_type_get_stats;
	result = aos_uart_driver.ioctl(type, &req);
	if (result == aos_result_not_open) {
		cli_printf("UART not open\n");
		return cli_parser_status_error;
	}

	if (result != aos_result_success) {
		cli_printf("UART error: %d\n", result);
		return cli_parser_status_error;
	}

	// Display stats
	cli_printf("Number of bytes received: %d\n", req.stats->rx_bytes);
	cli_printf("Number of transmitted bytes: %d\n", req.stats->tx_bytes);
	cli_printf("Number of times the UART has woken up: %d\n", req.stats->wakeup);
	cli_printf("Number of times the sleeping mode has been accepted: %d\n", req.stats->sleep_accepted);
	cli_printf("Number of times the sleeping mode has been refused: %d\n", req.stats->sleep_refused);
	cli_printf("Number of RX bytes with a parity error: %d\n", req.stats->parity_errors);
	cli_printf("Number of RX noise detection error: %d\n", req.stats->noise_errors);
	cli_printf("Number of RX framing errors: %d\n", req.stats->framing_errors);
	cli_printf("Number of RX overrun error: %d\n", req.stats->ovr_errors);
	cli_printf("Number of bytes rejected due to RX FIFO full: %d\n", req.stats->rx_fifo_full);

	return cli_parser_status_ok;
}


static cli_parser_status_t _cli_stats_clear(void *arg, int argc, char **argv)
{
	aos_uart_type_t type;
	aos_uart_ioctl_info_t req;
	aos_result_t result;

	if (argc < 2) {
		cli_print_missing_argument();
		return cli_parser_status_error;
	}
	type = _cli_stats_parse_uart_str(argv[1]);
	if (type == aos_uart_type_last) {
		return cli_parser_status_error;
	}

	req.type = aos_uart_ioctl_type_clear_stats;
	result = aos_uart_driver.ioctl(type, &req);

	if (result == aos_result_not_open) {
		cli_printf("UART not open\n");
		return cli_parser_status_error;
	}

	if (result != aos_result_success) {
		cli_printf("UART error: %d\n", result);
		return cli_parser_status_error;
	}

	cli_printf("UART stats cleared\n");
	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _uart_cmd_table[] = {
		PARSER_CMD_FUNC("stats <uart>", "Read statistics. Parameter uart can be 'lpuart' or 'usart' ", _cli_stats_display, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("clear <uart>", "Clear statistics. ", _cli_stats_clear, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_END
};


/*
 * ----------------------------------------------------------------------------
 * Main Command registration
 * ----------------------------------------------------------------------------
 */
CLI_COMMAND_TAB_REGISTER(uart,"UART commands", _uart_cmd_table, CLI_ACCESS_ALL_LEVELS );

