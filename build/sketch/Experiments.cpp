/*///////////////////////////////////////*/
/*      Experiments related library      */
/*///////////////////////////////////////*/

#include <C:\Users\Martin\Documents\Arduino\Bipotentiostat_Main_v2\Bipot_Main_v01\Experiments.h>

/*************************/
/*         Code          */
/*************************/

void set_timer1_frequency(double Frequency)
{

    //Select the preescaler according to the expected Frequency
    //Considering the discrete nature of 16 bit timer, must be careful with the frequency selection

    /*Related registers for Timer 1 control (16 bits) 
    * TCNT1H & TCNT1L: Timer registers high and low byte.
    * TCCR1A: Timer/Counter control register A.
    * └──> WGM01/WGM00: Setup CTC mode.
    * └──> COMnB1/COMnB0: For control of compare output pin (Not used)
    * └──> COMnA1/COMnA0: For control of compare output pin (Not used)
    * TCCR1B: Timer/Counter control register B.
    * └──> ICNC1:            Input capture noise canceller (not used)
    * └──> ICES1:            Input capture edge select (not used)
    * └──> WGM13/WGM12:      Setup CTC mode.
    * 
    * WGM mode bits: These configure the compare register into PWM or compare mode. In this case we might be only interested in CTC.
    * 3      2(CTC)  1(PWM)  0(PWM)
    * 0      0       0       0           Normal
    * 0      0       1       1           PWM, Phase corrected, 10-bit
    * 0      1       0       0           CTC mode
    * 1      0       0       0           PWM, Phase and Frequency corrected
    * 1      1       1       0           Fast PWM
    * 
    * └──> CS12/CS11/CS10:   Clock Select
    *    └──>000 Timer/Counter stopped   011 Div by 64       1XX External clock related (not used)
    *    └──>001 No preescaler           100 Div by 256
    *    └──>010 Div by 8                101 Div by 1024
    * TCCR1C: Timer/Counter control register C (not used).
    *  └──> FOC1A/FOC1B/FOC1C: For output compare mode (not used)
    * OCR1AH & OCR1AL: Output compare register with TCNT1. Can generate the proper output compare interrupt OCF1x or generate waveform on pin OC0y.
    * OCR1BH & OCR1BL: Idem, can generate output to OC1x pin.
    * OCR1CH & OCR1CL: Idem.C
    * ICR1H & ICR1L: Input Capture Register. It updates with TCNT1 each time an event occurs at ICPn pin.
    * TIFRx: Timer interrupt flag register (pending timer interrupt).
    *  └──> TOV0: Timer/Counter0 OVERLFOW Flag. Cleared automatically when executing corresponding ISR (that is, assuming interrupt is enabled).
    * TIMSK1: Timer interrupt mask register.
    *  └──> ICIE1: Input capture interrupt enable (not used).
    *  └──> TOIE1: Timer OVERFLOW interrupt ENABLE bit. If this is set, ISR(Timerx_OVF_vect) will be called.
    *  └──> OCIE1A/OCIE1B/OCIE1C: Timer/Counter compare match interrupt ENABLE.
    * TIFR1: Timer/Counter Interrupt flag register.
    *  └──> ICF1: Event flag for pin ICP1.
    *  └──> OCF1C/OCF1B/OCF1A: When there is a match of output compare register OCR1x with TCNT1.
    *  └──> TOV1: Timer/Counter Overflow flag. Cleared automatically if interrupt vector is executed.
    */

    int Preescaler = 1;
    int CTC = 0;

    TCCR1A = 0; //Normal operation of OCRA pin, WGM 0 and 1 are left in LOW
    TCCR1B = 0; //Ensure get rid of default settings.

    if (Frequency <= 8E6 && Frequency >= 256) //High frequencies might have truncation issues
    {
        Preescaler = 1;
        TCCR1A = 0;                           //WGM10 WGM11: LOW
        TCCR1B |= (1 << WGM12) | (1 << CS10); //WGM 2 is set HIGH --> CTC mode, Preescaler to 1
    }
    if (Frequency < 256 && Frequency >= 32)
    {
        Preescaler = 8;
        TCCR1A = 0;                           //WGM10 WGM11: LOW
        TCCR1B |= (1 << WGM12) | (1 << CS11); //WGM 2 is set HIGH --> CTC mode, Preescaler to 8
    }
    if (Frequency < 32 && Frequency >= 4)
    {
        Preescaler = 64;
        TCCR1A = 0;                                         //WGM10 WGM11: LOW
        TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10); //WGM 2 is set HIGH --> CTC mode, Preescaler to 64
    }
    if (Frequency < 4 && Frequency >= 1)
    {
        Preescaler = 256;
        TCCR1B |= (1 << WGM12) | (1 << CS12); //WGM 2 is set HIGH --> CTC mode, Preescaler to 256
    }
    if (Frequency < 1 && Frequency >= 0.25)
    {
        Preescaler = 1024;
        TCCR1A = 0;                                         //WGM10 WGM11: LOW
        TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); //WGM 2 is set HIGH --> CTC mode, Preescaler to 1024
    }

    //Determine the value of CTC register according to Preescaler
    CTC = int(round(Arduino_Frequency / Preescaler / Frequency));

    /*Let's setup a 2 Hz timer*/
    TCNT1 = 0;            //Initialize Timer
    OCR1A = CTC;          //Set the compare register
    TIFR1 = (1 << OCF1A); //Clear the compare flag
}

void timer1_EN_IntCTC(void)
{
    //Enables the register: interrupt by compare match
    bitSet(TIMSK1, OCIE1A); //Enable interrupt by compare match
}

void timer1_DIS_IntCTC(void)
{
    //Disables the register: interrupt by compare match
    bitClear(TIMSK1, OCIE1A); //Enable interrupt by compare match
}

void pot_init(void)
{
    /*
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
    * MUX_B_Gain         (MAX4617)   12    B               HIGH  |--This combination sets up transimpedance gain as 100 ohms
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
    /*Analog switches as digital outputs...*/
    pinMode(Analog_Switch_VIN, OUTPUT);
    pinMode(Analog_Switch_CERE, OUTPUT);
    pinMode(Analog_Switch_CE, OUTPUT);
    pinMode(Analog_Switch_WE1, OUTPUT);
    pinMode(Analog_Switch_WE2, OUTPUT);
    /*... and set their initial state*/
    digitalWrite(Analog_Switch_VIN, LOW);
    digitalWrite(Analog_Switch_CERE, HIGH);
    digitalWrite(Analog_Switch_CE, LOW);
    digitalWrite(Analog_Switch_WE1, LOW);
    digitalWrite(Analog_Switch_WE2, LOW);
}

void pot_set_gain(uint8_t gain)
{
    /*
   * Sets the multiplexer values according to the specified gain value.
   * 
   * @param   Gain      Definitions are defined on the header file
   * @return  Nothing
   */

    switch (gain)
    {
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