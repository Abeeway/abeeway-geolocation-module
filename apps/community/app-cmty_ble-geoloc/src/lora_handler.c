/*
 * \file lora_handler.c
 *
 * \brief This file contains
 * 			- setup a loramac datarate function;
 * 			- and a sending of a payload containing the information collected (MAC address + RSSI), function.
 *
 *  Created on: 21 oct. 2022
 *      Author: Jean-PierreTogbe
 *
 *  copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#include <stdio.h>
#include <string.h>

#include "encode_handling.h"
#include "srv_lmh.h"
#include "srv_ble_scan.h"
#include "srv_cli.h"

bool loramac_set_datarate(uint8_t datarate)
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

void lora_ble_send(srv_ble_scan_result_t *scan_result) {
	// Sending payload that contains message header, 4 BLE mac address and RSSI
	uint8_t mac_addr0[SRV_BLE_GAP_ADDR_LEN], mac_addr1[SRV_BLE_GAP_ADDR_LEN], mac_addr2[SRV_BLE_GAP_ADDR_LEN], mac_addr3[SRV_BLE_GAP_ADDR_LEN];

	baswap(mac_addr0, scan_result->entries[0].header.addr, SRV_BLE_GAP_ADDR_LEN);
	baswap(mac_addr1, scan_result->entries[1].header.addr, SRV_BLE_GAP_ADDR_LEN);
	baswap(mac_addr2, scan_result->entries[2].header.addr, SRV_BLE_GAP_ADDR_LEN);
	baswap(mac_addr3, scan_result->entries[3].header.addr, SRV_BLE_GAP_ADDR_LEN);

	uint8_t payload[] = { 0x03, 0x02, 0x60, 0x25, 0x07, 0x00,
			mac_addr0[0], mac_addr0[1], mac_addr0[2],mac_addr0[3], mac_addr0[4],mac_addr0[5], scan_result->entries[0].header.rssi,
			mac_addr1[0], mac_addr1[1], mac_addr1[2],mac_addr0[3], mac_addr1[4],mac_addr1[5], scan_result->entries[1].header.rssi,
			mac_addr2[0], mac_addr2[1], mac_addr2[2],mac_addr2[3], mac_addr2[4],mac_addr2[5], scan_result->entries[2].header.rssi,
			mac_addr3[0], mac_addr3[1], mac_addr3[2],mac_addr3[3], mac_addr3[4],mac_addr3[5], scan_result->entries[3].header.rssi};

	loramac_set_datarate(4);
	srv_lmh_send(payload, sizeof(payload));
}





