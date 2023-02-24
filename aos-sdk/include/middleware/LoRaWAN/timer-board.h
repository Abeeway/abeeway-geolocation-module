/*
 *
 * timer-board.h - MAC timer implementation. This replaces the RTC-specific
 * ties from the original Semtech code.
 *
 */

#pragma once

#include "timer.h"	// TimerTime_t

/*
 * This abstraction supports a number of different implementations, all
 * providing the same API but based on different timer peripherals. You
 * need to select one (and only one) that is not used elsewhere, ie not
 * used for FreeRTOS.
 *
 */
#define MAC_TIMER_USING_RTC_ALM 1	// Implementation based on RTC alarm A
#define MAC_TIMER_USING_LPTIM1	2	// Implementation based on LPTIM1

#if !defined(MAC_TIMER_IMPLEMENTATION)
#define MAC_TIMER_IMPLEMENTATION MAC_TIMER_USING_RTC_ALM
#endif

typedef TimerTime_t timer_ticks_t;	// Mirror the TimerTime_t definition from timer.h

typedef struct {
	/*
	 * init(handler) - initialize timer implementation, store timer handler. This
	 *                 function is to be called only once from the timer code.
	 *
	 * The handler function will be called when the timer expires, from user level.
	 */
	void        (*init)(void (*timer_event_handler)(void));

	/*
	 * stop() - stop the timer.
	 */
	void        (*stop)(void);

	/*
	 * start(ticks) - arm the timer to fire in 'timeout_ticks' ticks.
	 */
	void        (*start)(timer_ticks_t timeout_ticks);

	/*
	 * ticks() - get the current tick count.
	 */
	timer_ticks_t (*ticks)(void);

	/*
	 * minimum_timeout() - get the minimum number of ticks to start a timer.
	 */
	timer_ticks_t (*minimum_timeout)(void);

	/*
	 * ms_to_ticks() - convert milliseconds to ticks.
	 */
	timer_ticks_t (*ms_to_ticks)(uint32_t ms);

	/*
	 * ticks_to_ms() convert ticks to milliseconds.
	 */
	uint32_t    (*ticks_to_ms)(timer_ticks_t timer_ticks);

	/*
	 * Timer "context" - operations needed to calculate a delta time, mostly
	 * when using the RTC Alarm on an ST processor as besides the current ticks
	 * it also need to store the date and time. Without this, the context could
	 * just as well be handled inside the timer code.
	 */
	timer_ticks_t	(*set_context)(void);		// Save the laps start time
	timer_ticks_t	(*get_context)(void);		// Get the laps start time
	timer_ticks_t	(*get_elapsed_time)(void);	// Get the elapsed time, in ticks

	/*
	 * Temperature compensation. Used in class B calculations. Part of the timer
	 * abstraction as the compensation may differ depending on the peripheral.
	 */
	timer_ticks_t	(*temp_compensation)(timer_ticks_t period, float temperature);

} timer_implementation_t;

extern const timer_implementation_t aos_mac_timer;
