/*!
 * \file aos_calib_hsi.h
 *
 * \brief HSI calibration
 *
 * \details This module performs the HSI frequency measurement at different HSI trimming values,
 * then apply the best value found. The timer 16 is used for frequency measurements.
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
 * \defgroup aos_calib_hsi HSI clock calibration
 *
 * \addtogroup aos_calib_hsi
 * @{
 */

/*!
 * \brief Calibration result
 */
typedef struct {
	uint32_t initial_hsi_freq;				//!< HSI frequency before the calibration (Hertz)
	uint32_t corrected_hsi_freq;			//!< HSI frequency after the calibration (Hertz)
	uint32_t corrected_trimming_value;		//!< Trimming value for the corrected frequency
} aos_calib_hsi_result_t;


/*!
 * \fn uint32_t aos_calib_hsi_measure(void)
 *
 * \brief Perform the HSI frequency measure
 *
 * \return The measured frequency in Hertz
 *
 */
uint32_t aos_calib_hsi_measure(void);


/*!
 * \fn bool aos_calib_hsi_calibrate(void)
 *
 * \brief Perform the HSI calibration and update the HSI user trimming value.
 *
 * \return true if correct completion, false otherwise
 *
 * \note Usually, this function is called by AOS only
 */
bool aos_calib_hsi_calibrate(void);

/*!
 * \fn aos_calib_hsi_result_t aos_calib_hsi_get_result(void)
 *
 * \brief Return the calibration results
 *
 * \return Result of the calibration
 */
aos_calib_hsi_result_t* aos_calib_hsi_get_result(void);

/*! @}*/
#ifdef __cplusplus
}
#endif
