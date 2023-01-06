
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



Servo servo1;
Servo servo2;
Servo servo3;

void setup() {
  Serial.begin(9600);

  pinMode(channel1, OUTPUT);

  servo1.attach(channel2);
  servo2.attach(channel4);
  servo3.attach(channel5);

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

  //delay(250);
  digitalWrite(channel1, data.tSwitch1 == 1 ? HIGH : LOW);
  servo1.write(map(data.j1PotX, 0, 255, 0, 180));
  servo2.write(map(data.j1PotY, 0, 255, 0, 180));
  servo3.write(map(data.pot1, 255, 0, 0, 180));
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
