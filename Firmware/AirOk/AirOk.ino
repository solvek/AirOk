#include <SimpleTimer.h>
#include <CO2Sensor.h>
#include "U8glib.h"

//////////////////////////////////////
// Pins configuration
#define PIN_CO2 A0
#define PIN_DHT 2
#define PIN_BUTTON 3

//////////////////////////////////////
// Timing configuration
#define PERIOD_UPDATE 100

CO2Sensor co2sensor(PIN_CO2);
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);
SimpleTimer updateDataTimer;

struct {
  int co2;
} airok;

void setup() {
  Serial.begin(9600);
  Serial.println("=== Air'OK started ===");

  u8g.setColorIndex(1);
    
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
  airok.co2 = co2sensor.read();

  Serial.print("CO2 concentration: ");
  Serial.print(airok.co2);
  Serial.println(" ppm");
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
  u8g.setFont(u8g_font_unifont);
  String s = String(airok.co2)+" ppm";
  u8g.drawStr(0, 22, s.c_str());
}
