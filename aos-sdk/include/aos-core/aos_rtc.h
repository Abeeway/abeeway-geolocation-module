/*!
 * \file aos_rtc.h
 *
 * \brief RTC (Real Time Clock).
 *
 * \details This module drives the RTC. RTC Alarm A reserved for aos_timer, Alarm B reserved for LoRa (LBM).
 *
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif



/*!
 * \defgroup RTC facility
 *
 * \addtogroup rtc_facility
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "aos_timer.h"


/*!
 * \brief Days, Hours, Minutes and seconds of systime.h
 */
#define TM_DAYS_IN_LEAP_YEAR                        ( ( uint32_t )  366U )	//!< Number of day in a leap year
#define TM_DAYS_IN_YEAR                             ( ( uint32_t )  365U )	//!< Number of day in a year
#define TM_SECONDS_IN_1DAY                          ( ( uint32_t )86400U )	//!< Number of seconds in a day
#define TM_SECONDS_IN_1HOUR                         ( ( uint32_t ) 3600U )	//!< Number of seconds in an hour
#define TM_SECONDS_IN_1MINUTE                       ( ( uint32_t )   60U )	//!< Number of seconds in a minute
#define TM_MINUTES_IN_1HOUR                         ( ( uint32_t )   60U )	//!< Number of minutes in an hour
#define TM_HOURS_IN_1DAY                            ( ( uint32_t )   24U )	//!< Number of minutes in a day

/*!
 * \brief Months of systime.h
 */
#define TM_MONTH_JANUARY                            ( ( uint8_t ) 0U )		//!< January identifier
#define TM_MONTH_FEBRUARY                           ( ( uint8_t ) 1U )		//!< February identifier
#define TM_MONTH_MARCH                              ( ( uint8_t ) 2U )		//!< March identifier
#define TM_MONTH_APRIL                              ( ( uint8_t ) 3U )		//!< April identifier
#define TM_MONTH_MAY                                ( ( uint8_t ) 4U )		//!< May identifier
#define TM_MONTH_JUNE                               ( ( uint8_t ) 5U )		//!< June identifier
#define TM_MONTH_JULY                               ( ( uint8_t ) 6U )		//!< July identifier
#define TM_MONTH_AUGUST                             ( ( uint8_t ) 7U )		//!< August identifier
#define TM_MONTH_SEPTEMBER                          ( ( uint8_t ) 8U )		//!< September identifier
#define TM_MONTH_OCTOBER                            ( ( uint8_t ) 9U )		//!< October identifier
#define TM_MONTH_NOVEMBER                           ( ( uint8_t )10U )		//!< November identifier
#define TM_MONTH_DECEMBER                           ( ( uint8_t )11U )		//!< December identifier

/*!
 * \brief Week days of systime
 */
#define TM_WEEKDAY_SUNDAY                           ( ( uint8_t )0U )		//!< Sunday weekday identifier
#define TM_WEEKDAY_MONDAY                           ( ( uint8_t )1U )		//!< Monday weekday identifier
#define TM_WEEKDAY_TUESDAY                          ( ( uint8_t )2U )		//!< Tuesday weekday identifier
#define TM_WEEKDAY_WEDNESDAY                        ( ( uint8_t )3U )		//!< Wednesday weekday identifier
#define TM_WEEKDAY_THURSDAY                         ( ( uint8_t )4U )		//!< Thursday weekday identifier
#define TM_WEEKDAY_FRIDAY                           ( ( uint8_t )5U )		//!< Friday weekday identifier
#define TM_WEEKDAY_SATURDAY                         ( ( uint8_t )6U )		//!< Saturday weekday identifier

#define TM_SECOND_TO_MILLISECONDS					1000					//!< One second in milliseconds


/*!
 * \brief Number of seconds elapsed between Unix and GPS epoch
 */
#define TM_UNIX_GPS_EPOCH_OFFSET                      315964800

/*!
 * \brief Structure holding the system time in seconds and milliseconds.
 */
typedef struct {
    uint32_t seconds;					//!< Seconds
    int16_t  subseconds;				//!< Milliseconds
} aos_rtc_systime_t;

/*!
 * \brief RTC Backup register. (up to 20 registers).
 */
typedef enum {
	aos_rtc_backup_register_res0 = 0,		//!< Reserved for the RTC. Do not use
	aos_rtc_backup_register_res1,			//!< Reserved for the RTC. Do not use
	aos_rtc_backup_register_bootloader,		//!< Reserved for the Abeeway Bootloader commands
} aos_rtc_backup_register_t;

/*!
 * \brief Bootloader commands of the Abeeway bootloader
 */
typedef enum {
	aos_rtc_bootloader_rtc_cmd_none = 0,	//!< No bootloader request
	aos_rtc_bootloader_rtc_cmd_enter,		//!< Enter Bootloader after reset
}aos_rtc_bootloader_rtc_cmd_t;

/*!
 * \brief Alarm type
 */
typedef enum {
	aos_rtc_alarm_a = 0,	//!< RTC Alarm A
	aos_rtc_alarm_b,		//!< RTC Alarm B
}aos_rtc_alarm_type_t;

/*!
 * \brief time variable definition
 */
typedef uint64_t aos_rtc_time_t;


/*!
 * \brief Initializes the RTC timer
 *
 * \remark The timer is based on the RTC
 */
void aos_rtc_init(void);

/*!
 * \brief Initializes the callback for alarm
 *
 * \param alarm_type Alarm type
 * \param alarm_cbfn - RTC Alarm callback function. Invoked when an Alarm A interrupt is fired, at interrupt level.
 *
 * \remark The timer is based on the RTC
 */
void aos_rtc_set_alarm_callback(aos_rtc_alarm_type_t alarm_type, void (*alarm_cbfn)(void) );

/*!
 * \brief Returns the minimum timeout value
 *
 * \retval minTimeout Minimum timeout value in in ticks
 */
uint32_t aos_rtc_get_min_timeout( void );

/*!
 * \brief converts time in ms to time in ticks
 *
 * \param milliseconds Time in milliseconds
 * \retval returns time in timer ticks
 */
aos_rtc_time_t aos_rtc_ms_to_tick( aos_rtc_time_t milliseconds );

/*!
 * \brief converts time in ticks to time in ms
 *
 * \param tick time in timer ticks
 * \retval returns time in milliseconds
 */
aos_rtc_time_t aos_rtc_tick_to_ms( aos_rtc_time_t tick );

/*!
 * \brief Performs a delay of milliseconds by polling RTC
 *
 * \param milliseconds Delay in ms
 */
void aos_rtc_delay_ms( uint32_t milliseconds );

/*!
 * \brief Sets the alarm
 *
 * \note The alarm is set at now (read in this funtion) + timeout
 *
 * \param alarm_type Alarm type
 * \param timeout Duration of the Timer ticks
 */
void aos_rtc_set_alarm(aos_rtc_alarm_type_t alarm_type,  uint32_t timeout );

/*!
 * \brief Stops the Alarm
 * \param alarm_type Alarm type
 */
void aos_rtc_stop_alarm(aos_rtc_alarm_type_t alarm_type);

/*!
 * \brief Starts wake up alarm
 *
 * \param alarm_type Alarm type
 * \param timeout Timeout value in ticks
 *
 * \note  Alarm in RtcTimerContext.Time + timeout
 */
void aos_rtc_start_alarm(aos_rtc_alarm_type_t alarm_type, uint32_t timeout );


/*!
 * \brief Check if an alarm is started
 *
 * \param alarm_type Alarm type
 *
 * \retval True if the alarm is started, false otherwise
 *
 */
bool aos_rtc_alarm_is_started(aos_rtc_alarm_type_t alarm_type);

/*!
 * \brief Sets the alarm A time reference
 *
 * \retval value Timer reference value in ticks
 */
aos_rtc_time_t aos_rtc_set_alarm_a_context();

/*!
 * \brief Gets the alarm A time reference
 *
 * \retval value Alarm value in ticks
 */
aos_rtc_time_t aos_rtc_get_alarm_a_context();

/*!
 * \brief Gets the system time with the number of seconds elapsed since epoch
 *
 * \param milliseconds Number of milliseconds elapsed since epoch
 * \retval seconds Number of seconds elapsed since epoch
 */
uint32_t aos_rtc_get_calendar_time( uint16_t *milliseconds );

/*!
 * \brief Read the current time in ms
 *
 * \retval time returns current time (milliseconds)
 */
aos_rtc_time_t aos_rtc_get_current_time_ms( void );

/*!
 * \brief Read the current time in seconds
 *
 * \retval time returns current time in seconds
 */
aos_rtc_time_t aos_rtc_get_current_time_sec( void );

/*!
 * \brief Get the RTC time value in ticks
 *
 * \retval RTC Timer value in ticks
 */
aos_rtc_time_t aos_rtc_get_current_time_ticks( void );

/*!
 * \brief Get the RTC value in step of 100us
 *
 * \retval RTC Timer value
 */
aos_rtc_time_t aos_rtc_get_current_time_100us( void );

/*!
 * \brief Get the RTC alarm A elapsed time since the last Alarm was set
 *
 * \retval RTC Elapsed time since the last alarm in ticks.
 */
aos_rtc_time_t aos_rtc_get_alarm_a_elapsed_time( void );

/*!
 * \brief Write a data to a the RTC backup register
 *
 * \param reg Register to be written
 * \param data Data to be written
 */
void aos_rtc_backup_write(aos_rtc_backup_register_t reg, uint32_t data );

/*!
 * \brief Read a data from the RTC backup register
 *
 * \param reg Register to be written
 *
 * \retval Value read from the RTC backup register
 */
uint32_t aos_rtc_backup_read(aos_rtc_backup_register_t reg);

/*!
 * \brief Computes the temperature compensation for a period of time on a
 *        specific temperature.
 *
 * \param period Time period to compensate in milliseconds
 * \param temperature Current temperature
 *
 * \retval Compensated time period
 */
aos_rtc_time_t aos_rtc_temp_compensation( aos_rtc_time_t period, float temperature );

/*!
 * \brief Add two system time
 *
 * \param a Time system A
 * \param b Time system b
 *
 * \retval The sum a + b
 */
aos_rtc_systime_t aos_rtc_systime_add( aos_rtc_systime_t a, aos_rtc_systime_t b );

/*!
 * \brief Substract two system time
 *
 * \param a Time system A
 * \param b Time system b
 *
 * \retval The substraction a - b
 */
aos_rtc_systime_t aos_rtc_systime_sub( aos_rtc_systime_t a, aos_rtc_systime_t b );

/*!
 * \brief Set the system time
 *
 * \param systime Unix system time start Jan 1, 1970 at 0h00
 *
 * \note The new system time is not actually set in the RTC calendar time. Instead the time difference
 * between the calendar time and the new system time is kept in the RTC backup register
 */
void aos_rtc_systime_set( aos_rtc_systime_t systime );

/*!
 * \brief Get the system time
 *
 * \retval The actual time
 *
 * \note The  actual time is computed from the actual calendar value and the offset stored
 * in the RTC backup register
 */
aos_rtc_systime_t aos_rtc_systime_get( void );

/*!
 * \brief Get the MCU time (the uptime)
 *
 * \retval The RTC calendar time.
 *
 * \note The  actual time is computed from the actual calendar value and the offset stored
 * in the RTC backup register
 */
aos_rtc_systime_t aos_rtc_systime_get_mcu_time( void );

/*!
 * \brief Convert the input time in systime format to milliseconds
 *
 * \param systime System time to convert
 *
 * \retval The corresponding number of milliseconds
 */
aos_rtc_time_t aos_rtc_systime_to_ms( aos_rtc_systime_t systime );

/*!
 * \brief Convert the input time in milliseconds to the system time format
 *
 * \param time_ms Time in milliseconds to convert
 *
 * \retval The equivalent in system time format
 */
aos_rtc_systime_t aos_rtc_systime_from_ms( aos_rtc_time_t time_ms );

/*!
 * \brief Convert the usual UNIX time structure to the number of seconds
 *
 * \param localtime: Local system time
 *
 * \retval The number of seconds since the UNIX epoch.
 */
uint32_t aos_rtc_systime_mk_time( const struct tm* localtime );

/*!
 * \brief Convert a number of seconds to the usual UNIX time structure
 *
 * \param timestamp Number of seconds since the UNIX epoch
 * \param localtime Usual UNIX time structure
 *
 * \retval The number of seconds since the UNIX epoch.
 */
void aos_rtc_systime_local_time( const uint32_t timestamp, struct tm *localtime );

/*! @}*/
#ifdef __cplusplus
}
#endif

