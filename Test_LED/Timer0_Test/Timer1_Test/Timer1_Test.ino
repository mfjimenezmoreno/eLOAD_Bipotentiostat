void setup()
{
    /*Related registers for Timer 1 control (16 bits) 
   * TCNT1H & TCNT1L: Timer registers high and low byte.
   * TCCR1A: Timer/Counter control register A. (Do not touch for this application)
   * └──> WGM01/WGM00: For wave generation mode (Not used)
   * └──> COMnB1/COMnB0: For control of compare output pin (Not used)
   * └──> COMnA1/COMnA0: For control of compare output pin (Not used)
   * TCCR1B: Timer/Counter control register B.
   * └──> ICNC1:            Input capture noise canceller (not used)
   * └──> ICES1:            Input capture edge select (not used)
   * └──> WGM13/WGM12:      Wave generation mode (not used)
   * └──> CS12/CS11/CS10:   Clock Select
   *    └──>000 Timer/Counter stopped   011 Div by 64       1XX External clock related (not used)
   *    └──>001 No preescaler           100 Div by 256
   *    └──>010 Div by 8                101 Div by 1024
   * TCCR1C: Timer/Counter control register C (not used).
   *  └──> FOC1A/FOC1B/FOC1C: For output compare mode (not used)
   * OCR1AH & OCR1AL: Output compare register with TCNT1. Can generate the proper output compare interrupt OCF1x or generate waveform on pin OC0y.
   * OCR1BH & OCR1BL: Idem, can generate output to OC1x pin.
   * OCR1CH & OCR1CL: Idem.
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
    //Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    noInterrupts();
    TCCR1A = 0;                 //Non WGM mode, normal operation of OCRA pin
    TCCR1B = 0;                 //Initialization of prescaler (Timer stopped)
    /*Let's setup a 250 Hz timer*/
    TCNT1 = 3036;              //Load timer to generate a 250 Hz interrupt
    TCCR1B |= (1 << CS12);      //Activate timer, 256 preescaler (16 MHz)
    TIMSK1 |= (1 << TOIE1);     //Activate interrupt by Timer Overflow
    interrupts();               //Activate Global Interrupts
    
}

ISR(TIMER1_OVF_vect)
{
    digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN)^1);
    TCNT1 = 3036;
}

void loop()
{
    Serial.print("Timer 1 counter: ");
    Serial.println(TCNT1);
    Serial.print("TCCR1B register: ");
    Serial.println(TCCR1B, BIN);
}