/* COPYRIGHT (c) 2019 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include <stdio.h>
#include <stdarg.h>

#include "MegunoLink.h"
#include "common.h"

Message ControlMsgs("ControlMessages");
Message OdDataMsgs("OdData");
Message PumpMsgs("Pumps");

void UART_Printf(const char *format, ...)
{
  char buffer[200];
  va_list args;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  Serial.print(buffer);
}

void UART_PrintfMsg(const char *format, ...)
{
  char buffer[200];
  va_list args;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  ControlMsgs.Begin();
  Serial.print(buffer);
  ControlMsgs.End();
}

void UART_PrintfOdMsg(const char *format, ...)
{
  char buffer[200];
  va_list args;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  OdDataMsgs.Begin();
  Serial.print(buffer);
  OdDataMsgs.End();
}

void UART_PrintfPumpMsg(const char *format, ...)
{
  char buffer[200];
  va_list args;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  PumpMsgs.Begin();
  Serial.print(buffer);
  PumpMsgs.End();
}
