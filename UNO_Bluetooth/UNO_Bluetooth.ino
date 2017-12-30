/*
 * NeckCare
 * Version: 0.1.0-alpha.1
 * @author PeratX
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#define PT_USE_TIMER
#include <pt.h>

LiquidCrystal_I2C lcd(0x3F, 16, 4);
SoftwareSerial bt(3, 2);//RX, TX

const String PROG_NAME = "NeckCare";
const String VERSION = "0.1.0-a";

const int ACTION_DEC = 0;//flex decreasing
const int ACTION_INC = 1;//flex increasing

const int flexSensorPin = A0;
const int sdCardPin = 10;
const int ledPin = 4;
const int buzzerPin = 5;

static struct pt ledTask, mainTask;

const int DEFAULT_TIMEOUT = 5;
const int DEFAULT_MIN_FLEX = 30;

byte timeout;
byte minFlex;

long timeNotMoveStartFrom = 0;
int lastFlex = 0;
bool isWarning = false;

void printToSecLine(String str){
  lcd.setCursor(0, 1);
  lcd.print(str);
}

void printHeader(){
  lcd.print(PROG_NAME + " " + VERSION);
}

void resetConfig(){
  EEPROM.write(0x00, 0);
  loadConfig();
}

void loadConfig(){
  //ATmega328p with 1KBytes EEPROM
  byte initResult = EEPROM.read(0x00);
  if(initResult == 0){
    //first time start up
    Serial.println("First time startup!");
    EEPROM.write(0x00, 1);//loaded
    EEPROM.write(0x01, DEFAULT_TIMEOUT);
    EEPROM.write(0x02, DEFAULT_MIN_FLEX);
  } else {
    timeout = EEPROM.read(0x01);
    minFlex = EEPROM.read(0x02);
  }
}

void writeConfig(){
  EEPROM.write(0x01, timeout);
  EEPROM.write(0x02, minFlex);
}

String jsonData = "";

void readJsonDataFromSerial(){
  while(bt.available()){
    char info = char(bt.read());
    if(info == 13){
      //do nothing! \r
    } else if(info == 10){//Got the \n
      processJsonData();
    } else {
      jsonData += info;
    }
  }
}

void processJsonData(){
  Serial.print("Got JSON: ");
  Serial.println(jsonData);
  //start process
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonData);
  jsonData = "";
  String type = root["type"].as<String>();
  if(type == "flex"){
    root["data"] = lastFlex;
    root.printTo(bt);
    bt.print("\n");
  } else if(type == "set"){
    JsonObject& response = jsonBuffer.createObject();
    response["type"] = "set_result";
    if(root.containsKey("bl")){//backlight
      bool bl = root["bl"].as<bool>();
      if(bl){
        lcd.backlight();
      } else {
        lcd.noBacklight();
      }
      response["bl"] = true;
    }
    response.printTo(bt);
    bt.print("\n");
  }
}

static int processFlex(struct pt *pt){
  PT_BEGIN(pt);
  while(1){
    int flex = analogRead(flexSensorPin);
    if(abs(flex - lastFlex) >= minFlex){
      timeNotMoveStartFrom = millis();
    }
    if((millis() - timeNotMoveStartFrom) > (timeout * 1000)){
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
    }
    lastFlex = flex;
    PT_TIMER_DELAY(pt, 1000);
    PT_YIELD(pt);
  }
  PT_END(pt);
}

static int ledEntry(struct pt *pt){
  PT_BEGIN(pt);
  while(1){
    digitalWrite(ledPin, !digitalRead(ledPin));
    PT_TIMER_DELAY(pt, 500);
    PT_YIELD(pt);
  }
  PT_END(pt);
}

void setup(){
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  printHeader();
  bt.begin(38400);
  while(!bt){;}
  loadConfig();
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  Serial.println(PROG_NAME + " " + VERSION);
}

void loop(){
  //ledEntry(&ledTask);
  processFlex(&mainTask);
  readJsonDataFromSerial();
  printToSecLine(String(lastFlex) + " " + String(timeNotMoveStartFrom));
}

