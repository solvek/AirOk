#include <Wire.h>
#include <U8glib.h>

#include <CO2Sensor.h>
#include <SFE_BMP180.h>
#include "DHT.h"

// If the file is missing, make a copy of CloudSample.h naming it Cloud.h
#include "Cloud.h"

#ifdef USE_CLOUD
#include <SoftwareSerial.h>
#endif

#define UNDEFINED -1

#define LOG_WIFI
//#define LOG_DATA

//////////////////////////////////////
// Pins configuration
#define PIN_CO2_AOUT A0
//#define PIN_CO2_DOUT 5
#define PIN_DHT 2
#define PIN_BUTTON 3
#define PIN_LED_RED 9
#define PIN_LED_GREEN 11
#define PIN_LED_BLUE 10
#define PIN_WIFI_RX 6
#define PIN_WIFI_TX 7

//////////////////////////////////////
// Timing configuration
// Period of updating data from sensors and displaying
#define PERIOD_UPDATE 100L
// Period of sending data to cloud
#define PERIOD_SEND 1*60*1000L
//#define PERIOD_SEND 30*1000L

CO2Sensor co2sensor(PIN_CO2_AOUT, 0.999, 20);
SFE_BMP180 bmp;
DHT dht(PIN_DHT, DHT11);

#ifdef USE_CLOUD
SoftwareSerial wifi(PIN_WIFI_RX, PIN_WIFI_TX);
#endif

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

long lastUpdateDataTime;
long lastSendDataTime;

struct {
  int co2;
  int temperature;
  int pressure;
  int humidity;
} airok;

int co2raw;

void setup() {
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);

  #ifdef PIN_CO2_DOUT
  pinMode(PIN_CO2_DOUT, INPUT);
  #endif
  
  Serial.begin(115200);
  Serial.println(F("=== Air'OK started ==="));

#ifdef USE_CLOUD
  wifi.begin(115200);
#endif

  u8g.setColorIndex(1);

  if (!bmp.begin()) {
//    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
  }

  dht.begin();

  co2sensor.calibrate();

  updateData();

  lastUpdateDataTime = 0;
  lastSendDataTime = 0;
}

void loop() {  
  long now = millis();
  if (now - lastUpdateDataTime > PERIOD_UPDATE){  
//    Serial.print("now: ");
//    Serial.println(now);
//    Serial.print("lastUpdateDataTime: ");
//    Serial.println(lastUpdateDataTime);
//    Serial.print("Dif: ");
//    Serial.println(now-lastUpdateDataTime);    
//    Serial.print("PERIOD_UPDATE: ");
//    Serial.println(PERIOD_UPDATE);    
    updateData();
    lastUpdateDataTime = now;
  }

#ifdef USE_CLOUD
  if (now - lastSendDataTime > PERIOD_SEND){
//    Serial.print(F("now: "));
//    Serial.println(now);
//    Serial.print(F("lastSendDataTime: "));
//    Serial.println(lastSendDataTime);
//    Serial.print(F("Dif: "));
//    Serial.println(now-lastSendDataTime);    
//    Serial.print(F("PERIOD_SEND: "));
//    Serial.println(PERIOD_SEND);
    connectWifi();
    sendDataToCloud();
    delay(1000);
    disconnectWifi();
    lastSendDataTime = now;
  } 
#endif
 
  pictureLoop();
  checkButton();
}

//////////////////////////////////////
// Data
void updateData(){
//  Serial.println(F("Reading data"));
  airok.co2 = readCo2();
  co2raw = co2sensor.getVoltage();

#ifdef LOG_DATA
  Serial.print(F("CO2: "));
  Serial.print(airok.co2);
  Serial.print(F(", CO2 Raw: "));
  Serial.println(co2raw);
#endif

  #ifdef PIN_CO2_DOUT
  int isCritical = digitalRead(PIN_CO2_DOUT);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, isCritical);  
  #else
  analogWrite(PIN_LED_GREEN, co2sensor.getGreenLevel());
  analogWrite(PIN_LED_RED, co2sensor.getRedLevel());
  #endif

#ifdef LOG_DATA
  Serial.print(F("CO2 concentration: "));
  Serial.print(airok.co2);
  Serial.println(F(" ppm"));
#endif

  airok.temperature = readDhtTemperature();
  airok.humidity = readHumidity();

#ifdef LOG_DATA
  Serial.print(F("DHT Temperature: "));
  Serial.println(airok.temperature);
#endif

  double temperature2;

  temperature2 = readBmpTemperature();
  if (airok.temperature == UNDEFINED) airok.temperature = temperature2;
  else if (temperature2 != UNDEFINED) airok.temperature = (airok.temperature+temperature2)/2;

  airok.pressure = (temperature2 == UNDEFINED) ?
    UNDEFINED :
    readPressure(temperature2);

#ifdef LOG_DATA
  Serial.print("BMP Temperature: ");
  Serial.println(temperature2);
  Serial.print("BMP pressure: ");
  Serial.println(airok.pressure);
#endif
}

int readCo2(){
  return co2sensor.read();
}

double readBmpTemperature(){
  double t;
  
  char status = bmp.startTemperature();
  if (status == 0){
//    Serial.println("error starting temperature measurement");
    return UNDEFINED;
  }
  
  delay(status);
  
  status = bmp.getTemperature(t);
  if (status == 0){
//    Serial.println("error retrieving temperature measurement\n");
    return UNDEFINED;
  }

  return t;
}

double readPressure(double tmp){
  double pressure;
  
  char status = bmp.startPressure(3);
  if (status == 0){
//    Serial.println("error starting pressure measurement\n");
    return UNDEFINED;
  }
  
  delay(status);
  
  status = bmp.getPressure(pressure, tmp);
  
  if (status == 0){
//    Serial.println("error retrieving pressure measurement\n");
    return UNDEFINED;
  }

  return pressure;
}

int readDhtTemperature(){
  int temperature = dht.readTemperature();
  if (isnan(temperature)) {
//    Serial.println("Failed to read temperature from DHT sensor!");
    return UNDEFINED;
  }
  return temperature;
}

int readHumidity(){
  int humidity = dht.readHumidity();
  if (isnan(humidity)) {  
//    Serial.println("Failed to read humidity from DHT sensor!");
    return UNDEFINED;
  }
  return humidity;
}

//////////////////////////////////////
// Control
void checkButton(){
  if (digitalRead(PIN_BUTTON) == LOW) return;  

  co2sensor.calibrate();  

  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  
  digitalWrite(PIN_LED_BLUE, LOW);
  delay(100);
  digitalWrite(PIN_LED_BLUE, HIGH);
  delay(500);
  digitalWrite(PIN_LED_BLUE, LOW);

  while(digitalRead(PIN_BUTTON) == HIGH);
}

//////////////////////////////////////
// Display
#define WIDTH 128
#define HEIGHT 64

void pictureLoop(){
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() ); 
}

void draw() {
  u8g.setFont(u8g_font_gdr12);
  String s = String(airok.co2)+" ppm";
  u8g.drawStr(0, 25, s.c_str());

  s = String(airok.temperature)+(char)176+"C";
  char* c = s.c_str();
  int w = u8g.getStrWidth(c);
  u8g.drawStr(WIDTH-w, 25, c);

  s = String(airok.humidity)+"%";
  u8g.drawStr(0, 62, s.c_str());

  s = String(airok.pressure)+" mb";
  c = s.c_str();
  w = u8g.getStrWidth(c);
  u8g.drawStr(WIDTH-w, 62, c);
}

//////////////////////////////////////
// Cloud
#ifdef USE_CLOUD
#define WIFI_TIMEOUT 5000

#define SEPARATOR  F("&")

void sendDataToCloud(){
  Serial.println(F("Sending data to server"));
  
  String cmd = F("AT+CIPSTART=\"TCP\",\"");
  cmd += CLOUD_IP;
  cmd += F("\",");
  cmd += CLOUD_PORT;
  sendWifiCommand(cmd, F("OK"));

  String req = CLOUD_GET;
  req += CLOUD_FIELD_CO2;
  req += airok.co2;
  req += SEPARATOR;
  req += CLOUD_FIELD_TEMPERATURE;
  req += airok.temperature;
  req += SEPARATOR;
  req += CLOUD_FIELD_PRESSURE;
  req += airok.pressure;
  req += SEPARATOR;
  req += CLOUD_FIELD_HUMIDITY;
  req += airok.humidity;

  #ifdef CLOUD_FIELD_CO2_RAW
  req += SEPARATOR;
  req += CLOUD_FIELD_CO2_RAW;
  req += co2raw;    
  #endif

  cmd = F("AT+CIPSEND=");
  cmd += (req.length()+2);
  sendWifiCommand(cmd, ">");

  sendWifiCommand(req, F("OK"));

  sendWifiCommand("AT+CIPCLOSE", "OK");  
}

void connectWifi(){ 
// Serial.println("Connecting wifi");
 for(int i=0;i<10;i++){
  if (sendWifiCommand(F("AT+RST"), F("ready"))) break;
  delay(1000);
  // Serial.println("Wifi reseted");
 }
 
 sendWifiCommand(F("AT+CWMODE=1"), "OK"); 

 String auth = F("AT+CWJAP=\"");
 auth +=CLOUD_SSID;
 auth += F("\",\"");
 auth +=CLOUD_PASS;
 auth +=F("\"");
#ifdef LOG_WIFI
 Serial.print("Auth: ");
 Serial.println(auth);
#endif
 sendWifiCommand(auth, "OK");

#ifdef LOG_WIFI 
// Print current network parameters
 sendWifiCommand("AT+CIFSR", "OK");
#endif

// This command takes the wifi to low energy (sleep) mode for provided time in ms but it doesn't wake up for me
// sendWifiCommand("AT+GSLP=10000", "OK");
}

void disconnectWifi(){ 
  sendWifiCommand(F("AT+CWQAP"), "OK");
}

bool sendWifiCommand(String command, String ack){
#ifdef LOG_WIFI
  Serial.print("Sending command: ");
  Serial.println(command);
#endif

  wifi.println(command);
  if (expectResponse(ack))
    return true;
  else{
#ifdef LOG_WIFI
    Serial.println("Failed to execute command");
#endif
    return false;
  }
}

bool expectResponse(String keyword){
 byte current_char = 0;
 byte keyword_length = keyword.length();
 long deadline = millis() + WIFI_TIMEOUT;
 while(millis() < deadline){
  if (wifi.available()){
    char ch = wifi.read();
#ifdef LOG_WIFI
    Serial.write(ch);
#endif
    if (ch == keyword[current_char])
      if (++current_char == keyword_length){
#ifdef LOG_WIFI        
       Serial.println();
#endif
       return true;
    }
   }
  }
 return false; // Timed out
}
#endif
