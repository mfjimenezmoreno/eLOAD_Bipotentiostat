
volatile unsigned int x = 0;
unsigned int y = 0;

void setup()
{
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
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    noInterrupts();
    /*Set up CTC mode by setting WGM bits in TCCR1X registers*/
    TCCR1A = 0;                             //Normal operation of OCRA pin, WGM 0 and 1 are left in LOW
    TCCR1B = 0;                             //Ensure get rid of default settings.
    TCCR1B |= (1 << WGM12)|(1 << CS12);     //WGM 2 is set HIGH --> CTC mode, Preescaler to 256
    /*Let's setup a 2 Hz timer*/
    TCNT1 = 0;                              //Initialize Timer
    OCR1A = 15624;
    TIFR1 = (1 << OCF1A);                    //Clear the compare flag
    TIMSK1 |= (1 << OCIE1A);                //Enable interrupt by compare match
    interrupts();                           //Activate Global Interrupts
    
}

ISR(TIMER1_COMPA_vect)
{
    digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN)^1);
    //Serial.println(digitalRead(PORTB), BIN);
    x++;
    //TIFR1 = (1 << OCF1A);     Esto no es necesario, supuestamente sucede automáticamente al ejecutar la interrupción.
}

void loop()
{
    /*El error que cometí estaba relacioando con correr setup en cada iteración!*/
    Serial.print("Reloj: ");
    Serial.println(TCNT1,DEC);
    Serial.print("ISR: ");
    Serial.println(x,DEC);
    Serial.print("Compare FLAG: ");
    //Serial.println(TIFR1 & (1 << OCF1A), BIN);
    Serial.println(bitRead(TIFR1, OCF1A), BIN);
    Serial.print("Overflow FLAG: ");
    Serial.println(bitRead(TIFR1, TOV1), BIN);
}
