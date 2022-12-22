/**
  ******************************************************************************
  * @file    lls.h
  * @author  MCD Application Team
  * @brief   Header for lls.c module
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
#ifndef __LLS_H
#define __LLS_H

#ifdef __cplusplus
extern "C" 
{
#endif

/* Includes ------------------------------------------------------------------*/


/*!
 * \defgroup LLS BLE Link Loss Service
 *
 * \addtogroup LLS
 * @{
 */

/* Exported types ------------------------------------------------------------*/

/*!
 * \enum LLS_App_Opcode_Notification_evt_t
 *
 * \brief Link Loss service event
 */
typedef enum {
	LLS_NO_ALERT_EVT,      //!< No Alert event
	LLS_MID_ALERT_EVT,     //!< MID Alert event
	LLS_HIGH_ALERT_EVT,    //!< HIGH Alert event
	LLS_DISCONNECT_EVT,    //!< Disconnect event
	LLS_CONNECT_EVT        //!< Connect event
} LLS_App_Opcode_Notification_evt_t;

/*!
 * \struct LLS_App_Notification_evt_t
 *
 * \brief Link Loss service event struct
 */
typedef struct {
	LLS_App_Opcode_Notification_evt_t  LLS_Evt_Opcode;  //!< LLS notification event code
} LLS_App_Notification_evt_t;


/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/*!
 * \fn tBleStatus lls_init(void)
 *
 * \brief Link Loss service Initialization
 *
 * \return tBleStatus status
 */
tBleStatus lls_init(void);

/*!
 * \fn tBleStatus lls_update_char(uint16_t uuid, uint8_t *payload)
 *
 * \brief Update the data of the characteristic UUID with pPayload data
 *
 * \param UUID battery characteristic to update
 *
 * \param pPayload data to update
 *
 * \return tBleStatus status
 */
tBleStatus lls_update_char(uint16_t uuid, uint8_t *payload);

/*!
 * \fn void lls_app_notification(BAS_Notification_evt_t * pNotification)
 *
 * \brief Link Loss service notification function
 *
 * \param pNotification notification event
 */
void lls_app_notification(LLS_App_Notification_evt_t *pNotification);


/*! @}*/
#ifdef __cplusplus
}
#endif

#endif /*__LLS_H */


