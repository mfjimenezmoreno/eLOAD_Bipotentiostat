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
#include <Math.h>

const float Arduino_Frequency = 16E6;

void T1_Frequency(float Frequency)
{   
    //Select the preescaler according to the expected Frequency
    //Considering the discrete nature of 16 bit timer, must be careful with the frequency selection
    int Preescaler = 1;
    int CTC = 0;

    TCCR1A = 0; //Normal operation of OCRA pin, WGM 0 and 1 are left in LOW
    TCCR1B = 0; //Ensure get rid of default settings.

    if(Frequency <= 8E6 && Frequency >= 256)    //High frequencies might have truncation issues
    {
        Preescaler = 1;
        TCCR1A = 0;                                         //WGM10 WGM11: LOW
        TCCR1B |= (1 << WGM12) | (1 << CS10);               //WGM 2 is set HIGH --> CTC mode, Preescaler to 1
    }
    if (Frequency < 256 && Frequency >= 32)
    {
        Preescaler = 8;
        TCCR1A = 0;                                         //WGM10 WGM11: LOW
        TCCR1B |= (1 << WGM12) | (1 << CS11);               //WGM 2 is set HIGH --> CTC mode, Preescaler to 8
    }
    if (Frequency < 32  && Frequency >= 4)
    {
        Preescaler = 64;
        TCCR1A = 0;                                         //WGM10 WGM11: LOW
        TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10); //WGM 2 is set HIGH --> CTC mode, Preescaler to 64
    }
    if (Frequency < 4  && Frequency >= 1)
    {
        Preescaler = 256;
        TCCR1B |= (1 << WGM12) | (1 << CS12);               //WGM 2 is set HIGH --> CTC mode, Preescaler to 256
    }
    if (Frequency < 1 && Frequency >= 0.25)
    {
        Preescaler = 1024;
        TCCR1A = 0;                                         //WGM10 WGM11: LOW
        TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); //WGM 2 is set HIGH --> CTC mode, Preescaler to 1024
    }

    //Determine the value of CTC register according to Preescaler
    CTC = int(round(Arduino_Frequency/Preescaler/Frequency));
    
    
    /*Let's setup a 2 Hz timer*/
    TCNT1 = 0;                              //Initialize Timer
    OCR1A = CTC;                            //Set the compare register
    TIFR1 = (1 << OCF1A);                   //Clear the compare flag
    
}

T1_EN_IntCTC(void)
{
    //Enables the register: interrupt by compare match
    bitSet(TIMSK1, OCIE1A); //Enable interrupt by compare match
}

T1_DIS_IntCTC(void)
{
    //Disables the register: interrupt by compare match
    bitClear(TIMSK1, OCIE1A); //Enable interrupt by compare match
}