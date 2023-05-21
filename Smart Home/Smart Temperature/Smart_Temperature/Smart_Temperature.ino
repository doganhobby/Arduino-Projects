#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>

#include "DHT.h"

#define DHTTYPE DHT11
#define DHTpin 14

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

DHT dht(DHTpin, DHTTYPE);

int counter = 0;

float humidity = 0;
float temperature = 0;

unsigned long lastSensorRead = 0;
unsigned long sensorReadDelay = 1000;

void setup() {
   Serial.begin(115200);

  analogWriteRange(255);
  
  dht.begin();

  bool success = SPIFFS.begin();
  if (!success) Serial.println("Error mounting the file system");

  connectToWifi();
}

void loop() {
  if ((millis() - lastSensorRead) > sensorReadDelay) {
    lastSensorRead = millis();

    float tmp = dht.readTemperature();
    if (tmp != temperature) updateTemperature(tmp);

    float hmd = dht.readHumidity();
    if (hmd != humidity) updateHumidity(hmd);
  }
  
  httpServer.handleClient();
}


void connectToWifi() {
  Serial.print("Connecting");

  IPAddress staticIP(192, 168, 1, 160);
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
  httpServer.on("/getHumidity", getHumidity);
  httpServer.on("/getTemperature", getTemperature);
  httpServer.on("/get_device_type", get_device_type);  
  httpServer.begin();

  Serial.print("IP:");
  Serial.println(WiFi.localIP());

  delay(3000);
}

void updateTemperature(float value) {
  temperature = value ;
  //temperature = random(-20, 80); //value ;
}

void updateHumidity(float value) {
  humidity = value;
  //humidity = random(0, 100); //value;
}

//---------------------------------Http Functions---------------------------------------------------------------

void getHumidity(){
  httpServer.send(200, "text/plain", /*"hum : " +*/ String(humidity));  
}

void getTemperature(){
  httpServer.send(200, "text/plain", /*"temp : " +*/ String(temperature));  
}

void get_device_type(){
  httpServer.send(200, "text/plain", "Thermometer");
}
