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
//uint8_t x = 0;
//uint8_t array[3];

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
    /*
    if(Serial1.available()){
        //readout1 = Serial1.read();
        x++;
        message1 = Serial1.readStringUntil('.');
        Serial1.println(x);
    }
    */

   //Sends a bytearrays with three elements
   /*
   if(Serial1){
       for(int i = 0; i < sizeof(array); i++){
           array[i] = x*(i + 1);
       }
       x++;
       x = (x==51)? 0 : x;
       Serial1.write(array, sizeof(array));
       delay(500);
   }*/

    const int elements = 64;
    const float incr_rad = 2 * PI / (float)elements;
    float t[elements] = {0};
    float x[elements] = {0};
    float y[elements] = {0};
    uint8_t data[3];

    for (int i = 0; i < elements; i++)
    {
        t[i] = i * incr_rad;
        x[i] = uint8_t(round(100.0 * (cos(t[i]) + 1.0)));
        y[i] = uint8_t(round(100.0 * (sin(t[i]) + 1.0)));
    }
    uint8_t iterator = 0;

    //Sends a byte array: parametric circle
    while(Serial1){
        data[0] = iterator;
        data[1] = x[iterator];
        data[2] = y[iterator];

        Serial1.write(data, sizeof(data));
        delay(500);
        iterator++;
        iterator = (iterator == elements)? 0 : iterator;
    }
}
