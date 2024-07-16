/*
 * \file accelero.h
 *
 * \brief Accelerometer management
 */
#pragma once

void accelero_init(void);
bool accelero_start(void);
void accelero_stop(void);

bool accelero_get_consumption(uint64_t* cons);
bool accelero_clear_consumption(void);

bool accelero_cli_show_info(void);
