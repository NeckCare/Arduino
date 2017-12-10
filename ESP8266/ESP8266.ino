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
  sendHttpRequest("/");
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(getHttpResponse());
  server.send(200, "text/html", root["body"].as<char*>());
}

void sendHttpRequest(String page){
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = "req";
  root["page"] = page;
  root.printTo(Serial);
  Serial.print("\n");
}

String getHttpResponse(){
  String data = "";
  while(Serial.available()){
    char info = char(Serial.read());
    if(info == 10){//Got the \n
      return data;
    } else {
      data += info;
    }
  }
}

void processJsonData(){
  //start process
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonData);
  jsonData = "";
  String type = root["type"].as<String>();
  if(type == "init" && !inited){//Got SSID and pass!
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
  Serial.begin(115200);//Maybe too high
}

void loop() {
  server.handleClient();
  readJsonDataFromSerial();
}
