#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

void setup(void) 
{ 
 // start serial port 
 Serial.begin(9600); 
 Serial.println("Sensor Suhu SIAP!!!"); 
 sensors.begin(); 
} 
void loop(void) 
{ 
 Serial.print(""); 
 sensors.requestTemperatures();
 Serial.println(""); 
 Serial.print("SUHU = "); 
 Serial.print(sensors.getTempCByIndex(0)); //  
   delay(1000); 
} 
