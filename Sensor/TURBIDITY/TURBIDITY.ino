static float kekeruhan;
static float voltage;
void setup(){
Serial.begin(9600);
}
void loop() {
int sensorValue = analogRead(A3);// read the input on analog pin 0:
voltage = sensorValue*(5.0/1023.0);
kekeruhan = 100.00 -(voltage/5.0)*100.00;
//kekeruhan = ( voltage - 912.5) / -0.279;
//Serial.print("Tegangan = ");
//Serial.print(voltage);
//Serial.print("     ");
Serial.print (voltage);
Serial.print("     ");
Serial.print("Nilai ADC = ");
Serial.print(sensorValue);
Serial.print("     ");
Serial.print("Nilai Kekeruhan = ");
Serial.print(kekeruhan);
Serial.println(" NTU ");
delay(1000);
}



