 /*
 *Main program that communicates with central unit
 *
 *Author: Martin Jimenez
 *Version: alpha
 *Date:22-May-2018
 */
 
/*//////////////////////////////*/
/*            Mapping           */
/*//////////////////////////////*/
/*
 *The bipotentiostat is based on the DSTAT design by MDM Dryden, with the capability of performing readouts with two working electrodes.
 *Applied potential is performed by DAC MAX5443. Current is measured from a transimpedance amplifier by ADC ADS1255.
 *MAX5443 and ADS1255 are controlled via SPI port. It should be noted that eLOAD's embedded SD module also uses this port.
 *Interrupts are used to determine when ADC finished the readout. DAC voltage control is timed by the ARDUINO.
 *Gain is selected from a multiplexer MAX4617 from 3 bits addresses (Gains: 0.1k, 3k, 30k, 300k, 3M, 30M, 100M ohm).
 *
 *The following map shows the pin locations and definitions used within the code:
 * 
 *   /-----|
 *  /      |
 *  |      |== GND  ~   ~~~~~~~~~
 *  |      |-- A4   ~   Analog_Switch_1(MAX4737)  -Vin switch
 *  |      |-- A3   ~   Analog_Switch_2(MAX4737)  CE-RE short 
 *  |      |-- A2   ~   Analog_Switch_3(MAX4737)  CE swtch
 *  |      |-- A1   ~   Analog_Switch_4(MAX4737)  WE1
 *  |      |-- A0   ~   Analog_Switch_5(MAX4737)  WE2
 *  |  Up  |-- D4   ~   MUX_A_Gain (MAX4617)
 *  | Port |-- D12  ~   MUX_B_Gain (MAX4617)
 *  |      |-- D6   ~   MUX_C_Gain (MAX4617)
 *  |      |-- D8   ~   Chip_Select_ADC1 (ADS1255)
 *  |      |-- D9   ~   Chip_Select_ADC2 (ADS1255)
 *  |      |-- D10  ~   Chip_Select_DAC1 (MAX5443)
 *  |      |-- D5   ~   Chip_Select_DAC2 (MAX5443)
 *  |      |== 5V   ~   ~~~~~~~~~
 *  \      |
 *   \-----|
 * 
 *   /-----|
 *  /      | 
 *  |      |== Vin  ~   ~~~~~~~~~
 *  |      |-- 3.3v ~   ~~~~~~~~~
 *  |      |-- MISO ~   MISO: ADC1/ADC2/DAC1/DAC2/SD (ADS1255, MAX5443, eLOAD)
 *  |      |-- SCK  ~   SCK:  ADC1/ADC2/DAC1/DAC2/SD (ADS1255, MAX5443, eLOAD)
 *  |      |-- MOSI ~   MOSI: ADC1/ADC2/DAC1/DAC2/SD (ADS1255, MAX5443, eLOAD)
 *  |      |-- 5V   ~   ~~~~~~~~~
 *  | Down |-- D11  ~   Chip_Select_SD (eLOAD, NOTE: DO NOT CONNECT ANYTHING TO IT)
 *  | Port |-- Reset~   ~~~~~~~~~
 *  |      |-- A5   ~   ~~~N/A~~~
 *  |      |-- D3   ~   Interrupt_ADC1 (ADS1255)(INT0)
 *  |      |-- D2   ~   Interrupt_ADC2 (ADS1255) (INT1)
 *  |      |-- D1   ~   Bluetooth (eLOAD HM-11)
 *  |      |-- D0   ~   Bluetooth (eLOAD HM-11)
 *  |      |== GND  ~   ~~~~~~~~~
 *  \      |
 *   \-----|
 */

/*////////////////////////////////*/
/*            Libraries           */
/*////////////////////////////////*/
#include <SPI.h>
#include <SD.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v1\MAX5443.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v1\ADS1255.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v1\Experiment.h>

/*//////////////////////////////////*/
/*            Definitions           */
/*//////////////////////////////////*/
/* 
 * Defines are the same as in "Mapping" section
 */
#define Analog_Switch_VIN   A4
#define Analog_Switch_CERE  A3
#define Analog_Switch_CE    A2
#define Analog_Switch_WE1   A1 
#define Analog_Switch_WE2   A0

#define MUX_A_Gain        4
#define MUX_B_Gain        12
#define MUX_C_Gain        6

#define Chip_Select_ADC1  8
#define Chip_Select_ADC2  9
#define Chip_Select_DAC1  10
#define Chip_Select_DAC2  5
#define Chip_Select_SD    11

#define Interrupt_ADC1    3
#define Interrupt_ADC2    2

#define POT_GAIN_30k  3
/*////////////////////////////////////////*/
/*            Global Variables            */
/*////////////////////////////////////////*/

/*////////////////////////////////////////////*/
/*            Function declarations           */
/*////////////////////////////////////////////*/

  max5443 DAC1(Chip_Select_DAC1);
  max5443 DAC2(Chip_Select_DAC2);
  ads1255 ADC1(Chip_Select_ADC1, Interrupt_ADC1);
  ads1255 ADC2(Chip_Select_ADC2, Interrupt_ADC2);

void setup() {
  /*Initializes bus MOSI, SCK, SS as output and pulling SCK, MOSI low and SS high*/
  SPI.begin();
  /*Setting up SPI related pins*/
  DAC1.pins_init();
  DAC2.pins_init();
  ADC1.pins_init();
  ADC2.pins_init();
  pinMode(Chip_Select_SD, OUTPUT);
  /*Initialize MUX, Analog Switches, ADC and SD chip select pins*/
  pot_init();                             //Set analog and gain mux pins as digital outputs and initialize them
  digitalWrite(Chip_Select_ADC1, HIGH);   //Reminder: in SPI, True = chip is not selected, False = chip is selected
  digitalWrite(Chip_Select_ADC2, HIGH);
  digitalWrite(Chip_Select_SD, HIGH);
  
  /*Setting up ADC interrupts*/
  
  attachInterrupt(digitalPinToInterrupt(Interrupt_ADC1), ADC1_int, FALLING);
  attachInterrupt(digitalPinToInterrupt(Interrupt_ADC2), ADC2_int, FALLING);
  
  /*Setting up Timer interrupts and 100 kHz frequency for DAC control 
   * Timer interrupt seem not to be that supported, instead use registers. Hints for registers:
   * TCNT0: Timer register, 8 bits.
   * TCCR0A: Timer/Counter control register A.
   * └──> WGM01/WGM00: For wave generation mode (Not used)
   * └──> COM0B1/COM0B0: For control of compare output pin (Not used)
   * └──> COM0A1/COM0A0: For control of compare output pin (Not used)
   * TCCR0B: Timer/Counter control register B.
   * └──> CS02/CS01/CS00: These choose preescaler.
   * └──> WGM02: For wage generation mode (Not used)
   * └──> FOC0B/FOC0A: For wave generation mode
   * OCRxy: Output compare register with TCNTx. Can generate the proper output compare interrupt or generate waveform on pin OC0y.
   * TIFRx: Timer interrupt flag register (pending timer interrupt).
   *  └──> TOV0: Timer/Counter0 OVERLFOW Flag. Cleared automatically when executing corresponding ISR (that is, assuming interrupt is enabled).
   * TIMSK0: Timer interrupt mask register.
   *  └──> TOIE0: Timer OVERFLOW interrupt ENABLE bit. If this is set, ISR(Timerx_OVF_vect) will be called.
   *  └──> OCIE0y: Timer/Counter compare match interrupt ENABLE.
   */
  noInterrupts();//ªªªªCONSIDER CHANGING TO TIMER 2ªªªªªªªªªªªªª
  TCCR0A = 0;              //Non WGM mode, normal operation of OCRA pin
  TCCR0B = 0;              //Initialization of prescaler (Timer stopped)

  TCNT0 = 159;             //Load timer to generate a 100 kHz interrupt
  TCCR0B |= (1 << CS00);   //Activate timer, with no prescaling (16 MHz)
  TIMSK0 |= (1 << TOIE0);  //Activate interrupt by Timer Overflow
  interrupts();
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

