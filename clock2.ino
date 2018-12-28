#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define CLENGTH 60000
#define BLINKRATE 1000
#define SETTIMEOUT 4000
#define SENSE 64

#define HPIN 4
#define MPIN 5
#define BPIN A2

//Declare display object
Adafruit_7segment disp = Adafruit_7segment();

void setup() {
  disp.begin(0x70);
  Serial.begin(9600);
  pinMode(HPIN, INPUT);
  pinMode(MPIN, INPUT);
  pinMode(BPIN, INPUT);
}

void loop()
{
  unsigned long sysTime;
  unsigned long dispTime = 1234;
  unsigned long lastUpdate = 0;
  int dispGain;
  int brightness = analogRead(BPIN);
  int dispBrightness;

  printTime(dispTime);
  
  while (true)
  {
    //set consistent time for main loop
    sysTime = millis();

    //check millis overflow
    if (sysTime < lastUpdate)
    {
      if (sysTime > CLENGTH)
      {
        lastUpdate = 0;
      }
    }

    
    //Updates time if needed
    if (sysTime > lastUpdate + CLENGTH)
    {
      dispGain = (sysTime - lastUpdate) /  CLENGTH;
      dispTime += dispGain;
      dispTime = checkOverflow(dispTime);
      lastUpdate += dispGain * CLENGTH;
      printTime(dispTime);
    }

    //Check for setting
    if (digitalRead(HPIN) && digitalRead(MPIN))
    {
      printBlank();
      delay(250);
      dispTime = setTime(dispTime);
      lastUpdate = millis();
    }

    if (abs(analogRead(BPIN) - brightness) > SENSE)
    {
      Serial.println(analogRead(BPIN));
      brightness = analogRead(BPIN);
      dispBrightness = (double) brightness / 1024 * 15;
      Serial.println(dispBrightness);
      disp.setBrightness(dispBrightness);
      printTime(dispTime);
    }
  }
}

//Prints the time with semicolon
void printTime(int dispTime)
{
  disp.print(dispTime);
  disp.drawColon(true);
  disp.writeDisplay();
}

void printBlank()
{
  disp.print(10000);
  disp.drawColon(true);
  disp.writeDisplay();
}

//Checks to ensure the clock follows propper format
int checkOverflow(int dispTime)
{
  if (dispTime % 100 > 59)
  {
    dispTime += 40;
  }
  if (dispTime / 100 > 12)
  {
    dispTime -= 1200;
  }
  return dispTime;
}

//Sets the time
int setTime(int dispTime)
{
  unsigned long sysTime = millis();
  unsigned long lastUpdate = sysTime;
  unsigned long tsTime = sysTime;
  boolean setting = true;
  boolean blank = true;
  
  while (setting)
  {
    sysTime = millis();

    //Blink display
    if (sysTime > lastUpdate + BLINKRATE)
    {
      blank = !blank;
      lastUpdate = sysTime;
      if (blank)
      {
        printBlank();
      }
      else
      {
        printTime(dispTime);
      }
    }

    if (digitalRead(HPIN))
    {
      dispTime += 100;
      dispTime = checkOverflow(dispTime);
      printTime(dispTime);
      delay(200);
      tsTime = sysTime;
    }
    if (digitalRead(MPIN))
    {
      dispTime += 1;
      dispTime = checkOverflow(dispTime);
      printTime(dispTime);
      delay(100);
      tsTime = sysTime;
    }

    //Check Timeout
    if (millis() > tsTime + SETTIMEOUT)
    {
      setting = false;
    }
  }
  
  return dispTime;
}

