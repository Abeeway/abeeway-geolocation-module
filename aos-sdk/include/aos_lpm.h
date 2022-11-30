/*!
 * \file aos_lpm.h
 *
 * \brief Low power management facility
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup aos_lpm Low power manager
 *
 * \addtogroup aos_lpm
 * @{
 */

/*!
 * \enum aos_lpm_mode_t
 *
 * \brief Low power modes, from highest power mode to lowest power mode.
 */
typedef enum {
	aos_lpm_mode_no_sleep = 0,	//!< (10.2mA) No low power mode. The chip is in high speed mode.
	aos_lpm_mode_sleep,		//!< ( 5.1mA) Sleep mode
	aos_lpm_mode_stop1,		//!< ( 9.2uA) Medium low power. The chip can move in STOP1.
	aos_lpm_mode_stop2,		//!< ( 5.0uA) Lowest power mode. The chip can move in STOP2
	aos_lpm_mode_count		//!< Number of low power modes.
} aos_lpm_mode_t;

/*!
 * \enum aos_lpm_requester_t
 *
 * \brief Entities requesting the low power change.
 */
typedef enum {
	aos_lpm_requester_usb = 0,			//!< The requester is the USB driver.
	aos_lpm_requester_i2c_internal,		//!< The requester is the internal I2C driver (I2C1)
	aos_lpm_requester_i2c_external,		//!< The requester is the external I2C driver (I2C3)
	aos_lpm_requester_usart1,			//!< The requester is the USART driver
	aos_lpm_requester_lpuart,			//!< The requester is the low power UART driver
	aos_lpm_requester_spi_internal,		//!< The requester is the internal SPI driver
	aos_lpm_requester_spi_external,		//!< The requester is the external SPI driver
	aos_lpm_requester_rtc,				//!< The requester is the RTC driver
	aos_lpm_requester_pwm,				//!< The requester is the PWM driver
	aos_lpm_requester_application,		//!< The requester is the user application
	aos_lpm_requester_cli,				//!< The requester is the CLI
	aos_lpm_requester_flash,			//!< The requester is the Flash (EEPROM Emulation)
	aos_lpm_requester_ble,				//!< The requester is the BLE application
	aos_lpm_requester_last				//!< Last entry in the enumerated
} aos_lpm_requester_t;

/*!
 * \typedef aos_lpm_event_t
 *
 * \brief Low power manager event notification type
 *
 * \details These events are meant to allow various drivers to turn off or
 * on whatever can be to reduce power consumption. They are sent
 * immediately before and after the WFI instruction, so no system
 * services are available - only register accesses.
 */
typedef enum {
	aos_lpm_event_test_sleep,	//!< Ask a client if we may go to sleep
	aos_lpm_event_pre_sleep,	//!< The system is about to go to sleep
	aos_lpm_event_post_sleep,	//!< The system has just woken up
} aos_lpm_event_t;

/*!
 *  \typedef aos_lpm_requester_callback_t
 *
 * \brief Callback called before entering the low power mode.
 *
 * \param arg User argument
 *
 * \return true if the requestor can sleep (move to STOP1/2) mode, false otherwise
 *
 * \note Some action may be required by the requester before entering a low power mode.
 * The requester has to do them under this callback.
 */
typedef bool (aos_lpm_requester_callback_t)(void*arg, aos_lpm_event_t event);

/*!
 * \fn void aos_lpm_set_mode(aos_lpm_requester_t requester, aos_lpm_mode_t mode, aos_lpm_requester_callback_t cb, void* arg);
 *
 * \brief Disable/enable the low power mode of a given requester
 *
 * \param requester The driver requesting the low power setting
 * \param mode The lowest power mode that the requester accepts
 * \param cb Callback, which is called before entering the low power mode (can be NULL)
 * \param arg Argument passed along the callback (opaque)
 */
void aos_lpm_set_mode(aos_lpm_requester_t requester, aos_lpm_mode_t mode, aos_lpm_requester_callback_t cb, void* arg);

/*!
 * \fn void aos_lpm_set_hsi_mode(aos_lpm_requester_t requester, bool hsi_alway_on)
 *
 * \brief Disable/enable the HSI clock during stop mode
 *
 * \param requester The module requesting the low power setting
 * \param hsi_alway_on True to have the HSI clock ON during STOP mode, false otherwise.
 */
void aos_lpm_set_hsi_mode(aos_lpm_requester_t requester, bool hsi_alway_on);


/*!
 * \fn aos_lpm_mode_t aos_lpm_get_info(aos_lpm_requester_t requester, bool* hsi_always_on)
 *
 * \brief Retrieve the mode and the HSI state of the provided requester
 *
 * \param requester The module for which the information is expected
 * \param hsi_always_on Output value providing the state of the HSI for this module
 *
 * \return aos_lpm_mode_count if requester not found or not yet registered, otherwise the LPM mode.
 *
 */
aos_lpm_mode_t aos_lpm_get_info(aos_lpm_requester_t requester, bool* hsi_always_on);

/*!
 * \fn bool aos_lpm_can_sleep(void)
 *
 * \brief Check whether the system can sleep
 *
 * \return True if the system can sleep.
 *
 * \note Usually called by the system. The application may call this function to know if the system can move
 * to a low power mode
 */
bool aos_lpm_can_sleep(void);

/*!
 * \fn void aos_lpm_suspend_gpio_on_stop(unsigned gpio, bool allow)
 *
 * \brief Instruct the low power manager to suspend a GPIO when entering
 * low power mode. This may be required to reduce power consumption.
 *
 * \param gpio The GPIO number (such as PIN_EXT_USR_ADC)
 *
 * \param allow Boolean indicating whether to suspend the GPIO or not.
 *
 */
void aos_lpm_suspend_gpio_on_stop(unsigned gpio, bool allow);

/*!
 * \fn aos_lpm_mode_t aos_lpm_sleep_mode(void)
 *
 * \brief Test which sleep mode may be entered if the lpm can sleep.
 *
 * \return Returns the lowest sleep mode that may be entered.
 *
 */
aos_lpm_mode_t aos_lpm_sleep_mode(void);

/*!
 * \fn void aos_lpm_enter(void)
 *
 * \brief Enter low power mode. This function enters the lowest possible
 * low power mode. It is intended to be used internally by AOS.
 *
 */
void aos_lpm_enter(void);

/*! @}*/
#ifdef __cplusplus
}
#endif
