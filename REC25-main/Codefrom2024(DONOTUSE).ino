#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include <SPI.h>
#include <MFRC522.h>
#include "Arduino_BMI270_BMM150.h"
#include <Servo.h>

//RFID pins
#define RST_PIN 5
#define SS_PIN 53

//Sensor Data
#define SNSR_1 2
#define SNSR_2 3
#define SNSR_3 18

//Motor Pins
#define ENA 32
#define IN1 33
#define IN2 34
#define IN3 35
#define IN4 36
#define ENB 37

//LED LIGHT
#define RED 25
#define GREEN 27
#define BLUE 29

//Buttons
#define START 6
#define STOP 7

MFRC522 mfrc522(SS_PIN, RST_PIN); 
Servo dispatchServo();
LiquidCrystal_I2C lcd(0x27, 20, 4);

byte bryanUID[] = {0xD3, 0x6D, 0xC0, 0xF7};

extern int state = 0;
int servoPos = 0;
String user = "";
int error_codes[5];
//extern QueueList<int> error_queue(10); //initilize a queue with max size of 10

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcdSetup();
  RFIDsetup();
  buttonsSetup();
  sensorsSetup();
  lcdInitalState(); 
}

void loop() {
  // put your main code here, to run repeatedly:
  switch(state) {
    case 0: //rfid sign in state
      lcdInitalState();
      scanRFID();
    case 1: //dispatch
      if(true){//if error queue us empty
        /**
        STEP 1:
        - purposely wait for dispatch button idk how to do this seach up polling stuff 
        - since it stays in state 1 maybe just use an if statement to if button is high keeps looping in state 1 till button is hit?
        **/
        setGreen(); //green and ready for dispatch
        while(START == HIGH);
        clearLED(); //clear green led once dispatched
        /**
        STEP 2:
        - activate dispatch mode code for motors to move or brake to declamp whatever we use
        **/
        dispatch();
      }
      else{
        //handle erros here somehow? error display mode? maintenacne verify?
      }
      break;
    case 2: //brake run
      //brakeRun();
    break;

    case 3: //emergency stop function
      emergencyStop();
    break;

    default:
    break;
  }
}


//LCD FUNCTIONS
void lcdSetup(){
  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight
  lcd.clear(); // Clear the LCD screen
}
void lcdInitalState(){
  lcd.clear();
  lcd.setCursor(5,1);
  lcd.print("SCAN BADGE");
  lcd.setCursor(5,2);
  lcd.print("TO SIGN IN");
}
void display_employee(String name){ //have this as part of the sign in function somewhere
  int name_offset = 20 - name.length();
  lcd.setCursor(name_offset,0);
  lcd.print(name);
}
void lcdDefaultState(String name){
  //setup required for lcd screen
  lcd.clear();
  display_employee(name);
  lcd.setCursor(0,1);
  lcd.print("ZONE: "); //text remains contant add status starting coordinates (8,1)
  lcd.setCursor(0,2);
  lcd.print("Status: OK"); //to change status set cursor at (8,2)
}
void updateStatus(String status, String error_code = "0"){
  lcd.setCursor(8,1);
  lcd.print(status);
  if(status == "Error"){
    lcd.setCursor(14,2);
    lcd.print(error_code);
    lcd.setCursor(0,3);
    lcd.print("View OP Manual");
  }
}
void updateZone(int zone_number) { //updates zone on lcd screen
  lcd.setCursor(8,1); 
  switch(zone_number) {
    case 1:
      lcd.print("Dispatch");
      break;
    case 2:
      lcd.print("Brake Run");
      break;
    case 5:
      lcd.print("Elevator");
      break;
    default:
      lcd.print("Unknown");
      break;
  }
}

//LED functions
void setGreen() {
  digitalWrite(RED,LOW);
  digitalWrite(BLUE,LOW);
  digitalWrite(GREEN,HIGH);
}
void clearLED() {
  digitalWrite(RED,LOW);
  digitalWrite(BLUE,LOW);
  digitalWrite(GREEN,LOW);
}
void setRed() {
  digitalWrite(RED,HIGH);
  digitalWrite(BLUE,HIGH);
  digitalWrite(GREEN,HIGH);
}

//BUTTON FUNCTIONS
void buttonsSetup() {
  //setup required for buttons, pinMode etc.
  pinMode(RED,OUTPUT);
  pinMode(GREEN,OUTPUT);
  pinMode(BLUE,OUTPUT);

  pinMode(START,INPUT);
  pinMode(STOP,INPUT);
}
void emergencyStop() {
  //ESTOP fucntion
}
// void dispatch() {
//   //start green button functionality
//   for (servoPos = 0; servoPos <= 180; servoPos += 1) { // goes from 0 degrees to 180 degrees in steps of 1 degree
//     dispatchServo.write(servoPos);    // tell servo to go to position in variable 'pos'
//     delay(5);                       // waits 15ms for the servo to reach the position
//   }
//   for (servoPos = 180; servoPos >= 0; servoPos -= 1) { // goes from 180 degrees to 0 degrees
//     dispatchServo.write(servoPos);              // tell servo to go to position in variable 'pos'  
//   }                   // waits 15ms for the servo to reach the position
// }
void regularStop(){
  //brake run stop functionality
}


//RFID FUNCTIONS
void RFIDsetup(){
  SPI.begin();			
  mfrc522.PCD_Init();		// Init MFRC522
}
bool compareUID(byte* uid1, byte* uid2, byte length) {
  for (byte i = 0; i < length; i++) {
    if (uid1[i] != uid2[i]) {
      return false;  // UID does not match
    }
  }
  return true;  // UID matches
}
void scanRFID(){ //maybe set rfid setup state?
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    byte currentUID[4];  // Store the scanned UID

    for (byte i = 0; i < 4; i++) {
      currentUID[i] = mfrc522.uid.uidByte[i];
    }

    if (compareUID(currentUID, bryanUID, 4)) { //if rfid uid matches
      state = 1;
      user = "Bryan";
    } 
    else { //if rfid uid does not match
      lcd.clear();
      lcd.print("No Such User");
      delay(5000);
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}


//Sensor Interrupts
void sensorsSetup() {
  attachInterrupt(digitalPinToInterrupt(SNSR_1), dispatchISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SNSR_2), brakeRunISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SNSR_3), elevatorISR, HIGH);
}
void dispatchISR () {
  // motion_status = inMotion();
  // if (!motion_status) {
  //   updateZone(1);
  //   state = 1;
  // }
  Serial.println("dispatch")
}
void brakeRunISR () {
  // updateZone(2);
  // state = 2;
  Serial.println("brake run")
}
void elevatorISR () {
  // updateZone(3);
  // state = 2;
  Serial.println("elevator")
}
