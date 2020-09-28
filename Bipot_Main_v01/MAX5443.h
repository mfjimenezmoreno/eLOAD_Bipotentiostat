/*///////////////////////////////////////*/
/*          DAC related class            */
/*///////////////////////////////////////*/

/*
 * Library for DAC pin initialization and voltage polarization. 
 */

#ifndef MAX5443_h
#define MAX5443_h
#include <Arduino.h>

class max5443{
  public:
    max5443(int Chip_Select);
    void pins_init(void);
    void set_voltage(uint16_t Voltage_index);
  private:
    int _pin;
    static union {
    uint8_t ui8[2];
      uint16_t ui16;
    } _DAC_buffer;
};

#endif
