#include <ESP8266WiFi.h>
#include "heltec.h"
#include <Wire.h>
#include "HTU21D.h" // needs forked library from https://github.com/rene-gust/HTU21D-Sensor-Library
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

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

HTTPClient _httpAuth;
HTTPClient _http;
WiFiClient _wifiClient;
char server[] = "192.168.0.103";
const char* SENSOR_MON_SEND_DATA_URL = "http://192.168.0.103/api/sensor_records";
const char* SENSOR_MON_AUTHENTICATION_URL = "http://192.168.0.103/authentication_token";
const int SENSOR_MON_SENSOR_ID = 1;
const char* SENSOR_MON_SENSOR_PASSWORD = "tulpe";
const char* SENSOR_MON_SENSOR_USERNAME = "USERNAME";
const char* SENSOR_MON_SENSOR_USER_PASSWORD = "PASSWORD";

String token;

void setup() {
  Serial.begin(115200);
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

    token = getToken();

    postSensorData(token, temperature, SENSOR_MON_TEMPERATURE_UNIT);
    postSensorData(token, humidity, SENSOR_MON_HUMIDITY_UNIT);
  }

  prepareDisplay();
  Heltec.display->drawString(0, 10, String("Temperature: " + String(temperature, 1) + " °C"));
  Heltec.display->drawString(0, 20, String("Humidity: " + String(humidity, 1) + " %"));
  Heltec.display->display();

  delay(30000);
}


void sendJson(String json, String url, String bearerToken = String("")) {
  if (_wifiClient.connect(server, 80)) {
    Serial.println("connected to server");
    _wifiClient.println(String("POST ") + url + String(" HTTP/1.1"));
    _wifiClient.println(String("Host: ") + String(server));

    if (bearerToken.length() > 0) {
      _wifiClient.println(String("Authorization: Bearer ") + bearerToken);
    }
    
    _wifiClient.println("Content-type: application/json");
    _wifiClient.println("Content-length: " + String(json.length()));
    _wifiClient.println("");
    _wifiClient.println(json);
  }
}

void postSensorData(String token, float value, char* unit) {
  String json = "{\"sensorId\": " + String (SENSOR_MON_SENSOR_ID) + "," +
                     "\"password\": \"" + String(SENSOR_MON_SENSOR_PASSWORD) + "\"," +
                     "\"value\": " + String (value) + "," +
                     "\"unit\": \"" + String (unit) + "\"" +
                   "}";

 sendJson(json, String("/api/sensor_records"), token);

 int timoutTimer = 0;
  
  while (_wifiClient.available() == 0) {
    ++timoutTimer;
    if (timoutTimer > 20) {
      Serial.println(">>> Client Timeout !");
      return ;
    }
    delay(100);
  }
  
  String line;
  do {
    line = readLine();
  } while (line.indexOf("{") == -1) ;
  Serial.println(String("sent sensor data: ") + line);
}

String getToken() {
    String json = "{\"username\": \"" + String (SENSOR_MON_SENSOR_USERNAME) + "\"," +
                     "\"password\": \"" + String(SENSOR_MON_SENSOR_USER_PASSWORD) + "\"" +
                "}";

  sendJson(json, String("/authentication_token"));
  int timoutTimer = 0;
  
  while (_wifiClient.available() == 0) {
    ++timoutTimer;
    if (timoutTimer > 20) {
      Serial.println(">>> Client Timeout !");
      return "";
    }
    delay(100);
  }
  
  String line;
  do {
    line = readLine();
  } while (line.indexOf("{") == -1) ;
  
  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, line);
  
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return "";
  }
  const char* token = doc["token"];
  return String(doc["token"]);
}

String readLine() {
  String line;
  char c;

  do {
    c = _wifiClient.read();
    line = line + String(c);
  } while (String(c) != "\n");

  return line;
}
