
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

void loop() {
  Serial.print("Enter command : ");
  while (Serial.available() == 0) delay(10);

  String command = Serial.readString();
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

    servo_y_left_axis.setEaseTo(value - 10, speed); 
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
