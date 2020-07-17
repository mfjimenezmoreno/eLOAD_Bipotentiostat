//USB serial mirror
//String class management

void setup()
{
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    //Serial.begin(9600);
    Serial1.begin(9600);
    Serial1.setTimeout(1000);
}

char readout, readout1;
String message, message1;
uint8_t x = 0;

// the loop function runs over and over again forever
void loop()
{/*
    if(Serial.available()){
        digitalWrite(LED_BUILTIN, HIGH);
        readout = Serial.read();
        if(readout == 0x0A){            //When dot is read, data is sent and reset
            message.concat(readout);    //Concatenate end of line character
            Serial.print(message);
            Serial1.print((message));
            message = "";
        }
        else{
            message.concat(readout);
        }
    }

    if(Serial1.available())
    {
        readout1 = Serial1.read();
        if (readout1 == 0x0A)
        {                            //When dot is read, data is sent and reset
            message.concat(readout1); //Concatenate end of line character
            Serial.print((message1));
            message1 = "";
        }
        else
        {
            message.concat(readout1);
        }
    }
    digitalWrite(LED_BUILTIN, LOW);*/

    if(Serial1.available()){
        //readout1 = Serial1.read();
        x++;
        message1 = Serial1.readStringUntil('.');
        Serial1.println(x);
    }
}
