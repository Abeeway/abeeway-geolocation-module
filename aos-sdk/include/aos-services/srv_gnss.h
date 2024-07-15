/*!
 * \file srv_gnss.h
 *
 * \brief GNSS service
 *
 * The GNSS service  manages the different GPS supported by AOS.
 * An IOCTL API is provided for controlling specific parameters
 *
 */
#pragma once

#include "aos_system.h"
#include "aos_gnss_common.h"
#include "aos_rtc.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup gnss_srv GNSS service
 *
 * \addtogroup gnss_srv
 * @{
 */

/*!
 * \enum srv_gnss_type_t
 *
 * \brief Type of GPS driver to be used.
 *
 * \note Only one driver can be used at a time.
 */
typedef enum {
    srv_gnss_type_mt3333,			    //!< Use the Mediatek MT3333
	srv_gnss_type_last					//!< Last driver in the list
} srv_gnss_type_t;

/*!
 * \enum srv_gnss_mode_xgnss_t
 *
 * \brief GNSS mode
 */
typedef enum {
	srv_gnss_mode_agnss = 0,			//!< Assisted GNSS. Can move to non-assisted GNSS
	srv_gnss_mode_gnss					//!< Non-assisted GNSS.
} srv_gnss_mode_xgnss_t;


/*! @}*/
#ifdef __cplusplus
}
#endif


