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
extern void baswap(uint8_t *dest, uint8_t *src, uint8_t len);



#endif /* ENCODE_HANDLING_H_ */
