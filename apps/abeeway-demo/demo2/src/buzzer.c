/*
 * app_buzzer.c
 *
 *  Created on: Mar 20, 2023
 *      Author: hamza
 *
 *  Buzzer related functions.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "aos_log.h"

#include "board_EVK_ES2.h"
#include "board_drv_buzzer.h"

#include "buzzer.h"
#include "events.h"

// silent duration in ms
#define NOTE_TRANSITION_TIME_MS 25


// Melody. Duration 0.5s
static const board_drv_buzz_sequencer_entry_t _switch_on_seq[] = {
	{board_drv_buzz_freq, A_______3}, {board_drv_buzz_wait, SEMIQUAVER},
	{board_drv_buzz_freq, A_______4}, {board_drv_buzz_wait, SEMIQUAVER},
	{board_drv_buzz_freq, A_______5}, {board_drv_buzz_wait, SEMIQUAVER},
	{board_drv_buzz_freq, A_______6}, {board_drv_buzz_wait, SEMIQUAVER},
	{board_drv_buzz_freq, 0        }, {board_drv_buzz_wait, NOTE_TRANSITION_TIME_MS},
	{board_drv_buzz_done},
};

// Callback registered against the board driver
static void _buzzer_callback (void *param, board_drv_buzzer_event_info_t *info)
{
	app_event_type_t evt = app_event_count;

	if (info->event == board_drv_buzzer_event_seq_starting) {
		evt = app_event_buzzer_on;
	} else if (info->event == board_drv_buzzer_event_seq_stopped) {
		evt = app_event_buzzer_off;
	}

	if (evt != app_event_count) {
		event_send(evt);
	}
}

// Play the melody
void buzzer_play_melody(void)
{
	board_drv_buzzer_sequencer_start(_switch_on_seq);
}

// Play beeps
// Beep, 400ms, no state
static board_drv_buzz_sequencer_entry_t _beep_seq[] = {
		{ board_drv_buzz_freq, 2600 }, { board_drv_buzz_wait, 200 },
		{ board_drv_buzz_freq,  0 }, { board_drv_buzz_wait, 1000 },
		{ board_drv_buzz_loop, 2 },
};

void buzzer_play_beep(uint8_t nb_beep)
{
	if (!nb_beep) {
		return;
	}

	_beep_seq[4].ms = nb_beep;					// Change the number of loops
	board_drv_buzzer_sequencer_start(_beep_seq);
}

// Interrupt the buzzer
void buzzer_stop(void)
{
	board_drv_buzzer_sequencer_stop();
}

// Initialization
void buzzer_init ()
{
	uint32_t max_vol = 100;		// Max volume: 100%

	board_drv_buzzer_init(BOARD_BUZZER_CONSUMPTION, _buzzer_callback, NULL);

	board_drv_buzzer_set_volume_limit(max_vol);
}

// Get the buzzer consumption
uint64_t buzzer_get_consumption(void)
{
	return board_drv_buzzer_get_consumption_uah();
}

// Clear the buzzer consumption
void buzzer_clear_consumption(void)
{
	board_drv_buzzer_clear_consumption();
}

// Set the the volume
void buzzer_set_volume(uint8_t pct)
{
	board_drv_buzzer_set_volume(pct);
}

