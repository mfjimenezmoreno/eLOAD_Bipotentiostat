/*///////////////////////////////////////*/
/*          ADC related methods          */
/*///////////////////////////////////////*/

#include <Arduino.h>
#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v1\ADS1255.h>
#include <SPI.h>

SPISettings SPI_ADS1255(19000000,MSBFIRST,SPI_MODE1);

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
  pinMode(_CS_pin, OUTPUT);
  digitalWrite(_CS_pin, HIGH);
}

void ads1255::sync() {
  /*
     Purpose: Synchronizes the A/D conversion. Command should be followed by Wakeup command. Synchronization will happen on the first CLKIN rising edge.
  */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);
  SPI.transfer(ADS_SYNC);
  digitalWrite(_CS_pin, HIGH);
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
  digitalWrite(_CS_pin, LOW);
  while(digitalRead(_DRDY_pin) == HIGH);  //Hold on, until DRDY pin is LOW. Note: make sure the interrupt is deactivated.
  SPI.transfer(_ADC_buffer.ui8[0]);
  SPI.transfer(_ADC_buffer.ui8[1]);
  delayMicroseconds(6.5);                 //Should be minimum 50 times 1/CLKin, theoretically doesn't use timer0 so interrupt should be safe
  input_buffer[0] = SPI.transfer(0x00);
  input_buffer[1] = SPI.transfer(0x00);
  input_buffer[2] = SPI.transfer(0x00);
  input_buffer[3] = SPI.transfer(0x00);
  input_buffer[4] = SPI.transfer(0x00);
  digitalWrite(_CS_pin, HIGH);
  SPI.endTransaction();
}

void ads1255::reset() {
  /*
   * Purpose: Software reset. Registers are initialized to their default state except for CLK0 and CLK1 bits in ADCON register (controls
   * D0/CLKOUT pin). Self-calibration is always performed after reset.
   */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);
  //while(digitalRead(_DRDY_pin) == HIGH);  //Hold on, until DRDY pin is LOW. I don't think this is necessary at all from original source code.
  SPI.transfer(ADS_RESET);
  digitalWrite(_CS_pin, HIGH);
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

  buff &= 0x02;  //Masking out the possibility that user makes anything funny with byte order transfer, only BUFEN bit can be edited.
  
  //Create command buffer: Write from register 0 (STATUS), 3+1 commands (until DRATE), STATUS, MUX(choose AIN0 and AINCOM), ADCON(PGA), DRATE(RATE) 
  _command_buffer[0] = 0x50;
  _command_buffer[1] = 0x30;
  _command_buffer[2] = buff;
  _command_buffer[3] = 0x08;
  _command_buffer[4] = pga;
  _command_buffer[5] = rate;
  //Setup the read only sample_delay_ms_100div register according to specified rate
  switch (rate) {
    case ADS_DR_2_5:
      sample_delay_ms_100div = 40022;
      break;
    case ADS_DR_5:
      sample_delay_ms_100div = 20022;
      break;
    case ADS_DR_10:
      sample_delay_ms_100div = 10022;
      break;
    case ADS_DR_15:
      sample_delay_ms_100div = 6688;
      break;
    case ADS_DR_25:
      sample_delay_ms_100div = 4022;
      break;
    case ADS_DR_30:
      sample_delay_ms_100div = 3355;
      break;
    case ADS_DR_50:
      sample_delay_ms_100div = 2022;
      break;
    case ADS_DR_60:
      sample_delay_ms_100div = 1688;
      break;
    case ADS_DR_100:
      sample_delay_ms_100div = 1022;
      break;
    case ADS_DR_500:
      sample_delay_ms_100div = 222;
      break;
    case ADS_DR_1000:
      sample_delay_ms_100div = 122;
      break;
    case ADS_DR_2000:
      sample_delay_ms_100div = 72;
      break;
    case ADS_DR_3750:
      sample_delay_ms_100div = 48;
      break;
    case ADS_DR_7500:
      sample_delay_ms_100div = 35;
      break;
    case ADS_DR_15000:
      sample_delay_ms_100div = 29;
      break;
    case ADS_DR_30000:
      sample_delay_ms_100div = 25;
      break;
  }
  
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);
  //while(digitalRead(_DRDY_pin) == HIGH);  //Hold on, until DRDY pin is LOW. I don't think this is necessary at all from original source code.
  for(uint8_t n=0; n<=5; n++) {
    SPI.transfer(_command_buffer[n]);       //Send the 2 write command packets and 4 registers.
  }
  SPI.transfer(ADS_SYNC);                   //Synchronize A/D conversion with CLKIN rising edge from WAKEUP command.
  SPI.transfer(ADS_WAKEUP);                 //NOT IN Mr. Dryden's code: Completes synchronization
  SPI.transfer(ADS_SELFCAL);                //Perform a self offset and self gain calbiration.
  while(digitalRead(_DRDY_pin) == HIGH);    //Hold on. Calibration is complete until DRDY goes low.
  digitalWrite(_CS_pin, HIGH);
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

  _command_buffer[0] = 0x51;                //Write only the MUX register,
  _command_buffer[1] = 0x00;
  _command_buffer[2] = channel;
  
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);
  SPI.transfer(ADS_SDATAC);                 //Stop read data continously
  for(uint8_t n=0; n<=2; n++) {
    SPI.transfer(_command_buffer[n]);       //Send the 2 write command packets and 4 registers.
  }
  SPI.transfer(ADS_SYNC);                   //Synchronize A/D conversion with CLKIN rising edge from WAKEUP command.
  SPI.transfer(ADS_WAKEUP);                 //NOT IN Mr. Dryden's code: Completes synchronization
  SPI.transfer(ADS_SELFCAL);                //Perform a self offset and self gain calbiration.
  while(digitalRead(_DRDY_pin) == HIGH);    //Hold on. Until it is ready. (Page 36)
  digitalWrite(_CS_pin, HIGH);
  SPI.endTransaction();
}

void ads1255::standby(void){
  /*
   * Purpose: Puts device into low power mode. Make sure not to send clock cycles if CS pin is low. Use WAKEUP command to exit mode.
   */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);
  SPI.transfer(ADS_STANDBY);                //Commands ADS1255 into Standby mode
  digitalWrite(_CS_pin, HIGH);
  SPI.endTransaction();
}

void ads1255::wakeup(void){
  /*
   * Purpose: Command to exit Standby mode.
   */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);
  SPI.transfer(ADS_WAKEUP);                //Commands ADS1255 to exit Standby mode
  digitalWrite(_CS_pin, HIGH);
  SPI.endTransaction();
}

void ads1255::rdatac(void){
  /*
   * Purpose: Enables continous output of data. After 24 bits have been read, ~DRDY~ goes low. It is not necessary to read the 24 bits.
   * ~DRDY~ will not go high unless there is new data being updated. This mode can be stopped by SDATAC or RESET commands
   */
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);
  SPI.transfer(ADS_RDATAC);                   //Enter into continous data generation 
  //while(digitalRead(_DRDY_pin) == HIGH);    //Hold on. Until it is ready. I don't think it's necessary to add this.
  digitalWrite(_CS_pin, HIGH);
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
  _Input_buffer.ui16 = 0;                           //Initialize the data that shall be returned
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);

  for(int i = 1; i >= 0; --i) {
    while (!(SPSR & (1 << SPIF)));                  //Wait for any pending transmission/reception to complete (Stops only if SPIF is set), maybe not required with the SPI library, Mr.Dryden seem to use USART
    _Input_buffer.ui8[i] = SPI.transfer(0x00);      //Read SPDR register, start from MSB, consider changing to assembly code if this doesn't work.
  }

  digitalWrite(_CS_pin, HIGH);
  SPI.endTransaction();
  
  return _Input_buffer.ui16;                        //Regresar dato en entero de 16 bits
}

int16_t ads1255::read_fast_single(void){
  /*
   * Purpose: Reads a single conversion result. This one sends ADS_RDATA. Returns a 16 byte int value. Warning: only 2 bytes out of 3 are being read.
   * 
   */
  _Input_buffer.ui16 = 0;                          //Initialize the data that shall be returned
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);
  SPI.transfer(ADS_RDATA);                          //Read a single conversion result
  delayMicroseconds(6.5);                           //Mandatory to wait 50 times 1/CLKin, theoretically doesn't use timer0 so interrupt should be safe

  for(int i = 1; i >= 0; --i) {
    while (!(SPSR & (1 << SPIF)));                  //Wait for any pending transmission/reception to complete (Stops only if SPIF is set), maybe not required with the SPI library, Mr.Dryden seem to use USART
    _Input_buffer.ui8[i] = SPI.transfer(0x00);      //Read SPDR register, consider changing to assembly code if this doesn't work.
  }
  
  digitalWrite(_CS_pin, HIGH);
  SPI.endTransaction();
  ads1255::standby();                               //Issue a standby mode. 
  return _Input_buffer.ui16;                        //Return a 16 bit data.
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

  _Input_buffer_24.ui32 = 0;                        //Initialize variable.
  noInterrupts();                                   //Time critical operation
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);

  for(int i = 2; i >= 0; --i) {
    while (!(SPSR & (1 << SPIF)));                  //Wait for any pending transmission/reception to complete (Stops only if SPIF is set), maybe not required with the SPI library, Mr.Dryden seem to use USART
    _Input_buffer_24.ui8[i] = SPI.transfer(0x00);   //Read SPDR register, consider changing to assembly code if this doesn't work.
  }

  digitalWrite(_CS_pin, HIGH);
  SPI.endTransaction();

  if(_Input_buffer_24.ui8[2] > 0x7F)                //ADS1255 Positive full scale is 0x7FFFFF, LSB=2*Vref/(PGA(2^23-1)), Binary Two Complement format
    _Input_buffer_24.ui8[3] = 0xFF;                 //This byte is only used to signal exceeding full scale...
  else
    _Input_buffer_24.ui8[3] = 0x00;                 //... and when it doesn't exceed full scale
  
  interrupts();
  return _Input_buffer_24.ui32;                     //Buffer Format = Exceeding Full scale flag (either 00 or FF)|MSB (up to 7F)|Middle Byte|LSB Data
}

int32_t ads1255::read_single24(void){
  /*
   * Purpose: Same as  read_fast24, but sends an RDATA request for non-continous on-demand operation.
   */

   _Input_buffer_24.ui32 = 0;                        //Initialize variable.
  noInterrupts();                                   //Time critical operation
  SPI.beginTransaction(SPI_ADS1255);
  digitalWrite(_CS_pin, LOW);
  SPI.transfer(ADS_RDATA);                          //Read a single conversion result
  delayMicroseconds(6.5);                           //Mandatory to wait 50 times 1/CLKin, theoretically doesn't use timer0 so interrupt should be safe

  for(int i = 2; i >= 0; --i) {
    while (!(SPSR & (1 << SPIF)));                  //Wait for any pending transmission/reception to complete (Stops only if SPIF is set), maybe not required with the SPI library, Mr.Dryden seem to use USART
    _Input_buffer_24.ui8[i] = SPI.transfer(0x00);   //Read SPDR register, consider changing to assembly code if this doesn't work.
  }

  digitalWrite(_CS_pin, HIGH);
  SPI.endTransaction();

  if(_Input_buffer_24.ui8[2] > 0x7F)                //ADS1255 Positive full scale is 0x7FFFFF, LSB=2*Vref/(PGA(2^23-1)), Binary Two Complement format
    _Input_buffer_24.ui8[3] = 0xFF;                 //This byte is only used to signal exceeding full scale...
  else
    _Input_buffer_24.ui8[3] = 0x00;                 //... and when it doesn't exceed full scale
  
  interrupts();
  return _Input_buffer_24.ui32;                     //Buffer Format = Exceeding Full scale flag (either 00 or FF)|MSB (up to 7F)|Middle Byte|LSB Data
}

