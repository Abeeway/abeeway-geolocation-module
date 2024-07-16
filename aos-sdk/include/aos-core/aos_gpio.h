/*!
 * \file aos_gpio.h
 *
 * \brief GPIO driver
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "aos_common.h"
#include "aos_system.h"

/*!
 * \defgroup aos_gpio GPIO driver
 *
 * \addtogroup aos_system
 * @{
 */

/*!
 * \enum aos_gpio_id_t
 *
 * \brief GPIO identifiers (External module PINs)
 */
typedef enum {
	aos_gpio_id_intern_lr_dio,		//<! Internal pin. DIOx used by the LR1110. Do not use.
	aos_gpio_id_intern_lr_nss,		//<! Internal pin. SPI NSS used by the LR1110. Do not use.
	aos_gpio_id_intern_lr_busy,		//<! Internal pin. LR1110 busy. Do not use.
	aos_gpio_id_intern_lr_nreset,	//<! Internal pin. LR1110 reset. Do not use.
	aos_gpio_id_vbat_sense,			//!< VBAT_SENSE module PIN
	aos_gpio_id_1,					//!< GPIO 1
	aos_gpio_id_2,					//!< GPIO 2
	aos_gpio_id_3,					//!< GPIO 3
	aos_gpio_id_4,					//!< GPIO 4
	aos_gpio_id_5,					//!< GPIO 5
	aos_gpio_id_6,					//!< GPIO 6
	aos_gpio_id_7,					//!< GPIO 7
	aos_gpio_id_8,					//!< GPIO 8
	aos_gpio_id_9,					//!< GPIO 9
	aos_gpio_id_10,					//!< GPIO 10
	aos_gpio_id_lpuart_rx,			//!< External LPUART RX
	aos_gpio_id_lpuart_tx,			//!< External LPUART RX
	aos_gpio_id_lpuart_rts,			//!< External LPUART RTS
	aos_gpio_id_lpuart_cts,			//!< External LPUART CTS
	aos_gpio_id_usart_rx,			//!< External USART RX
	aos_gpio_id_usart_tx,			//!< External USART RX
	aos_gpio_id_i2c_power,			//!< External I2C power
	aos_gpio_id_spi_cs,				//!< External SPI chip select
	aos_gpio_id_mt3333_main_power,		//!< GNSS MT3333 main power
	aos_gpio_id_mt3333_backup_power,	//!< GNSS MT3333 backup power
	aos_gpio_id_user_adc1,				//!< USER_ADC1
	aos_gpio_id_i2c_int1,				//!< External I2C interrupt 1
	aos_gpio_id_i2c_int2,				//!< External I2C interrupt 2
	aos_gpio_id_pwm_ctrl,			//!< PWM control
	aos_gpio_id_last				//!< Last entry in the enumerated
} aos_gpio_id_t;

/*!
 * \enum aos_gpio_mode_t
 *
 * \brief GPIO mode
 */
typedef enum {
	aos_gpio_mode_input,				//!< GPIO configured as a digital input
	aos_gpio_mode_output,				//!< GPIO configured as a digital output
	aos_gpio_mode_analog,				//!< GPIO configured as an analog input
	aos_gpio_mode_alternate,			//!< GPIO configured as an analog input. Can be use only with reserve/unreserve
	aos_gpio_mode_last					//!< GPIO Last mode. Also mean not configured.
} aos_gpio_mode_t;

/*!
 * \enum aos_gpio_pull_type_t
 *
 * \brief: Pullup, pulldown or none
 */
typedef enum {
	aos_gpio_pull_type_none,		//!< No pullup/pulldown
	aos_gpio_pull_type_pullup,		//!< pullup selected
	aos_gpio_pull_type_pulldown,	//!< pulldown selected
	aos_gpio_pull_type_last,		//!< Last item in the enumerated
} aos_gpio_pull_type_t;

/*!
 * \enum aos_gpio_output_type_t
 *
 * \brief: Output type. Configurable only out output GPIO.
 */
typedef enum {
	aos_gpio_output_type_push_pull,			//!< Output in push pull mode
	aos_gpio_output_type_push_open_drain,	//!< Output in open drain
	aos_gpio_output_type_last,				//!< Last item in the enumerated
} aos_gpio_output_type_t;


/*!
 * \enum aos_gpio_irq_mode_t
 * \brief Define the GPIO interrupt request mode on a rising, falling or both edges
 * \note Applicable for GPIOs configured in digital input mode only. If an interrupt mode
 * is selected other than aos_gpio_irq_mode_none, the interrupt handler should be provided
 */
typedef enum {
    aos_gpio_irq_mode_none,					//!< No interrupts.
	aos_gpio_irq_mode_rising_edge,			//!< Interrupt on the rising edge only.
	aos_gpio_irq_mode_falling_edge,			//!< Interrupt on the falling edge only.
	aos_gpio_irq_mode_both_edges,			//Interrupt on falling and rising edges.
	aos_gpio_irq_mode_last					//!< Last item in the enumerated
} aos_gpio_irq_mode_t;

/*!
 * \enum aos_gpio_irq_priority_t
 * \brief Define the IRQ priority on the GPIO
 */
typedef enum {
	aos_gpio_irq_priority_very_low = 0,		//!< Very low priority  (0)
	aos_gpio_irq_priority_low,			//!< Low priority 		(1)
	aos_gpio_irq_priority_medium,		//!< Medium priority 	(2)
	aos_gpio_irq_priority_high,			//!< High priority		(3)
	aos_gpio_irq_priority_very_high,	//!< Very high priority (4)
	aos_gpio_irq_priority_last			//!< Last item in the enumerated
}aos_gpio_irq_priority_t;


/*!
 * \enum aos_gpio_irq_service_type_t
 * \brief Indicate how the interrupt should be serviced
 */
typedef enum {
	aos_gpio_irq_service_type_int,			//!< IRQ serviced under interrupt context
	aos_gpio_irq_service_type_thread,		//!< IRQ serviced under the system thread context
}aos_gpio_irq_service_type_t;

/*!
 * \fn aos_gpio_isr_callback_t
 *
 * \brief Callback function triggered upon events reception
 *
 * \param user_arg user argument. Opaque for the driver
 *
 * \note Applicable for digital input only.
 */
typedef void (*aos_gpio_isr_callback_t)(void* user_arg);

/*!
 * \union aos_gpio_callback_t
 *
 * \brief GPIO Callback function.
 */
typedef union {
	aos_system_user_callback_t sys_cb;	//!< Used if the interrupt is enabled and the servicing is IRP
	aos_gpio_isr_callback_t isr_cb;		//!< Used if the interrupt is enabled and the servicing is system background
} aos_gpio_callback_t;

/*!
 * \struct aos_gpio_config_t
 *
 * \brief GPIO configuration
 */
typedef struct {
	aos_gpio_mode_t mode;					//!< GPIO mode
	aos_gpio_pull_type_t pull;				//!< GPIO pullup/pulldown type
	aos_gpio_output_type_t output_type;		//!< GPIO output type
	aos_gpio_irq_mode_t	irq_mode;			//!< GPIO interruption mode. Applicable only for output mode
	aos_gpio_irq_priority_t irq_prio;		//!<GPIO interruption priority. Applicable only if irq_mode is not none
	aos_gpio_irq_service_type_t irq_servicing; //!< GPIO interruption service mode. Applicable only if irq_mode is not none
	aos_gpio_callback_t irq_handler;		//!< Interrupt handler. Applicable only if irq_mode is not none
	void* user_arg;							//!< User argument passed along the interrupt handler. Applicable only if irq_mode is not none
} aos_gpio_config_t;

/*!
 *\enum aos_gpio_state_t
 *
 *\brief input/output state of a digital GPIO
 */
typedef enum {
	aos_gpio_state_reset = 0,	//<! GPIO has a null value (logical 0)
	aos_gpio_state_set			//<! GPIO has a non null value (logical 1)
} aos_gpio_state_t;

/*!
 * \fn void aos_gpio_init(void)
 *
 * \brief Initialize the GPIO driver
 *
 * \note Used internally. Called by the system init
 */
void aos_gpio_init(void);

/*!
 * \brief Initialize the battery monitoring setting
 *
 * \param gpio_vbat_ctrl Battery monitoring. Voltage bridge control. If no voltage divider bridge, pass aos_gpio_id_last.
 * \param gpio_vbat_analog Analog pin for which we want to read the battery level value.
 * \param vbat_ctrl_ratio Voltage bridge control ratio. E.g if the bridge divides by 2 then ctrl_ratio = 2.
 *                        Value 0 is forbidden. If there is no bridge, the ctrl_ration equals 1.
 * \param vbat_ctrl_setup_delay Time in ms to stabilize the voltage divider bridge.
 * \param vbat_offset voltage offset introduced by the voltage divider bridge .
 *
 * \note The vbat parameters are used to read the battery level via the function aos_gpio_read_battery_level
 */
void aos_gpio_bat_setup(aos_gpio_id_t gpio_vbat_ctrl, aos_gpio_id_t gpio_vbat_analog, uint8_t vbat_ctrl_ratio, uint16_t vbat_ctrl_setup_delay, int16_t vbat_offset);

/*!
 * \fn aos_result_t aos_gpio_open(aos_gpio_id_t id, aos_gpio_mode_t mode)
 *
 * \brief Open and simply configure a GPIO as either a digital input or output, no pullup and
 *  push-pull in case of output.
 *
 * \param id GPIO identifier
 * \param mode GPIO mode
 *
 * \return The operation result
 */
aos_result_t aos_gpio_open(aos_gpio_id_t id, aos_gpio_mode_t mode);

/*!
 * \fn aos_result_t aos_gpio_open_ext(aos_gpio_id_t id, aos_gpio_config_t* config)
 *
 * \brief Extended opening function, which accepts a full configuration
 *
 * \param id GPIO identifier
 * \param config Full GPIO configuration
 *
 * \return The operation result
 */
aos_result_t aos_gpio_open_ext(aos_gpio_id_t id, aos_gpio_config_t* config);

/*!
 * \fn aos_result_t aos_gpio_close(aos_gpio_id_t id)
 *
 * \brief Close a GPIO
 *
 * \param id GPIO identifier
 *
 * \return The operation result
 */
aos_result_t aos_gpio_close(aos_gpio_id_t id);

/*!
 * \fn aos_result_t aos_gpio_reserve(aos_gpio_id_t id, aos_gpio_mode_t mode)
 *
 * \brief Reserve a GPIO but don't initialize it
 *
 * \param id GPIO identifier
 * \param mode GPIO mode of operation
 *
 * \return The operation result
 */
aos_result_t aos_gpio_reserve(aos_gpio_id_t id, aos_gpio_mode_t mode);

/*!
 * \fn aos_result_t aos_gpio_unreserve(aos_gpio_id_t id)
 *
 * \brief Un-reserve a GPIO but don't de-initialize it
 *
 * \param id GPIO identifier
 *
 * \return The operation result
 */
aos_result_t aos_gpio_unreserve(aos_gpio_id_t id);

/*!
 * \fn aos_result_t aos_gpio_write(aos_gpio_id_t id, aos_gpio_state_t value)
 *
 * \brief Write a value to the output of a digital output GPIO.
 *
 * \param id GPIO identifier
 * \param value Value to be written
 *
 * \return The operation result
 *
 * \warning. The GPIO must be configured as a digital output
 */
aos_result_t aos_gpio_write(aos_gpio_id_t id, aos_gpio_state_t value);

/*!
 * \fn aos_result_t aos_gpio_read(aos_gpio_id_t id, uint16_t* value)
 *
 * \brief Read a value of a GPIO
 *
 * \param id GPIO identifier
 * \param value Returned value in case of success.
 *
 * \return The operation result
 *
 */
aos_result_t aos_gpio_read(aos_gpio_id_t id, uint16_t* value);

/*!
 * \fn aos_result_t aos_gpio_toggle(aos_gpio_id_t id)
 *
 * \brief Toggle the output of a GPIO
 *
 * \param id GPIO identifier
 *
 * \return The operation result
 *
 * \warning. The GPIO must be configured as a digital output
 */
aos_result_t aos_gpio_toggle(aos_gpio_id_t id);


/*!
 * \fn aos_result_t aos_gpio_get_mode(aos_gpio_id_t id, aos_gpio_mode_t* mode)
 *
 * \brief Return the mode of a GPIO
 *
 * \param id GPIO identifier
 * \param mode returned value
 *
 * \return The operation result
 */
aos_result_t aos_gpio_get_mode(aos_gpio_id_t id, aos_gpio_mode_t* mode);


/*!
 * \fn aos_result_t aos_gpio_read_battery_voltage(uint16_t* bat_level)
 *
 * \brief Read the battery voltage, using the configuration provided at the initialization
 * of the GPIO driver.
 *
 * \param bat_level Returned the battery voltage in mV.
 *
 * \return The operation result
 */
aos_result_t aos_gpio_read_battery_voltage(uint16_t* bat_level);


/*!
 * \fn const char* aos_gpio_get_mode_str(aos_gpio_id_t id)
 *
 * \brief Return the configured mode in string format
 *
 * \param id GPIO identifier
 *
 * \return The mode in string format
 */
const char* aos_gpio_get_mode_str(aos_gpio_id_t id);
/*! @}*/

#if defined(__cplusplus)
}
#endif
