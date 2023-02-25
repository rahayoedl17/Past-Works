// Arduino9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Arduino9x_TX

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
#define RF95_FREQ 923.1

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

int packetnum = 0;

void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(100);
  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(WHITE);

  display.setCursor(0, 0); display.println("LoRa RX Test!"); display.display();
  Serial.println("Arduino LoRa RX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    display.setCursor(0, 8); display.println("Init Failed."); display.display();
    Serial.println("LoRa radio init failed");
    while (1);
  }
  display.setCursor(0, 8); display.println("Init OK."); display.display();
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    display.setCursor(0, 16); display.println("setFrequency Failed."); display.display();
    Serial.println("setFrequency failed");
    while (1);
  }
  display.setCursor(0, 16); display.print("Freq: "); display.println(RF95_FREQ); display.display();
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
  display.clearDisplay();
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      RH_RF95::printBuffer("Received: ", buf, len);
      packetnum++;
      display.setCursor(0, 0); display.print("Data ke : "); display.println(packetnum); display.display();
      display.setCursor(0, 8); display.print("Got : "); display.println((char*)buf); display.display();
      Serial.print("Got: ");
      Serial.println((char*)buf);
      
      display.setCursor(0, 24); display.print("RSSI: "); display.print(rf95.lastRssi(), DEC); display.display();
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      display.println(".Reply sent"); display.display();
      Serial.println("Sent a reply.");
    }
    else
    {
      display.setCursor(0, 0); display.println("Receive failed."); display.display();
      Serial.println("Receive failed.");
    }
  }
}
