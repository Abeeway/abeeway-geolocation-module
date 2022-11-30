/**
  ******************************************************************************
  * @file    ias.h
  * @author  MCD Application Team
  * @brief   Header for ias.c module
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
#ifndef __IAS_H
#define __IAS_H

#ifdef __cplusplus
extern "C" 
{
#endif


/* Includes ------------------------------------------------------------------*/


/*!
 * \defgroup IAS BLE Immediate Alert Service
 *
 * \addtogroup IAS
 * @{
 */

/* Exported types ------------------------------------------------------------*/

/*!
 * \enum IAS_App_Opcode_Notification_evt_t
 *
 * \brief Immediate Alert service event
 */
typedef enum {
	IAS_NO_ALERT_EVT,      //!< No Alert event
	IAS_MID_ALERT_EVT,     //!< MID Alert event
	IAS_HIGH_ALERT_EVT     //!< HIGH Alert event
} IAS_App_Opcode_Notification_evt_t;

/*!
 * \struct IAS_App_Notification_evt_t
 *
 * \brief Immediate Alert service event struct
 */
typedef struct {
	IAS_App_Opcode_Notification_evt_t  IAS_Evt_Opcode;  //!< IAS notification event code
} IAS_App_Notification_evt_t;


/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/*!
 * \fn tBleStatus ias_init(void)
 *
 * \brief Immediate Alert service Initialization
 *
 * \return tBleStatus status
 */
tBleStatus ias_init(void);

/*!
 * \fn tBleStatus ias_update_char(uint16_t uuid, uint8_t *payload)
 *
 * \brief Update the data of the characteristic UUID with pPayload data
 *
 * \param UUID battery characteristic to update
 *
 * \param pPayload data to update
 *
 * \return tBleStatus status
 */
tBleStatus ias_update_char(uint16_t uuid, uint8_t *payload);

/*!
 * \fn void ias_app_notification(BAS_Notification_evt_t * pNotification)
 *
 * \brief Immediate Alert service notification function
 *
 * \param pNotification notification event
 */
void ias_app_notification(IAS_App_Notification_evt_t *pNotification);


/*! @}*/
#ifdef __cplusplus
}
#endif

#endif /*__IAS_H */


