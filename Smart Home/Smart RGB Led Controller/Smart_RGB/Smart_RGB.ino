#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>

#include "FS.h"

#define LED_R 15
#define LED_G 13
#define LED_B 12

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

String RGB = "0,0,0,255,0";

unsigned long lastSensorRead = 0;
unsigned long sensorReadDelay = 1000;

void setup() {
   Serial.begin(115200);

  analogWriteRange(255);
  analogWrite(LED_R, 0);
  analogWrite(LED_G, 0);
  analogWrite(LED_B, 0);

  bool success = SPIFFS.begin();
  if (!success) Serial.println("Error mounting the file system");

  connectToWifi();

  readTheConfigFiles();
  updateRGBColors();
}

void loop() {
  if ((millis() - lastSensorRead) > sensorReadDelay) {
    lastSensorRead = millis();

    File f = SPIFFS.open("/mis_rgb.cfg", "w");
    if (f) {
      f.println(RGB);
      f.close();
    }
  }
  
  httpServer.handleClient();
}


void connectToWifi() {
  Serial.print("Connecting");

  IPAddress staticIP(192, 168, 1, 230);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(192, 168, 1, 1);
  IPAddress dns2(8, 8, 8, 8);

  const char* deviceName = "MakeItSmart_Smart_RGB";
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
  httpServer.on("/updateRGBColors", updateRGBColors);
  httpServer.on("/getRGBColors", getRGBColors);
  httpServer.on("/get_device_type", get_device_type);  
  httpServer.begin();

  Serial.print("IP:");
  Serial.println(WiFi.localIP());

  delay(3000);
}

void readTheConfigFiles() {  
  File f = SPIFFS.open("/mis_rgb.cfg", "r");
  if (f) {
    RGB = f.readStringUntil('\n');
    f.close();
  }
}

//---------------------------------Http Functions---------------------------------------------------------------
void updateRGBColors() {
  RGB = httpServer.arg("rgb_color");

  int R, G, B, A, toggle ;
  int n = sscanf(RGB.c_str(), "%d,%d,%d,%d,%d", &R, &G, &B, &A, &toggle);

  A *= toggle;
  analogWrite(LED_R, max(0, min(R * A / 255, 255)));
  analogWrite(LED_G, max(0, min(G * A / 255, 255)));
  analogWrite(LED_B, max(0, min(B * A / 255, 255)));
  
  httpServer.send(200, "text/plain", RGB);  
}

void getRGBColors(){
  httpServer.send(200, "text/plain", RGB);  
}

void get_device_type(){
  httpServer.send(200, "text/plain", "RGB Controller");
}
