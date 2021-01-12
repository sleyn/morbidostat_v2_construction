/* COPYRIGHT (c) 2019 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include "common.h"

static BOOL valve[NUM_TUBES][2];
static BOOL pumpOn[2];
static BOOL airOn;
static BOOL heaterOn;

void HWC_SetValve(int tube, int high, BOOL on)
{
  valve[tube][high] = on;
  LL_Valve(tube, high, on);
  GUI_SetValve(tube, high, on);
}

void HWC_ToggleValve(int tube, int high)
{
  HWC_SetValve(tube, high, !valve[tube][high]);
}

void HWC_SetPump(int high, BOOL on)
{
  pumpOn[high] = on;
  LL_Pump(high, on);
  GUI_SetPump(high, on);
}

void HWC_TogglePump(int high)
{
  HWC_SetPump(high, !pumpOn[high]);
}

void HWC_SetAir(BOOL on)
{
  airOn = on;
  LL_Air(on);
  GUI_SetAir(on);
}

void HWC_ToggleAir(void)
{
  HWC_SetAir(!airOn);
}

void HWC_SetHeater(BOOL on)
{
  heaterOn = on;
  LL_Heater(on);
  GUI_SetHeater(on);
}

void HWC_ToggleHeater(void)
{
  HWC_SetHeater(!heaterOn);
}

void HWC_Init(void)
{
  int i, j;

  HWC_SetHeater(OFF);
  HWC_SetAir(OFF);
  HWC_SetPump(PUMP_LOW, OFF);
  HWC_SetPump(PUMP_HIGH, OFF);

  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < NUM_TUBES; j++)
    {
      HWC_SetValve(j, i, OFF);
    }
  }
}