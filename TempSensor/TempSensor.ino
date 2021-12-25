//#include <ESP8266WiFi.h>
#include "heltec.h"
#include <Wire.h>
#include "HTU21D.h" // needs forked library from https://github.com/rene-gust/HTU21D-Sensor-Library

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PASSWORD"
#endif

//const char* ssid     = STASSID;
//const char* password = STAPSK;
const int SENSOR_ADDRESS = 0x40;
const int SENSOR_SDA = 2; // SDA is wired at GPIO02
const int SENSOR_SCL = 14; // SCL is wired at GPIO14

HTU21D sensor;
float temperature; 
float humidity;

void setup() {
  Serial.begin(9600);
  Heltec.begin(true , false);
  Heltec.display->init();
  
}

void setupDisplay() {
  Wire.begin(SDA, SCL);
  Heltec.display->clear();
}

void setupSensor() {
  sensor.begin(SENSOR_SDA, SENSOR_SCL);
}

void loop() {
  setupSensor();
  if (sensor.measure()) {
    temperature = sensor.getTemperature();
    humidity = sensor.getHumidity();

    Serial.print("Temperature (°C): ");
    Serial.println(temperature);

    Serial.print("Humidity (%RH): ");
    Serial.println(humidity);
  }

  setupDisplay();
  //Heltec.display->drawString(0, 0, String("IP-Adress: " + WiFi.localIP().toString()));
  Heltec.display->drawString(0, 10, String("Temperature: " + String(temperature, 1) + " °C"));
  Heltec.display->drawString(0, 20, String("Humidity: " + String(humidity, 1) + " %"));
  Heltec.display->display();

  delay(2000);
}
