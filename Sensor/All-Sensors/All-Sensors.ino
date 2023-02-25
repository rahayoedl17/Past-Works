#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2    // -------- Suhu
#define TdsSensorPin A1   // -------- TDS
#define VREF 5.0          // analog reference voltage(Volt) of the ADC
#define SCOUNT  30        // sum of sample point
#define SensorPin A0      //--------- pH meter Analog output to Arduino Analog Input 0
#define Offset 0.11       //deviation compensate
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    // times of collection

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//------------------------------------- TDS --------------------------------------------
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;
//-------------------------------- KEKERUHAN -------------------------------------------
static float kekeruhan;
static float voltage;
//------------------------------------- pH -------------------------------------------
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Sensor Suhu Starting.."); sensors.begin();
  Serial.println("Sensor TDS Starting.."); pinMode(TdsSensorPin, INPUT);
  Serial.println("Sensor Kekeruhan Starting..");
  Serial.println("Sensor pH Starting..");

}

void loop() {
  // put your main code here, to run repeatedly:

// ---------------------------------------------------------------------------------
// ---------------------------------- SENSOR TDS -----------------------------------
// ---------------------------------------------------------------------------------
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U)  //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient; //temperature compensation
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
    Serial.print("voltage:");
    Serial.print(averageVoltage, 2);
    Serial.print("V   ");
    Serial.print("TDS Value:");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");

    // ---------------------------------------------------------------------------------
    // --------------------------------- SENSOR SUHU -----------------------------------
    // ---------------------------------------------------------------------------------
    sensors.requestTemperatures();
    Serial.print("SUHU = "); Serial.print(sensors.getTempCByIndex(0)); //

    // ---------------------------------------------------------------------------------
    // ------------------------------ SENSOR KEKERUHAN ---------------------------------
    // ---------------------------------------------------------------------------------
    int sensorValue = analogRead(A3);
    voltage = sensorValue * (5.0 / 1023.0);
    kekeruhan = 100.00 - (voltage / 5.0) * 100.00;
    
    //Serial.print(voltage); Serial.print("     "); Serial.print("Nilai ADC = "); Serial.print(sensorValue); Serial.print("     ");
    Serial.print("Nilai Kekeruhan = ");
    Serial.print(kekeruhan);
    Serial.println(" NTU ");
  }
  
// ---------------------------------------------------------------------------------
// ---------------------------------- SENSOR pH ------------------------------------
// ---------------------------------------------------------------------------------
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
    if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
    pHValue = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
  if (millis() - printTime > printInterval)  //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
    Serial.print("Voltage:"); Serial.print(voltage, 2);
    Serial.print("    pH value: "); Serial.println(pHValue, 2);
    digitalWrite(LED, digitalRead(LED) ^ 1);
    printTime = millis();
  }
}

int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
