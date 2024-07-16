/*
 * \file aos_ble_ipcc.h
 *
 * \brief Inter-processor communication channel setup
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_ble_ipcc Inter-processor Communication Channel
 *
 * \addtogroup aos_ble_ipcc
 * @{
 */

/*!
 * \fn  void aos_ble_ipcc_reset(void)
 *
 * \brief Reset the inter-processor communication channel
 *
 * \note IPCC should be reset after a SW reset. That way the IPCC restarts as for the power on case.
 * Called by AOS only at the initialization time.
 */
void aos_ble_ipcc_reset( void );


/*!
 * \fn  bool aos_ble_ipcc_init(void)
 *
 * \brief Initialize the inter-processor communication channel
 *
 * \return True if correct initialization, false otherwise.
 *
 * \note Called by AOS only at the initialization time.
 */
bool aos_ble_ipcc_init(void);


/*! @}*/
#ifdef __cplusplus
}
#endif
