/*
Adafruit Arduino - Lesson 3. RGB LED
*/
 
int redPin = 10;
int greenPin = 11;
int bluePin = 9;
 
//uncomment this line if using a Common Anode LED
//#define COMMON_ANODE
 
void setup()
{
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  
}
 
void loop()
{
  setColor(0, 0, 255);
  delay(1000);
  
  setColor(0, 255, 0);
  delay(500);

  for (int i=0;i<255;i++){
    setColor(i, 255-i, 0);
    delay(50);    
  }
  
  setColor(255, 0, 0);
  delay(1000);
}
 
void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  digitalWrite(bluePin, blue);  
}
