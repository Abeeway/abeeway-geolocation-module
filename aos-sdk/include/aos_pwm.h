/*!
 * \file aos_pwm.h
 *
 * \brief PWM (Pulse Width Modulated) driver
 *
 * \details
 * The timer 16 is used. It is clocked by APB2 clock (timer part), which is PLCK2.
 * The current clocking frequency in running mode is 64 MHz.
 *
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "aos_common.h"


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_pwm PWM driver
 *
 * \addtogroup aos_pwm
 * @{
 */

/*!
 * \fn aos_result_t aos_pwm_start(uint32_t frequency, uint8_t duty_cycle)
 *
 * \brief Start the PWM at the given frequency and duty cycle
 *
 * \param frequency Output frequency in Hertz
 * \param duty_cycle Duty cycle in percentage.
 *
 * \return Result of the operation
 *
 * \note You can call this function multiple times without calling the aos_pwm_stop\n
 * This way you will change dynamically the couple frequency/duty-cycle.
 *
 */
aos_result_t aos_pwm_start(uint32_t frequency, uint8_t duty_cycle);

/*!
 * \fn aos_result_t aos_pwm_stop(void)
 *
 * \brief Stop the PWM (but kept configured and clocked).
 *
 * \return Result of the operation
 *
 */
aos_result_t aos_pwm_stop(void);

/*! @}*/
#ifdef __cplusplus
}
#endif
