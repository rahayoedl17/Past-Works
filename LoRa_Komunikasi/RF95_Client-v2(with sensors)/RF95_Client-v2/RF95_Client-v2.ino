#include <SPI.h>
#include <RH_RF95.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define RFM95_CS 10
#define RFM95_RST 7
#define RFM95_INT 2
#define RF95_FREQ 915.0

#define ONE_WIRE_BUS 3    // -------- Suhu
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
//--------------------------------------- pH -------------------------------------------
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex = 0;
static float pHValue;
static float temprValue;
//------------------------------------- LoRa -------------------------------------------
RH_RF95 rf95(RFM95_CS, RFM95_INT);
// suhu, kekeruhan, pH, TDS
char sensor1[10], sensor2[6], sensor3[6], sensor4[5], tempVal[10];
long previousMillis = 0;
long interval = 10000;


void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  while (!Serial);
  Serial.begin(9600);
  delay(100);

  Serial.println("Arduino LoRa TX Test!");
  Serial.println("Sensor Suhu Starting.."); sensors.begin();
  Serial.println("Sensor TDS Starting.."); pinMode(TdsSensorPin, INPUT);
  Serial.println("Sensor Kekeruhan Starting..");
  Serial.println("Sensor pH Starting..");

  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);
  delay(3000);
}

void loop()
{
  getSensorData();
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    Serial.println("Sending to rf95_server");

    dtostrf(tdsValue, 4, 0, tempVal);   sprintf(sensor1, "%s", tempVal); memset(tempVal, 0, 10);
    dtostrf(temprValue, 4, 2, tempVal); sprintf(sensor2, "%s", tempVal); memset(tempVal, 0, 10);
    dtostrf(kekeruhan, 3, 2, tempVal);  sprintf(sensor3, "%s", tempVal); memset(tempVal, 0, 10);
    dtostrf(pHValue, 2, 2, tempVal);    sprintf(sensor4, "%s", tempVal); memset(tempVal, 0, 10);

    //strcpy(sensor1, "555"); strcpy(sensor2, "555"); strcpy(sensor3, "555"); strcpy(sensor4, "555");

    char radiodata[30];
    strcpy(radiodata, "Node2;");
    strcat(radiodata, sensor1); strcat(radiodata, ";");
    strcat(radiodata, sensor2); strcat(radiodata, ";");
    strcat(radiodata, sensor3); strcat(radiodata, ";");
    strcat(radiodata, sensor4); strcat(radiodata, "#");

    Serial.print("Sending: "); Serial.println(radiodata);
    delay(10);
    rf95.send((uint8_t*)radiodata, strlen(radiodata) + 1);
    Serial.println("Waiting for packet to complete...");
    delay(10);
    rf95.waitPacketSent();

    // Now wait for a reply
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    Serial.println("Waiting for reply...");
    delay(10);
    if (rf95.waitAvailableTimeout(1000))
    {
      // Should be a reply message for us now
      if (rf95.recv(buf, &len))
      {
        Serial.print("Got reply: "); Serial.println((char*)buf);
        Serial.print("RSSI: "); Serial.println(rf95.lastRssi(), DEC);
      }
      else { Serial.println("Receive failed"); }
    }
    else { Serial.println("No reply, is there a listener around?"); }
    
    previousMillis = currentMillis;
  }


}

void getSensorData() {
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
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient; //temperature compensation
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
    //Serial.print("voltage:"); Serial.print(averageVoltage, 2);Serial.print("V   ");
    Serial.print("TDS Value:");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");

    // ---------------------------------------------------------------------------------
    // --------------------------------- SENSOR SUHU -----------------------------------
    // ---------------------------------------------------------------------------------
    sensors.requestTemperatures();
    temprValue = sensors.getTempCByIndex(0);
    Serial.print("SUHU = "); Serial.println(temprValue); //

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
  static float voltage;
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
    //Serial.print("Voltage:"); Serial.print(voltage, 2);
    //Serial.print("    pH value: "); Serial.println(pHValue, 2);
    Serial.print("pH value: "); Serial.println(pHValue, 2);
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

double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0) {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number < 5) { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  } else {
    if (arr[0] < arr[1]) {
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    }
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {
        amount += min;      //arr<min
        min = arr[i];
      } else {
        if (arr[i] > max) {
          amount += max;  //arr>max
          max = arr[i];
        } else {
          amount += arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount / (number - 2);
  }//if
  return avg;
}
