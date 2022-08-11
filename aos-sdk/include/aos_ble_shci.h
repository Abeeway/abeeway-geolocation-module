/*
 * \file aos_ble_shci.h
 *
 * \brief BLE Control transport access (SHCI)
 */
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_ble_shci BLE Control transport access
 *
 * \addtogroup aos_ble_shci
 * @{
 */

/*!
 * \fn  void aos_ble_shci_init(void)
 *
 * \brief Initialize the control transport with the M0+
 *
 * \note Called by AOS only
 */
void aos_ble_shci_init( void );


/*! @}*/
#ifdef __cplusplus
}
#endif
