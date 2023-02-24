/*
 * gnss_handler.h
 *
 *  Created on: Jan 27, 2023
 *      Author: jptogbe
 */

#ifndef GNSS_HANDLER_H_
#define GNSS_HANDLER_H_

#include "srv_gnss.h"

typedef struct {
	aos_gnss_constellation_t  constellation;   			//!< Constellation of the satellite
	uint8_t              sv_id;                         //!< Satellite identifier
	uint8_t              sync_flags;                    //!< Synchronization flags. Refer to GPS_SAT_SYNC_xxx
	uint8_t              cn0;                           //!< Carrier To Noise value
	uint32_t             compress_prn;           		//!< Pseudo range in meters.
} aos_gnss_satellite_compress_prn_t;

typedef struct {
	aos_gnss_time_info_t     gnss_time;                  					//!< GPS time information
	uint8_t             nb_sat;                          					//!< Number of satellites in the report
	aos_gnss_satellite_compress_prn_t sat_info[AOS_GNSS_MAX_SATELLITES]; 	//!< Array of the satellite information
} aos_gnss_satellite_data_t;

extern void lora_gnss_send(aos_gnss_satellite_prn_report_t*  prn_report);

#endif /* GNSS_HANDLER_H_ */
