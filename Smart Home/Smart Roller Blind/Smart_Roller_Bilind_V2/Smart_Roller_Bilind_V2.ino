#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Servo.h>
#include <NTPClient.h>
#include <math.h>

#include "FS.h"

//Static IP address configuration
IPAddress staticIP(192, 168, 1, 220);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);
IPAddress dns2(8, 8, 8, 8);

const char *deviceName = "MakeItSmart_Roller_Blind";
const char *ssid     = "**********Your_SSID************";
const char *password = "********Your Password**********";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;


#define left_Motor_forward_pin 14
#define left_Motor_backward_pin 12

#define right_Motor_forward_pin 15
#define right_Motor_backward_pin 13

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

int motor_direction = 0;
int timeZone = 3;

bool isReverse = false;

struct Position {
  String id;
  String name;
  int position;
  int time;
};

int numPositions = 0;
Position positionList[10];

void setup() {
  Serial.begin(115200);

  bool success = SPIFFS.begin();
  if (!success) Serial.println("Error mounting the file system");

  pinMode(left_Motor_forward_pin, OUTPUT);
  pinMode(left_Motor_backward_pin, OUTPUT);

  pinMode(right_Motor_forward_pin, OUTPUT);
  pinMode(right_Motor_backward_pin, OUTPUT);

  connectToWifi();
  loadLastValues();

  timeClient.begin();
  timeClient.setTimeOffset(timeZone * 60 * 60);  //GMT+3 => 3*60*60
}

void loop() {
  digitalWrite(left_Motor_forward_pin, motor_direction > 0 ? HIGH : LOW);
  digitalWrite(right_Motor_forward_pin, motor_direction > 0 ? HIGH : LOW);

  digitalWrite(left_Motor_backward_pin, motor_direction < 0 ? HIGH : LOW);
  digitalWrite(right_Motor_backward_pin, motor_direction < 0 ? HIGH : LOW);

  httpServer.handleClient();
}

//--------------------Operational functions-----------------------------------------
void connectToWifi() {
  WiFi.hostname(deviceName);
  WiFi.config(staticIP, gateway, subnet, dns, dns2);

  WiFi.mode(WIFI_STA);

  //If you are using the device with a powerbank, You can uncomment the ones you want to use to reduce power consuption of the project.
  //The default value is 20dbm and it's power consuption is 100mW.
  //when you reduce the output power it's connection range also will reduce. So you need to decide optimum value by trying yourself

  //WiFi.setOutputPower(20);  // Set to 20 dBm (100 mW) (default value)
  //WiFi.setOutputPower(18);  // Set to 18 dBm (63 mW)
  //WiFi.setOutputPower(12);  // Set to 12 dBm (16 mW)
  //WiFi.setOutputPower(6);  // Set to 6 dBm (4 mW)

  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 20) {
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) ESP.restart();

  httpUpdater.setup(&httpServer);
  httpServer.on("/getInfo", getInfo);
  httpServer.on("/get_device_type", get_device_type);
  httpServer.on("/motor_move", motor_move);
  httpServer.on("/go_to", go_to);
  httpServer.on("/setReverse", setReverse);
  httpServer.on("/addPosition", addPosition);
  httpServer.on("/deletePosition", deletePosition);
  httpServer.on("/getPositionList", getPositionList);
  httpServer.on("/getPosition", getPosition);

  httpServer.begin();
}


void loadLastValues() {
  File f = SPIFFS.open("/reverse.log", "r");
  if (f) {
    isReverse = f.readStringUntil('\n').toInt() == 1;
    f.close();
  }
}

//--------------------HTTP functions-----------------------------------------

void getInfo() {
  httpServer.send(200, "text/plain", "0," + String(isReverse));
}

void get_device_type() {
  httpServer.send(200, "text/plain", "Roller Blind");
}

void motor_move() {
  motor_direction = httpServer.arg("direction").toInt();
  if (isReverse)  motor_direction *= -1;

  httpServer.send(200, "text/plain", "Moving");
}

void go_to() {
  httpServer.send(200, "text/plain", "Ok.");
}

void setReverse() {
  int value = httpServer.arg("value").toInt();
  isReverse = value == 1;

  File f = SPIFFS.open("/reverse.log", "w");
  if (f) {
    f.println(isReverse);
    f.close();
  }

  httpServer.send(200, "text/plain", "Ok.");
}

void addPosition() {
  httpServer.send(200, "text/plain", "Ok.");
}

void deletePosition() {
  httpServer.send(200, "text/plain", "Ok.");
}

void getPositionList() {
  httpServer.send(200, "text/plain", "");
}

void getPosition() {
  httpServer.send(400, "text/plain", "Position not found");
}
