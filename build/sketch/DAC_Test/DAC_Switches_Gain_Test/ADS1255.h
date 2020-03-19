/*///////////////////////////////////////*/
/*          ADC related class            */
/*///////////////////////////////////////*/

/*
 * Library for ADC communication, handles pin configuration, ADC setup, control and readout
 */

#ifndef ADS1255_h
#define ADS1255_h
#include <Arduino.h>

//Commands (Datasheet page 34). Note: not included RREG and WREG for read/write registers
#define ADS_WAKEUP      0x00
#define ADS_RDATA       0x01
#define ADS_RDATAC      0x03
#define ADS_SDATAC      0x0F
#define ADS_SELFCAL     0xF0
#define ADS_SELFOCAL    0xF1
#define ADS_SELFGCAL    0xF2
#define ADS_SYSOCAL     0xF3
#define ADS_SYSGCAL     0xF4
#define ADS_SYNC        0xFC
#define ADS_STANDBY     0xFD
#define ADS_RESET       0xFE

//DRATE(0x03) Register, data rate in SPS (Datasheet page 32)
#define ADS_DR_2_5      0b00000011 //0x03
#define ADS_DR_5        0b00010011 //0x13
#define ADS_DR_10       0b00100011 //0x23
#define ADS_DR_15       0b00110011 //0x33
#define ADS_DR_25       0b01000011 //0x43
#define ADS_DR_30       0b01010011 //0x53
#define ADS_DR_50       0b01100011 //0x63
#define ADS_DR_60       0b01110010 //0x72
#define ADS_DR_100      0b10000010 //0x82
#define ADS_DR_500      0b10010010 //0x92
#define ADS_DR_1000     0b10100001 //0xA1
#define ADS_DR_2000     0b10110000 //0xB0
#define ADS_DR_3750     0b11000000 //0xC0
#define ADS_DR_7500     0b11010000 //0xD0
#define ADS_DR_15000    0b11100000 //0xE0
#define ADS_DR_30000    0b11110000 //0xF0

//ADCON Register, PGA setting (Datasheet page 31). Note: corresponds to the 3 least significant bits
#define ADS_PGA_1       0b000
#define ADS_PGA_2       0b001
#define ADS_PGA_4       0b010
#define ADS_PGA_8       0b011
#define ADS_PGA_16      0b100
#define ADS_PGA_32      0b101
#define ADS_PGA_64      0b110

//STATUS Register, analog input buffer enable or disable
#define ADS_BUFF_OFF    0b00000000 //0x0
#define ADS_BUFF_ON     0b00000010 //0x2

//MUX register, changes the setup if it works in pot or volt mode
#define ADS_MUX_VOLT    0x08    //AIN0 as positive and AINCOM as negative
#define ADS_MUX_POT     0x18    //AIN1 as positive and AINCOM as negative

class ads1255{
  public:
    ads1255(int Chip_Select, int DRDY);     //Constructor, which includes the CS and ~DRDY pins.
    void pins_init(void);                   //Pin initilization (CS pin and DRDY interrupt) as specified in the constructor. Must be used.
    void sync(void);                        //For A/D conversion synchronization. Note: Must issue wakeup afterwards.
    void reg_read(uint8_t address);         //Reads a register in a manual fashion (see register map in datasheet's page 30)
    void reset();                           //Reset ADS1255 to power-up values.
    void setup_reg(uint8_t buff, uint8_t rate, uint8_t pga);  //Setups specifically buff, rate and pga registers. Read comments for information.
    uint16_t sample_delay_ms_100div = 0;    //Read only, specifies samples per second. Changes automatically with setup_reg method.
    uint8_t input_buffer[5];                //Reads the data from 5 registers at ADS1255, from reg_read method.
    void mux(uint8_t channel);              //Setup the input MUX register. Designs these inputs as Positive, Negative or Common. Read comments for information.
    void standby(void);                     //Enter standbymode, shuts down analog circuitry.
    void wakeup(void);                      //Exits standymode or finished synchronization.
    void rdatac(void);                      //Request ADS1255 to continously read data, use read fast/fast24 for readout
    int16_t read_fast(void);                //For reading data when continous operation
    int16_t read_fast_single(void);         //For reading data on-demand
    int32_t read_fast24(void);              //For reading data when continous operation
    int32_t read_single24(void);            //For reading data on-demand
     
  private:
    int _CS_pin;                        //Chip Select pin
    int _DRDY_pin;                      //~DRDY pin
    uint8_t _command_buffer[6];
    static union Buffer{
      uint8_t ui8[2];
      uint16_t ui16;
    }_ADC_buffer, _Input_buffer;        //16 bit Buffers
    static union Buffer32{
      uint8_t ui8[4];
      uint32_t ui32;
    }_Input_buffer_24;                  //32 bit buffer for readout
};

#endif
