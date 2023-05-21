#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include <Ticker.h>

//Static IP address configuration
IPAddress staticIP(192, 168, 1, 170);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);
IPAddress dns2(8, 8, 8, 8);

const char *deviceName = "MakeItSmart_Plant_Watering";
  const char *ssid     = "**********Your_SSID************";
  const char *password = "********Your Password**********";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

#define mosfet 12
#define relay 14

const int analogInPin = A0;

Ticker timer;
bool state = false;
int humidity = 0;

Ticker relay_timer;

void setup() {
  Serial.begin(115200);

  pinMode(relay, OUTPUT);  
  pinMode(mosfet, OUTPUT);

  //analogWriteRange(1024);

  timer.attach_ms(1000, readSensorValue);

  connectToWifi();
}

void loop() {
  analogWrite(mosfet, state ? 130 : 0);
  digitalWrite(relay, state ? HIGH : LOW);

  httpServer.handleClient();
}

//--------------------Operational functions-----------------------------------------
void connectToWifi() {
  WiFi.hostname(deviceName);
  WiFi.config(staticIP, gateway, subnet, dns, dns2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 20) {
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) ESP.restart();

  httpUpdater.setup(&httpServer);
  httpServer.on("/get_device_type", get_device_type);
  httpServer.on("/get_humidity", get_humidity);
  httpServer.on("/get_state", get_state);
  httpServer.on("/waterize", waterize);

  httpServer.begin();
}

void readSensorValue() {
  humidity = (int)((1024 - analogRead(analogInPin)) / 10.24);
}

void stopWaterizing() {
  if (relay_timer.active()) relay_timer.detach();
  state = false;
}

//--------------------Http functions-----------------------------------------
void get_device_type() {
  httpServer.send(200, "text/plain", "Plant Watering");
}

void get_humidity() {
  httpServer.send(200, "text/plain", String(humidity));
}

void get_state() {
  httpServer.send(200, "text/plain", String(state));
}

void waterize() {
  state = httpServer.arg("value").toInt() == 1;
  if (state) {
    int duration = httpServer.arg("duration").toInt();
    if (!relay_timer.active()) relay_timer.attach_ms(duration > 0 ? duration : 3000, stopWaterizing);
  } else {
    if (relay_timer.active()) relay_timer.detach();
  }

  httpServer.send(200, "text/plain", "Ok");
}
