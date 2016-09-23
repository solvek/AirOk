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

//////////////////////////////////////
// Pins configuration
#define PIN_CO2_AOUT A0
#define PIN_CO2_DOUT 5
#define PIN_DHT 2
#define PIN_BUTTON 3
#define PIN_LED 12
#define PIN_WIFI_RX 7
#define PIN_WIFI_TX 6

//////////////////////////////////////
// Timing configuration
// Period of updating data from sensors and displaying
#define PERIOD_UPDATE 100L
// Period of sending data to cloud
#define PERIOD_SEND 10*60*1000L

//////////////////////////////////////
// Other configuration 
// Red LED indicator will be activated if actual concentration exceeds this value
// If CRITICAL_CO2 then signal from CO2 sensor will be used
#define CRITICAL_CO2 600

CO2Sensor co2sensor(PIN_CO2_AOUT);
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

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);

  #ifndef CRITICAL_CO2
  pinMode(PIN_CO2_DOUT, INPUT);
  #endif
  
  Serial.begin(115200);
//  Serial.println("=== Air'OK started ===");

#ifdef USE_CLOUD
  wifi.begin(115200);
  connectWifi();
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
//    Serial.print("now: ");
//    Serial.println(now);
//    Serial.print("lastSendDataTime: ");
//    Serial.println(lastSendDataTime);
//    Serial.print("Dif: ");
//    Serial.println(now-lastSendDataTime);    
//    Serial.print("PERIOD_SEND: ");
//    Serial.println(PERIOD_SEND);
    
    sendDataToCloud();
    lastSendDataTime = now;
  } 
#endif
 
  pictureLoop();
  checkButton();
}

//////////////////////////////////////
// Data
void updateData(){
//  Serial.println("Reading data");
  airok.co2 = readCo2();

  #ifdef CRITICAL_CO2
  digitalWrite(PIN_LED, airok.co2<CRITICAL_CO2 ? LOW : HIGH);
  #else
  int isCritical = digitalRead(PIN_CO2_DOUT);
  digitalWrite(PIN_LED, isCritical);
  #endif

//  Serial.print("CO2 concentration: ");
//  Serial.print(airok.co2);
//  Serial.println(" ppm");

  airok.temperature = readDhtTemperature();
  airok.humidity = readHumidity();

//  Serial.print("DHT Temperature: ");
//  Serial.println(airok.temperature);

  double temperature2;

  temperature2 = readBmpTemperature();
  if (airok.temperature == UNDEFINED) airok.temperature = temperature2;
  else if (temperature2 != UNDEFINED) airok.temperature = (airok.temperature+temperature2)/2;

  airok.pressure = (temperature2 == UNDEFINED) ?
    UNDEFINED :
    readPressure(temperature2);

//  Serial.print("BMP Temperature: ");
//  Serial.println(temperature2);
//  Serial.print("BMP pressure: ");
//  Serial.println(airok.pressure);
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

  digitalWrite(PIN_LED, LOW);
  delay(100);
  digitalWrite(PIN_LED, HIGH);
  delay(500);
  digitalWrite(PIN_LED, LOW);

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

void sendDataToCloud(){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += CLOUD_IP;
  cmd += "\",";
  cmd += CLOUD_PORT;
  sendWifiCommand(cmd, "OK");

  String req = CLOUD_GET;
  req += "field1=";
  req += airok.co2;
  req += "&field2=";
  req += airok.temperature;
  req += "&field3=";
  req += airok.pressure;
  req += "&field4=";
  req += airok.humidity;

  cmd = "AT+CIPSEND=";
  cmd += (req.length()+2);
  sendWifiCommand(cmd, ">");

  sendWifiCommand(req, "OK");

//  sendWifiCommand("AT+CIPCLOSE", "OK");  
}

void connectWifi(){ 
// Serial.println("Connecting wifi");
 sendWifiCommand("AT+RST", "ready");
// Serial.println("Wifi reseted");
 
 sendWifiCommand("AT+CWMODE=1", "OK"); 

 String auth = "AT+CWJAP=\"";
 auth +=CLOUD_SSID;
 auth += "\",\"";
 auth +=CLOUD_PASS;
 auth +="\"";
// Serial.print("Auth: ");
// Serial.println(auth);
 sendWifiCommand(auth, "OK");
// sendWifiCommand("AT+CIFSR", "OK");

// This command takes the wifi to low energy (sleep) mode for provided time in ms but it doesn't wake up for me
// sendWifiCommand("AT+GSLP=10000", "OK");
}

bool sendWifiCommand(String command, String ack){
//  Serial.print("Sending command: ");
//  Serial.println(command);

  wifi.println(command);
  if (expectResponse(ack))
    return true;
//  else
//    Serial.println("Failed to execute command");
}

bool expectResponse(String keyword){
 byte current_char = 0;
 byte keyword_length = keyword.length();
 long deadline = millis() + WIFI_TIMEOUT;
 while(millis() < deadline){
  if (wifi.available()){
    char ch = wifi.read();
//    Serial.write(ch);
    if (ch == keyword[current_char])
      if (++current_char == keyword_length){
//       Serial.println();
       return true;
    }
   }
  }
 return false; // Timed out
}
#endif
