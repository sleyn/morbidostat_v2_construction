/* COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include "DallasTemperature.h"
#include "Wire.h"
#include "EEPROM.h"
#include "common.h"

#define PIN_TEMP    3
#define PIN_HEATER  2
#define PIN_AIR    52
#define PIN_LED    13
#define PIN_PUMP_L  4
#define PIN_PUMP_H  5
#define PIN_VAL1_L 30
#define PIN_VAL1_H 31
#define PIN_VAL2_L 32
#define PIN_VAL2_H 33
#define PIN_VAL3_L 34
#define PIN_VAL3_H 35
#define PIN_VAL4_L 36
#define PIN_VAL4_H 37
#define PIN_VAL5_L 38
#define PIN_VAL5_H 39
#define PIN_VAL6_L 40
#define PIN_VAL6_H 41
#define PIN_LASER1 13
#define PIN_LASER2 12
#define PIN_LASER3 11
#define PIN_LASER4 10
#define PIN_LASER5  9
#define PIN_LASER6  8
#define PIN_DIOD1  A0
#define PIN_DIOD2  A3 
#define PIN_DIOD3  A6 
#define PIN_DIOD4  A9 
#define PIN_DIOD5  A12 
#define PIN_DIOD6  A15 

static const int ValPins[2][NUM_TUBES] = 
  {{PIN_VAL1_L, PIN_VAL2_L, PIN_VAL3_L, PIN_VAL4_L, PIN_VAL5_L, PIN_VAL6_L},
   {PIN_VAL1_H, PIN_VAL2_H, PIN_VAL3_H, PIN_VAL4_H, PIN_VAL5_H, PIN_VAL6_H}};
static const int LaserPins[NUM_TUBES] = 
  {PIN_LASER1, PIN_LASER2, PIN_LASER3, PIN_LASER4, PIN_LASER5, PIN_LASER6};
static const int DiodPins[NUM_TUBES] = 
  {PIN_DIOD1, PIN_DIOD2, PIN_DIOD3, PIN_DIOD4, PIN_DIOD5, PIN_DIOD6};
static OneWire oneWire(PIN_TEMP);
static DallasTemperature TempSensor(&oneWire);

void LL_Init(void)
{
  int i, j;

#if CONTROL_LED
  pinMode(PIN_LED, OUTPUT);
#endif
  LL_Heater(OFF);
  pinMode(PIN_HEATER, OUTPUT);
  LL_Air(OFF);
  pinMode(PIN_AIR, OUTPUT);
  LL_Pump(PUMP_LOW, OFF);
  LL_Pump(PUMP_HIGH, OFF);
  pinMode(PIN_PUMP_L, OUTPUT);
  pinMode(PIN_PUMP_H, OUTPUT);
  TempSensor.begin();
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < NUM_TUBES; j++)
    {
      LL_Valve(j, i, OFF);
      pinMode(ValPins[i][j], OUTPUT);
    }
  }
  for (i = 0; i < NUM_TUBES; i++)
  {
    pinMode(LaserPins[i], OUTPUT);
    LL_Laser(i, 0);
    pinMode(DiodPins[i], INPUT);
  }
}

void LL_Pump(int high, BOOL on)
{
  analogWrite(high == PUMP_HIGH ? PIN_PUMP_H : PIN_PUMP_L, on ? 255 : 0);
}

void LL_Valve(int tube, int high, BOOL on)
{
  digitalWrite(ValPins[high][tube], on ? HIGH : LOW);
}

void LL_Air(BOOL on)
{
  digitalWrite(PIN_AIR, on ? HIGH : LOW);
}

INT16 LL_Temp(void)
{
  TempSensor.requestTemperatures(); // Send the command to get temperatures
  return((TempSensor.getTempCByIndex(0) + 0.05) * 10);
}

void LL_LED(BOOL on)
{
#if CONTROL_LED
  digitalWrite(PIN_LED, on ? HIGH : LOW);
#else
  (void)on;
#endif
}

void LL_Heater(BOOL on)
{
  digitalWrite(PIN_HEATER, on ? HIGH : LOW);
}

int LL_ReadOd(int tube)
{
  return(analogRead(DiodPins[tube]));
}

void LL_Laser(int tube, int strength)
{
  analogWrite(LaserPins[tube], strength);
}
void LL_EEwrite(int address, UINT8 *data, UINT32 length)
{
  while (length--) EEPROM.update(address++, *data++);
}
void LL_EEread(int address, UINT8 *data, UINT32 length)
{
  while (length--) *data++ = EEPROM.read(address++);
}
