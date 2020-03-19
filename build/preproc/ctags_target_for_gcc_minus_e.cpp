# 1 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2.ino"
# 1 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2.ino"
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
# 64 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2.ino" 2
# 65 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2.ino" 2
# 66 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2.ino" 2
# 67 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2.ino" 2
# 68 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2.ino" 2

/*//////////////////////////////////*/
/*            Definitions           */
/*//////////////////////////////////*/
/* 
 * Defines are the same as in "Mapping" section
 */
# 95 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2.ino"
/*////////////////////////////////////////*/
/*            Global Variables            */
/*////////////////////////////////////////*/

/*////////////////////////////////////////////*/
/*            Function declarations           */
/*////////////////////////////////////////////*/

  max5443 DAC1(10);
  max5443 DAC2(5);
  ads1255 ADC1(8, 3);
  ads1255 ADC2(9, 2);

void setup() {
  /*Initializes bus MOSI, SCK, SS as output and pulling SCK, MOSI low and SS high*/
  SPI.begin();

  /*Setting up SPI related pins*/
  DAC1.pins_init();
  DAC2.pins_init();
  ADC1.pins_init();
  ADC2.pins_init();
  pinMode(11, 0x1);
  pot_init(); //Initialize multiplexer and analog switch related pins
  digitalWrite(8, 0x1);
  digitalWrite(9, 0x1);
  digitalWrite(11, 0x1);

  /*Setting up ADC interrupts*/

  attachInterrupt(((3) == 2 ? 0 : ((3) == 3 ? 1 : -1)), ADC1_int, 2);
  attachInterrupt(((2) == 2 ? 0 : ((2) == 3 ? 1 : -1)), ADC2_int, 2);

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
  __asm__ __volatile__ ("cli" ::: "memory");//ªªªªCONSIDER CHANGING TO TIMER 2ªªªªªªªªªªªªª
  (*(volatile uint8_t *)((0x24) + 0x20)) = 0; //Non WGM mode, normal operation of OCRA pin
  (*(volatile uint8_t *)((0x25) + 0x20)) = 0; //Initialization of prescaler (Timer stopped)

  (*(volatile uint8_t *)((0x26) + 0x20)) = 159; //Load timer to generate a 100 kHz interrupt
  (*(volatile uint8_t *)((0x25) + 0x20)) |= (1 << 0); //Activate timer, with no prescaling (16 MHz)
  (*(volatile uint8_t *)(0x6E)) |= (1 << 0); //Activate interrupt by Timer Overflow
  __asm__ __volatile__ ("sei" ::: "memory");

}

void loop() {
  // put your main code here, to run repeatedly:

}
# 1 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2_Experiment.ino"
/*///////////////////////////////////////////////*/
/*          Experiments related class            */
/*///////////////////////////////////////////////*/
/*
 * Code that handles characterization techniques, from polarization (MAX5443 class), readout(ADS1255 class), and analog switch management.
 */

/********************************/
/*          Definitions         */
/********************************/
# 32 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2_Experiment.ino"
/******************************/
/*          Includes          */
/******************************/
# 36 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2_Experiment.ino" 2
# 37 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2_Experiment.ino" 2
# 38 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2_Experiment.ino" 2
# 39 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2_Experiment.ino" 2
# 40 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2_Experiment.ino" 2

/**********************************/
/*          Declarations          */
/**********************************/
/*Public variables*/
uint16_t g_gain = 3;
uint8_t autogain_enable = 1;

/*Private variables*/
volatile int32_t voltage = 0; //Volatile declaration avoids optimizator to ignore these
volatile uint16_t dacindex = 0;
uint16_t dacindex_start = 0;
uint16_t dacindex_stop = 0;
volatile int8_t up = 1;

/*Private functions, callbacks*/
uint16_t set_timer0_period(uint32_t period, volatile int8_t *tc);
uint16_t set_timer1_period(uint32_t period, volatile int16_t *tc);
static void precond_rtc_callback(uint32_t time);
static void porte_int0_lsv(void);
static void tcf0_ovf_callback(void);
static void tce1_ovf_callback_lsv(void);
static void lsv_cca_callback(void);
static void lsv_dma_callback(void);
static void lsv_dma_callback1(void);
static void ca_cca_callback(void);
static void portd_int0_ca(void);
static uint8_t _swv_singledir(uint16_t dacindex, uint16_t dacindex_stop, uint16_t dacindex_pulse_height, uint16_t dacindex_step, uint8_t direction);
static uint8_t _dpv_singledir(uint16_t dacindex, uint16_t dacindex_stop, uint16_t dacindex_pulse_height, uint16_t dacindex_step, uint8_t direction);

/*Setting up ADC interrupts*/
/*
attachInterrupt(digitalPinToInterrupt(Interrupt_ADC1), ADC1_int, FALLING);
attachInterrupt(digitalPinToInterrupt(Interrupt_ADC2), ADC2_int, FALLING);
*/
/*************************/
/*         Code          */
/*************************/
uint16_t set_timer0_period(uint32_t period, volatile int8_t *tc) {
  /*
   * Purpose: Sets a suitible timer source and sets period for 8-bit timer (ideal for timer0, which I plan to use it for ADC coonversion)
   * @param period  |   32-bit period in CPU cycles
   * @param *tc     |   Pointer to timer to set, for Compare Style of Interrupt
   * @return        |   Divider
   * 
   * Reminder: In contrast to Mr. Dryden's version -which is based on a ATXMEGA256A3U- in ATMEGA16U4/32U4 the preescaler is controlled via
   * the TCCR0B register instead of CTRL, and it doesn't have a preescaler that divides by two or four.
   * 
   * TCCR0B (0x25): Timer/Counter 0 register B
   * └──> CS02/CS01/CS00: These choose preescaler.
   *    000=No clock source   001=No prescaling   010=clkio/8
   *    011=clkio/64          100=clkio/256       101=clkio/1024
   *    110=Ext.Fall.t0 pin   111=Ext.Ris.t0 pin
   * └──> WGM02: For wage generation mode (Not used, mask it out)
   * └──> FOC0B/FOC0A: For wave generation mode
   */

  uint16_t temp_div = (uint16_t)(ceil((double)period/256)); //temp_div values will rank between 1 and 1024 (Higher than 1024 means that its not feasible)
  uint16_t divider = 0; //The divider that the function will return

  if (temp_div == 1) {
    (*(volatile uint8_t *)((0x25) + 0x20)) = ((*(volatile uint8_t *)((0x25) + 0x20)) & 0xF8) | (0x01); //No Preescaler
    divider = 1;
  }
  else if (temp_div <= 8){
    (*(volatile uint8_t *)((0x25) + 0x20)) = ((*(volatile uint8_t *)((0x25) + 0x20)) & 0xF8) | (0x02); //Preescaler division by 8
    divider = 8;
  }
  else if (temp_div <= 64){
    (*(volatile uint8_t *)((0x25) + 0x20)) = ((*(volatile uint8_t *)((0x25) + 0x20)) & 0xF8) | (0x03); //Preescaler division by 64
    divider = 64;
  }
  else if (temp_div <= 256){
    (*(volatile uint8_t *)((0x25) + 0x20)) = ((*(volatile uint8_t *)((0x25) + 0x20)) & 0xF8) | (0x04); //Preescaler division by 256
    divider = 256;
  }
  else if (temp_div <= 1024){
    (*(volatile uint8_t *)((0x25) + 0x20)) = ((*(volatile uint8_t *)((0x25) + 0x20)) & 0xF8) | (0x05); //Preescaler division by 1024
    divider = 1024;
  }
  else{
    (*(volatile uint8_t *)((0x25) + 0x20)) = ((*(volatile uint8_t *)((0x25) + 0x20)) & 0xF8) | (0x00); //Frequency is too low, pretty much no clock source.
    divider = __null;
  }

  period /= divider; //Warning: not sure what happens, but it's a division between two integers of different lengths
  *tc = (uint8_t)period; //Just make sure that it is properly turned into a compatible integer
  return divider;
}

uint16_t set_timer1_period(uint32_t period) {
  /*
   * Purpose: Sets a suitible timer source and sets period for 16-bit timer (ideal for timer1, which I plan to use it for ADC coonversion)
   * @param period  |   32-bit period in CPU cycles
   * @return        |   Divider
   * 
   * Reminder: In contrast to Mr. Dryden's version -which is based on a ATXMEGA256A3U- in ATMEGA16U4/32U4 the preescaler is controlled via
   * the TCCR1B register instead of CTRL, and it doesn't have a preescaler that divides by two or four.
   * 
   * TCCR1B (0x81): Timer/Counter 1 register B
   * └──> CS02/CS01/CS00: These choose preescaler.
   *    000=No clock source   001=No prescaling   010=clkio/8
   *    011=clkio/64          100=clkio/256       101=clkio/1024
   *    110=Ext.Fall.t0 pin   111=Ext.Ris.t0 pin
   * └──> WGM13/WGM12: For wave generation mode (Not used, mask it out)
   * └──> ICNC1/ICES1: For input capture noise canceler (Not used, mask it out)
   */
  union {
    uint16_t int16;
    uint8_t int8[2];
    }Timer1_period; //Variable for TCNT1X counter register determination
  uint16_t temp_div = (uint16_t)(ceil((double)period/65536)); //temp_div values will rank between 1 and 1024 (Higher than 1024 means that its not feasible)
  uint16_t divider = 0; //The divider that the function will return

  if (temp_div == 1) {
    (*(volatile uint8_t *)(0x81)) = ((*(volatile uint8_t *)(0x81)) & 0xF8) | (0x01); //No Preescaler
    divider = 1;
  }
  else if (temp_div <= 8){
    (*(volatile uint8_t *)(0x81)) = ((*(volatile uint8_t *)(0x81)) & 0xF8) | (0x02); //Preescaler division by 8
    divider = 8;
  }
  else if (temp_div <= 64){
    (*(volatile uint8_t *)(0x81)) = ((*(volatile uint8_t *)(0x81)) & 0xF8) | (0x03); //Preescaler division by 64
    divider = 64;
  }
  else if (temp_div <= 256){
    (*(volatile uint8_t *)(0x81)) = ((*(volatile uint8_t *)(0x81)) & 0xF8) | (0x04); //Preescaler division by 256
    divider = 256;
  }
  else if (temp_div <= 1024){
    (*(volatile uint8_t *)(0x81)) = ((*(volatile uint8_t *)(0x81)) & 0xF8) | (0x05); //Preescaler division by 1024
    divider = 1024;
  }
  else{
    (*(volatile uint8_t *)(0x81)) = ((*(volatile uint8_t *)(0x81)) & 0xF8) | (0x00); //Frequency is too low, pretty much no clock source.
    divider = __null;
  }

  Timer1_period.int16 = (uint16_t)(period / divider); //Warning: not sure what happens, but it's a division between two integers of different lengths
  (*(volatile uint8_t *)(0x85)) = Timer1_period.int8[1]; //Pass the period value into the two registers related to timer
  (*(volatile uint8_t *)(0x84)) = Timer1_period.int8[0];
  return divider;
}

void pot_init(void) {
  /**
   * Initializes outputs of potentiostat pins that are related to Gain Selector and analog switches.
   * 
   * Name               IC          Pin   Purpose         Default
   * 
   * Analog_Switch_VIN  (MAX4737)   A4    -Vin switch     LOW     * Analog_Switch_CERE (MAX4737)   A3    CE-RE short     HIGH  |
|
   * Analog_Switch_CE   (MAX4737)   A2    CE switch       LOW   |--Potentiostat circuit is not in closed loop operation
   * Analog_Switch_WE1  (MAX4737)   A1    WE1 switch      LOW   |
   * Analog_Switch_WE2  (MAX4737)   A0    WE2 switch      LOW  /
   * MUX_A_Gain         (MAX4617)   4     A               HIGH    * MUX_B_Gain         (MAX4617)   12    B               HIGH  |--This combination sets up transimpedance gain as 100 ohms
s
   * MUX_C_Gain         (MAX4617)   6     C               HIGH /
   * 
   * @return Nothing.
   */
  /*MUX gain selector*/
  pinMode(4, 0x1);
  pinMode(12, 0x1);
  pinMode(6, 0x1);
  digitalWrite(4, 0x1);
  digitalWrite(12, 0x1);
  digitalWrite(6, 0x1);
  /*Analog switches as digital outputs...*/
  pinMode(A4, 0x1);
  pinMode(A3, 0x1);
  pinMode(A2, 0x1);
  pinMode(A1, 0x1);
  pinMode(A0, 0x1);
  /*... and set their initial state*/
  digitalWrite(A4, 0x0);
  digitalWrite(A3, 0x1);
  digitalWrite(A2, 0x0);
  digitalWrite(A1, 0x0);
  digitalWrite(A0, 0x0);
}

void pot_set_gain(uint8_t gain) {
  /*
   * Sets the multiplexer values according to the specified gain value.
   * 
   * @param   Gain      Definitions are defined on the header file
   * @return  Nothing
   */

   switch(gain) {
    case 7:
      digitalWrite(4, 0x0);
      digitalWrite(12, 0x0);
      digitalWrite(6, 0x0);
      break;

    case 6:
      digitalWrite(4, 0x1);
      digitalWrite(12, 0x0);
      digitalWrite(6, 0x0);
      break;

    case 5:
      digitalWrite(4, 0x0);
      digitalWrite(12, 0x1);
      digitalWrite(6, 0x0);
      break;

    case 4:
      digitalWrite(4, 0x1);
      digitalWrite(12, 0x1);
      digitalWrite(6, 0x0);
      break;

    case 3:
      digitalWrite(4, 0x0);
      digitalWrite(12, 0x0);
      digitalWrite(6, 0x1);
      break;

    case 2:
      digitalWrite(4, 0x1);
      digitalWrite(12, 0x0);
      digitalWrite(6, 0x1);
      break;

    case 0:
      digitalWrite(4, 0x0);
      digitalWrite(12, 0x1);
      digitalWrite(6, 0x1);
      break;

    case 1:
      digitalWrite(4, 0x1);
      digitalWrite(12, 0x1);
      digitalWrite(6, 0x1);
      break;
   }
}

void volt_exp_start(boolean mode) {
  /*
   * Connects the measurement cell in votlammetric mode to rest of circuit as Potentiostat or Bipotentiostat.
   * @param   mode      Single_Mode/FALSE Potentiostat mode
   *                    Dual_Mode/TRUE    Bipotentiostat mode
   * @return  Nothing
   */
  digitalWrite(A4, 0x1);
  digitalWrite(A3, 0x0);
  digitalWrite(A2, 0x1);
  digitalWrite(A1, 0x1);
  if(mode == false) digitalWrite(A0, 0x0); //Potentiostat configuration
  if(mode == true) digitalWrite(A0, 0x1); //Bipotentiostat configuration
}

void volt_exp_stop(void){
  /*
   * Disconnects the measurement cell from voltammetric measurement.
   * @return    Nothing
   */
  digitalWrite(A4, 0x0);
  digitalWrite(A3, 0x0); //#Warning: It is like this in Mr. Dryden's code, I expected this to be high as in pot_init(), but maybe not critical
  digitalWrite(A2, 0x0);
  digitalWrite(A1, 0x0);
  digitalWrite(A0, 0x0);
}

void pot_exp_start(void){
  /*
   * All switches are open.
   * @return    Nothing
   */
  digitalWrite(A4, 0x0);
  digitalWrite(A3, 0x0);
  digitalWrite(A2, 0x0);
  digitalWrite(A1, 0x0);
  digitalWrite(A0, 0x0);
}

void ocp_exp_start(void){
  /*
   * All switches but WE1 are open.
   * @return    Nothing
   */
  digitalWrite(A4, 0x0);
  digitalWrite(A3, 0x0);
  digitalWrite(A2, 0x0);
  digitalWrite(A1, 0x1);
  digitalWrite(A0, 0x0);
}

void precond(int16_t v1, uint16_t t1, int16_t v2, uint16_t t2) {
  /*
   * Performs experiment preconditioning. Ideal for stripping voltammetry.
   *
   * @param v1  First potential (DAC index).
   * @param t1  First duration (s).
   * @param v2  Second potential (DAC index).
   * @param t2  Second duration (s).
   * @return    Nothing
   */
  uint16_t time_old = 0;

  while (RTC.STATUS & RTC_SYNCBUSY_bm);
  RTC.PER = 65535;
  while (RTC.STATUS & RTC_SYNCBUSY_bm);
  RTC.CTRL = RTC_PRESCALER_DIV1024_gc; //1s tick
  rtc_set_callback((rtc_callback_t)precond_rtc_callback);

  up = 1;

  //first potential
  if (t1 > 0){
    max5443_set_voltage1(v1);
    rtc_set_alarm(t1);
    RTC.CNT = 0;
    volt_exp_start();
    while (up){
      if (udi_cdc_is_rx_ready()){
        if (fgetc((__iob[0])) == 'a'){
          precond_rtc_callback(t1);
          printf("##ABORT\n\r");
          goto aborting;
        }
      }
      if (time_old != RTC.CNT){
        time_old = RTC.CNT;
        printf("#%u\n\r",time_old);
      }
    }
  }

  up = 1;
  time_old = 0;

  if (t2 > 0){
    max5443_set_voltage1(v2);
    rtc_set_alarm(t2);
    RTC.CNT = 0;
    volt_exp_start();
    while (up){
      if (udi_cdc_is_rx_ready()){
        if (fgetc((__iob[0])) == 'a'){
          precond_rtc_callback(t2);
          printf("##ABORT\n\r");
          goto aborting;
        }
      }
      if (time_old != RTC.CNT){
        time_old = RTC.CNT;
        printf("#%u\n\r",time_old);
      }
    }
  }

  aborting:
    volt_exp_stop();
    return;
}

uint8_t lsv_experiment(int16_t start, int16_t stop, uint16_t slope, int8_t first_run){
 /**
	 * Perform a LSV experiment.
	 *
	 * @param   start     Start potential in mV. {-1500,1499.9}
	 * @param   stop      Stop potential in mV. {-1500,1499.9}
	 * @param   slope     Scan rate in mV/s.
	 * @param   first_run Keeps track if number if epxeriment is just starting.
   *  2 = for normal LSV.
   *  1 = First run.
	 * @return  ret       Returns an error, probable by abort.
   *  ERROR or 0= error  
   *  NO_ERROR or 1 = no error
	 * 
	 * Peripherals
	 * 	TIMER 	0: Usado como cronómetro --> Para modificar el DAC
	 * 	COUNTER	1: Usado como contador de voltaje --> Para determinar el índice de "polarización" del DAC, dado que DAC y ADC funcionan a frecuencias diferentes, esta estrategia permite trackear el "tiempo"/"instancia de voltaje"
	 * 
   * ///////////////////////////////////////////////////////////////////////////////////////////////////////*
   * //   Important   Important   Important   Important   Important   Important   Important   Important   //*
   * ///////////////////////////////////////////////////////////////////////////////////////////////////////*
	 * Interrupts                                                                                             *
   *                                                                                                        *
	 * 	TCC0 OVF:	Small priority 	--> TCC0 is a periodic counter to change DAC value according to what is     *
   *                                specified in Counter1 (TCC1)                                            *
	 * 	TCC1 OVF:	                --> Deactivates the other three interrupts, up = 0                          *
	 * 	TCC1 CCA:	High priority 	-->	Deactivates the other three interrupts, up = 0                          *
	 * 	INT0: 		Medium priority --> ADS1255 DRDY, get ADC data, get DAC data (TCC1), estimate a mean value  *
   *                                of DAC from current to last value, send data                            *
   * ///////////////////////////////////////////////////////////////////////////////////////////////////////*
	 */

 uint8_t ret = 0; //Return variable, by default error.
  uint32_t timer_period; //Timer period
 uint16_t temp_div; //Variable used for preescaler determination

 /*Make sure the parameters are within the potentiostat limits.*/
 if(start<-1500 || start>=1500 ||start==stop|| stop<-1500 || stop>=1500 || slope>7000)
 {
  printf("#ERR: Experiment parameters outside limits\n\r");
  return ret;
 }
 /*Translate voltage to 16 bit DAC data: {0x0000, 0xFFFF} is {-1500, 1499.90} mV, 1 bit  = 50 uV*/
 dacindex_start = ceil(start*(65536/(double)3000)+32768);
 dacindex_stop = ceil(stop*(65536/(double)3000)+32768);

  /*Set initial voltage*/
  DAC1.set_voltage(dacindex_start);

 /*If LSV is executed for the first time or is in LSV mode, then the next code block will prepare the cell polarization and timers*/
 if (first_run == 1 || first_run == 2){
    /*Setup the ADC, Timer0, Counter1, Interrupt callbacks{DRDY int, Counter1 OVF CCA, Timer0 OVF}*/
  volt_exp_start(false); //Connect cell: one WE
  ADC1.rdatac(); //Continous acquisition mode
  tc_enable(&TCC1); //Enable counter1 (Para almacenar voltaje DAC)
  ADC1.sync(); //ADC sync
  tc_enable(&TCC0); //Enable timer0
  tc_set_overflow_interrupt_callback(&TCC0, tcf0_ovf_callback); //Assign the proper callback for TCC0 OVF for LSV
  tc_set_overflow_interrupt_callback(&TCC1, tce1_ovf_callback_lsv); //Assign the proper callback for TCC1 OVF for LSV
  tc_set_cca_interrupt_callback(&TCC1, lsv_cca_callback); //Assign the proper callback for TCC1 CCA for LSV
  portd_int0_callback = porte_int0_lsv; //Callback assignement, will be invoked at ISR subroutine: reads ADC on demand, estimates a DAC value, sends serial data

    //set EVCH0 event "TCC0 se convierte el trigger para el cambio en el DAC, asociando CH0 con TCC0 OVF, los siguientes comentarios se pueden borrar después"
  /*EVENT ROUTING NETWORK
		8 canales idénticos, cada canal con un multiplexor y lógica de control y filtro(este es para los pines, cuantos ciclos de reloj para considerar evento)
		*/
  //EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;	//Configura el mux0 como Timer/counter 0 overflow (event system, an interesting approach free of interrupt)
  //EVSYS.CH0CTRL = 0;						//Acepta el cambio desde el primer muestreo


  //Configuracion del Internal clock source para TCC0 en función del slope y FCLK_IO
  //timer_period = ceil(1/((double)slope/(3000./65536))*(F_CPU)); //Timer period, depends on slope and CPU Frequency.
    timer_period = ceil((16000000L)*3000./((double)slope*65536.)); //Adimensionalization of Sweep Voltage (Slope) and multiplication to CPU frequency (without preescaler).
  //Definición del preescaler: ¿Cuantas veces los "ticks" caben en 2^16?
  temp_div = ceil(timer_period/65536.); //This calculation sets the prescaler, so everytime it fills the timer, it triggers interrupt

  /*Escribe el preescaler para timer0, y luego redeterminar el timer_period en función de preescaler*/
  if (temp_div <= 1)
   tc_write_clock_source(&TCC0,TC_CLKSEL_DIV1_gc);
  else if (temp_div == 2){
   tc_write_clock_source(&TCC0,TC_CLKSEL_DIV2_gc);
   timer_period /= 2;
  }
  else if (temp_div <= 4){
   tc_write_clock_source(&TCC0,TC_CLKSEL_DIV4_gc);
   timer_period /= 4;
  }
  else if (temp_div <= 8){
   tc_write_clock_source(&TCC0,TC_CLKSEL_DIV8_gc);
   timer_period /= 8;
  }
  else if (temp_div <= 64){
   tc_write_clock_source(&TCC0,TC_CLKSEL_DIV64_gc);
   timer_period /= 64;
  }
  else if (temp_div <= 256){
   tc_write_clock_source(&TCC0,TC_CLKSEL_DIV256_gc);
   timer_period /= 256;
  }
  else if (temp_div <= 1024){
   tc_write_clock_source(&TCC0,TC_CLKSEL_DIV1024_gc);
   timer_period /= 1024;
  }
  else{
   printf("ERR: Frequency/ADC rate is too low\n\r");
   return ret;
  }

  //printf("Period:%lu\n\r", timer_period);
  ads1255_wakeup(); //Despertar ADC
  tc_write_period(&TCC1, 0xffff); //CONSECUENCIA:TCC1.Per a full y provocar la interrupción tce1_ovf_callback_lsv, el cual desactivará las 3 interrupciones y asignará up=0)
  tc_write_period(&TCC0, (uint16_t)timer_period); //Modificar timer 0 de acuerdo a cálculos

 TCC1.CNT = dacindex_start; //TCC1 arranca con el votlaje inicial

 if (stop > start) //Si el barrido es anódico
 {
  up = 1; //Creo que up aquí se usa como un comodín, en voltametría representa un barrido anódico si es positivo, catódio si negativo
  tc_set_direction(&TCC1, TC_UP); //Sets counting direction up
 }
 else
 {
  up = -1; //Direction down
  tc_set_direction(&TCC1, TC_DOWN); //Sets counting direction
 }

 tc_write_cc(&TCC1, TC_CCA, dacindex_stop); //Escribe el valor de capture compare, donde frena el conteo
 tc_enable_cc_channels(&TCC1, TC_CCAEN); //Habilita capture/compare
 TCC0.CNT = 0; //Inicializar Timer0

 /*Asignar las prioridades a las interrupciones de con funciones y "manual"*/
 tc_set_cca_interrupt_level(&TCC1, TC_INT_LVL_HI); //Assign high priority to counter 1
 tc_set_overflow_interrupt_level(&TCC0, TC_OVFINTLVL_LO_gc); //Assign low priority to DACtimer0
 (*(volatile uint8_t *)((0x0B) + 0x20)).INTCTRL = PORT_INT0LVL_MED_gc; //Assign ADC interrupt with medium priority

 tc_write_clock_source(&TCC1, TC_CLKSEL_EVCH0_gc); //Asigna Canal 0 como fuente de reloj para TCC1, el cual incrementa cada vez que TCC0 se desborda!

 //Experiment run with interrupts LEER EL ISR
 /*STALLING LOOP: Break if aborting or if up changed with an interrupt*/
 while (up != 0){
  if (udi_cdc_is_rx_ready()){
   if (fgetc((__iob[0]))=='a'){
    tce1_ovf_callback_lsv();
    ret = 1; //Retorna 1 si aborta operación
    goto aborting;

   }
  }
 }
 /*Si first run es -1 o trabaja en modo LSV, deactivar todo*/
 if (first_run == -1 || first_run == 2) //first run es para evitar la desconexión de la celda si LSV se usa varias veces consecutivas. 2 representa una lectura normal de LSV.
 { //Fijarse, que si el valor es 1, no se detiene timer0, contador1, celda, ads1255
  aborting:
   tc_disable(&TCC0); //Deshabilita timer0
   TCC0.CNT = 0x0; //Reinicializa timer0
   tc_disable(&TCC1); //Deshabilitar counter1
   volt_exp_stop(); //Desconecta la celda
   ads1255_standby(); //Pone el ADC en standby
   return ret; //Parece ser que regresa un cero de inicialización
 }

 return ret; //Parece que regresa 0, en operación normal
}

void ADC1_int() {
  // put your main code here, to run repeatedly:

}

void ADC2_int() {

}

/**************************************/
/*          Callbacks Section         */
/**************************************/

////////////////////////////////////////
//Linear Voltammetry related callbacks//
////////////////////////////////////////

static void porte_int0_lsv(void){
 /*
   * This callback is triggered by ADC DRDY pin (when a readout is ready) for Voltammetric readouts.
   * 
   * This does two things:
   *  1. Read ADC data (Current data from T-I Amplifier), save it to data.result.
   *  2. Estimate the applied  voltage from DAC (Remember, DAC is connected to a reconstruction filter) by averaging the TCC1 last two
   *     values.
   *  3. Store the value into the SD card. (TO BE DONE)
	 */
 struct
 {
  uint16_t index; //Stores the DAC values, final purpose is to send an average estimate from the counter
  int32_t result; //Stores the ADC readout
 } data;

  data.result = ADC1.read_fast24(); //Read fast on demand
 static uint16_t last_value = 0; //Static variable: will continously store the last value from counter
 /*WARNING: BE CAREFUL WITH DATA SIZES OF CURRENT AND LAST VALUE*/
  uint32_t current = TCC1.CNT; //Current DAC value...
 data.index = (current+last_value)>>1; //...and average them, to obtain an estimate
  last_value = (uint16_t)current; //Store the current value as last value for the next DRDY interrupt

  /*WARNING: TO BE SUBSTITUTED WITH SD*/
 printf("B\n");
 udi_cdc_write_buf(&data, 6); //TRANSFERENCIA: el dato por USB, 2 bytes de index, 4 bytes de result
 printf("\n");

 return;
}

static void tcf0_ovf_callback(void){
  /*
   * This callback deals with timer0 overflow. If that happens, it is time to change the DAC voltage.
   */
  DAC1.set_voltage(TCC1.CNT); //Change the voltage according to the voltage counter1.
  return;
}

static void tce1_ovf_callback_lsv(void){
 /*
   * This callback deactivates interruptions from: int0/DRDY, Timer0 ovf, Timer1 ovf.
   * Changes the "up" variable to 0 --> WARNING: DEFINIR QUE HACE
   * Note: Timer1 Capture/Compare interrupt still standing up
   */
  (*(volatile uint8_t *)((0x0B) + 0x20)).INTCTRL = PORT_INT0LVL_OFF_gc; //Interrupt INT0 desactivado
 tc_set_overflow_interrupt_level(&TCC0, TC_OVFINTLVL_OFF_gc); //Interrupt Timer0 overflow desactivado
 tc_set_overflow_interrupt_level(&TCC1, TC_OVFINTLVL_OFF_gc); //Interrupt Counter1 por overflow desactivado
 up = 0; //La dirección de barrido es cero (creo que esto detiene la voltametría)
 return;
}

static void lsv_cca_callback(void){
  /*
   * This callback deactivates interruptions from: int0/DRDY, Timer0 ovf, Timer1 cca.
   * Changes the "up" variable to 0 --> WARNING: DEFINIR QUE HACE
   * Note: Timer1 OVF interrupt still standing up
   */
 (*(volatile uint8_t *)((0x0B) + 0x20)).INTCTRL = PORT_INT0LVL_OFF_gc; //Interrupt ADC INT0 desactivado
 tc_set_overflow_interrupt_level(&TCC0, TC_OVFINTLVL_OFF_gc); //Interrupt timer0 overflow desactivado
 tc_set_cca_interrupt_level(&TCC1, TC_INT_LVL_OFF); //Interrupt counter capture/compare desactivado
 up = 0; //Dirección de barrido cero
 return;
}
# 724 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipotentiostat_Main_v2_Experiment.ino"
/*Callbacks for Voltammetric methods*/

/*Callbacks for Chronoamperommetric methods*/

/*******************************/
/*          Interrupts         */
/*******************************/
//Timer interrupt
extern "C" void __vector_11 /* Timer/Counter1 Compare Match A */ (void) __attribute__ ((signal,used, externally_visible)) ; void __vector_11 /* Timer/Counter1 Compare Match A */ (void) {

}

extern "C" void __vector_14 /* TimerCounter0 Compare Match A */ (void) __attribute__ ((signal,used, externally_visible)) ; void __vector_14 /* TimerCounter0 Compare Match A */ (void) {

}
# 1 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Test_1_Constant_Voltage.ino"
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
# 64 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Test_1_Constant_Voltage.ino" 2
# 65 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Test_1_Constant_Voltage.ino" 2
# 66 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Test_1_Constant_Voltage.ino" 2
# 67 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Test_1_Constant_Voltage.ino" 2
# 68 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Test_1_Constant_Voltage.ino" 2

/*//////////////////////////////////*/
/*            Definitions           */
/*//////////////////////////////////*/
/* 
 * Defines are the same as in "Mapping" section
 */
# 95 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Test_1_Constant_Voltage.ino"
/*////////////////////////////////////////*/
/*            Global Variables            */
/*////////////////////////////////////////*/

/*////////////////////////////////////////////*/
/*            Function declarations           */
/*////////////////////////////////////////////*/

  max5443 DAC1(10);
  max5443 DAC2(5);
  ads1255 ADC1(8, 3);
  ads1255 ADC2(9, 2);

void setup() {
  /*Initializes bus MOSI, SCK, SS as output and pulling SCK, MOSI low and SS high*/
  SPI.begin();
  /*Setting up SPI related pins*/
  DAC1.pins_init();
  DAC2.pins_init();
  ADC1.pins_init();
  ADC2.pins_init();
  pinMode(11, 0x1);
  /*Initialize MUX, Analog Switches, ADC and SD chip select pins*/
  pot_init(); //Set analog and gain mux pins as digital outputs and initialize them
  digitalWrite(8, 0x1); //Reminder: in SPI, True = chip is not selected, False = chip is selected
  digitalWrite(9, 0x1);
  digitalWrite(11, 0x1);

  /*Setting up ADC interrupts*/

  attachInterrupt(((3) == 2 ? 0 : ((3) == 3 ? 1 : -1)), ADC1_int, 2);
  attachInterrupt(((2) == 2 ? 0 : ((2) == 3 ? 1 : -1)), ADC2_int, 2);

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
  __asm__ __volatile__ ("cli" ::: "memory");//ªªªªCONSIDER CHANGING TO TIMER 2ªªªªªªªªªªªªª
  (*(volatile uint8_t *)((0x24) + 0x20)) = 0; //Non WGM mode, normal operation of OCRA pin
  (*(volatile uint8_t *)((0x25) + 0x20)) = 0; //Initialization of prescaler (Timer stopped)

  (*(volatile uint8_t *)((0x26) + 0x20)) = 159; //Load timer to generate a 100 kHz interrupt
  (*(volatile uint8_t *)((0x25) + 0x20)) |= (1 << 0); //Activate timer, with no prescaling (16 MHz)
  (*(volatile uint8_t *)(0x6E)) |= (1 << 0); //Activate interrupt by Timer Overflow
  __asm__ __volatile__ ("sei" ::: "memory");

}

void loop() {
  // put your main code here, to run repeatedly:

}
