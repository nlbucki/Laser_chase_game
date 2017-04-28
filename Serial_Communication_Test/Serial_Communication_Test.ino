#include <LiquidCrystal.h>
#include <Servo.h>

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo panServo, tiltServo;
const int panAngle = A0;
const int tiltAngle = A1;
const int bufferSize = 5;
int panPoints [bufferSize];
int tiltPoints [bufferSize];
int index = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.setTimeout(50);
  //lcd.begin(16,2);
  panServo.attach(5);
  tiltServo.attach(6);
}

void loop() {
  if(Serial.available() > 0){
    String input = Serial.readStringUntil('\n');
    //lcd.clear();
    //lcd.print(input);
    if(input == "ON"){
      digitalWrite(LED_BUILTIN, HIGH);
    } else if(input == "OFF") {
      digitalWrite(LED_BUILTIN, LOW);
    } else if (input == "ANGLE") {
      //lcd.clear();
      //lcd.print(input);
      //lcd.setCursor(0,1);
      String pan = Serial.readStringUntil('\n');
      String tilt = Serial.readStringUntil('\n');
      //lcd.print(pan + "," + tilt);
      panServo.write(pan.toInt());
      tiltServo.write(tilt.toInt());
    }
  }
  panPoints[index] = analogRead(panAngle);
  tiltPoints[index] = analogRead(tiltAngle);
  if(index < 5){
    index++;
  }else{
    index = 0;
  }
  Serial.print(minVal(panPoints));
  Serial.print(",");
  Serial.println(minVal(tiltPoints));
  delay(10);
}

int minVal(int points[5]){
  int minPoint = 1024;
  for(int i = 0; i < 5; i++){
    if(points[i] < minPoint){
      minPoint = points[i];
    }
  }
  return minPoint;
}

