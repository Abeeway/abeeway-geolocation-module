/*!
 * \file app_debug.h
 *
 * \brief Application debug APIs
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_DEBUG_H
#define __APP_DEBUG_H


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup app_debug Application debug APIs
 *
 * \addtogroup app_debug
 * @{
 */


/* Exported functions ---------------------------------------------*/

/*!
 * \fn void APPD_Init( void )
 *
 * \brief Application debug initialization
 */
void APPD_Init( void );

/*!
 * \fn void APPD_EnableCPU2( void )
 *
 * \brief CPU2 application debug enable
 */
void APPD_EnableCPU2( void );


/*! @}*/
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__APP_DEBUG_H */
