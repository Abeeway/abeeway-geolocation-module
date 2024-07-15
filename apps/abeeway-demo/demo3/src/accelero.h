/*
 * \file accelero.h
 *
 * \brief Accelerometer management
 */
#pragma once

#include "board_drv_accelero.h"

void accelero_init(void);
bool accelero_start(void);
void accelero_stop(void);

bool accelero_get_consumption(uint64_t* cons);
bool accelero_clear_consumption(void);

bool accelero_cli_show_info(void);

board_accelero_result_t accelero_read_reg(board_accelero_reg_item_t* item);
bool accelero_write_reg(board_accelero_reg_item_t* item);
