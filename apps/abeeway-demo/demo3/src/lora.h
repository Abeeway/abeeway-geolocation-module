/*
 * \file accelero.h
 *
 * \brief Accelerometer management
 */
#pragma once

#include "srv_lora.h"

/*
 * \fn void core_payload_copy32(uint8_t* data_ptr, uint32_t d)
 *
 * \brief Copy a 32 bits in network format (Big endian)
 */
static inline void lora_copy32(uint8_t* data_ptr, uint32_t d)
{
	*(uint32_t*)data_ptr = __builtin_bswap32(d);
}

/*
 * \fn void core_payload_copy16(uint8_t* data_ptr, uint16_t d)
 *
 * \brief Copy a 16 bits in network format (Big endian)
 */
static inline void lora_copy16(uint8_t* data_ptr, uint16_t d)
{
	*(uint16_t*)data_ptr = __builtin_bswap16(d);
}

void lora_init(void);

bool lora_send(aos_lr1110_mgr_lora_dr_t dr, uint8_t port, uint8_t data_len, uint8_t*data);
