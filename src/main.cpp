#include <Arduino.h>
#include "GyverNTC.h"
#include "Relay.h"
// #define DEBUG

#define RELAY_DELAY 300000

// #define PROTECT 60000
// #define PROTECT_DEGREES 1
// #define PROTECT_RESET 60000

#define TEMP_MIN 20
#define TEMP_MAX 27

GyverNTC therm(A0, 10000, 3950);
Relay relay(12);

int16_t temp, tempSet, tempPrev, tempSetPrev;
void setup()
{
// pinMode(12, OUTPUT);
#ifdef DEBUG
  Serial.begin(9600);
#endif
  tempSet = map(analogRead(A2), 0, 1021, TEMP_MIN, TEMP_MAX);
  temp = (int)therm.getTempAverage();

  if (temp >= tempSet)
  {
    relay.set(0);
  }
  else
  {
    relay.set(1);
  }
#ifdef PROTECT
  tempPrev = temp;
#endif
  tempSetPrev = tempSet;
}

uint32_t timerRelay = 0;
uint32_t timerHandle = 0;
uint32_t timerTemp = 0;
#ifdef DEBUG
uint32_t timerSerial = 0;
#endif

#ifdef PROTECT
uint32_t timerTempProtect = 0;
#ifdef PROTECT_RESET
uint32_t timerTempProtectReset = 0;
#endif
#endif
bool flag = 1;

void loop()
{
  if (uint32_t (millis() - timerTemp) >= 500)
  {
    timerTemp = millis();
    temp = (int)therm.getTempAverage();
  }
  if (uint32_t (millis() - timerHandle) >= 100)
  {
    timerHandle = millis();
    tempSet = map(analogRead(A2), 0, 1021, TEMP_MIN, TEMP_MAX);
  }

  if (tempSet != tempSetPrev)
  {
    tempSetPrev = tempSet;
#ifdef DEBUG
    Serial.print("!!!!!!  ");
    Serial.println(tempSet);
#endif
    timerRelay = millis() - RELAY_DELAY;
  }

  if (uint32_t (millis() - timerRelay) >= RELAY_DELAY)
  {

    temp = (int)therm.getTempAverage();
    if (temp >= tempSet)
    {
      if (relay.set(0))
        timerRelay = millis();
    }
    else
    {
      if (relay.set(flag))
        timerRelay = millis();
    }
  }

#ifdef PROTECT
  if ((uint32_t (millis() - timerTempProtect) >= PROTECT) && flag)
  {
    timerTempProtect = millis();
#ifdef PROTECT_RESET
    timerTempProtectReset = millis();
#endif
    if (temp - tempPrev < PROTECT_DEGREES)
    {
      flag = 0;
      relay.set(0);
    }
  }
#ifdef PROTECT_RESET
  if (!flag && (uint32_t (millis() - timerTempProtectReset) >= PROTECT_RESET))
  {
    tempPrev = temp;
    timerTempProtectReset = millis();
    timerTempProtect = millis();
    flag = 1;
  }
#endif
#endif
#ifdef DEBUG
  if (uint32_t (millis() - timerSerial) >= 1000)
  {
    timerSerial = millis();
    Serial.print("Temp: ");
    Serial.println(temp);
    Serial.print("Set: ");
    Serial.println(tempSet);
    Serial.print("Handle: ");
    Serial.println(analogRead(A2));
    Serial.println("------------------");
  }
#endif
}
