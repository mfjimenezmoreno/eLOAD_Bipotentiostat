/*///////////////////////////////////////*/
/*        MUX/Gain related class         */
/*///////////////////////////////////////*/

/*
 * Library for MUX pin initialization and gain selection. 
 */

#ifndef MAX4617_h
#define MAX4617_h
#include <Arduino.h>

class max4617
{
public:
    max4617(int, int, int);
    void pins_init(void);
    void set_gain_100M(void);
    void set_gain_30M(void);
    void set_gain_3M(void);
    void set_gain_300k(void);
    void set_gain_30k(void);
    void set_gain_3k(void);
    void set_gain_100(void);
    void set_gain_0(void);

private:
    int _pin_MUX_A;
    int _pin_MUX_B;
    int _pin_MUX_C;
};
#endif
