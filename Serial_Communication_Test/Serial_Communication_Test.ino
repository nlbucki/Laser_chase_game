#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  lcd.begin(16,2);
}

void loop() {
  if(Serial.available() > 0){
    String input = Serial.readStringUntil(';');
    lcd.setCursor(0, 1);
    lcd.print(input);
    if(input == "ON"){
      digitalWrite(LED_BUILTIN, HIGH);
    } else if(input == "OFF") {
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

}
