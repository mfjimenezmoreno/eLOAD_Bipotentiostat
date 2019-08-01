/*///////////////////////////////////////*/
/*        Analog Switches class          */
/*///////////////////////////////////////*/

#include <Arduino.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\ADC_Test\Library_Test\ADC_Test\MAX4737.h>

max4737::max4737(int SW_Vin, int SW_CE, int SW_WE1, int SW_WE2)
{
  _SW_Vin = SW_Vin;
  _SW_CE = SW_CE;
  _SW_WE1 = SW_WE1;
  _SW_WE2 = SW_WE2;
}

void max4737::pins_init(){
  /*Set the MUX A, B, C as outputs and Amplifier in follower mode (a.k.a. Gain_0)*/
  pinMode(_SW_Vin, OUTPUT);
  pinMode(_SW_CE, OUTPUT);
  pinMode(_SW_WE1, OUTPUT);
  pinMode(_SW_WE2, OUTPUT);
  digitalWrite(_SW_Vin, LOW);
  digitalWrite(_SW_CE, LOW);
  digitalWrite(_SW_WE1, LOW);
  digitalWrite(_SW_WE2, LOW);
}

void max4737::Mode(int Experiment)
{
  /*This function controls the Analog switches depending on the desired cell state:
                            _SW_Vin  | _SW_CE | _SW_WE1  | _SW_WE2
    Switch_OFF              0       | 0     | 0       | 0
    Switch_Single_mode      1       | 1     | 1       | 0
    Switch_Dual_mode        1       | 1     | 1       | 1
    Switch_OCP_mode         0       | 0     | 1       | 0
  */
  switch (Experiment)
  {
    case Switch_OFF:
      digitalWrite(_SW_Vin, LOW);
      digitalWrite(_SW_CE, LOW);
      digitalWrite(_SW_WE1, LOW);
      digitalWrite(_SW_WE2, LOW);
      break;

    case Switch_Single_mode:
      digitalWrite(_SW_Vin, HIGH);
      digitalWrite(_SW_CE, HIGH);
      digitalWrite(_SW_WE1, HIGH);
      digitalWrite(_SW_WE2, LOW);
      break;

    case Switch_Dual_mode:
      digitalWrite(_SW_Vin, HIGH);
      digitalWrite(_SW_CE, HIGH);
      digitalWrite(_SW_WE1, HIGH);
      digitalWrite(_SW_WE2, HIGH);
      break;

    case Switch_OCP_mode:
      digitalWrite(_SW_Vin, LOW);
      digitalWrite(_SW_CE, LOW);
      digitalWrite(_SW_WE1, HIGH);
      digitalWrite(_SW_WE2, LOW);
      break;
  }
}
