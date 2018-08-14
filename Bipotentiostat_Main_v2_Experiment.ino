/*///////////////////////////////////////////////*/
/*          Experiments related class            */
/*///////////////////////////////////////////////*/
/*
 * Code that handles characterization techniques, from polarization (MAX5443 class), readout(ADS1255 class), and analog switch management.
 */

/********************************/
/*          Definitions         */
/********************************/
#define Interrupt_ADC1    3
#define Interrupt_ADC2    2

#define POT_GAIN_0    0
#define POT_GAIN_100  1
#define POT_GAIN_3k   2
#define POT_GAIN_30k  3
#define POT_GAIN_300k 4
#define POT_GAIN_3M   5
#define POT_GAIN_30M  6
#define POT_GAIN_100M 7

#define Single_Mode   false
#define Dual_Mode     true

/******************************/
/*          Includes          */
/******************************/
#include <Arduino.h>
#include <avr/interrupt.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v1\Experiment.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v1\ADS1255.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v1\MAX5443.h>

/**********************************/
/*          Declarations          */
/**********************************/
/*Public variables*/
uint16_t g_gain = POT_GAIN_30k;
uint8_t autogain_enable = 1;

/*Private variables*/
volatile int32_t voltage = 0;       //Volatile declaration avoids optimizator to ignore these
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
   
  uint16_t temp_div = (uint16_t)(ceil((double)period/256));     //temp_div values will rank between 1 and 1024 (Higher than 1024 means that its not feasible)
  uint16_t divider = 0;                                         //The divider that the function will return
  
  if (temp_div == 1) {
    TCCR0B = (TCCR0B & 0xF8) | (0x01);                //No Preescaler
    divider = 1;
  }
  else if (temp_div <= 8){
    TCCR0B = (TCCR0B & 0xF8) | (0x02);                //Preescaler division by 8
    divider = 8;
  }
  else if (temp_div <= 64){
    TCCR0B = (TCCR0B & 0xF8) | (0x03);                //Preescaler division by 64
    divider = 64;
  }
  else if (temp_div <= 256){
    TCCR0B = (TCCR0B & 0xF8) | (0x04);                //Preescaler division by 256
    divider = 256;
  }
  else if (temp_div <= 1024){
    TCCR0B = (TCCR0B & 0xF8) | (0x05);                //Preescaler division by 1024
    divider = 1024;
  }
  else{
    TCCR0B = (TCCR0B & 0xF8) | (0x00);                //Frequency is too low, pretty much no clock source.
    divider = NULL;
  }
  
  period /= divider;                                  //Warning: not sure what happens, but it's a division between two integers of different lengths
  *tc = (uint8_t)period;                              //Just make sure that it is properly turned into a compatible integer
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
    }Timer1_period;                                             //Variable for TCNT1X counter register determination
  uint16_t temp_div = (uint16_t)(ceil((double)period/65536));   //temp_div values will rank between 1 and 1024 (Higher than 1024 means that its not feasible)
  uint16_t divider = 0;                                         //The divider that the function will return
  
  if (temp_div == 1) {
    TCCR1B = (TCCR1B & 0xF8) | (0x01);                //No Preescaler
    divider = 1;
  }
  else if (temp_div <= 8){
    TCCR1B = (TCCR1B & 0xF8) | (0x02);                //Preescaler division by 8
    divider = 8;
  }
  else if (temp_div <= 64){
    TCCR1B = (TCCR1B & 0xF8) | (0x03);                //Preescaler division by 64
    divider = 64;
  }
  else if (temp_div <= 256){
    TCCR1B = (TCCR1B & 0xF8) | (0x04);                //Preescaler division by 256
    divider = 256;
  }
  else if (temp_div <= 1024){
    TCCR1B = (TCCR1B & 0xF8) | (0x05);                //Preescaler division by 1024
    divider = 1024;
  }
  else{
    TCCR1B = (TCCR1B & 0xF8) | (0x00);                //Frequency is too low, pretty much no clock source.
    divider = NULL;
  }
  
  Timer1_period.int16 = (uint16_t)(period / divider); //Warning: not sure what happens, but it's a division between two integers of different lengths
  TCNT1H = Timer1_period.int8[1];                     //Pass the period value into the two registers related to timer
  TCNT1L = Timer1_period.int8[0];
  return divider;
}

void pot_init(void) {
  /**
   * Initializes outputs of potentiostat pins that are related to Gain Selector and analog switches.
   * 
   * Name               IC          Pin   Purpose         Default
   * 
   * Analog_Switch_VIN  (MAX4737)   A4    -Vin switch     LOW  \
   * Analog_Switch_CERE (MAX4737)   A3    CE-RE short     HIGH  |
   * Analog_Switch_CE   (MAX4737)   A2    CE switch       LOW   |--Potentiostat circuit is not in closed loop operation
   * Analog_Switch_WE1  (MAX4737)   A1    WE1 switch      LOW   |
   * Analog_Switch_WE2  (MAX4737)   A0    WE2 switch      LOW  /
   * MUX_A_Gain         (MAX4617)   4     A               HIGH \
   * MUX_B_Gain         (MAX4617)   12    B               HIGH  |--Sets up transimpedance gain as 100 ohms
   * MUX_C_Gain         (MAX4617)   6     C               HIGH /
   * 
   * @return Nothing.
   */
  /*MUX gain selector*/
  pinMode(MUX_A_Gain, OUTPUT);
  pinMode(MUX_B_Gain, OUTPUT);
  pinMode(MUX_C_Gain, OUTPUT);
  digitalWrite(MUX_A_Gain, HIGH);
  digitalWrite(MUX_B_Gain, HIGH);
  digitalWrite(MUX_C_Gain, HIGH);
  /*Analog switches*/
  pinMode(Analog_Switch_VIN, OUTPUT);
  pinMode(Analog_Switch_CERE, OUTPUT);
  pinMode(Analog_Switch_CE, OUTPUT);
  pinMode(Analog_Switch_WE1, OUTPUT);
  pinMode(Analog_Switch_WE2, OUTPUT);
  digitalWrite(Analog_Switch_VIN, LOW);
  digitalWrite(Analog_Switch_CERE, HIGH);
  digitalWrite(Analog_Switch_CE, LOW);
  digitalWrite(Analog_Switch_WE1, LOW);
  digitalWrite(Analog_Switch_WE2, LOW);
}

void pot_set_gain(uint8_t gain) {
  /*
   * Sets the multiplexer values according to the specified gain value.
   * 
   * @param   Gain      Definitions are defined on the header file
   * @return  Nothing
   */

   switch(gain) {
    case POT_GAIN_100M:
      digitalWrite(MUX_A_Gain, LOW);
      digitalWrite(MUX_B_Gain, LOW);
      digitalWrite(MUX_C_Gain, LOW);
      break;
        
    case POT_GAIN_30M:
      digitalWrite(MUX_A_Gain, HIGH);
      digitalWrite(MUX_B_Gain, LOW);
      digitalWrite(MUX_C_Gain, LOW);
      break;
        
    case POT_GAIN_3M:
      digitalWrite(MUX_A_Gain, LOW);
      digitalWrite(MUX_B_Gain, HIGH);
      digitalWrite(MUX_C_Gain, LOW);
      break;
        
    case POT_GAIN_300k:
      digitalWrite(MUX_A_Gain, HIGH);
      digitalWrite(MUX_B_Gain, HIGH);
      digitalWrite(MUX_C_Gain, LOW);
      break;
        
    case POT_GAIN_30k:
      digitalWrite(MUX_A_Gain, LOW);
      digitalWrite(MUX_B_Gain, LOW);
      digitalWrite(MUX_C_Gain, HIGH);
      break;
        
    case POT_GAIN_3k:
      digitalWrite(MUX_A_Gain, HIGH);
      digitalWrite(MUX_B_Gain, LOW);
      digitalWrite(MUX_C_Gain, HIGH);
      break;
        
    case POT_GAIN_0:
      digitalWrite(MUX_A_Gain, LOW);
      digitalWrite(MUX_B_Gain, HIGH);
      digitalWrite(MUX_C_Gain, HIGH);
      break;
        
    case POT_GAIN_100:
      digitalWrite(MUX_A_Gain, HIGH);
      digitalWrite(MUX_B_Gain, HIGH);
      digitalWrite(MUX_C_Gain, HIGH);
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
  digitalWrite(Analog_Switch_VIN, HIGH);
  digitalWrite(Analog_Switch_CERE, LOW);
  digitalWrite(Analog_Switch_CE, HIGH);
  digitalWrite(Analog_Switch_WE1, HIGH);
  if(mode == Single_Mode) digitalWrite(Analog_Switch_WE2, LOW);   //Potentiostat configuration
  if(mode == Dual_Mode) digitalWrite(Analog_Switch_WE2, HIGH);    //Bipotentiostat configuration
}

void volt_exp_stop(void){
  /*
   * Disconnects the measurement cell from voltammetric measurement.
   * @return    Nothing
   */
  digitalWrite(Analog_Switch_VIN, LOW);
  digitalWrite(Analog_Switch_CERE, LOW);    //#Warning: It is like this in Mr. Dryden's code, I expected this to be high as in pot_init(), but maybe not critical
  digitalWrite(Analog_Switch_CE, LOW);
  digitalWrite(Analog_Switch_WE1, LOW);
  digitalWrite(Analog_Switch_WE2, LOW);
}

void pot_exp_start(void){
  /*
   * All switches are open.
   * @return    Nothing
   */
  digitalWrite(Analog_Switch_VIN, LOW);
  digitalWrite(Analog_Switch_CERE, LOW);
  digitalWrite(Analog_Switch_CE, LOW);
  digitalWrite(Analog_Switch_WE1, LOW);
  digitalWrite(Analog_Switch_WE2, LOW);
}

void ocp_exp_start(void){
  /*
   * All switches but WE1 are open.
   * @return    Nothing
   */
  digitalWrite(Analog_Switch_VIN, LOW);
  digitalWrite(Analog_Switch_CERE, LOW);
  digitalWrite(Analog_Switch_CE, LOW);
  digitalWrite(Analog_Switch_WE1, HIGH);
  digitalWrite(Analog_Switch_WE2, LOW);
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
        if (getchar() == 'a'){
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
        if (getchar() == 'a'){
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
   *  0=error  
   *  1=no error
	 * 
	 * Peripherals
	 * 	TIMER 	0: Usado como cronómetro --> Para modificar el DAC
	 * 	COUNTER	1: Usado como contador de voltaje --> Para determinar el índice de "polarización" del DAC, dado que DAC y ADC funcionan a frecuencias diferentes, esta estrategia permite trackear el "tiempo"/"instancia de voltaje"
	 * 
	 * Interrupts
	 * 	TCC0 OVF:	Small priority 	--> Cambia el valor del DAC
	 * 	TCC1 OVF:	 
	 * 	TCC1 CCA:	High priority 	-->	Cada vez que TCC0 se desborda, Countador 1 incrementa
	 * 	INT0: 		Medium priority --> ADS1255 DRDY, cada lectura completada activa esta interrupción
	 */
	
	uint8_t ret = 0;          //Return variable, by default no error.
  uint32_t timer_period;    //Timer period
	uint16_t temp_div;        //Variable used for preescaler determination

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
		volt_exp_start(Single_Mode);												//Connect cell: one WE
		ADC1.rdatac();                                      //Continous acquisition mode
		tc_enable(&TCC1);																		//Enable counter1 (Para almacenar voltaje DAc)
		ADC1.sync();																			  //ADC sync
		tc_enable(&TCC0);																		//Enable timer0
		tc_set_overflow_interrupt_callback(&TCC0, tcf0_ovf_callback);							//Asignación del Interrupt por overflow timer0 -> asigns TCC1.CTR as DAC voltage...????
		tc_set_overflow_interrupt_callback(&TCC1, tce1_ovf_callback_lsv);						//Asignación del Interrupt por overflow timer1 -> Deshabilita interrupts por timers y PORTD
		tc_set_cca_interrupt_callback(&TCC1, lsv_cca_callback);									//Asignación del Interrupt por capture/compare timer1, LSV solamente usa esta interrupción
		portd_int0_callback = porte_int0_lsv; 	//ADC read										La ISR de ADS1255, le asigna la funcion porte_int0_lsv
												//Propósito: Leer ADC y mean timer(promedio) y transferir por USB
		//set EVCH0 event
		/*EVENT ROUTING NETWORK
		8 canales idénticos, cada canal con un multiplexor y lógica de control y filtro(este es para los pines, cuantos ciclos de reloj para considerar evento)
		*/
		EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;	//Configura el mux0 como Timer/counter 0 overflow (event system, an interesting approach free of interrupt)
		EVSYS.CH0CTRL = 0;						//Acepta el cambio desde el primer muestreo
	
		
		//Configuracion del Internal clock source para TCC0 en función del slope y FCLK_IO
		timer_period = ceil(1/((double)slope/(3000./65536))*(F_CPU)); //Timer period, depends on slope and CPU Frequency. 3000 I think its a normalization factor between -1500 and 1499
		//Definición del preescaler: ¿Cuantas veces los "ticks" caben en 2^16?
		temp_div = ceil(timer_period/65536.);						//This calculation sets the prescaler, so everytime it fills the timer, it triggers interrupt
		
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
		ads1255_wakeup();										//Despertar ADC
		tc_write_period(&TCC1, 0xffff);							//CONSECUENCIA:TCC1.Per a full y provocar la interrupción tce1_ovf_callback_lsv, el cual desactivará las 3 interrupciones y asignará up=0)
		tc_write_period(&TCC0, (uint16_t)timer_period);			//Modificar timer 0 de acuerdo a cálculos
	
	TCC1.CNT = dacindex_start;			//TCC1 arranca con el votlaje inicial
	
	if (stop > start)					//Si el barrido es anódico
	{
		up = 1;							//Creo que up aquí se usa como un comodín, en voltametría representa un barrido anódico si es positivo, catódio si negativo
		tc_set_direction(&TCC1, TC_UP);	//Sets counting direction up
	}
	else
	{
		up = -1;								//Direction down
		tc_set_direction(&TCC1, TC_DOWN);		//Sets counting direction
	}
	
	tc_write_cc(&TCC1, TC_CCA, dacindex_stop);	//Escribe el valor de capture compare, donde frena el conteo
	tc_enable_cc_channels(&TCC1, TC_CCAEN);		//Habilita capture/compare
	TCC0.CNT = 0;								//Inicializar Timer0
	
	/*Asignar las prioridades a las interrupciones de con funciones y "manual"*/
	tc_set_cca_interrupt_level(&TCC1, TC_INT_LVL_HI); 			//Assign high priority to counter 1
	tc_set_overflow_interrupt_level(&TCC0, TC_OVFINTLVL_LO_gc); //Assign low priority to DACtimer0
	PORTD.INTCTRL = PORT_INT0LVL_MED_gc; 						//Assign ADC interrupt with medium priority
	
	tc_write_clock_source(&TCC1, TC_CLKSEL_EVCH0_gc);			//Asigna Canal 0 como fuente de reloj para TCC1, el cual incrementa cada vez que TCC0 se desborda!
	
	//Experiment run with interrupts LEER EL ISR
	/*STALLING LOOP: Break if aborting or if up changed with an interrupt*/
	while (up != 0){
		if (udi_cdc_is_rx_ready()){
			if (getchar()=='a'){
				tce1_ovf_callback_lsv();
				ret = 1;					//Retorna 1 si aborta operación
				goto aborting;
				
			}
		}
	}
	/*Si first run es -1 o trabaja en modo LSV, deactivar todo*/
	if (first_run == -1 || first_run == 2)	//first run es para evitar la desconexión de la celda si LSV se usa varias veces consecutivas. 2 representa una lectura normal de LSV.
	{										//Fijarse, que si el valor es 1, no se detiene timer0, contador1, celda, ads1255
		aborting:
			tc_disable(&TCC0);				//Deshabilita timer0
			TCC0.CNT = 0x0;					//Reinicializa timer0
			tc_disable(&TCC1);				//Deshabilitar counter1
			volt_exp_stop();				//Desconecta la celda
			ads1255_standby();				//Pone el ADC en standby
			return ret;						//Parece ser que regresa un cero de inicialización
	}
	
	return ret;								//Parece que regresa 0, en operación normal
}

void ADC1_int() {
  // put your main code here, to run repeatedly:

}

void ADC2_int() {

}

/******************************/
/*          Callbacks         */
/******************************/


/*Callbacks for Voltammetric methods*/

/*Callbacks for Chronoamperommetric methods*/

/*******************************/
/*          Interrupts         */
/*******************************/
//Timer interrupt
ISR(TIMER1_COMPA_vect) {
  
}

ISR(TIMER0_COMPA_vect) {
  
}