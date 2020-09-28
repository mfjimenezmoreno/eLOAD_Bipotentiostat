/*///////////////////////////////////////*/
/*        MUX/Gain related class         */
/*///////////////////////////////////////*/

#include <Arduino.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\Bipot_Main_v01\MAX4617.h>

/*This class writes the pins state according to desired Gain:
    Value   0           1           2           3           4           5           6           7
    Gain    Gain_100M   Gain_30M    Gain_3M     Gain_300k   Gain_30k    Gain_3k     Gain_100    Gain_0
    A       0           1           0           1           0           1           1           0
    B       0           0           1           1           0           0           1           1
    C       0           0           0           0           1           1           1           1
    */
max4617::max4617(int pin_MUX_A, int pin_MUX_B, int pin_MUX_C)
{
  _pin_MUX_A = pin_MUX_A;
  _pin_MUX_B = pin_MUX_B;
  _pin_MUX_C = pin_MUX_C;
}

void max4617::pins_init(){
  /*Set the MUX A, B, C as outputs and Amplifier in follower mode (a.k.a. Gain_0)*/
  pinMode(_pin_MUX_A, OUTPUT);
  pinMode(_pin_MUX_B, OUTPUT);
  pinMode(_pin_MUX_C, OUTPUT);
  digitalWrite(_pin_MUX_A, LOW);
  digitalWrite(_pin_MUX_B, LOW);
  digitalWrite(_pin_MUX_C, LOW);
}

void max4617::set_gain_100M(void){
  digitalWrite(_pin_MUX_A, LOW);
  digitalWrite(_pin_MUX_B, LOW);
  digitalWrite(_pin_MUX_C, LOW);
}

void max4617::set_gain_30M(void){
  digitalWrite(_pin_MUX_A, HIGH);
  digitalWrite(_pin_MUX_B, LOW);
  digitalWrite(_pin_MUX_C, LOW);
}

void max4617::set_gain_3M(void){
  digitalWrite(_pin_MUX_A, LOW);
  digitalWrite(_pin_MUX_B, HIGH);
  digitalWrite(_pin_MUX_C, LOW);
}

void max4617::set_gain_300k(void){
  digitalWrite(_pin_MUX_A, HIGH);
  digitalWrite(_pin_MUX_B, HIGH);
  digitalWrite(_pin_MUX_C, LOW);
}

void max4617::set_gain_30k(void){
  digitalWrite(_pin_MUX_A, LOW);
  digitalWrite(_pin_MUX_B, LOW);
  digitalWrite(_pin_MUX_C, HIGH);
}

void max4617::set_gain_3k(void){
  digitalWrite(_pin_MUX_A, HIGH);
  digitalWrite(_pin_MUX_B, LOW);
  digitalWrite(_pin_MUX_C, HIGH);
}

void max4617::set_gain_100(void){
  digitalWrite(_pin_MUX_A, HIGH);
  digitalWrite(_pin_MUX_B, HIGH);
  digitalWrite(_pin_MUX_C, HIGH);
}

void max4617::set_gain_0(void){
  digitalWrite(_pin_MUX_A, LOW);
  digitalWrite(_pin_MUX_B, HIGH);
  digitalWrite(_pin_MUX_C, HIGH);
}
