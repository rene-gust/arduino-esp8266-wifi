#include <ESP8266WiFi.h>
#include "heltec.h"
#include <Wire.h>
#include "HTU21D.h" // needs forked library from https://github.com/rene-gust/HTU21D-Sensor-Library
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PASSWORD"
#endif

#define SENSOR_MON_TEMPERATURE_UNIT "°C"
#define SENSOR_MON_HUMIDITY_UNIT "%"

const char* ssid     = STASSID;
const char* password = STAPSK;
const int SENSOR_ADDRESS = 0x40;
const int SENSOR_SDA = 2; // SDA is wired at GPIO02
const int SENSOR_SCL = 14; // SCL is wired at GPIO14

HTU21D sensor;
float temperature; 
float humidity;

HTTPClient _http;
WiFiClient _wifiClient;
const char* SENSOR_MON_URL = "http://rene-thinky/api/sensor_records";
const int SENSOR_MON_SENSOR_ID = 1;
const char* SENSOR_MON_SENSOR_PASSWORD = "tulpe";

void setup() {
  Serial.begin(9600);
  setupWifi();
  setupDisplay();
}

void setupDisplay() {
  Heltec.begin(true , false);
  Heltec.display->init();
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void prepareDisplay() {
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

    if (WiFi.status() == WL_CONNECTED) {
      send(temperature, SENSOR_MON_TEMPERATURE_UNIT);
      send(humidity, SENSOR_MON_HUMIDITY_UNIT);
    }
  }

  prepareDisplay();
  Heltec.display->drawString(0, 0, String("IP-Adress: " + WiFi.localIP().toString()));
  Heltec.display->drawString(0, 10, String("Temperature: " + String(temperature, 1) + " °C"));
  Heltec.display->drawString(0, 20, String("Humidity: " + String(humidity, 1) + " %"));
  Heltec.display->display();

  delay(30000);
}


void send(float value, char* unit) {
  _http.begin(_wifiClient, SENSOR_MON_URL);
  String json = "{\"sensorId\": " + String (SENSOR_MON_SENSOR_ID) + "," +
                     "\"password\": \"" + String(SENSOR_MON_SENSOR_PASSWORD) + "\"," +
                     "\"value\": " + String (value) + "," +
                     "\"unit\": \"" + String (unit) + "\"" +
                   "}";
  
  Serial.println(String("[HTTP] POST...") + String(SENSOR_MON_URL) + String(json));
  
  _http.addHeader("Content-Type", "application/json");
  int httpCode = _http.POST(json);
  if (httpCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
  } else {
    Serial.print("error status code: " + httpCode);
  }
}
