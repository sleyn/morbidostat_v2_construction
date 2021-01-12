/* COPYRIGHT (c) 2019 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#define EXTERN
#include "common.h"

void MAIN_Init(void)
{
  OS_MessageEveryTimer(TASK_LED, 0, 500);
}

void LED_Task(UINT32 msg)
{
#define LED_PATTERN 0b0001100110010101
  static int state;

  (void)msg;
  LL_LED((LED_PATTERN >> state) & 1);
  if (++state == 16) state = 0;

  GUI_Heartbeat();
}
