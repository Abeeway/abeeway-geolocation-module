
/**
  ******************************************************************************
  *  @file    ess.h
  * @author  MCD Application Team
  * @brief   Header for ess.c module
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
#ifndef __ESS_H
#define __ESS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


/*!
 * \defgroup ESS Environmental Sensing service
 *
 * \addtogroup ESS
 * @{
 */


/* Exported types ------------------------------------------------------------*/

/*!
 * \enum ESS_Opcode_evt_t
 *
 * \brief Environmental sensing service event
 */
typedef enum {
	ESS_TEMPERATURE_NOTIFY_ENABLED_EVT,   //!< Battery level notification enabled event
	ESS_TEMPERATURE_NOTIFY_DISABLED_EVT,  //!< Battery level notification enabled event
	ESS_TEMPERATURE_READ_EVT,             //!< Battery level notification enabled event
} ESS_Opcode_evt_t;

/*!
 * \struct ESS_App_Notification_evt_t
 *
 * \brief Environmental sensing service event struct
 */
typedef struct {
	ESS_Opcode_evt_t     ESS_Evt_Opcode;    //!< ESS notification event code
} ESS_App_Notification_evt_t;


/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/*!
 * \fn tBleStatus ess_init(void)
 *
 * \brief Environmental sensing service Initialization
 *
 * \return tBleStatus status
 */
tBleStatus ess_init(void);

/*!
 * \fn void ess_app_notification(BAS_Notification_evt_t * pNotification)
 *
 * \brief Environmental sensing service notification function
 *
 * \param pNotification notification event code
 */
void ess_app_notification(ESS_App_Notification_evt_t *pNotification);

/*!
 * \fn tBleStatus ess_udate_char(uint16_t uuid, uint8_t *payload)
 *
 * \brief Update the data of the characteristic UUID with pPayload data
 *
 * \param UUID Environmental sensing characteristic to update
 *
 * \param pPayload data to update
 *
 * \return tBleStatus status
 */
tBleStatus ess_udate_char(uint16_t uuid, uint8_t *payload);


/*! @}*/
#ifdef __cplusplus
}
#endif

#endif /*__ESS_H */


