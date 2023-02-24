/*
 * lora_handler.h
 *
 *  Created on: 21 oct. 2022
 *      Author: Jean-PierreTogbe
 */

#ifndef LORA_HANDLER_H_
#define LORA_HANDLER_H_

#include "srv_ble_scan.h"
#include "srv_gnss.h"


extern bool loramac_set_datarate(uint8_t datarate);
extern void lora_ble_send(srv_ble_scan_result_t *scan_result);



#endif /* LORA_HANDLER_H_ */
