#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


// Define the digital inputs
#define t1 9   // Toggle switch 1
#define t2 8   // Toggle switch 1
#define b1 2   // Button 1
#define b2 3   // Button 2


RF24 radio(5, 6);   // nRF24L01 (CE, CSN)
const uint64_t pipeOut = 0xE8E8F0F0E1LL;

// Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package {
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

Data_Package data; //Create a variable with the above structure

void setup() {
  Serial.begin(9600);
  
  // Define the radio communication
  
    radio.begin();
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);

    radio.openWritingPipe(pipeOut);
  
  pinMode(t1, INPUT_PULLUP);
  pinMode(t2, INPUT_PULLUP);
  pinMode(b1, INPUT_PULLUP);
  pinMode(b2, INPUT_PULLUP);
  
  resetData();
}

void loop() {
  data.j1PotX = map(analogRead(A2), 440, 605, 0, 255); 
  data.j1PotY = map(analogRead(A3), 587, 425, 0, 255);
  data.j2PotX = map(analogRead(A0), 432, 601, 0, 255);
  data.j2PotY = map(analogRead(A1), 436, 595, 0, 255);
  data.pot1 = map(analogRead(A7), 0, 1023, 0, 255);
  data.pot2 = map(analogRead(A6), 0, 1023, 0, 255);
  
  data.tSwitch1 = digitalRead(t1);
  data.tSwitch2 = digitalRead(t2);
  data.button1 = digitalRead(b1);
  data.button2 = digitalRead(b2);

  radio.write(&data, sizeof(Data_Package));
}

void resetData(){  
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