/*
 * NeckCare
 * Version: 1.0
 * @author PeratX
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

LiquidCrystal_I2C lcd(0x3F, 16, 4);
SoftwareSerial bt(3, 2);//RX, TX

const String PROG_NAME = "NeckCare";
const String VERSION = "v1.0";

const int flexSensorPin = A0;
const int sdCardPin = 10;

int flexSensor;
bool inited = false;

void printToSecLine(String str){
  lcd.setCursor(0, 1);
  lcd.print(str);
}

void printHeader(){
  lcd.print(PROG_NAME + " " + VERSION);
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
    root["data"] = flexSensor;
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

void readFlexSensor(){
  flexSensor = analogRead(flexSensorPin);
}

void setup(){
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  printHeader();
  bt.begin(38400);
  while(!bt){;}
  Serial.println(PROG_NAME + " " + VERSION);
}

void loop(){
  readJsonDataFromSerial();
  readFlexSensor();
  printToSecLine(String(flexSensor));
}

