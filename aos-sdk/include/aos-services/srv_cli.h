/*!
 * \file srv_cli.h
 *
 * \brief Command Line Interface service
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once

#include <stdio.h>
#include <stdarg.h>
#include "aos_uart.h"
#include "aos_cli.h"
#include "aos_cli_parser.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup cli_srv Command Line Interface service (CLI)
 *
 * \addtogroup cli_srv
 * @{
 */

/*!
 * \struct cli_cmd_option_t
 *
 * \brief Structure used to manage command options
 */
typedef struct {
	const char *name;	//!< Address of a NULL terminated option string.
	uint8_t id;			//!< User defined option ID. This should be a positive value.
} cli_cmd_option_t;

/*
 * Anonymous enum cannot be documented.
 *
 * \brief Returned value of the function cli_get_option_index in case of error.
 */
enum {
	cli_cmd_option_index_ambiguous = -2,	//!< cli_get_option_index() return value: error, the option is ambiguous
	cli_cmd_option_index_not_found = -1,	//!< cli_get_option_index() return value: error, The option is not found
};

/*!
 * \enum cli_access_mask_t
 *
 * \brief CLI access mask defining the acceptable level of credential for
 *        a command or a sub-commad.
 *
 * \note Bit field value.
 */
typedef enum {
	cli_access_mask_none = 0x00,		//!< No credential required
	cli_access_mask_user = 0x01,		//!< User level credential required
	cli_access_mask_super = 0x02		//!< Super user credential required.
} cli_access_mask_t;

/*!
 * \struct cli_config_param_t
 *
 * \brief CLI configuration parameters
 */
typedef struct {
	uint32_t user_psw;				//!< PIN code expected to grant the user level
	uint32_t super_psw;				//!< PIN code expected to grant the super user level
	aos_uart_config_t uart_cfg;		//!< Serial port configuration
} cli_config_param_t;

/*!
 * \def CLI_ACCESS_ALL_LEVELS
 *
 * \brief Macro helper to be used for a command requiring either the user
 *        or the super user credential
 */
#define CLI_ACCESS_ALL_LEVELS (cli_access_mask_user | cli_access_mask_super)

/*!
 * \def CLI_ACCESS_FROM_SUPER_LEVEL
 *
 * \brief Macro helper to be used for a command requiring the super user credential.
 */
#define CLI_ACCESS_FROM_SUPER_LEVEL  (cli_access_mask_super)


/*!
 * \def CLI_COMMAND_FUNC_REGISTER
 *
 * \brief Register a main command against the CLI manager.
 *
 * \param cmd_s Command name (string)
 * \param help_s Command help (string)
 * \param function_p Callback called when the command has been parsed.
 * \param access_u8 Access level required to execute this command.
 *
 * \note  The CLI_COMMAND_REGISTER() macro requires a specific linker script
 * modification within the SECTIONS block. Sorting by name is recommended, as
 * otherwise the commands appear in the order the linker finds them - that is,
 * randomly.
 *
 * The linker command should have the following lines:
 * <pre>
 *  .commands : {
 *      . = ALIGN(4);
 *      _cli_command_table = .;
 *	   KEEP(*(SORT_BY_NAME(.commands.*)));
 *      LONG (0)
 *  } > FLASH</pre>
 */
#define CLI_COMMAND_FUNC_REGISTER(cmd_s,help_s,function_p,access_u8) \
		__attribute__((section(".commands."#cmd_s),used))  \
		static const cli_parser_cmd_t __cmdfun_##cmd_s = { \
				.command = #cmd_s, .help = (help_s), \
				{ .func = (function_p) }, \
				.action = cli_parser_action_execute, \
				.access = (access_u8) }

/*!
 * \def CLI_COMMAND_TAB_REGISTER
 *
 * \brief Register a CLI sub-command table
 *
 * \param cmd_s Sub-command name for which the command table will apply
 * \param help_s Sub-command help
 * \param table_p Table containing the sub-command actions
 * \param access_u8 Access level required to execute this sub-command.
 */
#define CLI_COMMAND_TAB_REGISTER(cmd_s,help_s,table_p,access_u8) \
		__attribute__((section(".commands."#cmd_s),used))  \
		static const cli_parser_cmd_t __cmdtab_##cmd_s = { \
				.command = #cmd_s, .help = (help_s), \
				{ .table = (table_p) }, \
				.action = cli_parser_action_recurse, \
				.access = (access_u8) }


/*
 * ***************************************************************************
 * Service management
 * ***************************************************************************
 */
/*!
 *
 * \fn aos_result_t srv_cli_init(void)
 *
 * \brief Initialize the common line interface service
 *
 * \return The result status
 */
aos_result_t srv_cli_init(void);

/*!
 *
 * \fn aos_result_t srv_cli_open(aos_uart_type_t uart_type, cli_config_param_t* config)
 *
 * \brief Open the common line interface service
 *
 * \param uart_type Physical port on which the CLI should run
 * \param config CLI configuration parameters.
 * \return The result status
 */
aos_result_t srv_cli_open(aos_uart_type_t uart_type, cli_config_param_t* config);

/*!
 *
 * \fn aos_result_t srv_cli_close(void)
 *
 * \brief Close the common line interface service
 *
 * \return The result status
 */
aos_result_t srv_cli_close(void);

/*!
 * \fn void srv_cli_restart(cli_parser_echo_t echo)
 *
 * \brief Restart the CLI
 *
 * \param echo Indicate the echo mode when restarting
 */
void srv_cli_restart(cli_parser_echo_t echo);

/*!
 *
 * \fn aos_uart_type_t srv_cli_get_uart_type(void)
 *
 * \brief Return the cli interface in use
 *
 * \return The cli driver used.
 */
aos_uart_type_t srv_cli_get_uart_type(void);

/*
 * ***************************************************************************
 * Display utilities
 * ***************************************************************************
 */
/*!
 * \fn int cli_printf(const char *fmt, ...)
 *
 * \brief Print to the CLI console
 *
 * \param fmt Format string
 * \return The number of bytes printed. Negative values indicate an error.
 *
 * \note No timestamp are displayed.
 *
 */
int cli_printf(const char *fmt, ...);

/*!
 * \fn int cli_print_hex(const void *p, uint8_t len, bool lf)
 *
 * \brief Print in hexa format to the CLI console
 *
 * \param p Binary buffer to be displayed in hexa format
 * \param len Size of the binary buffer
 * \param lf True if a line feed and carriage return should be instered at the end of the display
 * \return The number of bytes printed. Negative values indicate an error.
 *
 * \note Values are separated by a space
 */
int cli_print_hex(const void *p, uint8_t len, bool lf);

/*!
 * \brief Print in hexa format to the CLI console
 *
 * \param p Binary buffer to be displayed in hexa format
 * \param len Size of the binary buffer
 * \param separator String containing the separator to use between the values
 * \param lf True if a line feed and carriage return should be instered at the end of the display
 * \return The number of bytes printed. Negative values indicate an error.
 *
 * \note The separator is not restricted to a single character
 */
int cli_print_hex_with_separator(const void *p, uint8_t len, char* separator, bool lf);

/*!
 * \fn int cli_fill_with_byte(char c, uint32_t count);
 *
 * \brief Fill the display which a given character
 *
 * \param c Filler character
 * \param count Size to fill
 * \return The number of bytes printed. Negative values indicate an error.
 *
 */
int cli_fill_with_byte(char c, uint32_t count);

/*!
 * \fn int cli_log(const char* prefix, bool timestamp, bool warning, const char *fmt, va_list ap)
 *
 * \brief log a message via the CLI console
 *
 * \param prefix  Log prefix
 * \param timestamp True if the time stamp should be inserted, false otherwise
 * \param warning True if it is a warning log
 * \param fmt Format string
 * \param ap Variable argument list
 * \return The number of bytes printed. Negative values indicate an error.
 *
 * \note Refer to the log service for details on how the log service works.
 */
int cli_log(const char* prefix, bool timestamp, bool warning, const char *fmt, va_list ap);


/*
 * ***************************************************************************
 * Help management
 * ***************************************************************************
 */
/*!
 * \fn bool srv_cli_show_help_ext(int argc, char *argv[])
 *
 * \brief Display the help of a command
 *
 * \param argc Number of arguments
 * \param argv List or arguments
 *
 * \return True if the help has been displayed, false otherwise.
 *
 * \note The argument list should contain the command tree containing the command or
 * sub-command you want to display.
 *
 * Example:
 * <pre>
	// Command callback
	static  cli_parser_status_t _cli_cmd_help(void *arg, int argc, char *argv[])
	{
		srv_cli_show_help_ext(argc, argv);
		return cli_parser_status_void;
	}

	// Command registration (Main command)
	CLI_COMMAND_FUNC_REGISTER(help, "[cmd] Display help information", _cli_cmd_help, CLI_ACCESS_ALL_LEVELS);</pre>
 *
 */
bool srv_cli_show_help_ext(int argc, char *argv[]);

/*!
 * \fn bool srv_cli_show_help(void)
 *
 * \brief Display the help of all commands and sub-commands
 *
 * \return True if the help has been displayed, false otherwise.
 */
bool srv_cli_show_help(void);

/*!
 * \fn bool srv_cli_set_access_mask(cli_access_mask_t mask)
 *
 * \brief Set the access level mask, granting a given credential to the current CLI session
 *
 * \param mask New access mask to apply to the current session
 *
 * \return True if the access mask has been correctly applied
 */
bool srv_cli_set_access_mask(cli_access_mask_t mask);


/*
 * ***************************************************************************
 * Toolbox
 * ***************************************************************************
 */

/*!
 * \fn bool cli_strcase_ncmp(const char* c1, const char* c2, uint16_t len)
 *
 * \brief Compare two strings regardless the case.
 *
 * \param c1 String 1 to be compared
 * \param c2 String 2 comapred against the string 1
 * \param len Comparison length
 *
 * \return True if the two strings are equals, false otherwise.
 */
bool cli_strcase_ncmp(const char* c1, const char* c2, uint16_t len);

/*!
 * \fn void cli_print_missing_argument(void)
 *
 * \brief Display the "Missing argument" string
 */
void cli_print_missing_argument(void);

/*!
 * \fn void cli_print_invalid_param_id(void);
 *
 * \brief Display the "Invalid parameter ID" string
 */
void cli_print_invalid_param_id(void);

/*!
 * \fn char* cli_str_on_off(bool value)
 *
 * \brief Return the string "on" or "off" according to the value parameter.
 *
 * \param value True to get the "on" string, false to have the "off" one.
 *
 * \return The string based on the value parameter.
 */
const char* cli_str_on_off(bool value);

/*!
 * \fn char* cli_str_yes_no(bool value)
 *
 * \brief Return the string "yes" or "no" according to the value parameter.
 *
 * \param value True to get the "yes" string, false to have the "no" one.
 *
 * \return The string based on the value parameter
 */
const char* cli_str_yes_no(bool value);

/*!
 * \fn char* cli_str_success_failure(bool value)
 *
 * \brief Return the string "success" or "failure" according to the value parameter
 *
 * \param value True to get the "success" string, false to have the "failure" one.
 *
 * \return The string based on the value parameter.
 */
const char* cli_str_success_failure(bool value);

/*!
 * \fn void cli_print_aos_result(aos_result_t result)
 *
 * \brief Display a message containing the result.
 *
 * \param result Result to convert
 *
 */
void cli_print_aos_result(aos_result_t result);

/*!
 * \fn bool cli_parse_int(const char* str, int32_t* value)
 *
 * \brief Convert a string to an integer. The hexadecimal format (0x...) is also accepted.
 *
 * \param str String to parse
 * \param value to the integer value parsed
 *
 * \return True if the operation is successful. False otherwise.
 */
bool cli_parse_int(const char* str, int32_t* value);

/*!
 * \fn bool cli_parse_float(const char *str, float *ret_value)
 *
 * \brief Convert a nul-terminated string to a float value.
 *
 * \param str pointer to a nul-terminated character string to parse
 * \param ret_value pointer to a float to receive the result.
 *
 * \return True if the operation is successful. False otherwise.
 */
bool cli_parse_float(const char *cp, float *ret_value);

/*!
 * \fn char* cli_remove_str_quotes(char* str)
 *
 * \brief Remove the double quotes enclosing a string if any.
 *
 * \param str String to parse
 *
 * \return The begininng of the string without the quote
 *
 * \note if the closing quote is found, it is removed
 */
char* cli_remove_str_quotes(char* str);

/*!
 * \fn int cli_get_option_index(const cli_cmd_option_t *options, int num_options, const char *option)
 *
 * \brief Return the index of an option.
 *
 * \param options Array of options to be parsed
 * \param num_options Numbers of option in the array
 * \param option Option in string format to be matched against the array.
 *
 * \return True if the operation is successful. False otherwise.
 */
int cli_get_option_index(const cli_cmd_option_t *options, int num_options, const char *option);


/*!
 * \fn const char* cli_get_option_name(unsigned value, const cli_cmd_option_t map[], size_t map_count)
 *
 * \brief Retrieve the name of an option.
 *
 * \param value Option value
 * \param map Array of options to be parsed
 * \param map_count Number of options in the array
 *
 * \return The name of the option if found, "unknown" otherwise
 */
const char* cli_get_option_name(unsigned value, const cli_cmd_option_t map[], size_t map_count);

/*!
 * \fn int cli_list_options(const char *heading, const cli_cmd_option_t *option_map, int option_map_entry_count)
 *
 * \brief List all possible options.
 *
 * \param heading Optional heading string to be displayed
 * \param option_map Array of options to be parsed
 * \param option_map_entry_count Numbers of option in the array
 *
 * \return nothing
 */
void cli_list_options(const char *heading, const cli_cmd_option_t *option_map, int option_map_entry_count);

/*!
 * \fn int cli_get_boolean(const char *option)
 *
 * \brief Parse a boolean option value.
 *
 * \param option pointer to a C string to process
 *
 * \return A negative value on error, else 0 (false) or 1 (true).
 */
int cli_get_boolean(const char *s);

/*!
 * \fn int cli_xdump(void *ptr, unsigned len)
 *
 * \brief Display a hex dump of a buffer
 *
 * \param ptr pointer to binary buffer to display
 * \param len length of binary buffer to display
 *
 */
void cli_xdump(void *p, unsigned len);


/*!
 * \fn void cli_print_systime(uint32_t seconds)
 *
 * \brief Display the system date/time
 *
 * \param seconds number of seconds since UNIX epoch
 *
 */
void cli_print_systime(uint32_t seconds);

/*!
 * \fn aos_uart_type_t srv_cli_get_uart_type(void)
 *
 * \brief Return UART type in use for CLI
 *
 * \return Return UART type
 *
 */
aos_uart_type_t srv_cli_get_uart_type(void);
/*! @}*/
#ifdef __cplusplus
}
#endif
