// LoRa 9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example LoRa9x_RX

#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define RFM95_CS 10
#define RFM95_RST 7
#define RFM95_INT 2

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

//String radiodata;
char sensor1[4], sensor2[4], sensor3[4], sensor4[4];

void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(9600);
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(100);

  //display.clearDisplay(); display.setCursor(0, 0); display.println("LoRa TX Test!"); display.display();
  Serial.println("Arduino LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    //display.setCursor(0, 16); display.println("Init Failed."); display.display();
    Serial.println("LoRa radio init failed");
    while (1);
  }
  //display.setCursor(0, 16); display.println("Init OK."); display.display();
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    //display.setCursor(0, 24); display.println("setFrequency failed"); display.display();
    Serial.println("setFrequency failed");
    while (1);
  }
  //display.setCursor(0, 24); display.print("Freq: "); display.println(RF95_FREQ); display.display();
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  delay(3000);
}

void loop()
{
  //display.clearDisplay();
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  
  strcpy(sensor1, "555");
  strcpy(sensor2, "555");
  strcpy(sensor3, "555");
  strcpy(sensor4, "555");
  
  char radiodata[20];
  strcpy(radiodata, "Node1;");
  strcat(radiodata, sensor1); strcat(radiodata, ";");
  strcat(radiodata, sensor2); strcat(radiodata, ";");
  strcat(radiodata, sensor3); strcat(radiodata, ";");
  strcat(radiodata, sensor4); strcat(radiodata, "#");

  //display.setCursor(0, 0); display.println("A"); display.display();
  Serial.print("Sending: "); Serial.println(radiodata);
  delay(10);
  rf95.send((uint8_t*)radiodata, strlen(radiodata)+1);

  //display.setCursor(0, 16); display.println("Waiting packet complete.."); display.display();
  Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();

  //-----------------------------------------
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  //display.fillRect(0, 16, 128, 16, BLACK); //x0, y0, w, h
  //display.setCursor(0, 16); display.println("Waiting for reply..."); display.display();
  Serial.println("Waiting for reply...");
  delay(10);
  if (rf95.waitAvailableTimeout(1000))
  {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len))
    {
      //display.setCursor(0, 16); display.print("Got reply: "); display.println((char*)buf);
      Serial.print("Got reply: "); Serial.println((char*)buf);
      //display.setCursor(0, 24); display.print("RSSI: "); display.println(rf95.lastRssi(), DEC);
      Serial.print("RSSI: "); Serial.println(rf95.lastRssi(), DEC);
      //display.display();
    }
    else
    {
      //display.clearDisplay(); display.setCursor(0, 0); display.println("Receive failed"); display.display();
      Serial.println("Receive failed");
    }
  }
  else
  {
    //display.clearDisplay(); display.setCursor(0, 16); display.println("No reply, none listen"); display.display();
    Serial.println("No reply, is there a listener around?");
  }
  delay(10000);
}
