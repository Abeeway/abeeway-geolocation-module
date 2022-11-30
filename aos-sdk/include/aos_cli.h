/*!
 * \file aos_cli.h
 *
 * \brief Common Line Interface low layer
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "aos_cli_parser.h"

/*!
 * \defgroup cli_low_layer CLI low layer management
 *
 * \addtogroup cli_low_layer
 * @{
 */

#ifndef CLI_OPTION_COMMAND_RECALL
/*!
 * \def CLI_OPTION_COMMAND_RECALL
 *
 * \brief CLI_OPTION_COMMAND_RECALL is a compile time conditional for
 * enabling or disabling the feature. This does need a certain
 * amount of RAM which we may not be able to afford on our MCU,
 * especially for the recall buffer which needs to be as big as
 * the command buffer.
 *
 * The code for the command line recall uses 200 bytes of flash
 * when compiled with -Os. Choices, choices...
 *
 * \warning THIS CONDITIONAL IS REQUIRED FOR BUILDING THE LIBRARY.
 * 		 DO NOT CHANGE IT FOR THE APPLICATION BUILD !
 *
 */
#define CLI_OPTION_COMMAND_RECALL (1)
#endif

#ifndef CLI_OPTION_COMMAND_EDITING
/*!
 * \def CLI_OPTION_COMMAND_EDITING
 *
 * \brief CLI_OPTION_COMMAND_EDITING enabled command line editing via
 * the terminal left/right arrow keys. The 'INS' key toggles between
 * insertion mode (default) and overstrike mode, ^A moves to the
 * beginning of the line, ^E to the end of the line. This does need
 * a certain amount of flash memory for the additional code.
 *
 * The code for the command line editing uses 314 bytes of flash
 * when compiled with -Os. Choices, choices...
 *
 * \warning THIS CONDITIONAL IS REQUIRED FOR BUILDING THE LIBRARY.
 *       DO NOT CHANGE IT FOR THE APPLICATION BUILD !
 *
 */
#define CLI_OPTION_COMMAND_EDITING (1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \enum cli_event_t
 *
 * \brief CLI event notifications are used by the CLI low layer to notify
 * the upper layer of certain events, such as the need to output
 * some text, the availability of a new command line, or a control
 * character.
 */
typedef enum {
	cli_event_put_string,	//!< Pointer points to asciiz buffer to be printed
	cli_event_cmdline,		//!< Pointer points to asciiz command line to be executed
	cli_event_signal,		//!< Value contains the control character received
} cli_event_t;

/*!
 * \struct cli_notification_t
 *
 * \brief Notification sent to the CLI user
 */
typedef struct {
	cli_event_t event;		//!< Event being notified
	union {
		void *pointer;		//!< Event related pointer
		uint32_t value;		//!< Event related value
	};
} cli_notification_t;

/*!
 * \typedef void (*cli_notification_fn_t)(void *arg, cli_notification_t *notification)
 *
 * \brief The cli notification function is called with the user argument
 * and a pointer to the notification.
 *
 * \param arg User argument (opaque)
 * \param notification Notification passed along the callback
 */
typedef void (*cli_notification_fn_t)(void *arg, cli_notification_t *notification);

/*!
 * \struct cli_t
 *
 * \brief The cli_t structure should be private/opaque, really. But since
 * we are doing plain C and don't have dynamic memory, that's just
 * not possible - so we rely on the user not to use or modify
 * anything in this structure.
 *
 * \warning This structure is NOT part of the API,  its contents may
 * 			change at any time without notice. You have been warned !
 *
 */
typedef struct {
	char *buffer;					//!< Caller provided command buffer
	const char *prompt;				//!< Caller provided prompt string
	void *user_arg;					//!< Caller provided argument
	cli_notification_fn_t user_notify;	//!< Caller provided event notification function
#if (CLI_OPTION_COMMAND_RECALL)
	char *recall_buffer;			//!< Caller provided - must be the same size as command buffer
	uint8_t recall_count;			//!< Number of bytes in the recall buffer
	uint8_t recall_index;			//!< Current index  in the recall buffer
#endif // (CLI_OPTION_COMMAND_RECALL)
	uint8_t buflen;					//!< Current length of command buffer
	uint8_t bufmax;					//!< Length of user-provided command (and history) buffers
#if (CLI_OPTION_COMMAND_EDITING)
	uint8_t bufpos;					//!< current position in command buffer
	bool overstrike;				//!< overstrike or insert ?
#endif
	bool authorized;				//!< Whether this session is authorized
	bool echo;						//!< Whether we should echo input (and display a prompt)
	enum {
		cli_esc_inactive,				//!< Not currently escaping
		cli_esc_escape_seen,			//!< 'ESC' char was seen
		cli_esc_o_seen,					//!< "'ESC'O" was seen
		cli_esc_bracket_seen			//!< "'ESC'[" was seen
	} escaping;						//!< Escaping state machine states
	uint16_t escval;				//!< Escape decimal value
} cli_t;

/*!
 * \enum cli_ioctl_req_t
 *
 * \brief CLI IOCTLs. These are used to get/set CLI parameters.
 */
typedef enum {
	cli_ioctl_req_set_user_arg,		//!< Set the user argument to pointer
	cli_ioctl_req_set_echo,			//!< Set command echo to value
	cli_ioctl_req_set_prompt,		//!< Set prompt to pointer
	cli_ioctl_req_set_authorized,	//!< Set authorized to value
	cli_ioctl_req_set_recall_buf,	//!< Set recall buffer to pointer
} cli_ioctl_req_t;

/*!
 * \struct cli_ioctl_t
 *
 * \brief Send IO control the the CLI low layer
 */
typedef struct {
	cli_ioctl_req_t req;				//!< IO Request type
	union {
		uint32_t value;					//!< Value belonging to the IO request
		void *pointer;					//!< Data belonging to the IO request
	};
} cli_ioctl_t;

/*!
 * \fn int aos_cli_init(cli_t *clip, char *buf, unsigned buflen, cli_notification_fn_t notify, void *user_arg);
 *
 * \brief initialize an instance of the CLI processor.
 * This function initialises an instance of the CLI processor.
 *
 * \param clip		Pointer to a cli_t structure where context is to be stored.
 * \param buf		Pointer to a buffer for command input
 * \param buflen	Size of the command input buffer
 * \param notify	Event notification function
 * \param user_arg	User argument to pass to the event notification function.
 * \return	This function returns 0 on success, non-zero on failure.
 */
int aos_cli_init(cli_t *clip, char *buf, unsigned buflen, cli_notification_fn_t notify, void *user_arg);

/*!
 * \fn void aos_cli_process_char(cli_t *clip, int ch)
 *
 * \brief This function accepts a character and processes it. This function must
 * be called by the upper layers to provide a character to process, since
 * the CLI itself has no knowledge of the operating system specific ways
 * to input characters from a terminal or whatever.
 *
 * \param clip		Pointer to the cli_t structure.
 * \param ch		Integer value representing the character to process.
 */
void aos_cli_process_char(cli_t *clip, int ch);

/*!
 *\fn int aos_cli_ioctl(cli_t *clip, cli_ioctl_t *ioc);
 *
 * \brief Control request to the CLI.
 * This function is used to get or set various CLI parameters.
 *
 * \param clip		Pointer to the cli_t structure.
 * \param ioc		Pointer to a cli_ioctl_t request.
 *
 * \return	This function returns 0 on success, non-zero on failure.
 *
 */
int aos_cli_ioctl(cli_t *clip, cli_ioctl_t *ioc);

/*!
 * \fn int aos_cli_ioctl_set_pointer(cli_t *cli, cli_ioctl_req_t req, char *ptr)
 *
 * \brief Generic wrapper functions to simplify IOCTLs.
 *
 * \param cli Pointer to the cli_t structure.
 * \param req IO request to be filled
 * \param ptr Pointer to the data that belong to the IO request
 *
 * \return	This function returns 0 on success, non-zero on failure.
 *
 */
static inline int aos_cli_ioctl_set_pointer(cli_t *cli, cli_ioctl_req_t req, char *ptr)
{
	cli_ioctl_t _ioc;

	_ioc.req = req;
	_ioc.pointer = ptr;

	return aos_cli_ioctl(cli, &_ioc);
}

/*!
 * \fn int aos_cli_ioctl_set_value(cli_t *cli, cli_ioctl_req_t req, uint32_t value)
 *
 * \brief Generic wrapper functions to simplify IOCTLs.
 *
 * \param cli Pointer to the cli_t structure.
 * \param req IO request to be filled
 * \param value Integer that belong to the IO request
 *
 * \return	This function returns 0 on success, non-zero on failure.
 */
static inline int aos_cli_ioctl_set_value(cli_t *cli, cli_ioctl_req_t req, uint32_t value)
{
	cli_ioctl_t _ioc;

	_ioc.req = req;
	_ioc.value = value;

	return aos_cli_ioctl(cli, &_ioc);
}


/*!
 * \def aos_cli_set_echo(c,v)
 *
 * \brief Helper to set the echo mode
 *
 * \param c Pointer to the cli_t structure.
 * \param v Echo mode to set
 *
 * \return	This function returns 0 on success, non-zero on failure.
 *
 */
#define aos_cli_set_echo(c,v) aos_cli_ioctl_set_value(c, cli_ioctl_req_set_echo, v)

/*!
 * \def aos_cli_set_authorized(c,v)
 *
 * \brief Helper to set whether the current CLI session is authorized.
 *
 * \param c Pointer to the cli_t structure.
 * \param v Value true to authorize, false to deny.
 *
 * \return	This function returns 0 on success, non-zero on failure.
 */
#define aos_cli_set_authorized(c,v) aos_cli_ioctl_set_value(c, cli_ioctl_req_set_authorized, v)

/*!
 * \def aos_cli_set_user_arg(c,v)
 *
 * \brief Helper to set the user argument
 *
 * \param c Pointer to the cli_t structure.
 * \param v User argument
 * \return	This function returns 0 on success, non-zero on failure.
 */
#define aos_cli_set_user_arg(c,v) aos_cli_ioctl_set_pointer(c, cli_ioctl_req_set_user_arg, v)

/*!
 * \def aos_cli_set_prompt(c,v)
 *
 * \brief Helper to set the prompt
 *
 * \param c Pointer to the cli_t structure.
 * \param v String containing the prompt
 * \return	This function returns 0 on success, non-zero on failure.
 */
#define aos_cli_set_prompt(c,v) aos_cli_ioctl_set_pointer(c, cli_ioctl_req_set_prompt, v)


/*!
 * \def aos_cli_set_recall_buf(c,v)
 *
 * \brief Helper to set the prompt
 *
 * \param c Pointer to the cli_t structure.
 * \param v Pointer to the recall buffer (same size that the command buffer).
 * \return	This function returns 0 on success, non-zero on failure.
 */
#define aos_cli_set_recall_buf(c,v) aos_cli_ioctl_set_pointer(c, cli_ioctl_req_set_recall_buf, v)
/*! @}*/

#ifdef __cplusplus
}
#endif

