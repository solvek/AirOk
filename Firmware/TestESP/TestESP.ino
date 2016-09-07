#include <SoftwareSerial.h>

#include <stdlib.h>

#define SSID "TechInc"

#define PASS "_default_mk2442"

#define IP "184.106.153.149" // thingspeak.com

String GET = "GET /update?key=MPV0PTE0OEKPPAPS&field1=";

SoftwareSerial monitor(10, 11); // RX, TX

void setup(){

  monitor.begin(9600);
  
  Serial.begin(9600);
  
  sendDebug("AT");
  
  delay(5000);
  
//  if(Serial.find("OK")){  
    monitor.println("RECEIVED: OK");
    connectWiFi();
//  }
//  else {
//    Serial.println("Serial not ok");
//  }
}

void loop(){

  float tempC = 23;

  char buffer[10];

  String tempF = dtostrf(tempC, 4, 1, buffer);

  updateTemp(tempF);

  delay(60000);
}

void updateTemp(String tenmpF){

  String cmd = "AT+CIPSTART=\"TCP\",\"";

  cmd += IP;

  cmd += "\",80";

  sendDebug(cmd);

  delay(2000);

  if(Serial.find("Error")){
    monitor.print("RECEIVED: Error");
    return;
  }

  cmd = GET;

  cmd += tenmpF;

  cmd += "\r\n";

  Serial.print("AT+CIPSEND=");

  Serial.println(cmd.length());

  if(Serial.find(">")){
    monitor.print(">");
    monitor.print(cmd);
    Serial.print(cmd);
  }else{
    sendDebug("AT+CIPCLOSE");
  }

  if(Serial.find("OK")){
    monitor.println("RECEIVED: OK");
  }else{
    monitor.println("RECEIVED: Error");
  }
}

void sendDebug(String cmd){
  monitor.print("SEND: ");
  monitor.println(cmd);
  Serial.println(cmd);
}

boolean connectWiFi(){
  Serial.println("AT+CWMODE=1");
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";

  sendDebug(cmd);

  delay(5000);

  if(Serial.find("OK")){
    monitor.println("RECEIVED: OK");
    return true;
  }else{
    monitor.println("RECEIVED: Error");
    return false;
  }
}
