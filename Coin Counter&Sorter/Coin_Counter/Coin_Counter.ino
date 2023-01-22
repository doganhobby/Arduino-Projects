#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

int currentStep = 0, pStep = 0;
int stepperPins[4] = { 7, 6, 5, 4 };

long started_at;

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.setBacklight(1);

  lcd.setCursor(0, 0);
  lcd.print("Make It Smart ");

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  started_at = millis();
}

int counter = 0;
int p_counter = -1;

bool sensorState_5_kr = false;
bool sensorState_10_kr = false;
bool sensorState_25_kr = false;
bool sensorState_50_kr = false;
bool sensorState_1_tl = false;

//This value should be define as your sensor reading value.
int sensorThreashHold = 75;

void loop() {
  // if (millis() - started_at > 3000) {
  int sensorValue_5_kr = analogRead(A0);
  int sensorValue_10_kr = analogRead(A1);
  int sensorValue_25_kr = analogRead(A2);
  int sensorValue_50_kr = analogRead(A3);
  
  //Important note; Because of the LCD I2C pins also use the SDA(A5) pin of Arduino, the default reading value should be greater than 700.
  int sensorValue_1_tl = analogRead(A5); 

  Serial.print("Sensor : ");
  Serial.print(sensorValue_5_kr);
  Serial.print(", ");
  Serial.print(sensorValue_10_kr);
  Serial.print(", ");
  Serial.print(sensorValue_25_kr);
  Serial.print(", ");
  Serial.print(sensorValue_50_kr);
  Serial.print(", ");
  Serial.println(sensorValue_1_tl); 

  if (sensorValue_5_kr < sensorThreashHold && sensorState_5_kr == false) counter += 5;
  if (sensorValue_10_kr < sensorThreashHold && sensorState_10_kr == false) counter += 5;
  if (sensorValue_25_kr < sensorThreashHold && sensorState_25_kr == false) counter += 15;
  if (sensorValue_50_kr < sensorThreashHold && sensorState_50_kr == false) counter += 25;
  if (sensorValue_1_tl < sensorThreashHold  && sensorState_1_tl == false) counter += 50;

  sensorState_5_kr = sensorValue_5_kr < sensorThreashHold;
  sensorState_10_kr = sensorValue_10_kr < sensorThreashHold;
  sensorState_25_kr = sensorValue_25_kr < sensorThreashHold;
  sensorState_50_kr = sensorValue_50_kr < sensorThreashHold;
  sensorState_1_tl = sensorValue_1_tl < sensorThreashHold ;


  if (counter != p_counter && sensorValue_1_tl > 700) {
    printTotal();
    p_counter = counter;
  }

  for (int a = 0; a < 10; a++) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(stepperPins[i], (i == currentStep || i == pStep) ? HIGH : LOW);
      delay(1);
    }
    if (pStep != currentStep) pStep = currentStep;
    else currentStep = (currentStep + 1) % 4;
  }
}

void printTotal() {
  lcd.setCursor(0, 1);
  lcd.print("Total : ");
  lcd.print(((double)counter) / 100.0);
  lcd.print(" tl ");
}
