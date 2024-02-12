#include <Arduino.h>
#include "GyverNTC.h"
#include "Relay.h"
// #define DEBUG

#define RELAY_DELAY_W0 300000
#define RELAY_DELAY_W1 3000
#define RELAY_DELAY_W2 5000

#define W1_WORK_TIME 20 * 60ULL * 1000
// #define PROTECT_W1 60000
// #define PROTECT_DEGREES_W1 1
// #define PROTECT_RESET_W1 60000

// #define PROTECT_W2 60000
// #define PROTECT_DEGREES_W2 1
// #define PROTECT_RESET_W2 30000

#define TEMP_MIN_W0 20
#define TEMP_MAX_W0 27

#define TEMP_MIN_W1 24
#define TEMP_MAX_W1 27

#define TEMP_MIN_W2 60
#define TEMP_MAX_W2 101

GyverNTC therm(A0, 10000, 3950);
Relay relay(12);

int16_t temp, tempSet, tempPrev, tempSetPrev;
uint16_t handle;
void setup()
{
// pinMode(12, OUTPUT);
#ifdef DEBUG
  Serial.begin(9600);
#endif
  handle = analogRead(A2);
  tempSet = map(handle, 0, 1021, TEMP_MIN_W0, TEMP_MAX_W0);
  temp = (int)therm.getTempAverage();

  if (temp >= tempSet)
  {
    relay.set(0);
  }
  else
  {
    relay.set(1);
  }
  tempPrev = temp;
  tempSetPrev = tempSet;
}

uint32_t timerRelay = 0;
uint32_t timerHandle = 0;
uint32_t timerTemp = 0;
uint32_t timerMode = 0;

#ifdef DEBUG
uint32_t timerSerial = 0;
#endif

uint32_t timerTempProtect = 0;
uint32_t timerTempProtectReset = 0;

bool flag = 1;
uint8_t mode = 0;
void work0();
bool w1_f = 1;
uint32_t timer_w1 = 0;
void work1();
void work2();

void loop()
{
  // Опрос ручки поворотной
  if (uint32_t(millis() - timerHandle) >= 100)
  {
    timerHandle = millis();
    handle = analogRead(A2);

#ifdef DEBUG
    if (handle < 3)
      Serial.println("WhileMode");
#endif
    timerMode = millis();
    while (handle < 3)
    {
      relay.set(0);
      handle = analogRead(A2);
#ifdef DEBUG
      if (handle >= 3)
      {
        Serial.println("Break!");
        break;
      }
#endif
      if (uint32_t(millis() - timerMode) >= 5000)
      {
        timerMode = millis();
        if (mode < 2)
        {
          mode++;
          for (int i = 0; i < mode + 1; i++)
          {
            relay.change();
            delay(200);
            relay.change();
            delay(300);
          }
        }
        else
        {
          mode = 0;
          relay.change();
          delay(200);
          relay.change();
          delay(300);
        }
        flag = 1;
        w1_f = 1;
        timerTempProtect = millis();
        timerTempProtectReset = millis();
      }
    }
  }

  switch (mode)
  {
  case 0:
    work0();
    break;
  case 1:
    work1();
    break;
  case 2:
    work2();
    break;

  default:
    mode = 0;
    break;
  }

#ifdef DEBUG
  if (uint32_t(millis() - timerSerial) >= 1000)
  {
    timerSerial = millis();
    Serial.print("Mode:");
    Serial.println(mode);
    Serial.print("Temp: ");
    Serial.println(temp);
    Serial.print("Set: ");
    Serial.println(tempSet);
    Serial.print("Handle: ");
    Serial.println(handle);
    Serial.println("------------------");
  }
#endif
}

uint32_t timerTempSetW0 = 0;
uint32_t timerTempSetW1 = 0;
uint32_t timerTempSetW2 = 0;

#ifdef DEBUG
uint32_t w0Timer = 0;
uint32_t w1Timer = 0;
uint32_t w2Timer = 0;

#endif

void work0()
{
#ifdef DEBUG
  if (uint32_t(millis() - w0Timer) >= 1000)
  {
    w0Timer = millis();
    Serial.println("W0 Here");
  }
#endif
  // Опрос датчика температуры
  if (uint32_t(millis() - timerTemp) >= 500)
  {
    timerTemp = millis();
    temp = (int)therm.getTempAverage();
  }
  if (uint32_t(millis() - timerTempSetW0) >= 100)
  {
    timerTempSetW0 = millis();
    tempSet = map(handle, 0, 1021, TEMP_MIN_W0, TEMP_MAX_W0);
  }
  // Сброс таймера при смене температуры
  if (tempSet != tempSetPrev)
  {
    tempSetPrev = tempSet;
    tempPrev = temp;
    timerTempProtect = millis();
    timerTempProtectReset = millis();
#ifdef DEBUG
    Serial.print("!!!!!!  ");
    Serial.println(tempSet);
#endif
    timerRelay = millis() - RELAY_DELAY_W0;
    flag = 1;
  }
  // работа реле
  if (uint32_t(millis() - timerRelay) >= RELAY_DELAY_W0)
  {

    // temp = (int)therm.getTempAverage();
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
}

void work1()
{
#ifdef DEBUG
  if (uint32_t(millis() - w1Timer) >= 1000)
  {
    w1Timer = millis();
    Serial.println("W1 Here");
  }
#endif
  if (uint32_t(millis() - timer_w1) >= W1_WORK_TIME && !w1_f)
  {
    flag = 0;
  }
  // Опрос датчика температуры
  if (uint32_t(millis() - timerTemp) >= 500)
  {
    timerTemp = millis();
    temp = (int)therm.getTempAverage();
  }
  if (uint32_t(millis() - timerTempSetW1) >= 100)
  {
    timerTempSetW1 = millis();
    tempSet = map(handle, 0, 1021, TEMP_MIN_W1, TEMP_MAX_W1);
  }
  // Сброс таймера при смене температуры
  if (tempSet != tempSetPrev)
  {
    tempSetPrev = tempSet;
    tempPrev = temp;
    timerTempProtect = millis();
    timerTempProtectReset = millis();
#ifdef DEBUG
    Serial.print("!!!!!!  ");
    Serial.println(tempSet);
#endif
    timerRelay = millis() - RELAY_DELAY_W1;
    flag = 1;
    w1_f = 1;
  }
  // работа реле
  if (uint32_t(millis() - timerRelay) >= RELAY_DELAY_W1)
  {

    // temp = (int)therm.getTempAverage();
    if (temp >= tempSet)
    {
      if (relay.set(0))
      {
        timerRelay = millis();
        if (w1_f)
        {
          w1_f = 0;
          timer_w1 = millis();
        }
      }
    }
    else
    {
      if (relay.set(flag))
        timerRelay = millis();
    }
  }

#ifdef PROTECT_W1
  if ((uint32_t(millis() - timerTempProtect) >= PROTECT_W1) && flag && w1_f)
  {
    timerTempProtect = millis();
#ifdef PROTECT_RESET_W1
    timerTempProtectReset = millis();
#endif
    if (relay.getState())
    {
      if (temp - tempPrev < PROTECT_DEGREES_W2)
      {
        flag = 0;
        relay.set(0);
      }

      tempPrev = temp;
    }
  }
#ifdef PROTECT_RESET_W1
  if (!flag && !w1_f && (uint32_t(millis() - timerTempProtectReset) >= PROTECT_RESET_W1))
  {
    tempPrev = temp;
    timerTempProtectReset = millis();
    timerTempProtect = millis();
    flag = 1;
    timerRelay = millis() - RELAY_DELAY_W1;
  }
#endif
#endif
}

void work2()
{
#ifdef DEBUG
  if (uint32_t(millis() - w2Timer) >= 1000)
  {
    w2Timer = millis();
    Serial.println("W2 Here");
  }
#endif

  // Опрос датчика температуры
  if (uint32_t(millis() - timerTemp) >= 500)
  {
    timerTemp = millis();
    temp = (int)therm.getTempAverage();
  }
  if (uint32_t(millis() - timerTempSetW2) >= 100)
  {
    timerTempSetW2 = millis();
    tempSet = map(handle, 0, 1021, TEMP_MIN_W2, TEMP_MAX_W2);
  }
  // Сброс таймера при смене температуры
  if (tempSet != tempSetPrev)
  {
    tempSetPrev = tempSet;
    tempPrev = temp;
    timerTempProtect = millis();
    timerTempProtectReset = millis();
#ifdef DEBUG
    Serial.print("!!!!!!  ");
    Serial.println(tempSet);
#endif
    timerRelay = millis() - RELAY_DELAY_W2;
    flag = 1;
  }
  // работа реле
  if (uint32_t(millis() - timerRelay) >= RELAY_DELAY_W2)
  {

    // temp = (int)therm.getTempAverage();
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

#ifdef PROTECT_W2
  if ((uint32_t(millis() - timerTempProtect) >= PROTECT_W2) && flag)
  {
    timerTempProtect = millis();
#ifdef PROTECT_RESET_W2
    timerTempProtectReset = millis();
#endif
    if (relay.getState())
    {
      if (temp - tempPrev < PROTECT_DEGREES_W2)
      {
        flag = 0;
        relay.set(0);
      }

      tempPrev = temp;
    }
  }
#ifdef PROTECT_RESET_W2
  if (!flag && (uint32_t(millis() - timerTempProtectReset) >= PROTECT_RESET_W2))
  {
    tempPrev = temp;
    timerTempProtectReset = millis();
    timerTempProtect = millis();
    flag = 1;
    timerRelay = millis() - RELAY_DELAY_W2;
  }
#endif
#endif
}