/**
  ******************************************************************************
  * @file    ble.h
  * @author  MCD Application Team
  * @brief   BLE interface
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
#ifndef __BLE_H
#define __BLE_H

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "aos_ble_conf.h"

/**< core */
#include "ble_core.h"
#include "ble_bufsize.h"
#include "ble_defs.h"
#include "ble_legacy.h"
#include "ble_std.h"

/**< blesvc (svc/Inc) */
#include "bas.h"
#include "ess.h"
#include "dis.h"
#include "ias.h"
#include "lls.h"
#include "tps.h"
#include "otas_stm.h"
#include "template_stm.h"
  
#include "aos_ble_common.h"
#include "custom_srvc.h"
#include "svc_ctl.h"

#include "uuid.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /*__BLE_H */


