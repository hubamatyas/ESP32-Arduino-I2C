#include <WiFi.h>
#include <Wire.h>
#include "esp_wpa2.h"
#include "ThingsBoard.h"

// define wife ssid, identity and password
#define EAP_IDENTITY "your_email"
#define EAP_PASSWORD "your_password"
#define ssid "your_ssid"

// define token and server (thingsboard credentials)
#define TOKEN               "DssDXSgSIhf7UchV8WO2"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD   115200

// define pins on esp32
#define SLAVE_ADDR 9
#define I2C_SDA 21
#define I2C_SCL 22

bool heater_power = true;
bool motor_power = true;
bool pump_power = true;

float tar_temp = 0.0;
int tar_rpm = 0;
float tar_ph = 0.0;

float temp;
float ph;
int rpm;

// Initialize MQTT and ThingsBoard client
WiFiClient espClient;
PubSubClient client(espClient); 

// Initialize ThingsBoard instance
ThingsBoard tb(espClient);      
int status = WL_IDLE_STATUS;


void InitWiFi()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F("Connected to AP successfully!"));
  Serial.print(F("IP address set: "));
  Serial.println(WiFi.localIP());
}

void reconnectWiFi() {
  // Loop until we're reconnected
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(ssid);
    Serial.print("reconnecting");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.print("Connected to ");
    Serial.println(ssid);
  }
}

// Processes function for RPC call "set_temperature"
// RPC_Data is a JSON variant, that can be queried using []
RPC_Response processTemperatureChange(const RPC_Data &data)
{
  Serial.println("Received the set temperature RPC method");

  // Process data
  float temperature = data;
  tar_temp = temperature;
  Serial.print("Temperature: ");
  Serial.println(temperature);

  return RPC_Response();
}

RPC_Response processPhChange(const RPC_Data &data)
{
  Serial.println("Received the set ph RPC method");

  // Process data
  float Ph = data;
  tar_ph = Ph;
  Serial.print("Ph: ");
  Serial.println(Ph);

  return RPC_Response();
}

RPC_Response processRpmChange(const RPC_Data &data)
{
  Serial.println("Received the set rpm RPC method");

  // Process data
  int rpm = data;
  tar_rpm = rpm;
  Serial.print("RPM : ");
  Serial.println(rpm);

  return RPC_Response();
}

// Processes function for RPC call "set_switch"
// RPC_Data is a JSON variant, that can be queried using []
RPC_Response processSwitchChange(const RPC_Data &data)
{
  Serial.println("Received the set switch method");

  // Process data
  bool switch_state = data;
  
  Serial.print("Switch state: ");
  Serial.println(switch_state);

  return RPC_Response();
}

const size_t callbacks_size = 4;
RPC_Callback callbacks[callbacks_size] = {
  { "setTemperature",    processTemperatureChange },
  { "setSwitch",         processSwitchChange },
  { "setPh",             processPhChange },
  { "setRpm",            processRpmChange }
};

bool subscribed = false;

// encodes integers to fit 1 byte Wire.write()
void encode(double num, int* ptr) {
  *ptr = int(num/256);
  *(ptr+1) = int(num)%256;
}

void setup() {
  Wire.begin (I2C_SDA, I2C_SCL);
  Serial.begin(SERIAL_DEBUG_BAUD);
  InitWiFi();

  client.setServer(THINGSBOARD_SERVER, 1883);
  client.setCallback(callback);
}

void loop() {
  delay(1500);

  // request 6 bytes from slave
  Wire.requestFrom(SLAVE_ADDR, 6);
  while (Wire.available()) {
    temp = int(Wire.read()) * 256;
    temp += int(Wire.read());
    rpm = int(Wire.read()) * 256;
    rpm += int(Wire.read());
    ph = int(Wire.read()) * 256;
    ph += int(Wire.read());
  }

  // debugging print statements
  temp = temp / 100;
  ph = ph / 100;
  Serial.println();
  Serial.println("data from Arduino: ");
  Serial.println(temp);
  Serial.println(rpm);
  Serial.println(ph);
  Serial.println();

  // reconnect to WiFi if connection is lost
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  }
  client.loop();

  // reconnect to ThingsBoard
  if (!tb.connected()) {
    // Connect to ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  Serial.println("Sending data...");

  // Uploads new telemetry to ThingsBoard using MQTT.
  // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api  
  
  tb.sendTelemetryFloat("temperature", temp);
  tb.sendTelemetryFloat("ph", ph);
  tb.sendTelemetryInt("rpm", rpm);
  
  if (!subscribed) {
    Serial.println("Subscribing for RPC...");

    // Perform a subscription. All consequent data processing will happen in
    // processTemperatureChange() and processSwitchChange() functions,
    // as denoted by callbacks[] array.
    if (!tb.RPC_Subscribe(callbacks, callbacks_size)) {
      Serial.println("Failed to subscribe for RPC");
      return;
    }

    Serial.println("Subscribe done");
    subscribed = true;
  }

  Serial.println("Waiting for data...");

  // send data to slave
  Wire.beginTransmission(SLAVE_ADDR);
  int* temp_encoded;
  int* rpm_encoded;
  int* ph_encoded;
  
  encode(tar_temp*100, temp_encoded);
  encode(tar_rpm, rpm_encoded);
  encode(tar_ph*100, ph_encoded);
  
  Wire.write(*temp_encoded);
  Wire.write(*(temp_encoded+1));
  Wire.write(*rpm_encoded);
  Wire.write(*(rpm_encoded+1));
  Wire.write(*ph_encoded);
  Wire.write(*(ph_encoded+1));
  Wire.endTransmission();  
  
  tb.loop();
}
