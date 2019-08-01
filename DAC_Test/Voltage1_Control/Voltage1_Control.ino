////////////////////////////////////
//   Libraries and definitions    //
////////////////////////////////////

#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\DAC_Test\Voltage1_Control\MAX5443.h>
#define Chip_Select_ADC1    8
#define Chip_Select_ADC2    9
#define Chip_Select_DAC1    10          //Must check if these declarations are correct
#define Chip_Select_DAC2    5           //IDEM
#define Chip_Select_SD      11

#define Anodic              true
#define Cathodic            false
////////////////////////////////////
//         Declarations           //
////////////////////////////////////

volatile boolean    bUpdate = false;        //An ISR updated flag
volatile boolean    Ramp = Anodic;
uint16_t            iIndex  = 32768;            //Voltage index
max5443 DAC1(Chip_Select_DAC1);
max5443 DAC2(Chip_Select_DAC2);

////////////////////////////////////
//           Main Code            //
////////////////////////////////////
void setup()
{
    //Serial.begin(9600);
    //Initialize SPI chip select pins as outputs and High/Unselected state
    DAC1.pins_init();
    DAC2.pins_init();
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    /*Let's setup a 0.5 Hz CTC timer, the idea is to change voltage with every interrupt call*/
    noInterrupts();
    TCCR1A = 0;                                                 //Normal operation of OCRA pin, WGM 0 and 1 are left in LOW
    TCCR1B = 0;                                                 //Ensure get rid of default settings.
    TCCR1B |= (1 << WGM12) | (1 << CS12);                       //WGM 2 is set HIGH --> CTC mode, Preescaler to 1024
    TCNT1 = 0;                                                  //Initialize Timer
    OCR1A = 15624;                                              //Setup the timer ceiling/TOP
    TIFR1 = (1 << OCF1A);                                       //Clear the compare flag (Might be redundant)
    TIMSK1 |= (1 << OCIE1A);                                    //Enable interrupt by compare match
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
    
    DAC1.pins_init();
    DAC2.pins_init();
    DAC1.set_voltage(iIndex);                                    //Start from 1.5 volts (Abs) | 0 Volts (WE)
    DAC2.set_voltage(iIndex);
    interrupts();                                               //Activate interrupts globally
    
}

ISR(TIMER1_COMPA_vect)
{
    bUpdate = true;                                             //Flag update
    digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN)^1);
}

void loop()
{
    if(bUpdate == true)                                         //Only executed when Timer1 matches OCR1A
    {
        if(Ramp == Anodic)
        {
            iIndex += 219;                                      //Add around 10 mV
        }
        else
        {
            iIndex -= 219;
        }

        if(iIndex > 65316)                                      //Flip direction Cathodicallly
        {
            Ramp = Cathodic;
            iIndex = 65535;
        }
        
        if(iIndex < 219)                                        //Flip the direction Anodically
        {
            Ramp = Anodic;
            iIndex = 0;
        }
        //Serial.println(iIndex,DEC);
        DAC1.set_voltage(iIndex);
        DAC2.set_voltage(iIndex);
        bUpdate = false;                                        //...and clear the flag
    }
}
