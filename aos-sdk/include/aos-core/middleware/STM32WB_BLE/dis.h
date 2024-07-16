/**
  ******************************************************************************
  * @file    dis.h
  * @author  MCD Application Team
  * @brief   Header for dis.c module
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
#ifndef __DIS_H
#define __DIS_H

#ifdef __cplusplus
extern "C" 
{
#endif

/* Includes ------------------------------------------------------------------*/


/*!
 * \defgroup DIS DIS Service
 *
 * \addtogroup DIS
 * @{
 */

/* Exported types ------------------------------------------------------------*/

/*!
 * \struct DIS_Data_t
 *
 * \brief Data to update to the DIS characteristics
 */
typedef struct {
	uint8_t     *pPayload;  //!< Characteristic data
	uint8_t     Length;     //!< Characteristic data length
} DIS_Data_t;


/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/*!
 * \fn void dis_init(void)
 *
 * \brief Initialize Device Information Service
 */
tBleStatus dis_init(void);

/*!
 * \fn tBleStatus dis_update_char(uint16_t uuid, DIS_Data_t *data)
 *
 * \brief Update DIS characteristic
 *
 * \param uuid Characteristic to update UUID
 *
 * \param data Pointer to the data
 *
 * \return result status
 */
tBleStatus dis_update_char(uint16_t uuid, DIS_Data_t *data);

/*! @}*/
#ifdef __cplusplus
}
#endif

#endif /*__DIS_H */


