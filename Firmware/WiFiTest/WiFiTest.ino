#include <SoftwareSerial.h>

#define PIN_WIFI_RX 6
#define PIN_WIFI_TX 7

SoftwareSerial wifi(PIN_WIFI_RX, PIN_WIFI_TX);

void setup() {
  Serial.begin(115200);
  wifi.begin(115200);
//  Serial.println("Initialized");
}

int b = 0;

void loop() {
 if (Serial.available()){
//  Serial.write(">");
  while(Serial.available()){   
    b = Serial.read();
//    Serial.write(b);
    wifi.write(b);
  }
 }

 if (wifi.available()){
//  Serial.write("<");
  while(wifi.available()){   
    b = wifi.read();
    Serial.write(b);
  }
 }
}
