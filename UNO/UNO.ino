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
SoftwareSerial wifi(3, 2);//RX, TX

const String PROG_NAME = "NeckCare";
const String VERSION = "v1.0";

const int flexSensorPin = A0;
const int sdCardPin = 10;

int flexSensor;

void printToSecLine(String str){
  lcd.setCursor(0, 1);
  lcd.print(str);
}

void printHeader(){
  lcd.print(PROG_NAME + " " + VERSION);
}

String jsonData = "";

void readJsonDataFromSerial(){
  while(wifi.available()){
    char info = char(wifi.read());
    Serial.print(info);
    if(info == 10){//Got the \n
      processJsonData();
    } else {
      jsonData += info;
    }
  }
}

void processJsonData(){
  //start process
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonData);
  jsonData = "";
  if(root["type"].as<String>() == "init"){//Got init result
    printToSecLine(root["ip"].as<String>());
  }
}

void setup(){
  Serial.begin(115200);
  wifi.begin(115200);
  lcd.init();
  lcd.backlight();
  printHeader();
  printToSecLine("Initiating...");
  //Init Micro SD card
  SD.begin(sdCardPin);
  //Send SSID & password to ESP8266
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = "init";
  root["ssid"] = "NeckCare-v1.0";
  root["pass"] = "neckcare";
  root.printTo(wifi);
  wifi.print("\n");//\n
  //Send data done
  delay(1000);
  lcd.clear();
  printHeader();
}

void loop(){
  readJsonDataFromSerial();
  flexSensor = analogRead(flexSensorPin);
  printToSecLine(String(flexSensor));
}

