/*
 * lora_handler.h
 *
 *  Created on: 21 oct. 2022
 *      Author: Jean-PierreTogbe
 */

#ifndef LORA_HANDLER_H_
#define LORA_HANDLER_H_


//void on_rx_data(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params, srv_ble_scan_param_t* ble_param);


extern bool loramac_set_datarate(uint8_t datarate);
extern void lora_ble_send(srv_ble_scan_result_t *scan_result);



#endif /* LORA_HANDLER_H_ */
