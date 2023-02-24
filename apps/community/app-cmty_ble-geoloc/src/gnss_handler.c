/*
 * gnss_handler.c
 *
 *  Created on: Jan 27, 2023
 *      Author: jptogbe
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h> // For abs()
#include <math.h>

#include "encode_handling.h"
#include "srv_lmh.h"
#include "srv_ble_scan.h"
#include "srv_cli.h"

#include "srv_gnss.h"
#include "gnss_handler.h"
#include "srv_cli.h"
#include "aos_gnss_common.h"
#include "lora_handler.h"

#define SPEED_OF_LIGHT	299792.45799999998	//!< Speed of light
#define TWO_TO_THE_TWENTY_FOURTH	16777216//!< 2^24

#define MIN_SATELLITE_DISTANCE	1800000000L //!< Minimum satellite distance in centimeters
#define MAX_SATELLITE_DISTANCE	2800000000L	//!< Maximum satellite distance in centimeters
#define GNSS_MAX_SATELLITE_ID		32  	//!< We support up to id 32 for AGPS for now
#define GNSS_GOOD_C_TO_N   			15 		//!< We need this C/N value to be considered good
#define	GNSS_SATELLITE_COUNT 		10		//!< We can see only 10 satellites
#define GNSS_MIN_GPS_BEIDOU_COUNT	 2		//!< In case of GPS and BEIDOU We must have at least 2 satellites GPS and 2 satellites BEIDOU


static aos_gnss_satellite_prn_report_t  gnss_prn_report;
static aos_gnss_satellite_prn_report_t* g_g_prn_report = &gnss_prn_report;

static aos_gnss_cfg_constellation_t gnss_cf_constel = AOS_GNSS_CFG_ENABLE_GPS;

static bool gps_beidou_acquire = false;

static uint8_t svid_real[]=      {11, 12, 14, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32, 33, 34, 35, 36, 37, 41, 42, 43, 44, 45, 46};  //!< Real beidou svid
static uint8_t svid_transmited[]={ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};	//!< Transmited svid

/**
 * Function to encode satelite's svid, cn and constellation type
 **/
static uint8_t _encode_type_svid_cn(aos_gnss_constellation_t type, uint8_t cn, uint8_t svid)
{
    uint8_t value;
    uint8_t cnr;

    if(cn < 15){
    	cnr = 0;
    }else if(cn < 25){
    	cnr = 1;
    }else if(cn < 35){
    	cnr = 2;
    }else{
    	cnr = 3;
    }
    value = svid & 0x1F;										//!< svid 					<< on 5 bits
    value |= (cnr & 0x3) << 5;									//!< cn 					<< on 2 bits
    value |= ((type == aos_gnss_constellation_gps)?0:1) << 7;	//!< constallation type 	<< on 1 bits

    return value;
}


/*

static void _collect_constel(aos_gnss_satellite_prn_t* collect_gps, aos_gnss_satellite_prn_t* collect_beidou, aos_gnss_satellite_prn_t* prn_src, uint8_t nb_sats)
{
	uint8_t i_gps = 0;
	uint8_t i_beidou = 0;

	for(uint8_t ii = 0; ii < nb_sats; ii++){
		if(prn_src[ii].constellation == aos_gnss_constellation_gps){
			collect_gps[i_gps] = prn_src[ii];
			i_gps++;
		}
		if(prn_src[ii].constellation == aos_gnss_constellation_beidou){
			collect_beidou[i_beidou] = prn_src[ii];
			i_beidou++;
		}
	}
}*/
/**
 * Function to sort by synchro (and or) C/N
 *
 **/

static void _sort_sync_cn(aos_gnss_satellite_prn_t* collect_constel, uint8_t nb_sats)
{
	aos_gnss_satellite_prn_t buf;

		for(uint8_t i = 0; i < nb_sats-1; i++){
			for(uint8_t j = i+1; j < nb_sats; j++){
				if(collect_constel[i].sync_flags < collect_constel[j].sync_flags){
					memcpy(&buf					, &collect_constel[i], sizeof(buf));
					memcpy(&collect_constel[i]	, &collect_constel[j], sizeof(buf));
					memcpy(&collect_constel[j]	, &buf				, sizeof(buf));
				}

				if(collect_constel[i].sync_flags == collect_constel[j].sync_flags){
					if(collect_constel[i].cn0 < collect_constel[j].cn0){
						memcpy(&buf					, &collect_constel[i], sizeof(buf));
						memcpy(&collect_constel[i]	, &collect_constel[j], sizeof(buf));
						memcpy(&collect_constel[j]	, &buf				 , sizeof(buf));
					}
				}
			}
		}
}

/*
static void _copy_gps_beidou_rest(aos_gnss_satellite_prn_t* collect_rest_gps_beidou, aos_gnss_satellite_prn_t* collect_gps, aos_gnss_satellite_prn_t* collect_beidou)
{
	uint8_t i_rest =0;
	for(uint8_t ii = 2; ii < sizeof(*collect_gps); ii++){
		memcpy(&collect_rest_gps_beidou[i_rest], &collect_gps[ii], sizeof(collect_gps[ii]));
		i_rest++;
	}
	for(uint8_t ii = 2; ii < sizeof(*collect_beidou); ii++){
		memcpy(&collect_rest_gps_beidou[i_rest], &collect_beidou[ii], sizeof(collect_beidou[ii]));
		i_rest++;
	}
}*/
/**
 * The svid is encode on 5 bits then to respect 5 bits for the svid we need to create a mapping between the real svid and the transmitted svid.
 * this function help to mapping Beidou svid.
 * svid_real[] 			>> real Beidou svid
 * svid_transmited[] 	>> transmitteg svid
 *
 **/
static uint8_t _svid_beidou_mapping(uint8_t value)
{
	uint8_t return_value = value;
	for(int i=0; i<=sizeof(svid_real); i++){
			if(svid_real[i]==value){
				return_value = svid_transmited[i];
				break;
			}
		}
		return return_value;
}

/**
 * THis function return true when the svid received is found in the real tab, and false if it is not.
 *
 **/
static bool _svid_beidou_mapping_ack(uint8_t value)
{
bool mapping_ack = false;
	for(int i=0; i<=sizeof(svid_real); i++){
		if(svid_real[i]==value){
			mapping_ack = true;
			break;
		}
	}
	return mapping_ack;
}

 /**
  * Function to compress a pseudo range
  * return uint32_t
  *
  **/
static uint32_t _gnss_compress_prn(aos_gnss_satellite_prn_t* prn)
{
	uint32_t 	prn_int = 0;
	double 		prn_dbl = 0;

	//	Scale
	prn_dbl = prn->pseudo_range/100;								//!< Convert in meters
	prn_dbl = prn_dbl * TWO_TO_THE_TWENTY_FOURTH / SPEED_OF_LIGHT;

	//	Modulo
	prn_int = round(prn_dbl);
	return prn_int % TWO_TO_THE_TWENTY_FOURTH;
}

/**
 * Function to sort pseudo range
 *
 **/
static void _gnss_sort_prn(aos_gnss_satellite_prn_report_t * prn_report, uint8_t nb_sats)
{
#define AGM(i) prn_report->sat_info[i]
	//uint8_t		index   = 0;
	aos_gnss_satellite_prn_t buf;

	for(uint8_t i = 0; i < nb_sats-1; i++){
		for(uint8_t j = i+1; j < nb_sats; j++){
			//cli_printf("i:%d	=> prn :%u ", i, AGM(i).pseudo_range);
			//cli_printf("| j:%d	=> prn :%u \n",j, AGM(j).pseudo_range);
			if(AGM(i).pseudo_range > AGM(j).pseudo_range){
				memcpy(&buf			, &AGM(i)  , sizeof(buf));
				memcpy(&AGM(i) 		, &AGM(j)  , sizeof(buf));
				memcpy(&AGM(j)		, &buf     , sizeof(buf));
			}
		}
	}
}

// When reaching this points, satellites are already filtered regarding their C/N and sync flag
// So they are considered as stable
static void _gnss_digest_prn(aos_gnss_satellite_prn_report_t* old_prn_rpt, aos_gnss_satellite_prn_report_t*  new_prn_report)
{
	uint8_t ii;
	uint8_t good_sat = 0;
	uint8_t max_sat;
	uint8_t nb_sat_for_payload = 0;
	uint8_t nb_gps = 0, nb_beidou = 0;
	aos_gnss_satellite_prn_t* sat_data = old_prn_rpt->sat_info;
	//aos_gnss_satellite_prn_t b_sat_data;
	//aos_gnss_satellite_prn_t* buf_sat_data = &b_sat_data;
/*
	aos_gnss_satellite_prn_t col_gps, col_beidou, col_rest_gps_beidou;
	aos_gnss_satellite_prn_t* collect_gps = &col_gps;
	aos_gnss_satellite_prn_t* collect_beidou = &col_beidou;
	aos_gnss_satellite_prn_t* collect_rest_gps_beidou = &col_rest_gps_beidou;
*/

	aos_gnss_ioctl_t ioctl;
	//aos_gnss_satellite_prn_t* sat_data = &ctx->prn_report.sat_info;
	bool tow_present = false;
	bool acquire = false;



	if((new_prn_report->gnss_time.type == aos_gnss_time_type_week_tow) & (new_prn_report->gnss_time.gnss_week_tow.week)){
		tow_present = true;
	}

	cli_printf("Process %d PRN. TOW: %s\n", new_prn_report->nb_sat, tow_present?"Yes":"No");

	//Get the constellation configuration
	ioctl.req = aos_gnss_ioctl_req_get_constel;
	if (srv_gnss_ioctl(aos_gnss_type_mt3333, &ioctl) != aos_result_success) {
			cli_printf("GNSS ioctl fails\n");
	}


	//Check if the news data is better than the previous measure
	if(ioctl.constellation == aos_gnss_cfg_constellation_gps_beidou){
		max_sat = new_prn_report->nb_sat;
	}else{
		max_sat = (new_prn_report->nb_sat <GNSS_SATELLITE_COUNT)? new_prn_report->nb_sat: GNSS_SATELLITE_COUNT;
	}


	for (ii = 0; ii < max_sat; ii ++) {
		if ((new_prn_report->sat_info[ii].pseudo_range >= MIN_SATELLITE_DISTANCE) &&
				(new_prn_report->sat_info[ii].pseudo_range <= MAX_SATELLITE_DISTANCE) &&
				(new_prn_report->sat_info[ii].cn0 > GNSS_GOOD_C_TO_N)  &&
				(AOS_GNSS_SAT_SYNC_GET_STATE(new_prn_report->sat_info[ii].sync_flags) >= AOS_GNSS_SAT_SYNC_BIT)) {

			if((new_prn_report->sat_info[ii].constellation == aos_gnss_constellation_gps)){
				good_sat ++;
				nb_gps ++;
			}
			if(new_prn_report->sat_info[ii].constellation == aos_gnss_constellation_beidou){

				if(_svid_beidou_mapping_ack(new_prn_report->sat_info[ii].sv_id)){
					good_sat ++;
					nb_beidou ++;
					cli_printf("Real svid:%d | Transmitted svid:%d\n",new_prn_report->sat_info[ii].sv_id,_svid_beidou_mapping(new_prn_report->sat_info[ii].sv_id));
				}
			}

		}
	}
	cli_printf("nb GPS :%d	| nb Beidou:%d\n", nb_gps, nb_beidou);
	//if (new_prn_report->gnss_time.gnss_week_tow.week){	//gnss_time.gnss_week_tow.week
		if((tow_present) && (good_sat >= old_prn_rpt->nb_sat)){
			acquire = true;
		}
		if((tow_present) && (good_sat >= old_prn_rpt->nb_sat) && (nb_gps >= GNSS_MIN_GPS_BEIDOU_COUNT) && (nb_beidou >= GNSS_MIN_GPS_BEIDOU_COUNT)){
			acquire = true;
			gps_beidou_acquire = true;
		}
	//}else{
		if((tow_present) || (good_sat >= old_prn_rpt->nb_sat)){
			acquire = true;
		}
		if((tow_present) || ((good_sat >= old_prn_rpt->nb_sat) && (nb_gps >= GNSS_MIN_GPS_BEIDOU_COUNT) && (nb_beidou >= GNSS_MIN_GPS_BEIDOU_COUNT))){
			acquire = true;
			gps_beidou_acquire = true;
		}
	//}

	if(acquire){
		cli_printf("enter ACQUIRE IF\n");
		//better collection. Reset the gnss_week_tow for the new collection
		good_sat = 0;
		nb_gps = 0;
		nb_beidou =0;



		if (ioctl.constellation == aos_gnss_cfg_constellation_gps_beidou){//<! GPS + BEIDOU
			if(gps_beidou_acquire == true){
				cli_printf("enter GPS_BEIDOU_ACQUIRE == true\n");
				for(ii = 0; ii < max_sat; ii++){
					if(new_prn_report->nb_sat < 6){
						continue; //!< skip it
					}
					if((!new_prn_report->sat_info[ii].pseudo_range) ||
							(AOS_GNSS_SAT_SYNC_GET_STATE(new_prn_report->sat_info[ii].sync_flags) < AOS_GNSS_SAT_SYNC_BIT)){
						continue; //!< skip it
					}

					// For now, the server supports only GPS and BEIDOU contellations
					if((new_prn_report->sat_info[ii].constellation != aos_gnss_constellation_gps) &&
							(new_prn_report->sat_info[ii].constellation != aos_gnss_constellation_beidou) &&
							(new_prn_report->sat_info[ii].constellation != aos_gnss_constellation_unknown)){
						continue; //!< skip it
					}
					if((new_prn_report->sat_info[ii].pseudo_range < MIN_SATELLITE_DISTANCE) ||
							(new_prn_report->sat_info[ii].pseudo_range > MAX_SATELLITE_DISTANCE)){
						continue; //!< skip it
					}

					if(new_prn_report->sat_info[ii].cn0 >= GNSS_GOOD_C_TO_N){
						nb_sat_for_payload++;
					}


					/*
					buf_sat_data[good_sat].cn0 			 = new_prn_report->sat_info[ii].cn0;
					buf_sat_data[good_sat].constellation = new_prn_report->sat_info[ii].constellation;
					buf_sat_data[good_sat].pseudo_range  = new_prn_report->sat_info[ii].pseudo_range;
					buf_sat_data[good_sat].sv_id 		 = new_prn_report->sat_info[ii].sv_id;
					buf_sat_data[good_sat].sync_flags    = new_prn_report->sat_info[ii].sync_flags;*/



					sat_data[good_sat].cn0 			 = new_prn_report->sat_info[ii].cn0;
					sat_data[good_sat].constellation = new_prn_report->sat_info[ii].constellation;
					sat_data[good_sat].pseudo_range  = new_prn_report->sat_info[ii].pseudo_range;
					sat_data[good_sat].sv_id 		 = new_prn_report->sat_info[ii].sv_id;
					sat_data[good_sat].sync_flags    = new_prn_report->sat_info[ii].sync_flags;

					good_sat++;
				}

				if(good_sat > 10){
					_sort_sync_cn(sat_data, good_sat);
				}

				//_sort_sync_cn(buf_sat_data, good_sat);
				//_collect_constel(collect_gps, collect_beidou, buf_sat_data, good_sat);

				//_sort_sync_cn(collect_gps, good_sat);
				//_sort_sync_cn(collect_beidou, good_sat);

				//_copy_gps_beidou_rest(collect_rest_gps_beidou, collect_gps, collect_beidou);

				//_sort_sync_cn(collect_rest_gps_beidou, (sizeof(*collect_gps)+sizeof(*collect_beidou)-4));
				/*good_sat = 0;
				for(uint8_t ii = 0; ii<sizeof(*buf_sat_data); ii++){
					sat_data[ii] = buf_sat_data [ii];
					good_sat++;
				}*/
				/*
				uint8_t jj =0;

				for(uint8_t ii = 0; ii<2; ii++){
					sat_data[jj] = collect_gps[ii];
					jj++;
					sat_data[jj] = collect_beidou[ii];
					jj++;
				}

				for(uint8_t ii = 0; ii<sizeof(*collect_rest_gps_beidou); ii++){
					jj++;
					sat_data[jj] = collect_rest_gps_beidou[ii];
				}

				good_sat = sizeof(*sat_data);*/
			}
		}else {
			//copy the satelites and compress the C/N
			for(ii = 0; ii < max_sat; ii++){
				if((!new_prn_report->sat_info[ii].pseudo_range) ||
						(AOS_GNSS_SAT_SYNC_GET_STATE(new_prn_report->sat_info[ii].sync_flags) < AOS_GNSS_SAT_SYNC_BIT)){
					continue; //!< skip it
				}

				// For now, the server supports only GPS and BEIDOU contellations
				if((new_prn_report->sat_info[ii].constellation != aos_gnss_constellation_gps) &&
				   (new_prn_report->sat_info[ii].constellation != aos_gnss_constellation_beidou) &&
				   (new_prn_report->sat_info[ii].constellation != aos_gnss_constellation_unknown)){
					continue; //!< skip it
				}


				if((new_prn_report->sat_info[ii].pseudo_range < MIN_SATELLITE_DISTANCE) ||
				   (new_prn_report->sat_info[ii].pseudo_range > MAX_SATELLITE_DISTANCE)){
					continue; //!< skip it
				}

				if(new_prn_report->sat_info[ii].cn0 >= GNSS_GOOD_C_TO_N){
					nb_sat_for_payload++;
				}

				sat_data[good_sat].cn0 			 = new_prn_report->sat_info[ii].cn0;
				sat_data[good_sat].constellation = new_prn_report->sat_info[ii].constellation;
				sat_data[good_sat].pseudo_range  = new_prn_report->sat_info[ii].pseudo_range;
				sat_data[good_sat].sv_id 		 = new_prn_report->sat_info[ii].sv_id;
				sat_data[good_sat].sync_flags    = new_prn_report->sat_info[ii].sync_flags;

				good_sat++;
			}

		}

		_gnss_sort_prn(old_prn_rpt, good_sat);	//!<	Sort by prn
		old_prn_rpt->nb_sat = good_sat;			//!<	Store result
		cli_printf("nb sat :%d acquire\n",old_prn_rpt->nb_sat);

		if(ioctl.constellation != gnss_cf_constel){
			gnss_cf_constel = ioctl.constellation;
			old_prn_rpt->nb_sat =0;
			memset(g_g_prn_report, 0, sizeof(*g_g_prn_report)); //gnss_prn_report
			//old_prn_rpt->nb_sat = 0;
			cli_printf("constellation changed\n");

		}

	}else{
		cli_printf("Sample ignored, nb sat:%d, previous:%d\n", good_sat, old_prn_rpt->nb_sat);
	}

}


// Gnss payload

static uint8_t _lora_payload[128];

static const uint8_t _lora_prefix[] = {0x03, 0x01, 0x60, 0x25, 0x05, 0x00};

void lora_gnss_send(aos_gnss_satellite_prn_report_t*  prn_rpt)
{
	//aos_gnss_ioctl_t ioctl;
	//aos_gnss_satellite_prn_report_t*  prn_rpt;
	aos_gnss_satellite_prn_t* sat;
	uint8_t ii;
	uint8_t offset;
	uint32_t min_prn;
	uint32_t prn;


	_gnss_digest_prn(g_g_prn_report,prn_rpt);

	if ((g_g_prn_report->nb_sat >=5) && (LmHandlerIsBusy() == false)) {

		memcpy(_lora_payload, _lora_prefix, sizeof(_lora_prefix));
		//sat = &prn_rpt->sat_info[0];
		sat = &g_g_prn_report->sat_info[0];
		offset = sizeof(_lora_prefix);

		for (ii=0; ii < g_g_prn_report->nb_sat; ii++, sat++) {

				if(sat->constellation == aos_gnss_constellation_gps){
					_lora_payload[offset]= _encode_type_svid_cn(sat->constellation,sat->cn0, sat->sv_id-1);
				}
				if (sat->constellation == aos_gnss_constellation_beidou){
					_lora_payload[offset]= _encode_type_svid_cn(sat->constellation,sat->cn0, _svid_beidou_mapping(sat->sv_id));
				}

				cli_printf("\nPADAWAN: Ofs:%d, const:%d, svid:%d, cn:%d, PL:0x%02x, prn :%u\n", offset, sat->constellation,  sat->sv_id, sat->cn0, _lora_payload[offset], sat->pseudo_range);
				offset ++;
				if (ii == 0) {
					min_prn = _gnss_compress_prn(sat);
				} else {
					prn = _gnss_compress_prn(sat) - min_prn;// mt_value_encode(_agps_compress_prn(sat), 0, 16777216, 24, 0);
					_lora_payload[offset ++] =  prn >> 16;
					_lora_payload[offset ++] =  prn >> 8;
					_lora_payload[offset ++] =  prn;
				}

		}
		loramac_set_datarate(4);
		srv_lmh_send(_lora_payload, offset);
		memset(g_g_prn_report, 0, sizeof(*g_g_prn_report)); //gnss_prn_report

	}

}
