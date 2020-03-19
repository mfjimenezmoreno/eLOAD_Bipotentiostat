/*///////////////////////////////////////////////*/
/*          Experiments related class            */
/*///////////////////////////////////////////////*/
/*
 * Code that handles characterization techniques, from polarization (MAX5443 class), readout(ADS1255 class), and analog switch management.
 */

#ifndef ADS1255_h
#define ADS1255_h
#include <Arduino.h>
#include <avr/interrupt.h>
/*
 * Definitions
 */

//???
#define POT_OCP       0
#define POT_POTENT    1
//???
#define POT_LP_OFF    0
#define POT_LP_ON     1
//SPS setting
#define ADS_F_2_5     0
#define ADS_F_5       1
#define ADS_F_10      2
#define ADS_F_15      3
#define ADS_F_25      4
#define ADS_F_30      5
#define ADS_F_50      6
#define ADS_F_60      7
#define ADS_F_100     8
#define ADS_F_500     9
#define ADS_F_1000    10
#define ADS_F_2000    11
#define ADS_F_3750    12
#define ADS_F_7500    13
#define ADS_F_15000   14
#define ADS_F_30000   15

//???
#define SIN_IMP_CYCLEPTS 50
//???
#define RTC_COMPARE_INT_LEVEL RTC_COMPINTLVL_HI_gc


extern uint16_t g_gain;
extern uint8_t autogain_enable;

 /*
  * Public Functions
  */
void pot_init(void);
void pot_set_gain(uint8_t Gain);
void volt_exp_start(boolean mode);
void volt_exp_stop(void);
void precond(int16_t v1, uint16_t t1, int16_t v2, uint16_t t2);
void cv_experiment(int16_t v1, int16_t v2, int16_t start, uint8_t scans, uint16_t slope);
uint8_t lsv_experiment(int16_t start, int16_t stop, uint16_t slope, int8_t first_run);
void ocp_exp_start(void);
void pot_exp_start(void);
void pot_experiment(uint16_t time_seconds, uint8_t exp_type);
void ca_experiment(uint16_t steps, uint16_t step_dac[], uint16_t step_seconds[]);
void swv_experiment(int16_t start, int16_t stop, uint16_t step, uint16_t pulse_height, uint16_t frequency, uint16_t scans);
void dpv_experiment(int16_t start, int16_t stop, uint16_t step, uint16_t pulse_height, uint16_t pulse_period, uint16_t pulse_width);


/*
 * Interrupt declarations
 */



 #endif
