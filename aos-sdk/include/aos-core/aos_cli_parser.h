/*!
 * \file aos_cli_parser.h
 *
 * \brief Command Line Interface parser module. Parse commands and call associated user callbacks.
 *
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup cli_driver CLI driver
 *
 * \addtogroup cli_driver
 * @{
 */


#if !defined(CONFIG_CLI_PARSER_HELP)
/*!
 * \def CONFIG_CLI_PARSER_HELP
 *
 * \brief Enable or disable the help of the CLI commands
 *
 * \note CLI parser help is quite memory hungry, so we may want to disable it.
 */
#define CONFIG_CLI_PARSER_HELP (1)
#endif


/*!
 * \enum cli_parser_action_t
 *
 * \brief Action to be applied on a parser entry
 */
typedef enum {
	cli_parser_action_execute,			//!< Execute the command function (.func)
	cli_parser_action_recurse,			//!< Recurse into command table (.table)
} cli_parser_action_t;

/*!
 * \def cli_parser_access_none
 *
 * \brief Define the lowest access grant: Not authorized (be it level or mask). A login prompt
 * will be displayed to enter the PIN code if enabled.
 */
#define cli_parser_access_none (0)


/*!
 * \enum cli_parser_login_t
 *
 * \brief Manage the login prompt.
 */
typedef enum {
	cli_parser_login_hidden,			//!< Hidden login (no prompt, no echo)
	cli_parser_login_manual				//!< Manual login (prompted)
} cli_parser_login_t;


/*!
 * \enum cli_parser_echo_t
 *
 * \brief Manage the console echo
 */
typedef enum {
	cli_parser_echo_off,				//!< No echo at all.
	cli_parser_echo_local,				//!< Local echo. The received characters are echo'ed.
	cli_parser_echo_remote				//!< Remote echo. The peer is expected to do the character echo itself.
} cli_parser_echo_t;

/*!
 * \enum cli_parser_status_t
 *
 * \brief Status returned by the user callback functions. Based on the value, the status will
 * be displayed on the console.
 */
typedef enum {
	cli_parser_status_ok,		//!< Display "OK"
	cli_parser_status_error,	//!< Display "ERROR"
	cli_parser_status_void,		//!< Display nothing
} cli_parser_status_t;

/*!
 * \typedef cli_parser_handle_t
 *
 * \brief Opaque CLI parser handle
 */
typedef void *cli_parser_handle_t;

/*!
 * \struct cli_parser_cmd_s
 *
 * \brief The cli parser command table entry. Use the macros below to set it up,
 * so that you do not need to revisit your code to disable or enable help.
 */
typedef struct cli_parser_cmd_s {
	const char *command;				//!< Command string (NULL terminated)
#if (CONFIG_CLI_PARSER_HELP)
	const char *help;					//!< Command help text (NULL terminated)
#endif
	union {
		cli_parser_status_t (*func)(void *arg, int argc, char **argv); //!< Command function to be called
		const struct cli_parser_cmd_s *table; //!< Table for cli_parser_action_recurse
	};
	uint8_t access;					//!< Access grant required to execute this command
	cli_parser_action_t action;		//!< Action to be done when the entry matches the entered (sub) command
} cli_parser_cmd_t;					//!< Parser command as a type

#if (CONFIG_CLI_PARSER_HELP)
/*!
 * \def PARSER_CMD_FUNC
 *
 * \brief Helper to define an entry of the command array. The entry contains a user callback.
 */
#define PARSER_CMD_FUNC(cmd_s,help_s,function_p,access_u8) \
		{ .command = (cmd_s), .help = (help_s), { .func = (function_p) },  .action = cli_parser_action_execute, .access = (access_u8) }

/*!
 * \def PARSER_CMD_TAB
 *
 * \brief Helper to define an entry of the command array. The entry contains a sub-command table.
 */
#define PARSER_CMD_TAB(cmd_s,help_s,table_p,access_u8) \
		{ .command = (cmd_s), .help = (help_s), { .table = (table_p) },  .action = cli_parser_action_recurse, .access = (access_u8) }

#else
/*!
 * \def PARSER_CMD_FUNC
 *
 * \brief Helper to define an entry of the command array. The entry contains a user callback.
 */
#define PARSER_CMD_FUNC(cmd_s,help_s,function_p,access_u8) \
		{ .command = (cmd_s), { .func = (function_p) },  .action = cli_parser_action_execute, .access = (access_u8) }
/*!
 * \def PARSER_CMD_TAB
 *
 * \brief Helper to define an entry of the command array. The entry contains a sub-command table.
 */
#define PARSER_CMD_TAB(cmd_s,help_s,table_p,access_u8) \
		{ .command = (cmd_s), { .table = (table_p) },  .action = cli_parser_action_recurse, .access = (access_u8) }

#endif // (CONFIG_CLI_PARSER_HELP)

/*!
 * \def PARSER_CMD_END
 *
 * \brief Helper to define the end of a command or sub-command table
 */
#define PARSER_CMD_END {}

/*!
 * \enum cli_parser_event_t
 *
 * \brief Notification events sent the user via the notification callback
 */
typedef enum {
	cli_parser_event_auth_req,		//!< Authentication request. The cli parser notifies the caller that
									//!< a password needs to be validated. The notification pointer points to the
									//!< asciiz password received.

	cli_parser_event_signal,		//!< Signal. The cli parser notifies the caller that a control character
	 	 	 	 	 	 	 	 	//!< not handled internally has been received. The notification value contains the
									//!<control character.

	cli_parser_event_puts,			//!< Puts. The cli parser notifies the caller that some text needs to
	 	 	 	 	 	 	 	 	//!< be output to the user terminal. The notification pointer points to the
									//!< asciiz buffer to be displayed.
} cli_parser_event_t;

/*!
 * \struct cli_parser_notification_t
 *
 * \brief This is the primary method of communication (notifications) from the
 * cli parser to the calling application.
 */
typedef struct {
	cli_parser_event_t event;		//!< Notification event
	union {
		uint32_t value;				//!< Value belonging to the notification
		void *pointer;				//!< String buffer belonging to the notification
	};
} cli_parser_notification_t;

/*!
 * \typedef cli_parser_event_fn_t
 *
 * \brief User callback definition to receive notifications
 *
 * \param arg User argument (opaque for the parser).
 * \param event Notification sent from the CLI parser to the application.
 */
typedef void (*cli_parser_event_fn_t)(void *arg, cli_parser_notification_t *event);


/*!
 * \fn cli_parser_handle_t aos_cli_parser_init(char *cli_buffer, unsigned cli_buflen,
	char **arg_vectors, int max_args,
	const cli_parser_cmd_t *cmd_table, cli_parser_event_fn_t event_hook, void *user_arg)
 *
 * \brief Parser initialization function. It sets up the CLI command parser for use.
 *
 * \param cli_buffer  Pointer to a ram area to use as the CLI buffer.
 * \param cli_buflen  Length of the cli_buffer ram area.
 * \param arg_vectors Pointer to a ram area for storing an array of pointers
 * \param max_args    Number of pointers that can be stored in the argv[] array.
 * \param cmd_table   Address of an array of command table entries, terminated
 * 						by a PARSER_CMD_END entry.
 * \param event_hook  CLI parser notification handler function.
 * \param user_arg    User argument to be passed back in notifications.
 *
 * \return  The return value is an opaque handle to use for subsequent cli_parser
 * 			function calls. Upon initialization failure, a NULL value is returned.
 *
 * \note If a command line being processed contains more arguments than can be stored
 *       in the argv[] table, the last entry contains the rest of the command line.
 */
cli_parser_handle_t aos_cli_parser_init(char *cli_buffer, unsigned cli_buflen,
	char **arg_vectors, int max_args,
	const cli_parser_cmd_t *cmd_table, cli_parser_event_fn_t event_hook, void *user_arg);

/*!
 * aos_cli_parser_process_char() - cli parser input character processing function.
 *
 * This function is used to provide the cli parser with input characters from a
 * serial stream.
 *
 * @param handle	- the handle returned by cli_parser_init().
 *
 * @param ch		- the character to be processed.
 *
 * @return This function returns 0 on success.
 *
 */
int aos_cli_parser_process_char(cli_parser_handle_t handle, int ch);

/*!
 * \enum cli_parser_help_t
 *
 * \brief Help display format
 */
typedef enum {
	cli_parser_help_normal,		//!< one line per command
	cli_parser_help_long,		//!< long recursive help
	cli_parser_help_compact,	//!< compact help
} cli_parser_help_t;

/*!
 * \fn int aos_cli_parser_show_help(cli_parser_handle_t handle, const cli_parser_cmd_t *table, char *cmd, cli_parser_help_t format)
 *
 * \brief This function requests the parser to display help associated with a
 * command table, the current access level, and an eventual command.
 *
 * \param handle	Handle returned by cli_parser_init().
 *
 * \param table		Pointer to the command table in which to look up the
 * 					commands
 *
 * \param cmd		Pointer to  abuffer containing a [partial] command to
 * 					use as a filter, or NULL to show help for all commands
 * 					accessible for the current access level.
 *
 * \param format	Whether to provide normal, long or compact help.
 *
 * \return This function returns the number of matching commands found.
 *
 */
int aos_cli_parser_show_help(cli_parser_handle_t handle, const cli_parser_cmd_t *table, char *cmd, cli_parser_help_t format);

/*!
 * \fn int aos_cli_parser_show_help_ext(cli_parser_handle_t handle, const cli_parser_cmd_t *table, int argc, char *argv[])
 *
 * \brief This function is requests the parser to display help associated with a
 * command table, the current access level, and an eventual sequence of commands.
 *
 * \param handle	Handle returned by cli_parser_init().
 *
 * \param table		Pointer to the command table in which to look up the
 * 					  commands
 *
 * \param argc		Number of commands passed in the argv[] array.
 *
 * \param argv[]	Pointer to  an array of string pointers containing
 * 					  the command line for which to provide help.
 *
 * \return  This function returns a numeric value indicating the status of
 * 			the request:
 * 				< 0	An unspecified error occurred (likely a bad parameter).
 * 				  0 No match was found for one of the commands in the list.
 * 				  1 A match was found for all of the commands in the list.
 * 				> 1 An ambiguity exists in one of the commands in the list.
 *
 * \note The return status is for informational purposes only, a corresponding
 *		 error message has already been displayed if needed.
 */
int aos_cli_parser_show_help_ext(cli_parser_handle_t handle, const cli_parser_cmd_t *table, int argc, char *argv[]);


/*!
 * \enum cli_parser_ioctl_req_t
 *
 * \brief IO control types
 */
typedef enum {
	cli_parser_ioctl_req_set_login_type,	//!< Set login type. Value = cli_parser_login_t
	cli_parser_ioctl_req_set_user_arg,		//!< Set user argument
	cli_parser_ioctl_req_set_cmdtab,		//!< Set user command table
	cli_parser_ioctl_req_set_event_cb,		//!< Set user event callback handler
	cli_parser_ioctl_req_set_access_level,	//!< Set terminal access level (0 logs out)
	cli_parser_ioctl_req_set_access_mask,	//!< Set terminal access mask (0 logs out)
	cli_parser_ioctl_req_set_echo,			//!< Set input echo, value = true / false
	cli_parser_ioctl_req_set_prompt,		//!< Pointer is asciiz prompt
	cli_parser_ioctl_req_set_history_buffer,//!< Pointer is buffer, len must be same as cmd buffer
} cli_parser_ioctl_req_t;

/*!
 * \struct cli_parser_ioctl_t
 *
 * \brief Information block belonging to an IO control
 */
typedef struct {
	cli_parser_ioctl_req_t req;			//!< IO Request type
	union {
		uint32_t value;					//!< Value belonging to the IO request
		void *pointer;					//!< Data belonging to the IO request
	};
} cli_parser_ioctl_t;


/*!
 * \fn int aos_cli_parser_ioctl(cli_parser_handle_t handle, cli_parser_ioctl_t *request)
 *
 * \brief CLI parser control function to get or set various parameters.
 * This is a classical ioctl function allowing to get or set parameters as specified
 * in the request.
 *
 * \param handle	Handle returned by cli_parser_init().
 * \param request	Request to be processed.
 *
 * \return This function returns 0 on success.
 */
int aos_cli_parser_ioctl(cli_parser_handle_t handle, cli_parser_ioctl_t *request);


/*!
 * \fn int int aos_cli_parser_set_ptr(cli_parser_handle_t h, cli_parser_ioctl_req_t req, void *ptr)
 *
 * \brief Helper that create an IO request with a pointer as data.
 *
 * \param h			Handle returned by cli_parser_init().
 * \param req		Request to be processed.
 * \param ptr		Pointer to the data to pass along the request
 *
 * \return This function returns 0 on success.
 */
static inline int aos_cli_parser_set_ptr(cli_parser_handle_t h, cli_parser_ioctl_req_t req, void *ptr)
{
	cli_parser_ioctl_t ioc;

	ioc.req = req;
	ioc.pointer = ptr;

	return aos_cli_parser_ioctl(h, &ioc);
}


/*!
 * \fn int int aos_cli_parser_set_value(cli_parser_handle_t h, cli_parser_ioctl_req_t req, uint32_t value)
 *
 * \brief Helper that create an IO request with an integer as data.
 *
 * \param h			Handle returned by cli_parser_init().
 * \param req		Request to be processed.
 * \param value		Value to pass along the request
 *
 * \return This function returns 0 on success.
 */
static inline int aos_cli_parser_set_value(cli_parser_handle_t h, cli_parser_ioctl_req_t req, uint32_t value)
{
	cli_parser_ioctl_t ioc;

	ioc.req = req;
	ioc.value = value;

	return aos_cli_parser_ioctl(h, &ioc);
}

/*!
 * \def aos_cli_parser_set_echo(_h,_v)
 *
 * \brief Helper to setup the echo mode
 *
 * \param _h Handle returned by cli_parser_init()
 * \param _v Echo mode expected.
 */
#define aos_cli_parser_set_echo(_h,_v) aos_cli_parser_set_value(_h,cli_parser_ioctl_req_set_echo, _v)

/*!
 * \def aos_cli_parser_set_prompt(_h,_v)
 *
 * \brief Helper to setup the prompt(_h,_v)
 *
 * \param _h Handle returned by cli_parser_init()
 * \param _v Pointer to the string to be used as the prompt
 */
#define aos_cli_parser_set_prompt(_h,_v) aos_cli_parser_set_ptr(_h,cli_parser_ioctl_req_set_prompt, _v)

/*!
 * \def aos_cli_parser_set_access_level(_h,_v)
 *
 * \brief Helper to setup the current access level
 *
 * \param _h Handle returned by cli_parser_init()
 * \param _v New access level
 */
#define aos_cli_parser_set_access_level(_h,_v) aos_cli_parser_set_value(_h,cli_parser_ioctl_req_set_access_level, _v)

/*!
 * \def aos_cli_parser_set_access_mask(_h,_v)
 *
 * \brief Helper to setup the current access mask
 *
 * \param _h Handle returned by cli_parser_init()
 * \param _v New access level mask
 */
#define aos_cli_parser_set_access_mask(_h,_v) aos_cli_parser_set_value(_h,cli_parser_ioctl_req_set_access_mask, _v)


/*! @}*/
#ifdef __cplusplus
}
#endif
