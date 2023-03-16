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
const char *ssid = "Your SSID";
const char *password = "Your Password";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

#define encoder_clk_pin 5
#define encoder_data_pin 4

#define left_Motor_forward_pin 14
#define left_Motor_backward_pin 12

#define right_Motor_forward_pin 15
#define right_Motor_backward_pin 13

#define int_max 2147483647
#define Position_check_interval 60000  //1 dk
#define Position_check_interval_as_sec 60

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

unsigned long lastSensorRead = 0;
unsigned long sensorReadDelay = 500;

int previous_clk;
int previous_data;
int current_clk;
int current_data;

long last_read = 0;
long last_position_saved = 0;
long last_Position_Checked = 0;

int motor_direction = 0;
int last_motor_move_direction = 0;
long motor_position_counter = 0;

int goingTo = int_max;


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

  pinMode(encoder_clk_pin, INPUT);
  pinMode(encoder_data_pin, INPUT);

  previous_clk = digitalRead(encoder_clk_pin);
  previous_data = digitalRead(encoder_data_pin);

  connectToWifi();
  loadLastValues();

  timeClient.begin();
  timeClient.setTimeOffset(timeZone * 60 * 60);  //GMT+3 => 3*60*60
}

void loop() {
  if (goingTo != int_max) {
    if (motor_position_counter < goingTo) motor_direction = 1;
    else if (motor_position_counter > goingTo) motor_direction = -1;
    else {
      motor_direction = 0;
      goingTo = int_max;
    }
  }

  if (motor_direction != 0) last_motor_move_direction = motor_direction;

  digitalWrite(left_Motor_forward_pin, motor_direction > 0 ? HIGH : LOW);
  digitalWrite(right_Motor_forward_pin, motor_direction > 0 ? HIGH : LOW);

  digitalWrite(left_Motor_backward_pin, motor_direction < 0 ? HIGH : LOW);
  digitalWrite(right_Motor_backward_pin, motor_direction < 0 ? HIGH : LOW);

  if (millis() - last_read > 0.01) read_encoder();

  if (motor_direction == 0) {
    if (millis() - last_position_saved > 3000) saveMotorPosition();

    if (millis() - last_Position_Checked > Position_check_interval) {
      last_Position_Checked = millis();
      timeClient.update();

      int hr = timeClient.getHours();
      int mnt = timeClient.getMinutes();
      int sec = timeClient.getSeconds() + 60 * mnt + 3600 * hr;

      for (int i = 0; i < numPositions; i++) {
        if (!isnan(positionList[i].time) && positionList[i].time > 0 && (sec - positionList[i].time) > 0 && (sec - positionList[i].time) < Position_check_interval_as_sec) {
          go(positionList[i].position);
          break;
        }
      }
    }
  }

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
  httpServer.on("/resetEncoder", resetEncoder);

  httpServer.begin();
}

void saveMotorPosition() {
  File f = SPIFFS.open("/motor_position1.log", "w");
  if (f) {
    f.println(motor_position_counter);
    f.close();
  }

  f = SPIFFS.open("/last_motor_move_direction.log", "w");
  if (f) {
    f.println(last_motor_move_direction);
    f.close();
  }

  last_position_saved = millis();
}

void loadLastValues() {
  File f = SPIFFS.open("/motor_position1.log", "r");
  if (f) {
    motor_position_counter = f.readStringUntil('\n').toInt();
    f.close();
  }

  f = SPIFFS.open("/last_motor_move_direction.log", "r");
  if (f) {
    last_motor_move_direction = f.readStringUntil('\n').toInt();
    f.close();
  }

  f = SPIFFS.open("/reverse.log", "r");
  if (f) {
    isReverse = f.readStringUntil('\n').toInt() == 1;
    f.close();
  }

  File file = SPIFFS.open("/Position.log", "r");
  if (file) {
    String line;
    while (file.available()) {
      line = file.readStringUntil('\n');
      int comma1 = line.indexOf(',');
      int comma2 = line.indexOf(',', comma1 + 1);
      int comma3 = line.indexOf(',', comma2 + 1);
      if (comma1 == -1 || comma2 == -1 || comma3 == -1) continue;

      String id = line.substring(0, comma1);
      String name = line.substring(comma1 + 1, comma2);
      int position = line.substring(comma2 + 1, comma3).toInt();
      int time = line.substring(comma3 + 1).toInt();

      Position newPosition;
      newPosition.id = id;
      newPosition.name = name;
      newPosition.position = position;
      newPosition.time = time;

      positionList[numPositions] = newPosition;
      numPositions++;
    }
    file.close();
  }
}

void go(int p) {
  goingTo = p;

  //if (motor_position_counter < goingTo && last_motor_move_direction == -1) motor_position_counter -= 2;
  //else if (motor_position_counter > goingTo && last_motor_move_direction == 1) motor_position_counter += 2;
}

void read_encoder() {
  current_clk = digitalRead(encoder_clk_pin);
  current_data = digitalRead(encoder_data_pin);

  if (previous_clk == 0 && previous_data == 1) {
    if (current_clk == 1 && current_data == 0) motor_position_counter++;
    else if (current_clk == 1 && current_data == 1) motor_position_counter--;
  } else if (previous_clk == 1 && previous_data == 0) {
    if (current_clk == 0 && current_data == 1) motor_position_counter++;
    else if (current_clk == 0 && current_data == 0) motor_position_counter--;
  } else if (previous_clk == 0 && previous_data == 0) {
    if (current_clk == 1 && current_data == 1) motor_position_counter--;
    else if (current_clk == 1 && current_data == 0) motor_position_counter++;
  } else if (previous_clk == 1 && previous_data == 1) {
    if (current_clk == 0 && current_data == 0) motor_position_counter--;
    else if (current_clk == 0 && current_data == 1) motor_position_counter++;
  }

  previous_clk = current_clk;
  previous_data = current_data;

  last_read = millis();
}



//--------------------HTTP functions-----------------------------------------
void resetEncoder() {
  motor_position_counter = 0;
  saveMotorPosition();

  httpServer.send(200, "text/plain", "Ok.");
}

void getInfo() {
  httpServer.send(200, "text/plain", String(motor_position_counter) + "," + String(isReverse));
}

void get_device_type() {
  httpServer.send(200, "text/plain", "Roller Blind");
}

void motor_move() {
  int m_direction = httpServer.arg("direction").toInt();
  if (goingTo == int_max) {
    motor_direction = m_direction;
    if (isReverse) motor_direction = motor_direction * -1;
  }

  if (m_direction == 0) goingTo = int_max;

  httpServer.send(200, "text/plain", "Moving");
}

void go_to() {
  int p = httpServer.arg("p").toInt();
  go(p);

  httpServer.send(200, "text/plain", "Going to p" + String(p));
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
  if (numPositions >= 10)
    httpServer.send(400, "text/plain", "You have reached max number of positions!");

  File f = SPIFFS.open("/Position.log", "w");
  if (f) {
    String id = httpServer.arg("id");
    String name = httpServer.arg("name");
    int position = httpServer.arg("position").toInt();
    int time = httpServer.arg("time").toInt();

    int index = -1;
    for (int i = 0; i < numPositions; i++) {
      if (positionList[i].id == id) {
        index = i;
        break;
      }
    }

    if (index == -1) {
      Position newPosition;
      newPosition.id = id;
      newPosition.name = name;
      newPosition.position = position;
      newPosition.time = time;

      positionList[numPositions] = newPosition;
      numPositions++;
    } else {
      positionList[index].id = id;
      positionList[index].name = name;
      positionList[index].position = position;
      positionList[index].time = time;
    }

    for (int i = 0; i < numPositions; i++) {
      f.print(positionList[i].id);
      f.print(",");
      f.print(positionList[i].name);
      f.print(",");
      f.print(positionList[i].position);
      f.print(",");
      f.println(positionList[i].time);
    }

    f.close();
    httpServer.send(200, "text/plain", "Position added.");
  } else {
    httpServer.send(400, "text/plain", "File system can't be accessed!");
  }
}

void deletePosition() {
  String id = httpServer.arg("id");

  int index = -1;
  for (int i = 0; i < numPositions; i++) {
    if (positionList[i].id == id) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    httpServer.send(400, "text/plain", "Position not found");
  } else {

    File f = SPIFFS.open("/Position.log", "w");
    if (f) {
      numPositions--;
      for (int i = index; i < numPositions; i++) {
        positionList[i] = positionList[i + 1];
      }

      for (int i = 0; i < numPositions; i++) {
        f.print(positionList[i].id);
        f.print(",");
        f.print(positionList[i].name);
        f.print(",");
        f.print(positionList[i].position);
        f.print(",");
        f.println(positionList[i].time);
      }

      f.close();

      httpServer.send(200, "text/plain", "Position removed successfully");
    } else {
      httpServer.send(400, "text/plain", "File system can't be accessed!");
    }
  }
}

void getPositionList() {
  String result = "";
  for (int i = 0; i < numPositions; i++) {
    result += positionList[i].id + "#*#" + positionList[i].name + "#*#" + positionList[i].position + "#*#" + positionList[i].time + "*p*";
  }

  httpServer.send(200, "text/plain", result);
}

void getPosition() {
  String id = httpServer.arg("id");

  int index = -1;
  for (int i = 0; i < numPositions; i++) {
    if (positionList[i].id == id) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    httpServer.send(400, "text/plain", "Position not found");
  } else {
    httpServer.send(200, "text/plain", positionList[index].id + "#*#" + positionList[index].name + "#*#" + positionList[index].position + "#*#" + positionList[index].time);
  }
}
