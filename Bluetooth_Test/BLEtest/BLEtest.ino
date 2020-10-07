#include <Arduino.h>

void read_serial_markers(bool &newdata, char *receivedChars, size_t size, char startmarker = '<', char endmarker = '>');
void read_number(char *value, size_t size, int16_t *output);

bool readyflag = false;
char message[20];
int16_t number;

void setup() {
    Serial1.begin(9600);
}

void loop(){
    if (readyflag == false) {
        Serial1.print(Serial1.available());
    }
    delay(1000);
    read_serial_markers(readyflag, message, sizeof(message));

    if (readyflag) {
        
        read_number(message, sizeof(message), number);

        Serial1.print(message);
        Serial1.print(number);
        
        readyflag = false;
        //Empty char variable message
        for (int i = 0; i < sizeof(message); i++) {
            message[i] = '\0'; //This is the same as '\0'
        }

    }
}

void read_serial_markers(bool &newdata, char *receivedChars, size_t size, char startmarker = '<', char endmarker = '>')
{
    /*Reads characters from buffer, identifies a starting and ending character.
    This function is non-blocking in nature.
    How to use: poll for newdata until it is true, then read data. Do not forget
    that once information is read, make new data false.
    Implication: requires that parent loop executes continiously as fast 
    as possible.*/
    static bool receptionInProgress = false;
    static uint8_t index = 0;
    char rc;

    while (Serial1.available() > 0 && newdata == false)
    {
        rc = Serial1.read();

        if (receptionInProgress == true)
        {

            //If we don't get the endmarker, then append character to receivedChars
            if (rc != endmarker)
            {
                receivedChars[index] = rc;
                index++;
                //Avoid overflow, stall the index value to the allowed maximum
                //This condition should be avoided at all costs
                if (index >= size)
                {
                    index = size - 1;
                }
            }

            //If we get the endmarker character
            else
            {
                receptionInProgress = false;
                index = 0;
                newdata = true;
            }
        }
        /*
        In the case we receive the starter marker, make our reception
        flag true and "filter out" the marker.
        */
        else if (rc == startmarker)
        {
            receptionInProgress = true;
        }
    }
}

void read_number(char *value, size_t size, int16_t *output)
{
    char number[20];
    //Return the transformation of char array to signed integer.
    for (int i = 2; i < size; i++){
        number[i-2] = value[i];
    }

    output = int16_t(number);

}