/* COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include "common.h"

void setup()
{
  LL_Init();
  CMD_Init();
  NVS_Init();
  POD_Init();
  HWC_Init();
  GUI_Init();
  TEMP_Init();
  CTL_Init();
  GUI_FwVersion();
}

void loop()
{
  CMD_Task();
  OS_TasksScheduler();
}
