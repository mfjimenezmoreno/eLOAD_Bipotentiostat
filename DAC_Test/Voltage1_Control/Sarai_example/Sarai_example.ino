#include <Wire.h>

void setup() {
  Wire.begin();       // join i2c bus 
  // initialize serial:
  Serial1.begin(9600);
  //All expander outputs OFF
  for (byte i=0x20;i<0x28;i++)
  {
    Wire.beginTransmission(i);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
    //Serial1.println(i, HEX);
  }
  pinMode(A2, OUTPUT);
  analogWrite(A2, 00);
}

void loop() {
// if there's any serial available, read it:
  while (Serial1.available() > 0) {
    
    // look for the next valid integer in the incoming serial stream:
    byte EXPANDER_ADDR = Serial1.parseInt(); // 8575 ADDR, from 0x20 to 0x27 
    //Serial1.println(EXPANDER_ADDR, HEX);
    // do it again:
    uint16_t data = Serial1.parseInt();
    //Serial1.println(data, HEX);

   
     // look for the newline. That's the end of your
     // sentence:
     if (Serial1.read() == '\n') {
      
        Wire.beginTransmission(EXPANDER_ADDR);
        byte low = data & 0x00FF;
        byte high = data >> 8;
        Wire.write(low);
        Wire.write(high);
        Wire.endTransmission();
  
       // print the three numbers in one string as hexadecimal:
       Serial1.print(EXPANDER_ADDR, HEX);
       Serial1.print(",");
       Serial1.print(data, HEX);
     }
  }
}

