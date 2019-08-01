# 1 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino"
# 1 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino"
////////////////////////////////////
//          Cheat Sheet           //
////////////////////////////////////
/*

 *   /-----|

 *  /      |

 *  |      |== GND  ~   ~~~~~~~~~

 *  |      |-- A4   ~   Analog_Switch_1(MAX4737)  -Vin switch

 *  |      |-- A3   ~   Analog_Switch_2(MAX4737)  CE-RE short (Not implemented in this version)

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

\ *   \-----|

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
# 43 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino"
////////////////////////////////////
//   Libraries and definitions    //
////////////////////////////////////

# 48 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino" 2
# 49 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino" 2
# 50 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino" 2
# 51 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino" 2
# 52 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino" 2

SPISettings SPI_Holi(19000000, 1, 0x04);
/*Chip Selects*/






/*Interrupts*/



/*Switches*/





/*Gain cases and MUX pins*/




/*Others*/


const float iVoltage = 0; //In milivolts vs. 1.5 vref
const float V_Res = 178.8E-9; //Resolution with a ADS_PGA_2

////////////////////////////////////
//         Declarations           //
////////////////////////////////////
union Holi {
    uint8_t ui8[4];
    uint32_t ui32;
};

union Holi x;

volatile boolean bUpdate = false; //An ISR updated flag
volatile boolean Ramp = true;
uint16_t iIndex = ceil(iVoltage*(65536/(double)3000)+32768); //Voltage index
max5443 DAC1(10 /*Must check if these declarations are correct*/);
max5443 DAC2(5 /*IDEM*/);
max4617 Gain(4, 12, 6);
max4737 A_Switch(A4, A2, A1, A0);
ads1255 ADC1(8, 3);
ads1255 ADC2(9, 2);

////////////////////////////////////
//           Main Code            //
////////////////////////////////////
void setup()
{
    //Serial.begin(9600);
    //Initialize SPI chip select pins as outputs and High/Unselected state
    Serial.begin(9600);
    DAC1.pins_init();
    DAC2.pins_init();
    Gain.pins_init();
    A_Switch.pins_init();
    ADC1.pins_init();
    ADC2.pins_init();
    pinMode(13, 0x1);
    digitalWrite(13, 0x0);
    /*Let's setup a 0.5 Hz CTC timer, the idea is to change voltage with every interrupt call*/
    __asm__ __volatile__ ("cli" ::: "memory");
    (*(volatile uint8_t *)(0x80)) = 0; //Normal operation of OCRA pin, WGM 0 and 1 are left in LOW
    (*(volatile uint8_t *)(0x81)) = 0; //Ensure get rid of default settings.
    (*(volatile uint8_t *)(0x81)) |= (1 << 3) | (1 << 2); //WGM 2 is set HIGH --> CTC mode, Preescaler to 1024
    (*(volatile uint16_t *)(0x84)) = 0; //Initialize Timer
    (*(volatile uint16_t *)(0x88)) = 15624; //Setup the timer ceiling/TOP
    (*(volatile uint8_t *)((0x16) + 0x20)) = (1 << 1); //Clear the compare flag (Might be redundant)
    (*(volatile uint8_t *)(0x6F)) |= (1 << 1); //Enable interrupt by compare match
    /*  INSTRUCTIONS: How Set_Voltage works

    The parameter is specified in index value from 0 to 65535, which corresponds an absolute voltage of [0, 3) volts respectively.

    However, TAKE NOTE that the analog circuitry is referred to 1.5 volts, meaning that the applied Working electrode voltage

    should be [-1.5, 1.5) respectively. Here is a helpful table:

    DAC Index value     Absolute voltage    WE Voltage

    0                   0                   -1.5    V

    1                   45.776  uV          -1.4999.V           WE_Voltage = (3 Volts)*DAC_Index/(65536) - (1.5 Volts)

    32768               1.5     V           0       V

    65535               2.999...V           1.4999..V

    */
# 138 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino"
    DAC1.set_voltage(iIndex); //Start from 1.5 volts (Abs) | 0 Volts (WE)
    DAC2.set_voltage(iIndex);
    Gain.set_gain_30k(); //Gain are chosen as 300K at both working electrodes
    A_Switch.Mode(1 /*Only one WE*/); //sSwitches activated as amperometric single-mode
    ADC1.mux(0x08 /*AIN0 as positive and AINCOM as negative*/);
    ADC2.mux(0x08 /*AIN0 as positive and AINCOM as negative*/);
    ADC1.setup_reg(0b00000010 /*0x2*/, 0b01110010 /*0x72*/, 0b001); //Setup ADC without filter, Data Rate: 60 SPS, Gain:2
    ADC1.sync(); //Sync A/D conversion
    ADC1.wakeup(); //Follow up to sync according to specifications
    ADC2.sync();
    ADC2.wakeup();
    __asm__ __volatile__ ("sei" ::: "memory"); //Activate interrupts globally

}

extern "C" void __vector_17 /* Timer/Counter1 Compare Match A */ (void) __attribute__ ((signal,used, externally_visible)) ; void __vector_17 /* Timer/Counter1 Compare Match A */ (void)
{

    bUpdate = false; //Flag update
    digitalWrite(13, digitalRead(13)^1);
    x.ui32 = ADC1.read_single24();
    /*ADC1.wakeup();

    

    x.ui16 = 0; //Initialize the data that shall be returned

    digitalWrite(Chip_Select_ADC1, LOW);

    SPI.beginTransaction(SPI_Holi);

    SPI.transfer(ADS_RDATA); //Read a single conversion result

    delayMicroseconds(6.5);  //Mandatory to wait 50 times 1/CLKin, theoretically doesn't use timer0 so interrupt should be safe

    

    for (int i = 1; i >= 0; --i)

    {

        //while(!bitRead(SPSR, SPIF));     //Wait for any pending transmission/reception to complete (Stops only if SPIF is set), maybe not required with the SPI library, Mr.Dryden seem to use USART

        x.ui8[i] = SPI.transfer(0x00); //Read SPDR register, consider changing to assembly code if this doesn't work.

    }

    

    digitalWrite(Chip_Select_ADC1, HIGH);

    SPI.endTransaction();

    //ADC1.standby();                 //Issue a standby mode.

*/
# 177 "c:\\Users\\Martin\\Documents\\Arduino\\Bipotentiostat_Main_v2\\ADC_Test\\Library_Test\\ADC_Test\\ADC_Test.ino"
}

void loop()
{
    if(Serial){
        int32_t y = (V_Res * (~(x.ui32 - 1) & 0x00FFFFFF));
        y = y;
        Serial.println("--------");
        Serial.println(V_Res,10);
        Serial.print(iVoltage,10);
        Serial.println(" mV");
        Serial.print("iVoltage = ");
        Serial.println(iIndex);
        if(x.ui8[3] == 0xFF){

            Serial.print("!");
            Serial.println(x.ui32, 16);
            Serial.print("0x");
            Serial.println(x.ui32 & 0x00FFFFFF, 16);
            Serial.println(~(x.ui32 - 1) & 0x00FFFFFF, 16);
            Serial.print("-");
            Serial.println(~(x.ui32 - 1) & 0x00FFFFFF, 10);
            Serial.print("-");
            Serial.print(V_Res * (~(x.ui32 - 1) & 0x00FFFFFF), 10);
            Serial.println(" V");
            Serial.print(y, 8);
            Serial.println(" V");
        }
        else{
            Serial.print("!");
            Serial.println(x.ui32, 16);
            Serial.print("+");
            Serial.println(x.ui32, 10);
            Serial.print("+");
            Serial.print(V_Res * x.ui32, 10);
            Serial.println(" V");
        }
    }
    if(bUpdate == true) //Only executed when Timer1 matches OCR1A
    {
        if(Ramp == true)
        {
            iIndex += 219; //Add around 10 mV
        }
        else
        {
            iIndex -= 219;
        }

        if(iIndex > 65316) //Flip direction Cathodicallly
        {
            Ramp = false;
            iIndex = 65535;
        }

        if(iIndex < 219) //Flip the direction Anodically
        {
            Ramp = true;
            iIndex = 0;
        }
        //Serial.println(iIndex,DEC);
        DAC1.set_voltage(iIndex);
        DAC2.set_voltage(iIndex);
        bUpdate = false; //...and clear the flag

    }
}
