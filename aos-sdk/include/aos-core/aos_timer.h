/*!
 * \file  aos_timer.h
 *
 * \brief Timers based on the RTC alarm.
 *
 * \details These timers are usually restricted to the system. The application should
 * 			use the FreeRTOS's ones.
 * 			They run in the system timer thread.
 *
 *\copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \defgroup System timers
 *
 * \addtogroup sys_timer
 * @{
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief Timer object description
 */
typedef struct aos_timer_s {
    uint32_t timestamp;                  //!< Current timer value
    uint32_t reloadValue;                //!< Timer delay value
    bool isStarted;                      //!< Is the timer currently running
    bool isNext2Expire;                  //!< Is the next timer to expire
    void ( *callback )( void* context ); //!< Timer IRQ callback function
    void *context;                       //!< User defined data object pointer to pass back
    struct aos_timer_s *next;           //!< Pointer to the next Timer object.
} aos_timer_t;

/*!
 * \brief Timer time variable definition
 */
typedef uint32_t aos_timer_time_t;

/*!
 * \brief Initializes the timer module
 */

void aos_timer_init_module(void);

/*!
 * \brief Initializes the timer object
 *
 * \remark TimerSetValue function must be called before starting the timer.
 *         this function initializes timestamp and reload value at 0.
 *
 * \param obj          Structure containing the timer object parameters
 * \param callback     Function callback called at the end of the timeout
 */
void aos_timer_init( aos_timer_t *obj, void ( *callback )( void *context ) );

/*!
 * \brief Sets a user defined object pointer
 *
 * \param obj Structure containing the timer object parameters
 * \param context User defined data object pointer to pass back  on IRQ handler callback
 */
void aos_timer_set_context( aos_timer_t *obj, void* context );

/*!
 * \brief Timer IRQ event handler
 */
void aos_timer_irq_handler( void );

/*!
 * \brief Starts and adds the timer object to the list of timer events
 *
 * \param obj Structure containing the timer object parameters
 */
void aos_timer_start( aos_timer_t *obj );

/*!
 * \brief Checks if the provided timer is running
 *
 * \param obj Structure containing the timer object parameters
 *
 * \retval status  returns the timer activity status [true: Started, false: Stopped]
 */
bool aos_timer_is_started( aos_timer_t *obj );

/*!
 * \brief Stops and removes the timer object from the list of timer events
 *
 * \param obj Structure containing the timer object parameters
 */
void aos_timer_stop( aos_timer_t *obj );

/*!
 * \brief Resets the timer object
 *
 * \param obj Structure containing the timer object parameters
 */
void aos_timer_reset( aos_timer_t *obj );

/*!
 * \brief Set timer new timeout value in milliseconds
 *
 * \param obj Structure containing the timer object parameters
 * \param value New timer timeout value (milliseconds)
 */
void aos_timer_set_timeout( aos_timer_t *obj, uint32_t value );

/*!
 * \brief Set timer new timeout value in ticks
 *
 * \param obj Structure containing the timer object parameters
 * \param ticks New timer timeout value (ticks)
 */
void aos_timer_set_ticks( aos_timer_t *obj, uint32_t ticks );

/*!
 * \brief Read the current ticks
 *
 * \retval returns current timer ticks
 */
aos_timer_time_t aos_timer_get_current_ticks( void );

/*!
 * \brief Return the Time elapsed since a fix moment in Time
 *
 * \remark TimerGetElapsedTime will return 0 for argument 0.
 *
 * \param past fix moment in Time
 * \retval returns elapsed time
 */
aos_timer_time_t aos_timer_get_elapsed_time( aos_timer_time_t past );

/*!
 * \brief Computes the temperature compensation for a period of time on a
 *        specific temperature.
 *
 * \param period Time period to compensate
 * \param temperature Current temperature
 *
 * \retval Compensated time period
 */
aos_timer_time_t aos_timer_compensation( aos_timer_time_t period, float temperature );

/*!
 * \brief Processes pending timer events
 */
void aos_timer_process( void );

/*! @}*/
#ifdef __cplusplus
}
#endif
