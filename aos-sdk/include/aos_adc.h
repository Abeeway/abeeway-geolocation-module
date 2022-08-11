/*!
 * \file aos_adc.h
 *
 * \brief Analog to Digital Converter (ADC)
 *
 * \details The table below provides the ADC channel number associated to a given internal GPIO.<BR>
 * Note that the last column indicates the signal of the module attached to the GPIO whenever applicable.
 *
 * <TABLE>
 * <TR>
 * 	<TH>PIN</TH><TH>Channel</TH><TH>Signal</TH>
 * </TR><TR>
 *	<TD>PA0</TD><TD>IN5</TD><TD>user_adc1</TD>
 * </TR><TR>
 * <TD>PA1</TD><TD>IN6</TD><TD>vbat_sense</TD>
 * </TR><TR>
 *	<TD>PA2</TD><TD>IN7</TD><TD>NA</TD>
 * </TR><TR>
 *	<TD>PA3</TD><TD>IN8</TD><TD>NA</TD>
 * </TR><TR>
 *	<TD>PA4</TD><TD>IN9</TD><TD>NA</TD>
 * </TR><TR>
 *	<TD>PA5</TD><TD>IN10</TD><TD>NA</TD>
 * </TR><TR>
 *	<TD>PA6</TD><TD>IN9</TD><TD>NA</TD>
 * </TR><TR>
 *	<TD>PA7</TD><TD>IN10</TD><TD>NA</TD>
 * </TR><TR>
 *	<TD>PA8</TD><TD>IN11</TD><TD>NA</TD>
 * </TR><TR>
 *	<TD>PC0</TD><TD>IN1</TD><TD>I2C3 SCL possible</TD>
 * </TR><TR>
 *	<TD>PC2</TD><TD>IN2</TD><TD>NA</TD>
 * </TR><TR>
 *	<TD>PC3</TD><TD>IN4</TD><TD>I2C_INT1 possible</TD>
 * </TR><TR>
 *	<TD>PC4</TD><TD>IN13</TD><TD>GPIO8</TD>
 * </TR><TR>
 *	<TD>PC5</TD><TD>IN14</TD><TD>GPI09</TD>
 * </TR>
 *	</TABLE>
 *
 * \note Usually, this facility is used internally by AOS.
 * For AOS user, this facility should be not used directly. Instead, use the GPIO driver with
 * analog GPIO and for battery measurements. Use the system facility for the CPU temperature.
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#pragma once

#include "aos_common.h"
#include "aos_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_adc ADC driver
 *
 * \addtogroup aos_adc
 * @{
 */

/*!
 * \fn aos_result_t aos_adc_read(aos_gpio_id_t gpio_analog, uint16_t* read_value)
 *
 * \brief Read an analog GPIO using the ADC
 *
 * \param gpio_analog Analog pin for which we want to read the value
 * \param read_value Output value containing the value. The value is in steps of 1/4095
 * based on vRef+
 *
 * \return Result of the operation
 *
 * \note, The function expects the gpio opened before calling
 */
aos_result_t aos_adc_read(aos_gpio_id_t gpio_analog, uint16_t* read_value);

/*!
 * \fn aos_result_t aos_adc_read_cpu_temperature(int16_t* temperature)
 *
 * \brief Read the CPU temperature
 *
 * \param temperature Output value containing the temperature level. The value is in degree.
 *
 * \return Result of the operation
 *
 * \note, The function manages the GPIO by itself.
 */
aos_result_t aos_adc_read_cpu_temperature(int16_t* temperature);

/*! @}*/
#ifdef __cplusplus
}
#endif
