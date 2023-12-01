
#include <Arduino.h>
#include "ServoEasing.hpp"

ServoEasing servo_x_axis;
ServoEasing servo_y_left_axis;
ServoEasing servo_y_right_axis;
ServoEasing servo_z_axis;
ServoEasing servo_r_axis;

Servo clamp;

int x_axis_degree = 90;
int y_axis_degree = 120;
int z_axis_degree = 40;
int r_axis_degree = 110;

int clamp_open_degree = 120;
int clamp_half_open_degree = 60;
int clamp_closed_degree = 35;

int general_speed = 38;
int low_speed = 20;

void setup() {
  Serial.begin(9600);

  servo_y_left_axis.setReverseOperation(true);

  servo_x_axis.attach(3, x_axis_degree);
  servo_y_left_axis.attach(5, y_axis_degree);
  servo_y_right_axis.attach(6, y_axis_degree);
  servo_z_axis.attach(9, z_axis_degree);
  servo_r_axis.attach(10, r_axis_degree);

  clamp.attach(11);
  clamp.write(clamp_closed_degree);

  //setSpeedForAllServos(general_speed);
  //setEasingTypeForAllServos(EASE_CUBIC_IN_OUT);
  //setEasingTypeForAllServos(EASE_QUADRATIC_IN_OUT);
  //setEasingTypeForAllServos(EASE_SINE_IN_OUT);
  setEasingTypeForAllServos(EASE_SINE_IN_OUT);

  delay(1000);  // Wait for servo to reach start position.
}

String commands[] = {
  "X=50",
  "C=2",

  "Y=40,Z=75,R=150",
  "C=1",
  "Y=120,Z=40,R=110",
  "X=90",
  "Y=23,Z=85,R=160",
  "C=2",
  "Y=120,Z=40,R=110",
  "X=50",

  "Y=35,Z=75,R=150",
  "C=1",
  "Y=120,Z=40,R=110",
  "X=90",
  "Y=40,Z=78,R=150",
  "C=2",
  "Y=120,Z=40,R=110",
  "X=50",

  "Y=35,Z=75,R=150",
  "C=1",
  "Y=120,Z=40,R=110",
  "X=90",
  "Y=52,Z=75,R=140",
  "C=2",
  "Y=120,Z=40,R=110",
  "X=50",

  "Y=35,Z=75,R=150",
  "C=1",
  "Y=120,Z=40,R=110",
  "X=90",
  "Y=59,Z=80,R=130",
  "C=2",
  "Y=120,Z=40,R=110",
  "X=50",


  "Y=22,Z=83,R=160",
  "C=1",
  "Y=120,Z=40,R=110",
  "X=90",
  "Y=65,Z=90,R=110",
  "C=2",
  "Y=120,Z=40,R=110",
};

void loop() {
  delay(3000);

  for (int i = 0; i < 41; i++) {
    String command = commands[i];
    Serial.println(command);

    int index = 0;
    while ((index = command.indexOf(',')) > 0) {
      String subcommand = command.substring(0, index);
      command = command.substring(index + 1);
      subcommand.trim();
      parse_command(subcommand);
    }
    parse_command(command);

    synchronizeAllServosStartAndWaitForAllServosToStop();
    Serial.print("Command ");
    Serial.print(i + 1);
    Serial.println(" completed.");

    delay(1000);
  }

  Serial.print("Press enter to contine.");
  while (Serial.available() == 0) delay(10);
}

void parse_command(String command) {
  String axis = command.substring(0, 1);
  int value = command.substring(2).toInt();

  if (axis == "X" || axis == "x") {
    int speed = (abs(value - x_axis_degree) <= 20) ? low_speed : general_speed;

    servo_x_axis.setEaseTo(value, general_speed);
    x_axis_degree = value;
  } else if (axis == "Y" || axis == "y") {
    int speed = (abs(value - y_axis_degree) <= 20) ? low_speed : general_speed;

    servo_y_left_axis.setEaseTo(value - 10, speed);  // Use x.y with trailing f (to specify a floating point constant) to avoid compiler errors.
    servo_y_right_axis.setEaseTo(value - 10, speed);

    y_axis_degree = value;
  } else if (axis == "Z" || axis == "z") {
    int speed = (abs(value - z_axis_degree) <= 20) ? low_speed : general_speed;

    servo_z_axis.setEaseTo(value, speed);
    z_axis_degree = value;
  } else if (axis == "R" || axis == "r") {
    int speed = (abs(value - r_axis_degree) <= 20) ? low_speed : general_speed;

    servo_r_axis.setEaseTo(value, speed);
    r_axis_degree = value;
  } else if (axis == "C" || axis == "c") {
    clamp.write(value == 2 ? clamp_open_degree : (value == 1 ? clamp_half_open_degree : clamp_closed_degree));
  }
}
