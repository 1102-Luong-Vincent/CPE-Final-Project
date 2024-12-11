//Vincent Luong and Yuhan Tang

#include <SimpleDHT.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <RTClib.h>   // Include RTC library for DS1307

#define START_BUTTON 37

#define LED_YELLOW 1
#define LED_GREEN 2
#define LED_RED 3
#define LED_BLUE 4

#define STEPPER_PIN_1 47
#define STEPPER_PIN_2 49
#define STEPPER_PIN_3 51
#define STEPPER_PIN_4 53
int step_number = 0;

// water level sensor
// int resval;
int respin = A5;

// fan and motor
#define ENABLE 23
#define DIRA 25
#define DIRB 27
int i;

// temperture sensor
int pinDHT11 = 5;
SimpleDHT11 dht11;
// lcd
LiquidCrystal lcd(12, 11, 9, 8, 7, 6); // RS EN D4 D5 D6 D7

RTC_DS1307 rtc;

bool fanSwitch;
bool isWaterLevelHigh;
bool isTempertureHigh;

volatile bool isDisabled = true; // Tracks the current mode

// void fanOnOff(bool onoff);
// bool tempertureHighLow();
// bool waterLevel();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is not running, setting time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set the initial time and date to compile time
  }

  lcd.begin(16, 2);
  pinMode(ENABLE, OUTPUT);
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);

  pinMode(STEPPER_PIN_1, OUTPUT);
  pinMode(STEPPER_PIN_2, OUTPUT);
  pinMode(STEPPER_PIN_3, OUTPUT);
  pinMode(STEPPER_PIN_4, OUTPUT);

  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Wire.begin(); // Join i2c bus

  pinMode(START_BUTTON, INPUT_PULLUP); // Set up the Start button with pull-up resistor
  attachInterrupt(digitalPinToInterrupt(START_BUTTON), toggleMode, FALLING); // Attach ISR
  digitalWrite(LED_YELLOW, HIGH); // Turn YELLOW LED on initially (DISABLED mode)
}

void loop() {
  // put your main code here, to run repeatedly:
  // if (isDisabled) {
  //   digitalWrite(LED_YELLOW, HIGH); // Keep the YELLOW LED ON
  //   digitalWrite(LED_GREEN, LOW);
  //   digitalWrite(LED_RED, LOW);
  //   digitalWrite(LED_BLUE, LOW);
  //   // Skip monitoring
  //   return;
  // }

  recordTime();
  isWaterLevelHigh = waterLevel();
  delay(1000);
  isTempertureHigh = tempertureHighLow();
  if(isWaterLevelHigh && tempertureHighLow){
    for(int a = 0; a < 1000; a++){
      OneStep(false);
      delay(2);
    }
    for(int a = 0; a < 1000; a++){
      OneStep(true);
      delay(2);
    }
    fanSwitch = true;
  }
  else{
    fanSwitch = false;
  }
  fanOnOff(fanSwitch);

  
  // DS1307_display(); // Display time & calendar
  delay(1000); // Update once every second


}
void stepperMotorOn(){
  for(int a = 0; a < 1000; a++){
    OneStep(false);
    delay(2);
  }
  for(int a = 0; a < 1000; a++){
    OneStep(true);
    delay(2);
  }
}

void OneStep(bool dir){
  if(dir){
    switch(step_number){
      case 0:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
    }
  }
  else{
    switch(step_number){
      case 0:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
    }
  }
  step_number++;
  if(step_number > 3){
    step_number = 0;
  }
}

bool waterLevel(){
  int resval;
  bool levelHighLow;
  resval = analogRead(respin);
  Serial.println(resval);
  if(resval <= 100){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Water level:");
    lcd.setCursor(0, 1);
    lcd.print("Empty");
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, LOW);
    levelHighLow = false;
    return levelHighLow;
  }
  else if(resval > 100 && resval <= 150){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Water level:");
    lcd.setCursor(0, 1);
    lcd.print("Low");
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, HIGH);
    levelHighLow = true;
    return levelHighLow;
  }
  else if(resval > 150 && resval <= 250){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Water level:");
    lcd.setCursor(0, 1);
    lcd.print("Medium");
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, HIGH);
    levelHighLow = true;
    return levelHighLow;
  }
  else if(resval > 250){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR!!");
    // lcd.print("Water level:");
    // lcd.setCursor(0, 1);
    // lcd.print("High");
    // levelHighLow = true;
    // return levelHighLow;
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_BLUE, LOW);
    levelHighLow = false;
    return levelHighLow;
  }
}

bool tempertureHighLow(){
  byte temperture = 0;
  byte humidity = 0;
  byte data[40] = {0};
  if(dht11.read(pinDHT11, &temperture, &humidity, data)){
    Serial.print("Read DHT11 failed");
    return;
  }
  // Serial.print((int)humidity); Serial.print(" %\n");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print((int)temperture);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print((int)humidity);
  lcd.print(" %");
  bool highLow;
  if((int)temperture <= 18){
    highLow = false;
    return highLow;
  }
  else{
    highLow = true;
    return highLow;
  }
}

void fanOnOff(bool onoff){
  if(onoff){
    //delay(1000);
    //stepperMotorOn();
    digitalWrite(ENABLE, HIGH);
    digitalWrite(DIRA, HIGH);
    digitalWrite(DIRB, LOW);
  }
  else{
    //delay(1000);
    digitalWrite(DIRA, LOW);
    digitalWrite(DIRB, LOW);
    digitalWrite(ENABLE, LOW);
  }
}

// void DS1307_display() {
// Wire.beginTransmission(0x68); // Start I2C protocol with DS1307 address
// Wire.write(0); // Send register address
// Wire.endTransmission(false); // I2C restart
// Wire.requestFrom(0x68, 7); // Request 7 bytes from DS1307 and release I2C bus at end of reading
// byte second = Wire.read(); // Read seconds from register 0
// byte minute = Wire.read(); // Read minutes from register 1
// byte hour = Wire.read(); // Read hour from register 2
// Wire.read(); // Read day from register 3 (not used)
// byte date = Wire.read(); // Read date from register 4
// byte month = Wire.read(); // Read month from register 5
// byte year = Wire.read(); // Read year from register 6
// // Convert BCD to decimal
// second = (second >> 4) * 10 + (second & 0x0F);
// minute = (minute >> 4) * 10 + (minute & 0x0F);
// hour = (hour >> 4) * 10 + (hour & 0x0F);
// date = (date >> 4) * 10 + (date & 0x0F);
// month = (month >> 4) * 10 + (month & 0x0F);
// year = (year >> 4) * 10 + (year & 0x0F);
// // Create time and date strings
// char Time[11]; // "HH:MM:SS\0"
// sprintf(Time, "%02d:%02d:%02d", hour, minute, second);
// char Calendar[13]; // "DD/MM/YYYY\0"
// sprintf(Calendar, "%02d/%02d/20%02d", date, month, year);
// // Display time and date
// lcd.setCursor(0, 0);
// lcd.print(Time); // Display time
// // Serial.println(Time);
// lcd.setCursor(0, 1);
// lcd.print(Calendar); // Display date
// // Serial.println(Calendar);
// }

void recordTime() {
  // Get the current time from RTC
  DateTime now = rtc.now();
  
  // Print the current date and time
  Serial.print("Time: ");
  Serial.print(now.hour());
  printDigits(now.minute());
  printDigits(now.second());
  Serial.print(" ");
  Serial.print(now.day());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.println(now.year());
}

void printDigits(int digits) {
  // Utility function to print leading 0
  if (digits < 10) {
    Serial.print('0');
  }
  Serial.print(digits);
}

void toggleMode() {
  isDisabled = !isDisabled; // Toggle the mode
}
