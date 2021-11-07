// This example just provide basic function test;
// For more informations, please vist www.heltec.cn or mail to support@heltec.cn

#include <ESP8266WiFi.h>
#include "heltec.h"
#include <Wire.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PASSWD"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
const int ADDRESS = 0x40;
double temperature, humidity;
int raw_temp_data;

void setup() {
  Wire.begin();
  Heltec.begin(true , true);
  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
}

void setupDisplay() {
  Wire.begin();
  Heltec.begin(true, false);
  Heltec.display->clear();
}

void loop() {
  sensor_init(ADDRESS);
  temperature = read_temperature(ADDRESS);
  humidity = read_humidity(ADDRESS);
  Serial.println(String("sensors read" + String(temperature)));
  setupDisplay();
  Heltec.display->drawString(0, 0, String("IP-Adress: " + WiFi.localIP().toString()));
  Heltec.display->drawString(0, 10, String("Temperature: " + String(temperature, 1) + " °C"));
  Heltec.display->drawString(0, 20, String("Humidity: " + String(humidity, 1) + " %"));
  Heltec.display->display();

  delay(1000);
}

char* temperatureString(double temperature) {
  char* buffer;
  sprintf(
    buffer,
    "Temperature: %f °C",
    temperature
  );

  return buffer;
}


char* humidityString(double humidity) {
  char* buffer;
  sprintf(
    buffer,
    "Humidity: %f %",
    humidity
  );

  return buffer;
}

void sensor_init(const int addr) {
  Wire.begin(2, 14); // SDA at GPIO02, SCL at GPIO14
  delay(100);
  Wire.beginTransmission(addr);
  Wire.endTransmission();
}

double read_temperature(const int addr) {
  double temperature;
  int low_byte, high_byte, raw_data;
  /**Send command of initiating temperature measurement**/
  Wire.beginTransmission(addr);
  Wire.write(0xE3);
  Wire.endTransmission();
  /**Read data of temperature**/
  Wire.requestFrom(addr, 2);

  if (Wire .available() <= 2) {
    high_byte = Wire.read();
    low_byte = Wire.read();
    high_byte = high_byte << 8;
    raw_data = high_byte + low_byte;
  }
  raw_temp_data = high_byte;
  temperature = (175.72 * raw_data) / 65536;
  temperature = temperature - 46.85;

  return temperature;
}

double read_humidity(const int addr) {
  double humidity, raw_data_1, raw_data_2;
  int low_byte, high_byte, container;
  /**Send command of initiating relative humidity measurement**/
  Wire.beginTransmission(addr);
  Wire.write(0xE5);
  Wire.endTransmission();
  /**Read data of relative humidity**/
  Wire.requestFrom(addr, 2);

  if (Wire .available() <= 2) {
    high_byte = Wire.read();
    container = high_byte / 100;
    high_byte = high_byte % 100;
    low_byte = Wire.read();
    raw_data_1 = container * 25600;
    raw_data_2 = high_byte * 256 + low_byte;
  }
  raw_data_1 = (125 * raw_data_1) / 65536;
  raw_data_2 = (125 * raw_data_2) / 65536;
  humidity = raw_data_1 + raw_data_2;
  humidity = humidity - 6;

  return humidity;
}
