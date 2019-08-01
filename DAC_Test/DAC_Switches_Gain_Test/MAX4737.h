/*///////////////////////////////////////*/
/*        Analog Switches class          */
/*///////////////////////////////////////*/

/*
 * Library for Analog Switch selection. 
 */

#ifndef MAX4737_h
#define MAX4737_h
#include <Arduino.h>

#define Switch_OFF          0   //Disconnected Cell
#define Switch_Single_mode  1   //Only one WE
#define Switch_Dual_mode    2   //Both WE
#define Switch_OCP_mode     3

class max4737{
  public:
    max4737(int SW_Vin, int SW_CE, int SW_WE1, int SW_WE2);
    void pins_init(void);
    void Mode(int Experiment);
  private:
    int _SW_Vin, _SW_CE, _SW_WE1, _SW_WE2;
  };

#endif
