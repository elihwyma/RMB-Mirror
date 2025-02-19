#include "Servo.h";

Servo Shoulder;
Servo Elbow;

float currentx;
float currenty;

void StraightLine(float targetx, float targety){
  float m = (currenty - targety)/(currentx - targetx);
  float c = targety - m * targetx;
  if (abs(targetx - currentx) > abs(targety - currenty)){
    if (currentx < targetx){
      for (int x = currentx; x < targetx; x++){
        float y = m * x + c;
        InverseKinematics(x, y);
      }
    }
    else if (currentx > targetx){
      for (int x = currentx; x > targetx; x--){
        float y = m * x + c;
        InverseKinematics(x, y);
      }
    }
  }
  else{
    if (currenty < targety){
      for (int y = currenty; y < targety; y++){
        float x = (y - c)/m;
        InverseKinematics(x, y);
      }
    }
    else if (currenty > targety){
      for (int y = currenty; y > targety; y--){
        float x = (y - c)/m;
        InverseKinematics(x, y);
      }
    }
  }
}

void Circle(float centrex, float centrey, float radius){
  Arc(centrex, centrey, radius, 1);
  Arc(centrex, centrey, radius, 2);
  Arc(centrex, centrey, radius, 3);
  Arc(centrex, centrey, radius, 4);
}

void Arc(float centrex, float centrey, float radius, int segment){
  float targetx;
  float targety;
  if ((segment == 1)||(segment == 4)){
    if (segment == 1){
      currentx = centrex;
      currenty = centrey + radius;
      targetx = centrex + radius;
      targety = centrey;
    }
    else if (segment == 4){
      currentx = centrex - radius;
      currenty = centrey;
      targetx = centrex;
      targety = centrey + radius;
    }
    for (int x = currentx; x < targetx; x++){
      int y = sqrt(sq(radius) - sq(x - centrex)) + centrey;
      InverseKinematics(x,y);
    }
  }
  else if ((segment == 2)||(segment == 3)){
    if (segment == 2){
      currentx = centrex + radius;
      currenty = centrey;
      targetx = centrex;
      targety = centrey - radius;
    }
    else if (segment == 3){
      currentx = centrex;
      currenty = centrey - radius;
      targetx = centrex - radius;
      targety = centrey;
    }
    for (int x = currentx; x > targetx; x--){
      int y = -sqrt(sq(radius) - sq(x - centrex)) + centrey;
      InverseKinematics(x,y);
    }
  }
}

void InverseKinematics(float x, float y) {
  float l1 = 110;
  float l2 = 135;
  float angle1;
  float angle2;
  float rad_angle1;
  float rad_angle2;
  float pi = 3.1415926535897932384626433832795;
  //Calculate IK & convert to radians - rad_angle2 = beta, rad_angle1 = alpha
  rad_angle2 = acos( (sq(x) + sq(y) - sq(l1) - sq(l2) ) / (2.0 * l1 * l2) );
  rad_angle1 = atan2(y, x) - atan2(l2 * sin(- rad_angle2), l1 + l2 * cos(- rad_angle2) );
  //Convert to degrees
  angle1 = rad_angle1 * (180 / pi);
  angle2 = rad_angle2 * (180 / pi);
  //Apply degrees to servos
  Shoulder.write(angle1);
  Elbow.write(angle2);
  currentx = x;
  currenty = y;
  delay(50);
}

void setup() {
  Shoulder.attach(5);
  Elbow.attach(6);
  Serial.begin(9600);
  currentx = 0;
  currenty = 50;
}

void loop() {
  // Smiley Face
  Circle(0,100,40);
  Circle(-15,110,10);
  Circle(15,110,10);
  Arc(0,100,30,2);
  Arc(0,100,30,3);
  Arc(0,100,40,4);
}
