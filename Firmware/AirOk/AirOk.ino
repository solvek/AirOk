#include <SimpleTimer.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <U8glib.h>

#include <CO2Sensor.h>
#include <SFE_BMP180.h>
#include "DHT.h"

#define UNDEFINED -1

//////////////////////////////////////
// Pins configuration
#define PIN_CO2 A0
#define PIN_DHT 2
#define PIN_BUTTON 3

//////////////////////////////////////
// Timing configuration
#define PERIOD_UPDATE 2000

CO2Sensor co2sensor(PIN_CO2);
SFE_BMP180 bmp;
DHT dht(PIN_DHT, DHT11);

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

SimpleTimer updateDataTimer;

struct {
  int co2;
  int temperature;
  int pressure;
  int humidity;
} airok;

void setup() {
  Serial.begin(9600);
  Serial.println("=== Air'OK started ===");

  u8g.setColorIndex(1);

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
  }

  dht.begin();

  co2sensor.calibrate();

  updateData();
    
  updateDataTimer.setInterval(PERIOD_UPDATE, updateData);
}

void loop() {  
  updateDataTimer.run();
  pictureLoop();
  checkButton();
}

//////////////////////////////////////
// Data
void updateData(){
  airok.co2 = readCo2();

//  Serial.print("CO2 concentration: ");
//  Serial.print(airok.co2);
//  Serial.println(" ppm");

  airok.temperature = readDhtTemperature();
  airok.humidity = readHumidity();

  double temperature2;

  temperature2 = readBmpTemperature();
  if (airok.temperature == UNDEFINED) airok.temperature = temperature2;
  else if (temperature2 != UNDEFINED) airok.temperature = (airok.temperature+temperature2)/2;

  airok.pressure = (temperature2 == UNDEFINED) ?
    UNDEFINED :
    readPressure(temperature2);
}

int readCo2(){
  return co2sensor.read();
}

double readBmpTemperature(){
  double t;
  
  char status = bmp.startTemperature();
  if (status == 0){
    Serial.println("error starting temperature measurement");
    return UNDEFINED;
  }
  
  delay(status);
  
  status = bmp.getTemperature(t);
  if (status == 0){
    Serial.println("error retrieving temperature measurement\n");
    return UNDEFINED;
  }

  return t;
}

double readPressure(double tmp){
  double pressure;
  
  char status = bmp.startPressure(3);
  if (status == 0){
    Serial.println("error starting pressure measurement\n");
    return UNDEFINED;
  }
  
  delay(status);
  
  status = bmp.getPressure(pressure, tmp);
  
  if (status == 0){
    Serial.println("error retrieving pressure measurement\n");
    return UNDEFINED;
  }

  return pressure;
}

int readDhtTemperature(){
  int temperature = dht.readTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed to read temperature from DHT sensor!");
    return UNDEFINED;
  }
  return temperature;
}

int readHumidity(){
  int humidity = dht.readHumidity();
  if (isnan(humidity)) {  
    Serial.println("Failed to read humidity from DHT sensor!");
    return UNDEFINED;
  }
  return humidity;
}

//////////////////////////////////////
// Control
void checkButton(){
  if (digitalRead(PIN_BUTTON) == LOW) return;
  while(digitalRead(PIN_BUTTON) == HIGH);

  co2sensor.calibrate();  
}

//////////////////////////////////////
// Display
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
  u8g.drawStr(70, 25, s.c_str());

  s = String(airok.humidity)+"%";
  u8g.drawStr(0, 62, s.c_str());

  s = String(airok.pressure)+" mb";
  u8g.drawStr(64, 62, s.c_str());
}
