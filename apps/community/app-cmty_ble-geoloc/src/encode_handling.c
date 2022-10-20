/*
 * handling.c
 *
 *  Created on: 13 oct. 2022
 *      Author: Jean-PierreTogbe
 */

// new constant definitions
#include "encode_handling.h"
#include "srv_lmh.h"
#include "srv_ble_scan.h"
#include "srv_cli.h"
#include <stdio.h>

#define MINM( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define MAXM( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )


static float _step_size(float lo, float hi, unsigned nbits, unsigned nresv)
{
	return 1.0 / ((((1 << nbits) - 1) - nresv) / (hi - lo));
}

uint32_t mt_value_encode(float value, float lo, float hi, unsigned nbits,unsigned nresv)
{
	//Ensure value is within lo..hi bounds.
	value = MINM(value, hi);
	value = MAXM(value, lo);
	//return value offset by half the reserved value (the other half is at the end).
	return (nresv / 2) + ((value - lo) / _step_size(lo, hi, nbits, nresv));
}


void _baswap(uint8_t *dest, uint8_t *src, uint8_t len)
{
	uint8_t i;

	if (!dest || !src)
		return;

	for (i = 0; i < len; i++) {
		dest[i] = src[len - 1 - i];
	}
}


bool _loramac_set_datarate(uint8_t datarate)
{
	//Data rate 4 -> Spreading factor 8
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t rc;

    mibReq.Type = MIB_CHANNELS_DATARATE;
    mibReq.Param.ChannelsDatarate = datarate;
    rc = LoRaMacMibSetRequestConfirm( &mibReq );
    if (rc == LORAMAC_STATUS_OK) {
        return false;
    }
    return true;
}

void _lora_ble_send(srv_ble_scan_result_t *scan_result) {

	// Sending payload that contains message header, 4 BLE mac address and RSSI
	uint8_t mac_addr0[SRV_BLE_GAP_ADDR_LEN], mac_addr1[SRV_BLE_GAP_ADDR_LEN], mac_addr2[SRV_BLE_GAP_ADDR_LEN], mac_addr3[SRV_BLE_GAP_ADDR_LEN];

	_baswap(mac_addr0, scan_result->entries[0].header.addr, SRV_BLE_GAP_ADDR_LEN);
	_baswap(mac_addr1, scan_result->entries[1].header.addr, SRV_BLE_GAP_ADDR_LEN);
	_baswap(mac_addr2, scan_result->entries[2].header.addr, SRV_BLE_GAP_ADDR_LEN);
	_baswap(mac_addr3, scan_result->entries[3].header.addr, SRV_BLE_GAP_ADDR_LEN);

	uint8_t payload[] = { 0x03, 0x02, 0x60, 0x25, 0x07, 0x00,
			mac_addr0[0], mac_addr0[1], mac_addr0[2],mac_addr0[3], mac_addr0[4],mac_addr0[5], scan_result->entries[0].header.rssi,
			mac_addr1[0], mac_addr1[1], mac_addr1[2],mac_addr0[3], mac_addr1[4],mac_addr1[5], scan_result->entries[1].header.rssi,
			mac_addr2[0], mac_addr2[1], mac_addr2[2],mac_addr2[3], mac_addr2[4],mac_addr2[5], scan_result->entries[2].header.rssi,
			mac_addr3[0], mac_addr3[1], mac_addr3[2],mac_addr3[3], mac_addr3[4],mac_addr3[5], scan_result->entries[3].header.rssi};

	_loramac_set_datarate(4);
	srv_lmh_send(payload, sizeof(payload));
}

void _print_scan_result(srv_ble_scan_result_t* scan_result)
{
	// Function to scan and print the result of
	uint8_t *pdata;
	uint8_t data_len = 0;
	uint8_t mac_addr[SRV_BLE_GAP_ADDR_LEN];


	cli_printf("Scan results\n");
	if (scan_result->scan_count > 0) {

		for (int i = 0; i < scan_result->scan_count; ++i) {
			cli_printf(" ADDR: ");
			_baswap(mac_addr, scan_result->entries[i].header.addr, SRV_BLE_GAP_ADDR_LEN);

			cli_print_hex(mac_addr, SRV_BLE_GAP_ADDR_LEN, false);
			cli_printf("- TxPow: %d ", scan_result->entries[i].binfo.tx_power);
			cli_printf("- RSSI: %d - Data: ", scan_result->entries[i].header.rssi);
			switch (scan_result->entries[i].binfo.type) {
			case srv_ble_scan_beacon_type_ibeacon:
				data_len = sizeof(srv_ble_scan_ibeacon_info_t);
				pdata = scan_result->entries[i].binfo.info.ibeacon.company_uuid;
				break;
			case srv_ble_scan_beacon_type_eddy_all:
			case srv_ble_scan_beacon_type_eddy_url:
			case srv_ble_scan_beacon_type_eddy_uid:
				data_len = scan_result->entries[i].binfo.info.ebeacon.data_len;
				pdata = scan_result->entries[i].binfo.info.ebeacon.data;
				break;
			case srv_ble_scan_beacon_type_altbeacon:
				data_len = sizeof(srv_ble_scan_altbeacon_info_t);
				pdata = scan_result->entries[i].binfo.info.abeacon.manufacturer_id;
				break;
			case srv_ble_scan_beacon_type_exposure:
				data_len = sizeof(srv_ble_scan_exposure_info_t);
				pdata = scan_result->entries[i].binfo.info.exposure_beacon.rpi;
				break;
			case srv_ble_scan_beacon_type_custom:
				data_len = sizeof(srv_ble_scan_custom_info_t);
				pdata = scan_result->entries[i].binfo.info.custom.data;
				break;
			case srv_ble_scan_beacon_type_all:
			default:
				pdata = NULL;
				return;
			}

			if (pdata) {
				for (int j = 0; j < data_len; ++j) {

						cli_printf("%02x ", pdata[j]);
				}
				cli_printf("\n");
			}
		}
	} else {
		cli_printf("No beacon detected\n");
	}
}

