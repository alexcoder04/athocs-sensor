#include <WiFi.h>
#include <HTTPClient.h>
#include <Bme280.h>
#include <ArduinoJson.h>

#define DEBUG 0
#define STATION_ID "E6X0-ALEX"
#define SENSOR_PORT_A D4 // "SDA"
#define SENSOR_PORT_B D5 // "SCL"
#define BATTERY_PORT A0 // battery voltage monitoring
#define UNLOCK_PIN D2 // set this pin to HIGH to enable serial interface
#if DEBUG
  #define DATA_UPLOAD_INTERVAL (30)
#else
  #define DATA_UPLOAD_INTERVAL (5 * 60)
#endif

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
#if DEBUG
  Serial.print("Connected to Wifi as ");
  Serial.println(WiFi.localIP());
#endif
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
      http.setReuse(false);

#if DEBUG
      Serial.printf("Requesting url '%s'\n", uploadUrl);
#endif

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
#if DEBUG
        Serial.println("Uploaded data successfully.");
#endif
      } else {
#if DEBUG
        Serial.println("Data upload failed");
        Serial.println(httpResponseCode);
        Serial.println(http.errorToString(httpResponseCode));
#endif
          // TODO save data on device and try to upload later
      }

      http.end();
    } else {
      // TODO save data on device and try to upload later
    }
}

void mainRoutine() {
  MeasurementData *data = readMeasurement();
#if DEBUG
  Serial.printf("t=%f, h=%f, p=%f, b=%d\n", data->temperature, data->humidity, data->pressure, data->battery);
#endif
  uploadData(data);
  delete data;
}

void setup() {
#if DEBUG
  Serial.begin(115200);
#endif

  initBattery();
  initWifi();
  initSensor();

#if DEBUG
  Serial.println("Setup finished");
  Serial.println("Reading data...");
#endif

  mainRoutine();

#if DEBUG
  Serial.println("Debug enabled, not going into deep sleep");
  delay(DATA_UPLOAD_INTERVAL * 1000);
#else
  pinMode(UNLOCK_PIN, INPUT);
  if (digitalRead(UNLOCK_PIN) != HIGH) {
    // default mode: we go into deep sleep and restart
    esp_sleep_enable_timer_wakeup(DATA_UPLOAD_INTERVAL * 1000 * 1000); // this is in MICROseconds
    esp_deep_sleep_start();
  } else {
    // debug mode: only a delay, so we stay accessible
    // activated until next full power cycle
    delay(DATA_UPLOAD_INTERVAL * 1000);
  }
#endif
}

void loop() {
  mainRoutine();
  delay(DATA_UPLOAD_INTERVAL * 1000);
}
