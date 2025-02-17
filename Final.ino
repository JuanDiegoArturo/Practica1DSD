#include "DHT.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include <UbiConstants.h>
#include <UbiTypes.h>
#include <UbidotsEsp32Mqtt.h>

#define DHTPIN 27
#define DHTTYPE DHT11

const char *UBIDOTS_TOKEN = "BBUS-FsPOdewcDF9Dj3sAtPpa1pV9ttWRo7"; // Put here your Ubidots TOKEN
const char *WIFI_SSID = "UPB_WiFi";                                // Put here your Wi-Fi SSID
const char *WIFI_PASS = "";                                        // Put here your Wi-Fi password
const char *DEVICE_LABEL = "esp32";                                // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL1 = "Temperatura";                       // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL2 = "Humedad";

const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds

unsigned long timer;

TFT_eSPI tft = TFT_eSPI();
DHT dht(DHTPIN, DHTTYPE);
Ubidots ubidots(UBIDOTS_TOKEN);

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));

  dht.begin();

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Temperatura", 30, 10, 2);
  tft.drawString("Grados Celsius", 25, 100, 2);
  tft.drawString("----------------------", 0, 120, 2);
  tft.drawString("Humedad", 45, 140, 2);
  tft.drawString("%", 67, 220, 2);

  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  timer = millis();
}

void loop()
{
  delay(2000);

  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }

  if ((millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    tft.drawString(String(t), 0, 40, 7);
    tft.drawString(String(h), 0, 160, 7);

    Serial.print(F("Humedad: "));
    Serial.print(h);
    Serial.print(F("% Temperatura: "));
    Serial.print(t);
    Serial.println(F("°C "));

    ubidots.add(VARIABLE_LABEL1, t);
    ubidots.add(VARIABLE_LABEL2, h);
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }

  ubidots.loop();
}
