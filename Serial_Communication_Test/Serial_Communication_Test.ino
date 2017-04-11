#include <LiquidCrystal.h>
#include <Servo.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo panServo, tiltServo;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  lcd.begin(16,2);
  panServo.attach(9);
}

void loop() {
  if(Serial.available() > 0){
    String input = Serial.readStringUntil(';');
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
      String pan = Serial.readStringUntil(';');
      String tilt = Serial.readStringUntil(';');
      //lcd.print(pan + "," + tilt);
      panServo.write(pan.toInt());
    }
  }
}
