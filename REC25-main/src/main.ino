#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <stdio.h>
#include <Servo.h>

//LED Things
#define RED1 22
#define GREEN1 23
#define BLUE1 24
#define RED2 25
#define GREEN2 26
#define BLUE2 27
#define RED3 28
#define GREEN3 29
#define BLUE3 30
#define RED4 31
#define GREEN4 32
#define BLUE4 33
#define RED5 34
#define GREEN5 35
#define BLUE5 36
#define RED6 37
#define GREEN6 38
#define BLUE6 39

//FUNC
#define START 6
#define STOP 7

//RFID pins
#define RST_PIN 5
#define SS_PIN 53

//BUTTON pins
#define BTN_1 9
#define BTN_2 10
#define BTN_3 11
#define BTN_4 12
#define KEY_SW 12
#define SNSR_3 18

//MM EXIT PASSWORD
#define PWE "321232"
//MM MOTOR ACTIVATE PASSWORD
#define PWM "111123"

//MOTOR PINS
#define DIR 10
#define PWM 11

//LEDS
int led1[] = {RED5, GREEN5, BLUE5};
int led2[] = {RED2, GREEN2, BLUE2};
int led3[] = {RED1, GREEN1, BLUE1};
int led4[] = {RED6, GREEN6, BLUE6};
int led5[] = {RED4, GREEN4, BLUE4};
int led6[] = {RED3, GREEN3, BLUE3};
int *ledlist[] = {led1,led2,led3,led4,led5,led6};

//MFRC key and pin for reader
MFRC522 mfrc522(SS_PIN, RST_PIN);

//Authorized RFID Users
unsigned char authorizedUIDs[][4] = {{0xD3, 0xB5, 0xA8, 0xF7}, {0xA3, 0x02, 0xB1, 0xFA}};

/* State of Buttons
* ~btn1S : dispatch or 1 in password mode
* ~btn2S : Estop or 2 in password mode
* ~btn3S : Maintenence Mode or 3 in password mode
*/
int btn1S = 0;
int btn2S = 0;
int btn3S = 0;
int btn4S = 0;

//LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

//Ride State Things
extern int state = 0;
unsigned long duration = 0;

//    MISC     //
//-------------//

//Ride Operator
String user = "";
//Estop Password
String password = "";
//Ride Error Status
String status = "OK";
//Ride state text
String tState= "";

//-------------//

void setup() {
  
  Serial.begin(9600);
  buttonsSetup();
	lcdSetup();
	RFIDsetup();
	lcdInitalState();
  for(int i = 0; i < 6; i++){
    setGreen(ledlist[i]);
  }
}


void loop() {
    if(state == 0){
      if(user != ""){
        tState = "Idle";
        errorCheck();
        stats(tState);
        state = 1;
      }else{
        lcdRFIDCheck();
      }  
    }
    if(state == 1){
      errorCheck();
      btn1S = digitalRead(BTN_1);
      btn2S = digitalRead(BTN_2);
      btn3S = digitalRead(BTN_3);
      if (btn1S == LOW) {
        lcd.clear();
        maintenanceMode();
        state = 4;
      } 
      if (btn2S == HIGH){
        stats("Ride Started");
        delay(1000);
        dispButton();
      }
      if (btn3S == HIGH){
        lcd.clear();
        eStopButton();
      }
    }
    if(state == 2){
      dispatch();
      duration++;
      delay(900);
      btn3S = digitalRead(BTN_3);
      if (btn3S == HIGH){
        lcd.clear();
        eStopButton();
      }
    }
   if(state == 3){
      tState = "Dis Idle";
      for(int i = 0; i < 15; i++){
        delay(900);
        duration++;
        stats(tState);
        btn3S = digitalRead(BTN_3);
        if (btn3S == HIGH){
          lcd.clear();
          eStopButton();
        }
      }
      lcd.clear();
      tState = "Accelerating";
      state = 2;
    }
    if(state == 4){
      btn1S = digitalRead(BTN_4);
      btn2S = digitalRead(BTN_2);
      btn3S = digitalRead(BTN_3);

      if (btn4S == HIGH) {
        state = 1;
        lcd.clear();
      } 
      if (btn2S == HIGH){
        state = 0;
        lcd.clear();
      }
      if (btn3S == HIGH){
        lcd.clear();
        eStopButton();
      }
    }
    if(state == 5){ 
      btn3S = digitalRead(BTN_3);
      if(btn3S == LOW){
        setGreen(led6);
        for(int i = 0; i < 6; i++){
          if(ledlist[i][1] != HIGH){
            break;
          }
        }
        if(tState != "No Error"){
          tState = "No Error";
          status = "OK";
          errorCheck();
        }
        while(status == "OK" && tState == "No Error"){
          btn4S = digitalRead(BTN_4);
          if(btn4S == LOW){
            while(btn4S == LOW){
              btn4S = digitalRead(BTN_4);
            }
            lcd.clear();
            user = "";
            lcdInitalState();
            state = 0;
            tState = "Idle";
            loop();
          }
        }
      }
    }
  
}

void maintenanceMode(){
  state = 4;

}

void passwordcheck(){
  if(btn1S == HIGH && btn2S == HIGH){
    while(btn1S == HIGH && btn2S == HIGH){
      btn1S = digitalRead(BTN_1);
      btn2S = digitalRead(BTN_2);
    }
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Password: " + password);
    delay(2000);
    lcd.clear();
    password = "";
  }
  else if(btn1S == HIGH){
    while(btn1S == HIGH){
      btn1S = digitalRead(BTN_1);
    }
    delay(10);
    password += "1";
  }
  else if(btn2S == HIGH){
    while(btn2S == HIGH){
      btn2S = digitalRead(BTN_2);
    }
    delay(10);
    password += "2";
  }
  else if(btn3S == HIGH){
    while(btn3S == HIGH){
      btn3S = digitalRead(BTN_3);
    }
    delay(10);
    password += "3";
  }
  if(password.length() > 6){
    lcd.clear();
    password = "";
  }
  if(password == PWE){
    password = "";
    status = "OK";
    state = 0;
  }
  if(password == PWM){
    password = "";
    status = "MO";
    //activate motors
  }
}
void stats(String s){
  if(s != tState){
    tState = s;
    lcd.clear();
  }
	display_employee(user);
	lcd.setCursor(0,1);
	lcd.print("State: " + s);
	lcd.setCursor(0,2);
	lcd.print("Status: " + status);
  lcd.setCursor(0,3);
  if(state == 2 || state == 3){
    lcd.print("Duration: ");
    lcd.setCursor(10,3);
    lcd.print(duration);
  }
}

void dispButton(){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Dispatched");
  delay(1000);
  lcd.clear();
  state = 2;
}
void eStopButton(){
  setRed(led6);
  tState = "Estop";
  status = "Estop";
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("ESTOP"); 
  delay(1000);
  state = 5;
  stats(tState);
}


//RIDE FUNCTIONS
void dispatch() {
    if(duration == 0){
    tState = "Accelerating";
  }
  if(duration == 14){
    stats("Constant Speed");
    tState = "Constant Speed";
  }
  if(duration == 30){
    stats("Slowing Down");
    tState = "Slowing Down";
  }
  if(duration < 14){
    stats("Accelerating");
  }
  if(duration > 14 && duration < 30){
    stats("Constant Speed");

  }
  if(duration > 30 && duration < 45){
    stats("Slowing Down");
  }
  if(duration > 45){
    duration = 0;
    state = 3;
    lcd.clear();
  }
  for(int i = 0; i < 100; i++){
    delay(1);
    btn2S = digitalRead(BTN_2);
    if (btn2S == HIGH){
      lcd.clear();
      eStopButton();
    }
  }
}


bool voltCheck(){
  bool t = true;
  int t1 = digitalRead(led1[0]);
  int t2 = digitalRead(led1[1]);
	int t3 = digitalRead(led1[2]);
  //volt check here
  if(!t){
    if(t1 == HIGH && t2 == LOW && t3 == LOW){
      lcd.clear();
      setRed(led1);
      if(status == "OK"){
        status = "VE";
      }else if(status.indexOf("VE") != -1){
        status += ", VE";
      }
      stats(tState);
    }
    if(t1 == LOW && t2 == LOW && t3 == LOW){
      lcd.clear();
      setRed(led1);
      if(status == "OK"){
        status = "VE";
      }else if(status.indexOf("VE") != -1){
        status += ", VE";
      }
      stats(tState);
    }
    return false;
  }else{
    if(t1 == HIGH && t2 == LOW && t3 == LOW){
      lcd.clear();
      setGreen(led1);
      if(status.compareTo("VE") && status.length() == 2){
        status = "OK";
      }
      else if(status.indexOf("V") != 0){
        status.replace("VE, ", "");
      }
      else{
        status.replace(", VE", "");
      }
      stats(tState);
    }
    if(t1 == LOW && t2 == LOW && t3 == LOW){
      lcd.clear();
      setGreen(led1);
      if(status.compareTo("VE") && status.length() == 2){
        status = "OK";
      }
      else if(status.indexOf("V") != 0){
        status.replace("VE, ", "");
      }
      else{
        status.replace(", VE", "");
      }
      stats(tState);
    }
    return true;
  }
}

bool posCheck(){
  bool t = true;
  int t1 = digitalRead(led2[0]);
  int t2 = digitalRead(led2[1]);
	int t3 = digitalRead(led2[2]);
  int t4 = digitalRead(led5[0]);
  int t5 = digitalRead(led5[1]);
	int t6 = digitalRead(led5[2]);
  //volt check here
  if(!t){
    if(t1 == HIGH && t2 == LOW && t3 == LOW){
      lcd.clear();
      setRed(led2);
      if(status == "OK"){
        status = "PE";
      }else if(status.indexOf("PE") != -1){
        status += ", PE";
      }

    }if(t4 == HIGH && t5 == LOW && t6 == LOW){
      lcd.clear();
      setRed(led5);
      if(status == "OK"){
        status = "PE";
      }else if(status.indexOf("PE") != -1){
        status += ", PE";
      }
    }
    if(t1 == LOW && t2 == LOW && t3 == LOW){
      lcd.clear();
      setRed(led2);
      if(status == "OK"){
        status = "PE";
      }else if(status.indexOf("PE") != -1){
        status += ", PE";
      }
    }if(t4 == LOW && t5 == LOW && t6 == LOW){
      lcd.clear();
      setRed(led5);
      if(status == "OK"){
        status = "PE";
      }else if(status.indexOf("PE") != -1){
        status += ", PE";
      }
    }
    return false;
  }else{
    if(t1 == HIGH && t2 == LOW && t3 == LOW){
      lcd.clear();
      setGreen(led2);
      if(status.compareTo("PE") && status.length() == 2){
        status = "OK";
      }
      else if(status.indexOf("P") != 0){
        status.replace("PE, ", "");
      }
      else{
        status.replace(", PE", "");
      }
    }
    if(t4 == HIGH && t5 == LOW && t6 == LOW){
      lcd.clear();
      setGreen(led5);
      if(status.compareTo("PE") && status.length() == 2){
        status = "OK";
      }
      else if(status.indexOf("P") != 0){
        status.replace("PE, ", "");
      }
      else{
        status.replace(", PE", "");
      }
    }
    if(t1 == LOW && t2 == LOW && t3 == LOW){
      lcd.clear();
      setGreen(led2);
      if(status.compareTo("PE") && status.length() == 2){
        status = "OK";
      }
      else if(status.indexOf("P") != 0){
        status.replace("PE, ", "");
      }
      else{
        status.replace(", PE", "");
      }
    }
    if(t4 == LOW && t5 == LOW && t6 == LOW){
      lcd.clear();
      setGreen(led5);
      if(status.compareTo("PE") && status.length() == 2){
        status = "OK";
      }
      else if(status.indexOf("P") != 0){
        status.replace("PE, ", "");
      }
      else{
        status.replace(", PE", "");
      }
    }
    return true;
  }
}

bool sensorCheck(){
  bool t = true;
  int t1 = digitalRead(led4[0]);
  int t2 = digitalRead(led4[1]);
	int t3 = digitalRead(led4[2]);
  //volt check here
  if(!t){
    if(t1 == HIGH && t2 == LOW && t3 == LOW){
      lcd.clear();
      setRed(led4);
      if(status == "OK"){
        status = "SE";
      }else if(status.indexOf("SE") != -1){
        status += ", SE";
      }
    }
    if(t1 == LOW && t2 == LOW && t3 == LOW){
      lcd.clear();
      setRed(led4);
      if(status == "OK"){
        status = "SE";
      }else if(status.indexOf("SE") != -1){
        status += ", SE";
      }
    }
    return false;
  }else{
    if(t1 == HIGH && t2 == LOW && t3 == LOW){
      lcd.clear();
      setGreen(led4);
      if(status.compareTo("SE") && status.length() == 2){
        status = "OK";
      }
      else if(status.indexOf("S") != 0){
        status.replace("SE, ", "");
      }
      else{
        status.replace(", SE", "");
      }
    }
    if(t1 == LOW && t2 == LOW && t3 == LOW){
      lcd.clear();
      setGreen(led4);
      if(status.compareTo("SE") && status.length() == 2){
        status = "OK";
      }
      else if(status.indexOf("S") != 0){
        status.replace("SE, ", "");
      }
      else{
        status.replace(", SE", "");
      }
    }
    return true;
  }
}

void errorCheck(){
  bool t1 = voltCheck();
  bool t2 = posCheck();
  bool t3 = sensorCheck();
  if(!t1 || !t2 || !t3){
    state = 5;
    tState = "ERROR CHECK";
  }
  if(tState != "ERROR CHECK" && tState != "No Error"){
    stats(tState);
  }
}

//LCD FUNCTIONS
void lcdSetup(){
	lcd.init(); //Initialize LCD
	lcd.backlight(); //Turn on the backlight
}

void lcdInitalState(){
	lcd.clear();
	lcd.setCursor(5,1);
	lcd.print("SCAN BADGE");
	lcd.setCursor(5,2);
	lcd.print("TO SIGN IN");
}

void lcdRFIDCheck(){
  MFRC522::StatusCode status;
  if(!mfrc522.PICC_IsNewCardPresent()){
    return;
  }
  if(!mfrc522.PICC_ReadCardSerial()){
    return;
  }
  lcd.clear();
  String temp = "";
  for(byte i = 0; i < mfrc522.uid.size; i++){
    temp += String(mfrc522.uid.uidByte[i], HEX);
  }
  if(temp == "a32b1fa"){
    user = "Trainee";
  }else if(temp == "d3b5a8f7"){
    user = "Jordan";
  }else{
    lcd.clear();
    lcd.setCursor(5,1);
    lcd.print("Invalid RFID");
  }
}

void display_employee(String name){
	int name_offset = 20 - name.length();
	lcd.setCursor(name_offset,0);
	lcd.print(name);
}

//LED functions
void clearLED(int n[]) {
	digitalWrite(n[0],LOW);
  digitalWrite(n[1],LOW);
	digitalWrite(n[2],LOW);

}

void setGreen(int n[]) {
	digitalWrite(n[0],LOW);
  digitalWrite(n[1],HIGH);
	digitalWrite(n[2],LOW);

}

void setRed(int n[]) {
	digitalWrite(n[0],HIGH);
  digitalWrite(n[1],LOW);
	digitalWrite(n[2],LOW);
}

void setBlue(int n[]) {
	digitalWrite(n[0],LOW);
  digitalWrite(n[1],LOW);
	digitalWrite(n[2],HIGH);
}


//BUTTON FUNCTIONS
void buttonsSetup() {
  for(int i = 0; i < 6; i++){
    pinMode(ledlist[i][0],OUTPUT);
    pinMode(ledlist[i][1],OUTPUT);
    pinMode(ledlist[i][2],OUTPUT);
  }
  pinMode(DIR, OUTPUT);
  pinMode(PWM, OUTPUT);
  pinMode(BTN_1, INPUT);
  pinMode(BTN_2, INPUT);
  pinMode(BTN_3, INPUT);
  pinMode(BTN_4, INPUT);
	pinMode(START,INPUT);
	pinMode(STOP,INPUT);
}

//RFID FUNCTIONS
void RFIDsetup(){
	SPI.begin();
	mfrc522.PCD_Init(); // Init MFRC522
}
