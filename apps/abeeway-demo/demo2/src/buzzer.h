/*
 * @file buzzer.h
 *
 * Buzzer related functions.
 */


#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#define TEMPO   120

#define SEMIBREVE       1000*  4* 60/TEMPO         // Ronde 2000
#define MINIM           1000*  2* 60/TEMPO         // Blanche 1000
#define CROTCHET        1000*     60/TEMPO         // Noire 500
#define QUAVER          1000*     60/TEMPO  /2     // Croche 250
#define SEMIQUAVER      1000*     60/TEMPO  /4     // Double croche 125
#define DEMISEMIQUAVER	1000*     60/TEMPO  /16    // Triple croche 31.25

#define C_______3	262
#define C_SHARP_3	277
#define D_______3	294
#define D_SHARP_3	311
#define E_______3	330
#define F_______3	349
#define F_SHARP_3	370
#define G_______3	392
#define G_SHARP_3	415
#define A_______3	440
#define A_SHARP_3	466
#define B_______3	494

#define C_______4	523
#define C_SHARP_4	554
#define D_______4	587
#define D_SHARP_4	622
#define E_______4	659
#define F_______4	698
#define F_SHARP_4	740
#define G_______4	784
#define G_SHARP_4	831
#define A_______4	880
#define A_SHARP_4	932
#define B_______4	988

#define C_______5	1046
#define C_SHARP_5	1109
#define D_______5	1175
#define D_SHARP_5	1245
#define E_______5	1319
#define F_______5	1397
#define F_SHARP_5	1480
#define G_______5	1568
#define G_SHARP_5	1661
#define A_______5	1760
#define A_SHARP_5	1865
#define B_______5	1976

#define C_______6	2093
#define C_SHARP_6	2217
#define D_______6	2349
#define D_SHARP_6	2489
#define E_______6	2637
#define F_______6	2794
#define F_SHARP_6	2960
#define G_______6	3136
#define G_SHARP_6	3322
#define A_______6	3520
#define A_SHARP_6	3729
#define B_______6	3951

#define C_______7	4186
#define C_SHARP_7	4435
#define D_______7	4697
#define D_SHARP_7	4978
#define E_______7	5274
#define F_______7	5588
#define F_SHARP_7	5920
#define G_______7	6272
#define G_SHARP_7	6645
#define A_______7	7040
#define A_SHARP_7	7459
#define B_______7	7902



/*
 * Must be called before using this buzzer module.
 */
void buzzer_init();

/*
 * Function to stop buzzer melody from being played
 */
void buzzer_stop();

/*
 * Function to play the buzzer melody
 */
void buzzer_play_melody(void);

/*
 * Function to play beeps
 */
void buzzer_play_beep(uint8_t nb_beep);

/*
 * Get the buzzer consumption in uAh
 */
uint64_t buzzer_get_consumption(void);

/*
 * Clear the buzzer consumption
 */
void buzzer_clear_consumption(void);

/*
 * Set the buzzer volume
 */
void buzzer_set_volume(uint8_t pct);

#if defined(__cplusplus)
}
#endif


