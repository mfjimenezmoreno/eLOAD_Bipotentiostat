#include <SoftwareSerial.h>
#include <SPI.h>

 
String collecter="";
float receivedCurrent=0;
float receivedTemp=0;
int Pump=0;

const int slaveSelectPin=11;
 
void setup() 
{
  SPI.begin();
  pinMode(slaveSelectPin, OUTPUT);
  
  Serial.begin(9600);
  Serial1.begin(9600);
  
}
 
void loop()
{
     // Read from the Bluetooth module and turn the LED on and off
    while (Serial1.available()>0)
    {   
      collecter = Serial1.readString();
      Serial.println(collecter);
    if(collecter.indexOf("c")>= 0)
      {
      collecter.remove(0,1);
      receivedCurrent=collecter.toFloat();
      Serial.println(receivedCurrent);
      }
      if(collecter.indexOf("v")>=0)
      {
        collecter.remove(0,1);
      receivedTemp=collecter.toFloat();
      Serial.println(receivedTemp);
      }
      if(collecter.indexOf("p")>=0)
      {
        collecter.remove(0,1);
      Pump=collecter.toInt();
      Serial.println(Pump);
      }
      } 
    }    
          


