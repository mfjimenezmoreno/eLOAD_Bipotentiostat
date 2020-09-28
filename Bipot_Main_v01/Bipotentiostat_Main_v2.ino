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
 *  |      |-- A2   ~   Analog_Switch_3(MAX4737)  CE switch
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
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <avr/interrupt.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\Bipot_Main_v01\MAX5443.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\Bipot_Main_v01\ADS1255.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\Bipot_Main_v01\Experiments.h>

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

/*////////////////////////////////////////*/
/*            Global Variables            */
/*////////////////////////////////////////*/
volatile boolean WE1_ADC_SPI_rdy = false;
volatile boolean WE2_ADC_SPI_rdy = false;
//WARNING why did I wrote down option and number again?
const double DAC_frequency = 100E3; //in Hz
bool receivedDataRdy = false;
char serialData[12];


/*////////////////////////////////////////////*/
/*            Function declarations           */
/*////////////////////////////////////////////*/

max5443 DAC1(Chip_Select_DAC1);
max5443 DAC2(Chip_Select_DAC2);
ads1255 ADC1(Chip_Select_ADC1, Interrupt_ADC1);
ads1255 ADC2(Chip_Select_ADC2, Interrupt_ADC2);

cell_parameters sensor;   //Stores experimental parameters
void (*timer1_callback)(void);
void CV_SM_experiment(void);
void timer1_DAC_callback(void);

/*////////////////////////////////////////////*/
/*                    Main                    */
/*////////////////////////////////////////////*/

void interrupt_ADC1_rdy()
{
  WE1_ADC_SPI_rdy = true;
}

void interrupt_ADC2_rdy() {
  WE2_ADC_SPI_rdy = true;
}

void setup() {
  /*BLE HM-11*/
  Serial1.begin(9600);
  /*Initializes bus MOSI, SCK, SS as output and pulling SCK, MOSI low and SS high*/
  SPI.begin();
  //FIXME Might remve obbject, in favor of a simpler function
  /*Set MUX pins as outputs and initial state*/
  //Gain.pins_init();
  /*Setting up SPI related CS pins as outputs and HIGH(i.e. unselected)*/
  DAC1.pins_init();
  DAC2.pins_init();
  ADC1.pins_init();
  ADC2.pins_init();
  /*SPI chip selects a soutputs and high state*/
  pinMode(Chip_Select_SD, OUTPUT);
  digitalWrite(Chip_Select_SD, HIGH);
  /*LED Pin*/
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  /*Set analog switches & multiplexer as outputs */
  analog_switch_init(); //Initialize analog switches with switches off
  pot_init();
  
  /*Setting up ADC interrupts
  Configuration:
    1. Timer1 CTC compare: 100 kHz interrupt, changes values of DAC
    2. External Interrupt pins: ADC reports when it is ready for transmission
  */
  noInterrupts();
  //TODO remove the following comments
  //set_timer1_frequency(100E3);
  attachInterrupt(digitalPinToInterrupt(Interrupt_ADC1), &interrupt_ADC1_rdy, FALLING);
  attachInterrupt(digitalPinToInterrupt(Interrupt_ADC2), &interrupt_ADC2_rdy, FALLING);
  //timer1_EN_IntCTC();
  interrupts();
  ON_LED();
}


ISR(TIMER1_COMPA_vect) {
  /*
  Timer 1 interrupt handle triggers the moment that DAC will update
  its output voltage.
  */

  //Pointer to the appropiate function, changes according to chosen
  //technique and single or dual potentiostatic mode.
  timer1_callback();
}

void CV_SM_experiment(void)
{
  /*
        Parameters required for CV experiment 
        @cell struct
            Voltages: start, floor, ceiling.
            Initial scanning direction, Scan rate,
            Segment numbers, Gain.
            Not required: WE2 voltage.
        @t1_callback:
    */

  //DAC acquisition: Setup timer1 (std. @100 kHz)
  set_timer1_frequency(DAC_frequency);
  timer1_callback = timer1_DAC_callback;
  pot_set_gain(sensor.ga);

}

void timer1_DAC_callback(void)
{
  //Interrupt callback for CV
  if (sensor.mode == Single_Mode)
  {
    DAC1.set_voltage(0);
  }
  else if (sensor.mode == Dual_Mode)
  {
  }
}

void loop() {
  //Look for BLE commands from PC (e.g. parameters, experiments)
  static uint32_t oldtime = millis();
  /*eLOAD waits for two types of instructions:
  1. Update cell parameters.
  2. Start experiment.
  */
  /*
  if (receivedDataRdy == false)
  {
    Serial1.print(Serial1.available());
  }
  */
  while (millis() - oldtime < 1000){
    
  }
  toggle_LED();
  oldtime = millis();
  read_serial_markers(receivedDataRdy, serialData, sizeof(serialData));

  if (receivedDataRdy)
  {

    Serial1.print(serialData);
    receivedDataRdy = false;
    //Empty char variable serialData
    for (int i = 0; i < sizeof(serialData); i++)
    {
      serialData[i] = '\0'; //This is the same as '\0'
    }
  }

    //Update experiment parameters
    /*
  if (buffer[0] == "U")
  {
    //If we receive update, then proceed to change parameters
    /*Format of serialData:
      - Update,
      - XX (two letter command per cell parameter)
      - #, or ?, (a number related to the parameter, or question
        mark to ask for a parameter).
      
      Example:
      Update,V2500, --> Updates WE2 voltage to 500 mV
      Update,VS?    --> Asks for Voltage Start potential
    */
    /*


   
    update_parameters(sensor);
    }

    //Begin experiment
    else if (buffer == "B")
    {
      //If we receive start, we execute the experiment
      Serial1.print("Understood");
    }

    //Stop experiment
    else if (buffer == "S")
    {
      //Emergency stop
      //NOTE: might be changed?
      Serial1.print("Entendido");
    }
  */
  }
