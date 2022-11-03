/*
 * \file handling.h
 *
 * \brief Contains functions for encoding
 *
 * Created on: 13 oct. 2022
 * 	   Author: Jean-PierreTogbe
 *
 * copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#ifndef ENCODE_HANDLING_H_
#define ENCODE_HANDLING_H_

#include <stdio.h>
#include "srv_lmh.h"
#include "srv_ble_scan.h"
/*!
 *
 * \fn      uint32_t mt_value_encode(float value, float lo, float hi, unsigned nbits,unsigned nresv);
 *
 * \brief   encode in a condensed form to reduce packet airtime.
 *
 * \param 	value   - zero-based address of the word in user nvm.
 * \param	lo
 * \param	hi
 * \param	nbits
 * \param	nresv
 *
 * \retval  value offset by half the reserved value (the other half is at the end).
 */
extern uint32_t mt_value_encode(float value, float lo, float hi, unsigned nbits,unsigned nresv);

/*!
 *
 * \fn      void baswap(uint8_t *dest, uint8_t *src, uint8_t len);
 *
 * \brief   do the swapping of the content in the pointer src to pointer dest with length len.
 *
 * \param 	dest   - pointer to a uint8_t value,
 * \param	src	   - pointer to an uint8_t value,
 * \param	len
 *
 *
 */
extern void baswap(uint8_t *dest, uint8_t *src, uint8_t len);

#endif /* ENCODE_HANDLING_H_ */
