/**
  ******************************************************************************
  * @file    tps.h
  * @author  MCD Application Team
  * @brief   Header for tps.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TPS_H
#define __TPS_H

#ifdef __cplusplus
extern "C" 
{
#endif

/* Includes ------------------------------------------------------------------*/


/*!
 * \defgroup TPS BLE TX power service
 *
 * \addtogroup TPS
 * @{
 */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/*!
 * \fn tBleStatus tps_init(void)
 *
 * \brief TX power service Initialization
 *
 * \return tBleStatus status
 */
tBleStatus tps_init(void);

/*!
 * \fn tBleStatus tps_update_char(uint16_t uuid, uint8_t *payload)
 *
 * \brief Update the data of the characteristic UUID with pPayload data
 *
 * \param UUID TX power characteristic to update
 *
 * \param pPayload data to update
 *
 * \return tBleStatus status
 */
tBleStatus tps_update_char(uint16_t uuid, uint8_t *payload);

/*! @}*/
#ifdef __cplusplus
}
#endif

#endif /*__TPS_H */


