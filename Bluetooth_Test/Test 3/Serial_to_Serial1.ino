//USB serial mirror
//String class management

void setup()
{
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
    //Serial1.begin(9600);
}

char readout;
String message;

// the loop function runs over and over again forever
void loop()
{
    if (Serial.available())
    {
        readout = Serial.read();
        if (readout == '.')
        { //When dot is read, data is sent and reset
            Serial.print((message));
            message = "";
        }
        else
        {

            message.concat(readout);
        }
        digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
        delay(100);                     // wait for a second
    }
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(100);                      // wait for a second
}
