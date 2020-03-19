/*///////////////////////////////////////*/
/*          DAC related class            */
/*///////////////////////////////////////*/

#include <Arduino.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\ADC_Test\Library_Test\ADC_Test\MAX5443.h>
#include <SPI.h>

//SPI Object, 24 MHz, MSB, Mode 0
SPISettings SPI_max5443(25000000, MSBFIRST, SPI_MODE0);

max5443::max5443(int Chip_Select){
  _pin = Chip_Select;
  SPI.begin();
}

void max5443::pins_init(){
  /*
     Purpose: DAC Chip select pin is set as output and turns it HIGH (i.e. chip is not selected)
     |----------------------|-------------------------------------------------|
     |Parameter             |Description                                      |
     |----------------------|-------------------------------------------------|
     |Chip_Select           |Pin where DAC is located                         |
     |----------------------|-------------------------------------------------|
  */
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, HIGH);
}

void max5443::set_voltage(uint16_t Voltage_index) {
  /*
     Puprpose: Sends the index value voltage to selected DAC
     |----------------------|-------------------------------------------------|
     |Parameter             |Description                                      |
     |----------------------|-------------------------------------------------|
     |Voltage_index         |DAC index value from 0 to 65535 (0 to ~3 volts)  |
     |----------------------|-------------------------------------------------|
  */

  //Impose limits to Voltage_index within 0 and 65535
  if(Voltage_index > 0xFFFF)
    Voltage_index = 0xFFFF;
  else if(Voltage_index < 0)
    Voltage_index = 0;  

  _DAC_buffer.ui16 = Voltage_index;

  //Transfer two bytes to selected DAC
  digitalWrite(_pin, LOW);
  SPI.beginTransaction(SPI_max5443);
  SPI.transfer(_DAC_buffer.ui8[1]);
  SPI.transfer(_DAC_buffer.ui8[0]);
  digitalWrite(_pin, HIGH);
  SPI.endTransaction();
}

void max5443::debug()
{
  digitalWrite(_pin, digitalRead(_pin)^1);
}