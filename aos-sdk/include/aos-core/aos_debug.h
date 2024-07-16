/*!
 * \file aos_debug.h
 *
 * \brief Low level debug output, only functional when compiled with AOS_DEBUG_ENABLED
 *
 * \copyright (c)2022, Abeeway (www.abeeway.com)
 *
 */

#pragma once


#if defined(AOS_DEBUG_ENABLED)

#include <stdio.h>
#include "cmsis_gcc.h"	// __disable_irq()
/*
 * \fn aos_debug_printf(...)
 *
 * \brief synchronously displays a message on the system console.
 *
 * The purpose is to display low level errors (assertions, hard faults, etc).
 *
 * \note The user must define the __io_putchar() function to output data as appropriate
 *       for the call to printf() to produce some output.
 *
 * \note THIS IS ONLY INTENDED FOR LOW LEVEL DEBUGGING - DO NOT USE FOR LOGGING !
 *
 */
#define aos_debug_printf(s...) do { \
	uint32_t m = __get_PRIMASK(); \
	__disable_irq(); \
	printf(s); \
	__set_PRIMASK(m); \
	} while (0)

/*
 * \fn aos_debug_halt(...)
 *
 * \brief Halt the system for debugging
 *
 * When compiled with -DDEBUG=1, this function disables interrupts,
 * displays a message on the console, and then spins the processor
 * to allow intervention with a debugger.
 *
 * Otherwise, the message is displayed and the system is reset.
 *
 */
#define aos_debug_halt(s...) do { \
	__disable_irq(); \
	aos_debug_printf(s); \
	aos_debug_printf("\r\n"); \
	while (1); \
	} while (0)

#else
// Release mode.
#define aos_debug_printf(s...) //!< No operation

//
// In release mode, it is preferable to reset rather than tank
// the system, until we have a watchdog that resets it.
//
#include "stm32wb55xx.h"	// NVIC_SystemReset()

/*!
 * \def aos_debug_halt
 *
 * \brief Halt the system for debugging
 *
 * When compiled with -DDEBUG=1, this function disables interrupts,
 * displays a message on the console, and then spins the processor
 * to allow intervention with a debugger.
 *
 * Otherwise, the message is displayed and the system is reset.
 *
 */
#define aos_debug_halt(s...) do { /* System reset */ \
	NVIC_SystemReset(); \
	} while (0)

#endif

/*!
 * \def aos_assert_failed
 *
 * \brief Internal use function to log an assertion message,
 *        for use by aos_assert_warn() and aos_assert_halt().
 *
 * \param condstr	char *, stringified assertion condition
 * \param file		char *, file name
 * \param line		unsigned, line number
 *
 */

#define aos_assert_failed(condstr, file, line) do { \
	aos_debug_printf("\r\nAOS assertion '%s' failed, %s:%u\r\n", condstr, file, line); \
	} while (0)

/*!
 * \def aos_assert_warn
 *
 * \brief log a warning message if an assertion fails
 *
 * \param cond	condition that must be false for the assertion to fail.
 *
 */
#define aos_assert_warn(cond) if ((cond) == 0) { \
	aos_assert_failed( #cond, __FILE__, __LINE__); \
	}

/*!
 * \def aos_assert_halt
 * \brief log a warning message and halt (or reset) if an assertion fails
 *
 * \note see also aos_debug_halt()
 */
#define aos_assert_halt(cond) if ((cond) == 0) { \
	aos_assert_failed( #cond, __FILE__, __LINE__); \
	aos_debug_halt("Program halted."); \
	}
