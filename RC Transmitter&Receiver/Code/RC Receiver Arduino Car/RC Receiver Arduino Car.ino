#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Servo.h>

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

#define servo_pin 3

#define motor_forward_pin 5
#define motor_backward_pin 6

#define horn 2
#define front_led 10
#define back_led 9

Servo servo;

void setup() {
  Serial.begin(9600);

  servo.attach(servo_pin);

  pinMode(horn, OUTPUT);
  pinMode(front_led, OUTPUT);
  pinMode(back_led, OUTPUT);

  pinMode(motor_forward_pin, OUTPUT);
  pinMode(motor_backward_pin, OUTPUT);

  resetData();

  radio.begin();

  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);

  radio.openReadingPipe(1, pipeIn);
  radio.startListening();
}

void loop() {
  while (radio.available()) {
    radio.read(&data, sizeof(MyData));

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

    //int angle = min(130, max(45, data.j2PotX - 35));
    int angle = min(130, max(45, (87 + (data.j2PotX - 127) / 2)));
    servo.write(angle);

    if (data.j1PotY < 100) {
      digitalWrite(motor_forward_pin, LOW);
      //digitalWrite(motor_backward_pin, HIGH);
      analogWrite(motor_backward_pin, map(data.j1PotY, 100, 0, 180, 255));
    } else if (data.j1PotY > 150) {
      //digitalWrite(motor_forward_pin, HIGH);
      digitalWrite(motor_backward_pin, LOW);
      analogWrite(motor_forward_pin, map(data.j1PotY, 150, 255, 180, 255));
    } else {
      digitalWrite(motor_forward_pin, LOW);
      digitalWrite(motor_backward_pin, LOW);
    }

    digitalWrite(horn, data.button2 == 0 ? HIGH : LOW);

    digitalWrite(front_led, data.tSwitch1 == 1 ? HIGH : LOW);
    digitalWrite(back_led, data.tSwitch1 == 1 ? HIGH : LOW);

    lastRecvTime = millis();
  }

  if (millis() - lastRecvTime > 1000) {
    resetData();

    servo.write(min(140, max(35, data.j2PotX - 35)));

    digitalWrite(motor_forward_pin, LOW);
    digitalWrite(motor_backward_pin, LOW);
    digitalWrite(horn, LOW);
    digitalWrite(front_led, LOW);
    digitalWrite(back_led, LOW);
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
