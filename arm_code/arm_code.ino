#include "Servo.h";

Servo Shoulder;
Servo Elbow;

void setup() {
  // put your setup code here, to run once:
  Shoulder.attach(5);
  Elbow.attach(6);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Shoulder.write(10);
  Elbow.write(170);
  delay(5000);
  Shoulder.write(170);
  Elbow.write(60);
  delay(5000);
}
