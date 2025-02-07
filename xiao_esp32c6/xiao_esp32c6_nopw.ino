#include <WiFi.h>
#include <HTTPClient.h>
#include <Bme280.h>
#include <ArduinoJson.h>

#define STATION_ID "E6X0-ALEX"
#define SENSOR_PORT_A D4 // "SDA"
#define SENSOR_PORT_B D5 // "SCL"
#define BATTERY_PORT A0 // battery voltage monitoring
#define RESET_PIN D2 // set this pin to HIGH to enable serial interface

const char* uploadUrl = "http://192.168.0.86:1111/api/upload";
const char* ssid = "CREDENTIALS";
const char* password = "CREDENTIALS";

struct MeasurementData {
  float temperature;
  float humidity;
  float pressure;
  int battery;
};

Bme280TwoWire sensor;

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    // TODO go to sleep if low on battery
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void initSensor() {
  Wire.begin(SENSOR_PORT_A, SENSOR_PORT_B);
  sensor.begin(Bme280TwoWireAddress::Primary); // I2C
  sensor.setSettings(Bme280Settings::indoor());
}

void initBattery() {
  pinMode(BATTERY_PORT, INPUT);
}

float getBatteryVoltage() {
  uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(A0);
  }
  return float(2 * Vbatt / 16 / 1000.0);
}

MeasurementData* readMeasurement() {
  MeasurementData *data = new MeasurementData;
  data->temperature = sensor.getTemperature();
  data->humidity = sensor.getHumidity();
  data->pressure = sensor.getPressure() / 100;
  data->battery = int((getBatteryVoltage() - 3.3) / (4.2 - 3.3) * 100);
  return data;
}

void uploadData(MeasurementData *data) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      http.begin(uploadUrl);
      http.addHeader("Content-Type", "application/json");

      StaticJsonDocument<200> jsonDoc;
      jsonDoc["timestamp"] = "auto";
      jsonDoc["temperature"] = data->temperature;
      jsonDoc["humidity"] = data->humidity;
      jsonDoc["pressure"] = data->pressure;
      jsonDoc["station"] = STATION_ID;
      jsonDoc["battery"] = data->battery;

      String jsonString;
      serializeJson(jsonDoc, jsonString);

      int httpResponseCode = http.POST(jsonString);
      if (httpResponseCode > 0) {
          String response = http.getString();
      } else {
          // TODO save data on device and try to upload later
      }

      http.end();
    } else {
      // TODO save data on device and try to upload later
    }
}

void setup() {
  initBattery();
  initWifi();
  initSensor();

  MeasurementData *data = readMeasurement();
  uploadData(data);
  delete data;

  pinMode(RESET_PIN, INPUT);
  if (digitalRead(RESET_PIN) != HIGH) {
    esp_sleep_enable_timer_wakeup(5 * 60 * 1000 * 1000); // this is in MICROseconds
    esp_deep_sleep_start();
  }
}

void loop() {} // unused because of deep sleep

