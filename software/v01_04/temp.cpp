/* COPYRIGHT (c) 2019 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include "common.h"

#define TEMP_MIN (-2730)
#define TEMP_MAX 10000

static INT16 SetTemp, maxTemp, minTemp; // 0.1 C
static BOOL TempOn;
static INT16 tempOld = TEMP_MIN;

void TEMP_Init(void)
{
  TEMP_SetTempOn(FALSE);
  TEMP_SetTemp(370);
  TEMP_ResetTempMin();
  TEMP_ResetTempMax();
  HWC_SetHeater(OFF);
  OS_Message(TASK_TEMP, 0);
  OS_MessageEveryTimer(TASK_TEMP, 0, 10000);
}

void TEMP_Task(UINT32 msg)
{
  INT16 temp, t;

  (void)msg;
  temp = LL_Temp();
  if (TempOn)
  {
    if (temp >= SetTemp) HWC_SetHeater(OFF);  
    else if (temp < SetTemp) HWC_SetHeater(ON);
  }
  if (temp != tempOld)
  {
    if (temp > maxTemp) maxTemp = temp; 
    if (temp < minTemp) minTemp = temp;
    GUI_TempValues(temp, minTemp, maxTemp);
    if (temp / 10 != tempOld / 10 && (temp < SetTemp - 2 || temp > SetTemp + 2))
    {
      t = temp > 0 ? temp : -temp;
      UART_PrintfMsg("Temp is %c%d.%d", temp < 0 ? '-' : ' ', t / 10, t % 10);
    }
    tempOld = temp;
  }
}

void TEMP_SetTempOn(BOOL on)
{
  TempOn = on;
  if (!on) HWC_SetHeater(OFF);
  GUI_SetTempOn(on);
  UART_PrintfMsg("Temp set %s", on ? "ON" : "OFF");
}

void TEMP_ToggleTempOn(void)
{
  TEMP_SetTempOn(!TempOn);
}

void TEMP_ResetTempMax(void)
{
  maxTemp = TEMP_MIN;
  tempOld = TEMP_MIN;
  OS_Message(TASK_TEMP, 0);
  UART_PrintfMsg("Temp reset max");
}

void TEMP_ResetTempMin(void)
{
  minTemp = TEMP_MAX;
  tempOld = TEMP_MIN;
  OS_Message(TASK_TEMP, 0);
  UART_PrintfMsg("Temp reset min");
}

void TEMP_SetTemp(INT16 temp)
{
  SetTemp = temp;
  GUI_SetTempValue(temp);
  UART_PrintfMsg("Temp set to %d.%d", temp / 10, temp % 10);
}
