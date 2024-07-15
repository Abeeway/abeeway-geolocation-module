/*
 * @file led.h
 *
 * LED related functions.
 */


#pragma once


/*
 * Must be called before using this LED module.
 */
void led_init();

/*
 * Function to stop LED melody from being played
 */
void led_stop();

/*
 * Function to play a LED sequence
 */
void led_play_sequence(void);

/*
 *
 * Get the LED consumption in uAh
 */
uint64_t led_get_consumption(void);

/*
 * Clear the LED consumption
 */
void led_clear_consumption(void);


