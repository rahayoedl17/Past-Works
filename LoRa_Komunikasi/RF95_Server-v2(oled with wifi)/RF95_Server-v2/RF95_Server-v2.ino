// Arduino9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Arduino9x_TX

#include <SPI.h>
#include <RH_RF95.h>
#include <SoftwareSerial.h>

#define RFM95_CS 10
#define RFM95_RST 7
#define RFM95_INT 2
#define RF95_FREQ 915.0 // Change to 434.0 or other frequency, must match RX's freq!
RH_RF95 rf95(RFM95_CS, RFM95_INT); // Singleton instance of the radio driver

/*#define RX A1
  #define TX A2
  int countTrueCommand;
  int countTimeCommand;
  boolean found = false;
  String cmd;
  String AP = "linux";
  String PASS = "123Abc123";
  String HOST = "localhost";
  String PORT = "80";
  SoftwareSerial esp8266(RX, TX);*/

int packetnum = 0;
char str[30];
char *token1;
char *token2;
const char delim1[2] = "#";
const char delim2[2] = ";";
char *strs[5];
char *strss[5];

void setup()
{
  /*esp8266.begin(115200);
    esp8266.println("AT");
    sendCommandToESP8266("AT", 5, "OK");
    sendCommandToESP8266("AT+CWMODE=1", 5, "OK");
    sendCommandToESP8266("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");*/

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(9600);
  delay(100);
  Serial.println("Arduino LoRa RX Test!");

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
  if (rf95.available())
  {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      RH_RF95::printBuffer("Received: ", buf, len);
      packetnum++;
      Serial.print("Data ke : "); Serial.println(packetnum);
      Serial.print("Got: "); strcpy(str, (char*)buf);//str = (char*)buf;
      Serial.println(str);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply.");

      //------------------------------------------------------
      int a = 0;
      int b = 0;
      token1 = strtok(str, delim1);

      /* walk through other tokens */
      while ( token1 != NULL ) {
        strs[a++] = token1;
        Serial.println(strs[a]);

        token1 = strtok(NULL, delim1);
      }
      
      for (int i = 0; i < a; i++) {
        token2 = strtok(strs[i], delim2);
        while ( token2 != NULL ) {
          strss[b++] = token2;
          Serial.println(strss[b]);

          token2 = strtok(NULL, delim2);
        }
      }

      /*cmd = "/TugasAkhir/insertData.php?node=";
        cmd += strs[0];
        cmd += "&sensor1=";
        cmd += strs[1];
        cmd += "&sensor2=";
        cmd += strs[2];
        cmd += "&sensor3=";
        cmd += strs[3];
        cmd += "&sensor4=";
        cmd += strs[4];


        String postRequest = "GET " + cmd  + " HTTP/1.1\r\n" +
                           "Host: localhost" + ":" + PORT + "\r\n" +
                           "Connection: close\r\n\r\n";

        sendCommandToESP8266("AT+CIPMUX=1", 5, "OK");
        sendCommandToESP8266("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
        String cipSend = "AT+CIPSEND=0," + String(postRequest.length());
        sendCommandToESP8266(cipSend, 4, ">");
        sendData(postRequest);
        sendCommandToESP8266("AT+CIPCLOSE=0", 5, "OK");*/
    }
    else
    {
      Serial.println("Receive failed.");
    }
  }
}

/*void sendCommandToESP8266(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    esp8266.println(command);
    if (esp8266.find(readReplay))
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("Success");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
  }

  void sendData(String postRequest) {
  Serial.println(postRequest);
  esp8266.println(postRequest);
  delay(1500);
  countTrueCommand++;
  }*/
