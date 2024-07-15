/*!
 * \file aos_log.h
 *
 * \brief log facility
 *
 * \details The message logger facility manages the AOS SDK tracing as well as the application
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 * <BR>
 *
 * <H3>Facility usage</H3>
 * <DIV>
 * The AOS SDK uses the message logger to provide status or warning about the system.<BR>
 * The logger should be initialized via the function aos_log_init. This function requires a
 * function callback, which is triggered each time a message is logged. <BR>
 * This redirection function allows a dedicated processing other than just sending the message over the console.<BR>
 * If you wish to have the logs over the CLI, just pass the cli_log function (refer to srv_cli module).<BR>
 * <BR>
 * The system logger can manage several internal modules and the application. The list of current supported modules
 * is given by the enumerated aos_log_module_id_t.<BR>
 * <BR>
 * The system logger defines up to 3 different levels:
 * <UL>
 *  <LI>
 *   warning: Warning messages, generally used to indicate a recoverable failure.
 *  </LI>
 *  <LI>
 *   status: status messages, generally used to indicate either a state of the system or a general information.
 *  </LI>
*   <LI>
 *   trace: trace messages, generally used to debug or simply trace not important facts.
 *  </LI>
 * </UL>
 * The system logger allows the selection of the level of each module.
 * </DIV>
 */
#pragma once

#include "stdio.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup aos_log Log facility
 *
 * \addtogroup aos_log
 * @{
 */


/*!
 * \enum aos_log_module_id_t
 *
 * \brief module registered for log
 */
typedef enum {
	aos_log_module_system = 0,		//!< Trace the system
	aos_log_module_config,			//!< Trace the config manager
	aos_log_module_ble,				//!< Trace the module BLE
	aos_log_module_lr1110,			//!< Trace the LR1110 manager
	aos_log_module_lora,			//!< Trace the LoRa connectivity
	aos_log_module_gnss,			//!< Trace the GNSS service
	aos_log_module_geolocation,		//!< Trace the geolocation engines
	aos_log_module_accelero,		//!< Trace the accelerometer
	aos_log_module_gm02s,			//!< Trace the GM02S driver (LTE module)
	aos_log_module_cell,			//!< Trace the cellular network manager
	aos_log_module_app,				//!< Trace the application
	aos_log_module_last,			//!< Last module registered for tracing
	aos_log_module_all = aos_log_module_last //!< Access all module at once
} aos_log_module_id_t;

/*!
 * \enum aos_log_level_t
 *
 * \brief Level of the trace
 *
 * \warning Do not change the order since it is prioritized
 */
typedef enum {
	aos_log_level_disabled = 0,		//!< Traces are disabled
	aos_log_level_warning,			//!< Only warning level traces are displayed
	aos_log_level_status,			//!< Warning and status level traces are displayed
	aos_log_level_debug,			//!< All trace levels are displayed
	aos_log_level_last				//!< End of enum marker. Must be the last item
} aos_log_level_t;


/*!
 * \fn aos_log_fnct_t
 *
 * \brief Function prototype to be provided to the system logger. It will be called to actually process a
 * log message (redirecting the message where you want).
 *
 * \param prefix  Log prefix
 * \param timestamp True if the time stamp should be inserted, false otherwise
 * \param warning True if it is a warning log
 * \param fmt Format string
 * \param ap Variable argument list
 * \return The number of bytes printed. Negative values indicate an error.
 *
 * \note Refer to the aos_log_init function.
 */
typedef int (*aos_log_fnct_t)(const char* prefix, bool timestamp, bool warning, const char *fmt, va_list ap);

/*!
 * \fn void aos_log_generic(aos_log_module_id_t mid, aos_log_level_t level, bool prefix, bool timestamp, const char* format, va_list ap)
 *
 * \brief Generic function for logs
 *
 * \param mid Module identifier
 * \param level Log level
 * \param prefix True if the prefix should be printed, false otherwise
 * \param timestamp True if the timestamp is expected, false otherwise
 * \param format String which accepts format qualifiers
 * \param ap Parameter
 *
 * \note Used internally
 */
void aos_log_generic(aos_log_module_id_t mid, aos_log_level_t level, bool prefix, bool timestamp, const char* format, va_list ap);

/*!
 * \fn void aos_log_msg(aos_log_module_id_t mid, aos_log_level_t level, bool timestamp, const char* format, ...)
 *
 * \brief Log a message
 *
 * \param mid Module identifier
 * \param level Log level of the message to be displayed.
 * \param timestamp True if the timestamp is expected, false otherwise
 * \param format String which accepts format qualifiers
 *
 * \note The function adds the module prefix to the log. If you don't want the prefix, use aos_log_msg_feed instead.
 * The message is displayed only if the trace are enabled and if the module log level is greater than or
 * equals to the level argument.
 */
void aos_log_msg(aos_log_module_id_t mid, aos_log_level_t level, bool timestamp, const char* format, ...);

/*!
 * \fn void aos_log_msg_feed(aos_log_module_id_t mid, aos_log_level_t level, const char* format, ...)
 *
 * \brief Feed a log a message
 *
 * \param mid Module identifier
 * \param level Log level of the message to be displayed.
 * \param format String which accepts format qualifiers
 *
 * \note The function does not add the module prefix to the log. It should be used when the initial log message
 * requires additional data to be displayed on the same line.
 * The message is displayed only if the trace are enabled and if the module log level is greater than or
 * equals to the level argument.
 *
 * Example 1:
 * <pre>
	// Define a string
	 static const char* _nice_str = "nice";

	// Start the log message
 	 aos_log_msg(aos_log_module_sys, aos_log_level_trace,"Hello %s world", _nice_str);

 	// Terminate the message
 	 aos_log_msg_feed(aos_log_module_sys, aos_log_level_trace," . Smart log system.\n"); </pre>

 * Will produce:
 * <pre>
 *  01:18:53.767272 (SYS) Hello nice world. Smart log system. </pre>
 *
 * Example 2:  Same example with aos_log_msg only.
 * <pre>
	// Define a string
	static const char* _nice_str = "nice";
	// Log the message
	aos_log_msg(aos_log_module_sys, aos_log_level_trace,"Hello %s world", _nice_str);

	// Log another message
	aos_log_msg(aos_log_module_sys, aos_log_level_trace," . Smart log system.\n"); </pre>

 * Will produce:
 * <pre>
 *  01:18:53.767272 (SYS) Hello nice world. 01:18:53.767272 (SYS) Smart log system.</pre>
 */
void aos_log_msg_feed(aos_log_module_id_t mid, aos_log_level_t level, const char* format, ...);

/*!
 * \fn void aos_log_status(aos_log_module_id_t mid, bool timestamp, const char* format, ...)
 *
 * \brief Wrapper to log a status message
 *
 * \param mid Module identifier
 * \param timestamp  True if the timestamp is expected, false otherwise
 * \param format: String which accepts format qualifiers
 *
 *  \note Equivalent to aos_log_msg(aos_log_module_id_t mid, aos_log_level_status, const char* format, ...)
 *  but reduce the code size.
 */
void aos_log_status(aos_log_module_id_t mid, bool timestamp, const char* format, ...);

/*!
 * \fn void aos_log_warning(aos_log_module_id_t mid, bool timestamp, const char* format, ...)
 *
 * \brief Wrapper to log a warning message
 *
 * \param mid Module identifier
 * \param timestamp  True if the timestamp is expected, false otherwise
 * \param format  String which accepts format qualifiers
 *
 *  \note Equivalent to aos_log_msg(aos_log_module_id_t mid, aos_log_level_status, const char* format, ...)
 *  but reduce the code size.
 */
void aos_log_warning(aos_log_module_id_t mid, bool timestamp, const char* format, ...);

/*!
 * \fn void aos_log_dump_hex(aos_log_module_id_t mid, aos_log_level_t level, const void *p, uint8_t len, bool lf)
 *
 * \brief Output an hexadecimal string paying attention to the log level of the module.
 *
 * \param mid Module identifier
 * \param level Log level of the message to be displayed.
 * \param p Pointer to the binary array to be displayed
 * \param len Length of the binary array
 * \param lf True if a line feed and carriage return must be inserted at the end of the dump string.
 *
 * \note Does not output the module name. Before calling this function you should use
 * either aos_log_msg or one of its wrapper to have the module name displayed.
 */
void aos_log_dump_hex(aos_log_module_id_t mid, aos_log_level_t level, const void *p, uint8_t len, bool lf);

/*!
 * \fn void aos_log_init(aos_log_fnct_t log_fnct)
 *
 * \brief Initialize the module
 *
 * \param log_fnct Function to process the log message. Passing a null pointer disables the log system.
 *
 * \note To have the log message displayed on the console (CLI), pass the cli_log function.
 */
void aos_log_init(aos_log_fnct_t log_fnct);

/*!
 * \fn void aos_log_set_module_level(aos_log_module_id_t mid, aos_log_level_t level)
 *
 * \brief Set the log level to a module
 *
 * \param mid module identifier
 * \param level Log level of the message to be set.
 */
void aos_log_set_module_level(aos_log_module_id_t mid, aos_log_level_t level);


/*!
 * \fn aos_log_level_t aos_log_get_module_level(aos_log_module_id_t mid)
 *
 * \brief Return the log level of the given module.
 *
 * \param mid module identifier
 *
 * \return The level Log level of the module
 */
aos_log_level_t aos_log_get_module_level(aos_log_module_id_t mid);

/*!
 * \fn void aos_log_enable_disable(bool enable)
 *
 * \brief Enable/disable globally the logs
 *
 * \param enable True to enable the logs, false to disabled them
 *
 *  \note Logs are globally enabled/disabled. However this function does not affect the
 *  per module log level behavior.
 */
void aos_log_enable_disable(bool enable);

/*!
 * \fn bool aos_log_is_enabled(void)
 *
 * \brief Return the global state of the system logger
 *
 * \return true if the system logger is globally enabled, false otherwise
 */
bool aos_log_is_enabled(void);

/*!
 * \fn const char* aos_log_get_module_str(aos_log_module_id_t mid);
 *
 * \brief Return the module prefix string
 *
 * \param mid Module identifier
 *
 * \return The prefix in string format of the given module identifier
 */
const char* aos_log_get_module_str(aos_log_module_id_t mid);

/*!
 * \fn const char* aos_log_get_level_str(aos_log_level_t lid)
 *
 * \brief Return the level in string format
 *
 * \param lid Level identifier
 *
 * \return The string matching the level identifier
 */
const char* aos_log_get_level_str(aos_log_level_t lid);

/*!
 * \fn aos_log_module_id_t aos_log_str_to_module_id(const char* name)
 *
 *
 * \brief Return the module identifier of the provided module prefix
 *
 * \param name Module prefix name
 *
 * \return The identifier matching the string if any. Otherwise return aos_log_module_last
 */
aos_log_module_id_t aos_log_str_to_module_id(const char* name);

/*!
 * \fn aos_log_level_t aos_log_str_to_level_id(const char* name)
 *
 * \brief Return the level identifier matching the provided string
 *
 * \param name Module prefix name
 *
 * \return The identifier matching the string if any. Otherwise return aos_log_level_last
 */
aos_log_level_t aos_log_str_to_level_id(const char* name);


/*! @}*/
#ifdef __cplusplus
}
#endif


