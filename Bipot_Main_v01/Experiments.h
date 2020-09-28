/*/////////////////////////////////////////////////*/
/*          Experiments related library            */
/*//////////////////////////////////////////////////*/
/*
 * Code that handles characterization techniques,
 * from polarization (MAX5443 class), readout(ADS1255 class),
 * and analog switch management.
 */

#ifndef Experiments_h
#define Experiments_h
#include <Arduino.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\Bipot_Main_v01\MAX5443.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\Bipot_Main_v01\ADS1255.h>

/********************************/
/*          Definitions         */
/********************************/
#define Chronoamperometry   0
#define Voltammetry         1
#define Normal_LSV          3
#define First_LSV           4

#define Interrupt_ADC1  3
#define Interrupt_ADC2  2

#define POT_GAIN_0      0
#define POT_GAIN_100    1
#define POT_GAIN_3k     2
#define POT_GAIN_30k    3
#define POT_GAIN_300k   4
#define POT_GAIN_3M     5
#define POT_GAIN_30M    6
#define POT_GAIN_100M   7

#define Analog_Switch_VIN   A4
#define Analog_Switch_CERE  A3
#define Analog_Switch_CE    A2
#define Analog_Switch_WE1   A1 
#define Analog_Switch_WE2   A0

#define MUX_A_Gain 4
#define MUX_B_Gain 12
#define MUX_C_Gain 6

#define Chip_Select_ADC1  8
#define Chip_Select_ADC2  9
#define Chip_Select_DAC1  10
#define Chip_Select_DAC2  5
#define Chip_Select_SD    11

#define Interrupt_ADC1    3
#define Interrupt_ADC2    2

#define Single_Mode false
#define Dual_Mode   true

#define Anodic      "Anodic"
#define Cathodic    "Cathodic"

#define Switch_OFF          0
#define Switch_Single_mode  1
#define Switch_Dual_mode    2
#define Switch_OCP_mode     3

/**********************************/
/*          Declarations          */
/**********************************/
/*Public variables*/
const float Arduino_Frequency = 16E6;

struct cell_parameters
{
    int technique = Voltammetry;
    bool mode = Single_Mode;
    int16_t v2 = 200; //WE2 voltage, in milivolts
    int16_t vl = 100; //WE1 voltage, in milivolts
    int16_t vh = 300;
    int16_t vs = 200;
    int16_t sr = 100;   //WE1 scan rate in milivolts/s
    String sd = Anodic; //Starting sweep direction
    uint8_t se = 3;     //Number of segments
    uint8_t ga = POT_GAIN_0;  //Gain
};

/*Private functions, callbacks*/
void update_parameters(cell_parameters &cell);
void set_timer1_frequency(double Frequency);
void timer1_EN_IntCTC(void);
void timer1_DIS_IntCTC(void);
void pot_init(void);
void pot_set_gain(uint8_t gain);
void toggle_LED(void);
void ON_LED(void);
void OFF_LED(void);
void analog_switch_init(void);
void analog_switch(uint8_t mode);
char read_serial_char(bool &newdata);
char read_serial_characters(bool &newdata, char endmarker = '\n');
void read_serial_markers(bool &newdata, char *receivedChars, size_t size, char startmarker = '<', char endmarker = '>');

#endif
