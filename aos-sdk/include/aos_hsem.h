/*!
 * \file aos_hsem.h
 *
 * \brief AOS Hardware Semaphore interface (part of system initialization)
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_hsem Hardware semaphore initialization
 *
 * \addtogroup aos_hsem
 * @{
 */

/*!
 * \fn aos_hsem_init(void)
 *
 * \brief Initialize the HSEM subsystem
 *
 * \details This function is called as part of the aos system initialization.
 *
 * \return none
 */
void aos_hsem_init(void);
/*! @}*/

#ifdef __cplusplus
}
#endif
