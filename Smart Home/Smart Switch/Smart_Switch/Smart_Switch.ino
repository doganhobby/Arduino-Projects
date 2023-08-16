#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>

#define relay_pin 2

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

bool state = false;
int counter = 0;

unsigned long lastSensorRead = 0;
unsigned long sensorReadDelay = 1000;



void setup() {
  Serial.begin(115200);

  analogWriteRange(255);

  pinMode(relay_pin, OUTPUT);

  bool success = SPIFFS.begin();
  if (!success) Serial.println("Error mounting the file system");

  connectToWifi();
}

void loop() {
  if ((millis() - lastSensorRead) > sensorReadDelay) {

  }

  digitalWrite(relay_pin, state ? HIGH : LOW);

  httpServer.handleClient();
}


void connectToWifi() {
  Serial.print("Connecting");

  IPAddress staticIP(192, 168, 1, 150);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(192, 168, 1, 1);
  IPAddress dns2(8, 8, 8, 8);

  const char* deviceName = "MakeItSmart_Smart_Thermometer";
  const char *ssid     = "**********Your_SSID************";
  const char *password = "********Your Password**********";

  WiFi.hostname(deviceName);
  WiFi.config(staticIP, gateway, subnet, dns, dns2);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter++ < 20) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) ESP.restart();

  Serial.print("IP:");
  Serial.println(WiFi.localIP());

  httpUpdater.setup(&httpServer);
  httpServer.on("/get_state", get_state);
  httpServer.on("/toggle", toggle);
  httpServer.on("/get_device_type", get_device_type);
  httpServer.begin();

  Serial.print("IP:");
  Serial.println(WiFi.localIP());

  delay(3000);
}

//---------------------------------Http Functions---------------------------------------------------------------

void get_state() {
  httpServer.send(200, "text/plain", /*"hum : " +*/ String(state));
}

void toggle() {
  state = !state;
  httpServer.send(200, "text/plain", /*"temp : " +*/ String(state));
}

void get_device_type() {
  httpServer.send(200, "text/plain", "Switch");
}
