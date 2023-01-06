
#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

struct MyData {
  byte j1PotX;
  byte j1PotY;
  byte j2PotX;
  byte j2PotY;
  byte pot1;
  byte pot2;
  byte tSwitch1;
  byte tSwitch2;
  byte button1;
  byte button2;
};

MyData data;
unsigned long lastRecvTime = 0;

RF24 radio(7, 8);
const uint64_t pipeIn = 0xE8E8F0F0E1LL;

#define channel1 2
#define channel3 4

#define channel2 3
#define channel4 5
#define channel5 6
#define channel6 9
#define channel7 10

Servo servo_z_axis;
Servo servo_x_axis;
Servo servo_y_axis;
Servo servo_clamp;

int x_axis_degree = 90;
int y_axis_degree = 90;
int z_axis_degree = 85;
int clamp_degree = 90;

void setup() {
  Serial.begin(9600);


  servo_x_axis.attach(3);
  servo_y_axis.attach(5);
  servo_z_axis.attach(6);
  servo_clamp.attach(10);

  resetData();

  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);

  radio.openReadingPipe(1, pipeIn);
  radio.startListening();
}

void loop() {
  recvData();
  unsigned long now = millis();
  if (now - lastRecvTime > 1000) {
    Serial.println("Signal lost!");
    resetData();
  }


  if (data.j1PotX < 80) x_axis_degree += 7;
  else if (data.j1PotX > 160) x_axis_degree -= 7;
  
  if (data.j1PotY < 80) y_axis_degree -= 7;
  else if (data.j1PotY > 160) y_axis_degree += 7;

  if (data.j2PotY < 80) z_axis_degree -= 7;
  else if (data.j2PotY > 160) z_axis_degree += 7;

  clamp_degree = data.tSwitch1 ==1 ? 75 : 90;

  //*************************************
  //You should decide the max/min angles.
  z_axis_degree = min(145, max(15, z_axis_degree));
  x_axis_degree = min(175, max(40, x_axis_degree));
  y_axis_degree = min(150, max(5, y_axis_degree));
  clamp_degree = min(90, max(75, clamp_degree));

  Serial.print("x_axis_degree : ");
  Serial.print(x_axis_degree);
  Serial.print(", y_axis_degree : ");
  Serial.print(y_axis_degree);
  Serial.print(", z_axis_degree 4 : ");
  Serial.print(z_axis_degree);
  Serial.print(", clamp_degree : ");
  Serial.println(clamp_degree);

  servo_clamp.write(clamp_degree);
  servo_x_axis.write(x_axis_degree);
  servo_y_axis.write(y_axis_degree);
  servo_z_axis.write(z_axis_degree);
}

void recvData() {
  while (radio.available()) {
    radio.read(&data, sizeof(MyData));

    /*analogWrite(throtle_pin, data.throttle);
        servo_yaw.write(map(data.yaw, 0, 255, 0, 180));
        servo_pitch.write(map(data.pitch, 0, 255, 0, 180));
        servo_roll.write(map(data.roll, 0, 255, 0, 180));*/

    /*Serial.print(data.j1PotX);
    Serial.print(", ");
    Serial.print(data.j1PotY);
    Serial.print(", ");
    Serial.print(data.j2PotX);
    Serial.print(", ");
    Serial.print(data.j2PotY);
    Serial.print(", ");
    Serial.print(data.pot1);
    Serial.print(", ");
    Serial.print(data.pot2);
    Serial.print(", ");
    Serial.print(data.tSwitch1);
    Serial.print(", ");
    Serial.print(data.tSwitch2);
    Serial.print(", ");
    Serial.print(data.button1);
    Serial.print(", ");
    Serial.println(data.button2);*/

    lastRecvTime = millis();
  }
}

void resetData() {
  data.j1PotX = 127;
  data.j1PotY = 127;
  data.j2PotX = 127;
  data.j2PotY = 127;
  data.pot1 = 1;
  data.pot2 = 1;
  data.tSwitch1 = 1;
  data.tSwitch2 = 1;
  data.button1 = 1;
  data.button2 = 1;
}
