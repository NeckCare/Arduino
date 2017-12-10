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
  while(wifi.available()){
    byte in = wifi.read();
    Serial.print(in);
    char info = char(in);
    if(info == 10){//Got the \n
      processJsonData();
    } else {
      jsonData += info;
    }
  }
}

void processJsonData(){
  //start process
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonData);
  jsonData = "";
  String type = root["type"].as<String>();
  if(type == "init"){//Got init result
    //format: ip: String
    printToSecLine(root["ip"].as<String>());
  }
  if(type == "req"){//Receive http request from ESP8266
    //format: page: String, "/data?type=all", "/time"
    String page = root["page"].as<String>();

    StaticJsonBuffer<256> jsonResponse;
    JsonObject& resp = jsonResponse.createObject();
    resp["type"] = "resp";
    resp["body"] = millis();
    resp.printTo(wifi);
    wifi.print("\n");
  }
}

void readFlexSensor(){
  flexSensor = analogRead(flexSensorPin);
}

String ssid = "";
String pass = "";

void setup(){
  Serial.begin(115200);
  wifi.begin(115200);//Maybe too high
  //cannot receive full info from ESP8266 or the info is in wrong coding format
  lcd.init();
  lcd.backlight();
  //Init Micro SD card
  if(SD.begin(sdCardPin)){
    File conf;
    if(SD.exists("config.txt")){
      conf = SD.open("config.txt");
      String configuration = "";
      while(conf.available()){
        char chr = char(conf.read());
        if(chr == 10){
          if(ssid.equals("")){
            ssid = configuration;
          } else {
            pass = configuration;
          }
          configuration = "";
        } else {
          configuration += chr;
        }
      }
      Serial.println(ssid);
      Serial.println(pass);
    } else {
      conf = SD.open("config.txt", FILE_WRITE);
      ssid = "NeckCareAP";
      pass = "neckcare";
      conf.print(ssid + "\n");
      conf.print(pass + "\n");
    }
    conf.close();
  }
}

void loop(){
  if(!inited){
    inited = true;
    printHeader();
    //Send SSID & password to ESP8266
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["type"] = "init";
    root["ssid"] = ssid;
    root["pass"] = pass;
    root.printTo(wifi);
    wifi.print("\n");//\n
    //Send data done
    printToSecLine(ssid);
    delay(3000);
    lcd.clear();
    printHeader();
    printToSecLine(pass);
    delay(3000);
    lcd.clear();
    printHeader();
  }
  readJsonDataFromSerial();
  readFlexSensor();
  printToSecLine(String(flexSensor));
}

