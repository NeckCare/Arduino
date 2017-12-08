/*
 * NeckCare
 * Version: 1.0
 * @author PeratX
 * 
 * Sketch for ESP8266
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);

bool inited = false;
String jsonData = "";
IPAddress myIp;

void handleRoot(){
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void processJsonData(){
  //start process
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonData);
  jsonData = "";
  if(root["type"].as<String>() == "init" && !inited){//Got SSID and pass!
    inited = true;
    WiFi.softAP(root["ssid"].as<char*>(), root["pass"].as<char*>());
    myIp = WiFi.softAPIP();
    server.on("/", handleRoot);
    server.begin();
    JsonObject& resp = jsonBuffer.createObject();
    resp["type"] = "init";
    resp["ip"] = myIp.toString();
    resp.printTo(Serial);
    Serial.print("\n");
    //response to the UNO
  }
}

void readJsonDataFromSerial(){
  while(Serial.available()){
    char info = char(Serial.read());
    if(info == 10){//Got the \n
      processJsonData();
    } else {
      jsonData += info;
    }
  }
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  server.handleClient();
  readJsonDataFromSerial();
}
