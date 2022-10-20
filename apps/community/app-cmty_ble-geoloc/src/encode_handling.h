/*
 * handling.h
 *
 *  Created on: 13 oct. 2022
 *      Author: Jean-PierreTogbe
 */

#ifndef ENCODE_HANDLING_H_
#define ENCODE_HANDLING_H_

#include <stdio.h>
#include "srv_lmh.h"
#include "srv_ble_scan.h"

extern uint32_t mt_value_encode(float value, float lo, float hi, unsigned nbits,unsigned nresv);
extern void _baswap(uint8_t *dest, uint8_t *src, uint8_t len);

extern bool _loramac_set_datarate(uint8_t datarate);
extern void _lora_ble_send(srv_ble_scan_result_t *scan_result);

extern void _print_scan_result(srv_ble_scan_result_t* scan_result);

#endif /* ENCODE_HANDLING_H_ */
