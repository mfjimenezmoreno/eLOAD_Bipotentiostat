# 1 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\ADS1255.cpp"
# 1 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\ADS1255.cpp"
/*///////////////////////////////////////*/
/*          ADC related methods          */
/*///////////////////////////////////////*/

# 6 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\ADS1255.cpp" 2
# 7 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\ADS1255.cpp" 2
# 8 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\ADS1255.cpp" 2

SPISettings SPI_ADS1255(19000000, 1, 0x04);

ads1255::ads1255(int Chip_Select, int DRDY){
  /*
   * Class constructor
   * @param Chip_Select   Pin where DAC chip select is located
   * @param DRDY          Pin where DRDY is connected to (interrupt)
  */
  _CS_pin = Chip_Select;
  _DRDY_pin = DRDY;
}

void ads1255::pins_init() {
  /*
     Purpose: ADC Chip select pin is set as output and turns it HIGH (i.e. chip is not selected)
  */
  pinMode(_CS_pin, 0x1);
  digitalWrite(_CS_pin, 0x1);
}

void ads1255::sync() {
  /*
     Purpose: Synchronizes the A/D conversion. Command should be followed by Wakeup command. Synchronization will happen on the first CLKIN rising edge.
  */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  SPI.transfer(0xFC);
  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();
}

void ads1255::reg_read(uint8_t address) {
  /*
   * Purpose: reads the specified register. Format according to page 34. Note: returns five readings into the public variable input_buffer[5].
   * Format: 
   * └──>1st command byte: 0001 rrrr (starting register to read commands)
   * └──>2nd command byte: 0000 nnnn (number of registers to be read +1, herein it is fixed to 4, requesting 5 registers starting from address)
   */

  //1st command byte definition
  _ADC_buffer.ui8[0] = address;
  _ADC_buffer.ui8[0] |= 0x10;
  //2nd command byte definition
  _ADC_buffer.ui8[1] = 4;

  //Send read register command to ADC
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  while(digitalRead(_DRDY_pin) == 0x1); //Hold on, until DRDY pin is LOW. Note: make sure the interrupt is deactivated.
  SPI.transfer(_ADC_buffer.ui8[0]);
  SPI.transfer(_ADC_buffer.ui8[1]);
  delayMicroseconds(6.5); //Should be minimum 50 times 1/CLKin, theoretically doesn't use timer0 so interrupt should be safe
  input_buffer[0] = SPI.transfer(0x00);
  input_buffer[1] = SPI.transfer(0x00);
  input_buffer[2] = SPI.transfer(0x00);
  input_buffer[3] = SPI.transfer(0x00);
  input_buffer[4] = SPI.transfer(0x00);
  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();
}

void ads1255::reset() {
  /*
   * Purpose: Software reset. Registers are initialized to their default state except for CLK0 and CLK1 bits in ADCON register (controls
   * D0/CLKOUT pin). Self-calibration is always performed after reset.
   */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  //while(digitalRead(_DRDY_pin) == HIGH);  //Hold on, until DRDY pin is LOW. I don't think this is necessary at all from original source code.
  SPI.transfer(0xFE);
  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();
}

void ads1255::setup_reg(uint8_t buff, uint8_t rate, uint8_t pga) {
  /*
   * Purpose: Setup 4 registers, user inputs the desired STATUS, ADCON and DRATE Register. The method firs send two bytes that commands the ADS1255
   * to edit 4 registers starting from STATUS (register 0x00). This function has the main purpose of enabling or disabling analog input buffer (buff),
   * the data rate (rate) and the proggramable gain amplifier (pga). Performs gain, offset calibration at the end while synchronizing A/D conversion
   * with CLKIN as well.
   * 
   * NOTE: for your convenience, use the constant defines from the header file as arguments. (ADCON, DRATE and PGA registers).
   * 
   * Registers:
   * └──>STATUS: 7-4: read only, 3: Order: MSB(0) or LSB(1), 2:ACAL: autocal disabled (0) enabled (1), 1:BUFEN analog input buffer enable(0) or disable (1), 0:~DRDY~ (read only)
   * └──>MUX: 7-4: for positive Input selection (only two options in ADS1255), 3-0: same, but for negative.
   * └──>ADCON: 7: Reserved, 6-5: Clkout rate setting,4-2: Sensor detect currents, 2-0-: programmable gain amplifier setting
   * └──>DRATE: 7-0: Data rate setting!
   */

  buff &= 0x02; //Masking out the possibility that user makes anything funny with byte order transfer, only BUFEN bit can be edited.

  //Create command buffer: Write from register 0 (STATUS), 3+1 commands (until DRATE), STATUS, MUX(choose AIN0 and AINCOM), ADCON(PGA), DRATE(RATE) 
  _command_buffer[0] = 0x50;
  _command_buffer[1] = 0x30;
  _command_buffer[2] = buff;
  _command_buffer[3] = 0x08;
  _command_buffer[4] = pga;
  _command_buffer[5] = rate;
  //Setup the read only sample_delay_ms_100div register according to specified rate
  switch (rate) {
    case 0b00000011 /*0x03*/:
      sample_delay_ms_100div = 40022;
      break;
    case 0b00010011 /*0x13*/:
      sample_delay_ms_100div = 20022;
      break;
    case 0b00100011 /*0x23*/:
      sample_delay_ms_100div = 10022;
      break;
    case 0b00110011 /*0x33*/:
      sample_delay_ms_100div = 6688;
      break;
    case 0b01000011 /*0x43*/:
      sample_delay_ms_100div = 4022;
      break;
    case 0b01010011 /*0x53*/:
      sample_delay_ms_100div = 3355;
      break;
    case 0b01100011 /*0x63*/:
      sample_delay_ms_100div = 2022;
      break;
    case 0b01110010 /*0x72*/:
      sample_delay_ms_100div = 1688;
      break;
    case 0b10000010 /*0x82*/:
      sample_delay_ms_100div = 1022;
      break;
    case 0b10010010 /*0x92*/:
      sample_delay_ms_100div = 222;
      break;
    case 0b10100001 /*0xA1*/:
      sample_delay_ms_100div = 122;
      break;
    case 0b10110000 /*0xB0*/:
      sample_delay_ms_100div = 72;
      break;
    case 0b11000000 /*0xC0*/:
      sample_delay_ms_100div = 48;
      break;
    case 0b11010000 /*0xD0*/:
      sample_delay_ms_100div = 35;
      break;
    case 0b11100000 /*0xE0*/:
      sample_delay_ms_100div = 29;
      break;
    case 0b11110000 /*0xF0*/:
      sample_delay_ms_100div = 25;
      break;
  }

  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  //while(digitalRead(_DRDY_pin) == HIGH);  //Hold on, until DRDY pin is LOW. I don't think this is necessary at all from original source code.
  for(uint8_t n=0; n<=5; n++) {
    SPI.transfer(_command_buffer[n]); //Send the 2 write command packets and 4 registers.
  }
  SPI.transfer(0xFC); //Synchronize A/D conversion with CLKIN rising edge from WAKEUP command.
  SPI.transfer(0x00); //NOT IN Mr. Dryden's code: Completes synchronization
  SPI.transfer(0xF0); //Perform a self offset and self gain calbiration.
  while(digitalRead(_DRDY_pin) == 0x1); //Hold on. Calibration is complete until DRDY goes low.
  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();
}

void ads1255::mux(uint8_t channel) {
  /*
   * Purpose: Edit the mux register:
   * BITS: 7-4 PSEL 3-0 NSEL
   * PSEL = Positive input channel select AIN0(0000)(default), AIN1(0001) and AINCOM(1XXX)
   * NSEL = Negative input channel select AIN0(0000), AIN1(0001)(default) and AINCOM(1XXX)
   * 
   * Expected channel values from Bipotentiostat application:
   *  If ADC from WE1:
   *  Voltammetry   0x08 (AIN0 - AINCOM or V_IWE1 node vs. 1.5 volts reference)
   *  Potentiometry 0x18 (AIN1 - AINCOM or Ref_V  node vs. 1.5 volts reference)
   * 
   *  If ADC from WE2:
   *  Voltammetry   0x08 (AIN0 - AINCOM or V_IWE2 node vs. 1.5 volts reference)
   *  Potentiometry N/A 
   * 
   * Note: Use the header definitions! Two options available depending if it works in potentiometric or voltammetric mode.
   * 
   */

  _command_buffer[0] = 0x51; //Write only the MUX register,
  _command_buffer[1] = 0x00;
  _command_buffer[2] = channel;

  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  SPI.transfer(0x0F); //Stop read data continously
  for(uint8_t n=0; n<=2; n++) {
    SPI.transfer(_command_buffer[n]); //Send the 2 write command packets and 4 registers.
  }
  SPI.transfer(0xFC); //Synchronize A/D conversion with CLKIN rising edge from WAKEUP command.
  SPI.transfer(0x00); //NOT IN Mr. Dryden's code: Completes synchronization
  SPI.transfer(0xF0); //Perform a self offset and self gain calbiration.
  while(digitalRead(_DRDY_pin) == 0x1); //Hold on. Until it is ready. (Page 36)
  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();
}

void ads1255::standby(void){
  /*
   * Purpose: Puts device into low power mode. Make sure not to send clock cycles if CS pin is low. Use WAKEUP command to exit mode.
   */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  SPI.transfer(0xFD); //Commands ADS1255 into Standby mode
  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();
}

void ads1255::wakeup(void){
  /*
   * Purpose: Command to exit Standby mode.
   */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  SPI.transfer(0x00); //Commands ADS1255 to exit Standby mode
  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();
}

void ads1255::rdatac(void){
  /*
   * Purpose: Enables continous output of data. After 24 bits have been read, ~DRDY~ goes low. It is not necessary to read the 24 bits.
   * ~DRDY~ will not go high unless there is new data being updated. This mode can be stopped by SDATAC or RESET commands
   */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  SPI.transfer(0x03); //Enter into continous data generation 
  //while(digitalRead(_DRDY_pin) == HIGH);    //Hold on. Until it is ready. I don't think it's necessary to add this.
  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();
}

int16_t ads1255::read_fast(void){
  /*
   * Purpose: Reads a single conversion from ADS1255. Returns a 16 byte int value and enter standby mode.
   * Warning: only 2 bytes out of 3 are being read and no RDATA or RDATAC is being requiested.
   * 
   * Reference of Arduino's SPI registers if built-in library is not clear enough:
   * 
   * SPDR (0x2E): SPI Data Register. Read/Write register for data transfer to the SPI shift register.
   *  |----> Note: one buffer for transmission, use SPIF to know when to transmit.
   *  |----> Note: two buffers for reception (but only one is accessed), reading clears the buffer. If reading is not performed, a byte will be shifted out/lost.
   *  |----> Note: Writing to SPDR initiates transmission. Reading it causes the shift register to be read.
   *  
   * SPCR (0x2C): SPI Control Register (I suggest not this register, it is handled by Arduino code!)
   *  |----> SPIE (bit 7): SPI Interrupt Enable
   *  |----> SPE  (bit 6): SPI Enable
   *  |----> DORD (bit 5): 0 = LSB 1 = MSB
   *  |----> MSTR (bit 4): Master/Slave Select
   *  |----> CPOL (bit 3): Clock Polarity 0 = Rising leading edge, Falling  trailing edge
   *  |----> CPHA (bit 2): Clock Phase, is data sampled or setup in the trailing or rising edge?
   *  |----> SPR1 (bit 1): SPI Clock rate select
   *  |----> SPR0 (bit 0): SPI Clock rate select
   *  
   * SPSR (0x2D): SPI Status Register
   *  |----> SPIF (bit 7): end of transmission or reception flag. Cleared by either interrupt handler or **by reading SPIF set and then accesing SPDR**!
   *  |----> WCOL (bit 6): write collision flag. Set if SPDR is written during a data transfer. WCOL and SPIF are cleared after reading WCOL set and then accessing SPDR
   *  |----> SPI2X(bit 0): Double SPI Speed bit. I suggest not touching this.
   */
  _Input_buffer.ui16 = 0; //Initialize the data that shall be returned
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);

  for(int i = 1; i >= 0; --i) {
    while (!((*(volatile uint8_t *)((0x2D) + 0x20)) & (1 << 7))); //Wait for any pending transmission/reception to complete (Stops only if SPIF is set), maybe not required with the SPI library, Mr.Dryden seem to use USART
    _Input_buffer.ui8[i] = SPI.transfer(0x00); //Read SPDR register, start from MSB, consider changing to assembly code if this doesn't work.
  }

  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();

  return _Input_buffer.ui16; //Regresar dato en entero de 16 bits
}

int16_t ads1255::read_fast_single(void){
  /*
   * Purpose: Reads a single conversion result. This one sends ADS_RDATA. Returns a 16 byte int value. Warning: only 2 bytes out of 3 are being read.
   * 
   */
  _Input_buffer.ui16 = 0; //Initialize the data that shall be returned
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  SPI.transfer(0x01); //Read a single conversion result
  delayMicroseconds(6.5); //Mandatory to wait 50 times 1/CLKin, theoretically doesn't use timer0 so interrupt should be safe

  for(int i = 1; i >= 0; --i) {
    while (!((*(volatile uint8_t *)((0x2D) + 0x20)) & (1 << 7))); //Wait for any pending transmission/reception to complete (Stops only if SPIF is set), maybe not required with the SPI library, Mr.Dryden seem to use USART
    _Input_buffer.ui8[i] = SPI.transfer(0x00); //Read SPDR register, consider changing to assembly code if this doesn't work.
  }

  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();
  ads1255::standby(); //Issue a standby mode. 
  return _Input_buffer.ui16; //Return a 16 bit data.
}

int32_t ads1255::read_fast24(void){
  /*
   * Purpose: Performs an ADC readout of 24 bits while halting and reactivating interrupts.
   * Returns: 4 bytes buffer
   *  [3] = Exceeding full scale flag (FF if true, 00 if clear). ADS1255 positive full scale is 0x7FFFFF. In two complementary format.
   *  [2] = MSB from readout (up to 7F)
   *  [1] = Middle significance byte
   *  [0] = LSB from readout. LSB value is 2*Vref/(PGA(2^23-1)).
   */

  _Input_buffer_24.ui32 = 0; //Initialize variable.
  __asm__ __volatile__ ("cli" ::: "memory"); //Time critical operation
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);

  for(int i = 2; i >= 0; --i) {
    while (!((*(volatile uint8_t *)((0x2D) + 0x20)) & (1 << 7))); //Wait for any pending transmission/reception to complete (Stops only if SPIF is set), maybe not required with the SPI library, Mr.Dryden seem to use USART
    _Input_buffer_24.ui8[i] = SPI.transfer(0x00); //Read SPDR register, consider changing to assembly code if this doesn't work.
  }

  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();

  if(_Input_buffer_24.ui8[2] > 0x7F) //ADS1255 Positive full scale is 0x7FFFFF, LSB=2*Vref/(PGA(2^23-1)), Binary Two Complement format
    _Input_buffer_24.ui8[3] = 0xFF; //This byte is only used to signal exceeding full scale...
  else
    _Input_buffer_24.ui8[3] = 0x00; //... and when it doesn't exceed full scale

  __asm__ __volatile__ ("sei" ::: "memory");
  return _Input_buffer_24.ui32; //Buffer Format = Exceeding Full scale flag (either 00 or FF)|MSB (up to 7F)|Middle Byte|LSB Data
}

int32_t ads1255::read_single24(void){
  /*
   * Purpose: Same as  read_fast24, but sends an RDATA request for non-continous on-demand operation.
   */

   _Input_buffer_24.ui32 = 0; //Initialize variable.
  __asm__ __volatile__ ("cli" ::: "memory"); //Time critical operation
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, 0x0);
  SPI.transfer(0x01); //Read a single conversion result
  delayMicroseconds(6.5); //Mandatory to wait 50 times 1/CLKin, theoretically doesn't use timer0 so interrupt should be safe

  for(int i = 2; i >= 0; --i) {
    while (!((*(volatile uint8_t *)((0x2D) + 0x20)) & (1 << 7))); //Wait for any pending transmission/reception to complete (Stops only if SPIF is set), maybe not required with the SPI library, Mr.Dryden seem to use USART
    _Input_buffer_24.ui8[i] = SPI.transfer(0x00); //Read SPDR register, consider changing to assembly code if this doesn't work.
  }

  digitalWrite(_CS_pin, 0x1);
  SPI.endTransaction();

  if(_Input_buffer_24.ui8[2] > 0x7F) //ADS1255 Positive full scale is 0x7FFFFF, LSB=2*Vref/(PGA(2^23-1)), Binary Two Complement format
    _Input_buffer_24.ui8[3] = 0xFF; //This byte is only used to signal exceeding full scale...
  else
    _Input_buffer_24.ui8[3] = 0x00; //... and when it doesn't exceed full scale

  __asm__ __volatile__ ("sei" ::: "memory");
  return _Input_buffer_24.ui32; //Buffer Format = Exceeding Full scale flag (either 00 or FF)|MSB (up to 7F)|Middle Byte|LSB Data
}
# 1 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\Bipotentiostat_Main_v2.ino"
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
# 64 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\Bipotentiostat_Main_v2.ino" 2
# 65 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\Bipotentiostat_Main_v2.ino" 2
# 66 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\Bipotentiostat_Main_v2.ino" 2
# 67 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\Bipotentiostat_Main_v2.ino" 2
# 68 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\Bipotentiostat_Main_v2.ino" 2
# 69 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\Bipotentiostat_Main_v2.ino" 2
# 70 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\Bipotentiostat_Main_v2.ino" 2

 /*//////////////////////////////////*/
 /*            Definitions           */
 /*//////////////////////////////////*/
 /* 
 * Defines are the same as in "Mapping" section
 */
# 96 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\Bipot_Main_v01\\Bipotentiostat_Main_v2.ino"
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

max5443 DAC1(10);
max5443 DAC2(5);
ads1255 ADC1(8, 3);
ads1255 ADC2(9, 2);

cell_parameters sensor; //Stores experimental parameters
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
  pinMode(11, 0x1);
  digitalWrite(11, 0x1);
  /*LED Pin*/
  pinMode(13, 0x1);
  digitalWrite(13, 0x0);
  /*Set analog switches & multiplexer as outputs */
  analog_switch_init(); //Initialize analog switches with switches off
  pot_init();

  /*Setting up ADC interrupts
  Configuration:
    1. Timer1 CTC compare: 100 kHz interrupt, changes values of DAC
    2. External Interrupt pins: ADC reports when it is ready for transmission
  */
  __asm__ __volatile__ ("cli" ::: "memory");
  //TODO remove the following comments
  //set_timer1_frequency(100E3);
  attachInterrupt(((3) == 0 ? 2 : ((3) == 1 ? 3 : ((3) == 2 ? 1 : ((3) == 3 ? 0 : ((3) == 7 ? 4 : -1))))), &interrupt_ADC1_rdy, 2);
  attachInterrupt(((2) == 0 ? 2 : ((2) == 1 ? 3 : ((2) == 2 ? 1 : ((2) == 3 ? 0 : ((2) == 7 ? 4 : -1))))), &interrupt_ADC2_rdy, 2);
  //timer1_EN_IntCTC();
  __asm__ __volatile__ ("sei" ::: "memory");
  ON_LED();
}


extern "C" void __vector_17 /* Timer/Counter1 Compare Match A */ (void) __attribute__ ((signal,used, externally_visible)) ; void __vector_17 /* Timer/Counter1 Compare Match A */ (void) {
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
  if (sensor.mode == false)
  {
    DAC1.set_voltage(0);
  }
  else if (sensor.mode == true)
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

  //When the arduino receives ">", receivedDataRdy is true
  /**Debug code, reflect information as received by PC*/
  /*
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
  */
  //Update experiment parameters

  //When flag is ready...
  if (receivedDataRdy) {
    /*The following proceedure is the protocol to change cell parameters.
    - If setting is acknowledged, then return OK.
    - If not acknowledged, then return ?.
    - If parameter is requested, return parameter. */

    char command[2] = serialData[0:1];
    char

    //Reset flag and empty serialData
    receivedDataRdy = false;
    for (int i = 0; i < sizeof(serialData); i++) {
      serialData[i] = '\0';
    }

  }
/*
  if (serialData[0:1] == "U")
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
