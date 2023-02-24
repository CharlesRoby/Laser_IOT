#include <Arduino.h>

int pin_Laser = 2;

void setup(){
  Serial.begin(9600);
  pinMode(pin_Laser, OUTPUT);
}

void loop(){
  delay(2000);
  digitalWrite(pin_Laser, HIGH);
  delay(500);
  digitalWrite(pin_Laser, HIGH);
  delay(500);
  digitalWrite(pin_Laser, LOW);
  delay(500);
  digitalWrite(pin_Laser, HIGH);
  delay(500);
  digitalWrite(pin_Laser, LOW);
  Serial.println("Fin");
  
}