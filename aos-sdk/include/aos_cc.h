/*!
 * \file aos_cc.h
 *
 * \brief Cycle Counter API, for precise execution time measurements.
 *
 * The cycle counter API is based on the Cortex-M DWT (Data Watchpoint and Trace),
 * and can be used for precise timing measurements. Depending on the system clock
 * speed, the cycle counter will wrap more or less quickly, so it should be used
 * essentially for measuring reasonably short durations.  Practical use is code
 * optimization during development.
 *
 * All functions are declared static inline to avoid the overhead of a function call.
 *
 * \note There is only one cycle counter in the system.
 *
 * \copyright (c)2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once

#include "stm32wb55xx.h"

/*
 * \fn	void aos_cc_disable(void)
 *
 * \brief	Disable the Cycle Counter
 *
 * This function disables the cycle counter.
 *
 * \param	none
 *
 * \return	none
 *
 */
static inline void aos_cc_disable(void)
{
	DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
	CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
}

/*
 * \fn	void aos_cc_enable(void)
 *
 * \brief	Clear and enable the Cycle Counter
 *
 * This function clears and enables the cycle counter.
 *
 * \param	none
 *
 * \return	none
 *
 */
static inline void aos_cc_enable(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/*
 * \fn	void aos_cc_reset(void)
 *
 * \brief	Reset (clear) the Cycle Counter
 *
 * This function clears the cycle counter value.
 *
 * \param	none
 *
 * \return	none
 *
 */
static inline void aos_cc_reset(void)
{
	DWT->CYCCNT = 0;
}

/*
 * \fn	uint32_t aos_cc_value(void)
 *
 * \brief	Return the Cycle Counter value
 *
 * \param	none
 *
 * \return	An uint32_t representing the Cycle Counter value.
 *
 * \note	The cycle counter wraps after counting UINT32_MAX cycles.
 *
 */
static inline uint32_t aos_cc_value(void)
{
	return DWT->CYCCNT;
}

/*
 * \fn	uint32_t aos_cc_cycles2ns(uint32_t cycles, uint32_t clock_speed)
 *
 * \brief	Convert a number of cycles to nanoseconds.
 *
 * \param[IN] cycles	Number of cycles as returned by aos_cc_value()
 *
 * \param[IN] clock_speed	System clock speed in Hz
 *
 * \return	An uint32_t representing the number of nanoseconds.
 *
 */
static inline uint32_t aos_cc_cycles2ns(uint32_t cycles, uint32_t clock_speed)
{
	return ((double)cycles * 1000000000. / (double)clock_speed );
}

/*
 * \fn	uint32_t aos_cc_cycles2us(uint32_t cycles, uint32_t clock_speed)
 *
 * \brief	Convert a number of cycles to microseconds
 *
 * \param[IN] cycles	Number of cycles as returned by aos_cc_value()
 *
 * \param[IN] clock_speed	System clock speed in Hz
 *
 * \return	An uint32_t representing the number of microseconds.
 *
 */
static inline uint32_t aos_cc_cycles2us(uint32_t cycles, uint32_t clock_speed)
{
	return aos_cc_cycles2ns(cycles, clock_speed) / 1000;
}

/*
 * \fn	uint32_t aos_cc_cycles2ms(uint32_t cycles, uint32_t clock_speed)
 *
 * \brief	Convert a number of cycles to milliseconds
 *
 * \param[IN] cycles	Number of cycles as returned by aos_cc_value()
 *
 * \param[IN] clock_speed	System clock speed in Hz
 *
 * \return	An uint32_t representing the number of milliseconds.
 *
 */
static inline uint32_t aos_cc_cycles2ms(uint32_t cycles, uint32_t clock_speed)
{
	return aos_cc_cycles2us(cycles, clock_speed) / 1000;
}
