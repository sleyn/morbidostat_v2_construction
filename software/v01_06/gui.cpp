/* COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include <string.h>

#include "MegunoLink.h"

#include "common.h"

#define GUIMSG_HEARTBEAT      0
#define GUIMSG_ENABLE_MAN_DIL 1

InterfacePanel mobMainControlPanel("Main Control");
InterfacePanel mobDebugPanel("Debug Control");
InterfacePanel mobExperimentSettingsPanel("Experiment Settings");
InterfacePanel mobCalibrationPanel("Calibration");

static void gui_SetButtonState(InterfacePanel panel, char *name, BOOL on)
{
  panel.SetBackColor(name, on ? "LIME" : "WHITE");
}

void GUI_Mode(int mode)
{
  char control[12] = "Button_Run ";
  char init_params[20] = "Checkbox_InitParams";
  int i;

  if (mode == eMode_Off)
  {
    for (i = eMode_OdReadings; i <= eMode_CleanPump2; i++)
    {
      control[10] = '0' + i;
      mobMainControlPanel.SetBackColor(control, "LIME");
      mobMainControlPanel.SetText(control, "Start");
      mobMainControlPanel.EnableControl(control);
    }
    mobMainControlPanel.EnableControl(init_params);
  }
  else
  {
    for (i = eMode_OdReadings; i <= eMode_CleanPump2; i++)
    {
      control[10] = '0' + i;
      if (i == mode)
      {
        mobMainControlPanel.SetBackColor(control, "RED");
        mobMainControlPanel.SetText(control, "Stop");
        mobMainControlPanel.EnableControl(control);
      }
      else
      {
        mobMainControlPanel.SetBackColor(control, "LightGray");
        mobMainControlPanel.SetText(control, "Start");
        mobMainControlPanel.DisableControl(control);
      }
    }
    mobMainControlPanel.DisableControl(init_params);
  }
  mobMainControlPanel.SetCheck(init_params, FALSE);
}

void GUI_Pause(BOOL enabled, BOOL paused)
{
  char control[6] = "Pause";

  if (enabled) mobMainControlPanel.EnableControl(control);
  else mobMainControlPanel.DisableControl(control);
  mobMainControlPanel.SetBackColor(control, !enabled ? "LightGray" :
                                            (paused ? "RED" : "LIME"));
  mobMainControlPanel.SetText(control, paused ? "Resume" : "Pause");
}

void GUI_SetTempOn(BOOL on)
{
  gui_SetButtonState(mobMainControlPanel, (char *)"SetTempOn", on);
}

void GUI_SetTempValue(INT16 temp)
{
  mobMainControlPanel.SetNumber("SetTemp", (double)temp / 10);
}

void GUI_TempValues(INT16 temp, INT16 minTemp, INT16 maxTemp)
{
  char buf[10];
  INT16 t;
  
  t = temp > 0 ? temp : -temp;
  sprintf(buf, "%c%d.%d", temp < 0 ? '-' : ' ', t / 10, t % 10);
  mobMainControlPanel.SetText("TempCurrent", buf);
  t = minTemp > 0 ? minTemp : -minTemp;
  sprintf(buf, "%c%d.%d", minTemp < 0 ? '-' : ' ', t / 10, t % 10);
  mobMainControlPanel.SetText("TempMin", buf);
  t = maxTemp > 0 ? maxTemp : -maxTemp;
  sprintf(buf, "%c%d.%d", maxTemp < 0 ? '-' : ' ', t / 10, t % 10);
  mobMainControlPanel.SetText("TempMax", buf);
}

void GUI_TubeOn(int tube, BOOL on)
{
  char name[11] = "SetTubeOnX";
  
  name[9] = '1' + tube;
  gui_SetButtonState(mobMainControlPanel, name, on);
}

void GUI_Heartbeat(void)
{
  static BOOL hb = TRUE;

  gui_SetButtonState(mobMainControlPanel, (char *)"Heartbeat", hb);
  hb = !hb;
}

void GUI_AirOn(BOOL on)
{ 
  gui_SetButtonState(mobMainControlPanel, (char *)"Air", on);
}

void GUI_AirDelay(UINT32 msecs)
{
  mobMainControlPanel.SetNumber("AirDelay", (double)msecs / 1000);
}

void GUI_MeasureInterval(int secs)
{
  mobMainControlPanel.SetNumber("OdMeasInterval", secs);
}

void GUI_NumCleanCycles(int cycles)
{
  mobMainControlPanel.SetNumber("NumCleanCycles", cycles);
}

void GUI_RemainingCycles(int cycles)
{
  mobMainControlPanel.SetText("RemainingCycles", cycles);
}

void GUI_FwVersion(void)
{
  char str[10];

  sprintf(str, "%02d.%02d", MOB_VER_MAJOR, MOB_VER_MINOR);
  mobMainControlPanel.SetText("FwVersion", str);
}

void GUI_DilPeriodBelowLT(int tube, int tbl)
{
  char control[5] = "TBL ";
  control[3] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, tbl);
}

void GUI_DilPeriodAboveLT(int tube, int tal)
{
  char control[5] = "TAL ";
  control[3] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, tal);
}

void GUI_ThresholdLow(int tube, UINT32 lt)
{
  char control[4] = "LT ";
  control[2] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, (double)lt / 1000);
}

void GUI_ThresholdDrug(int tube, UINT32 dt)
{
  char control[4] = "DT ";
  control[2] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, (double)dt / 1000);
}

void GUI_ThresholdUpper(int tube, UINT32 ut)
{
  char control[4] = "UT ";
  control[2] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, (double)ut / 1000);
}

void GUI_AddingVolume(int tube, UINT32 dV)
{
  char control[4] = "DV ";
  control[2] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, (double)dV / 10);
};

void GUI_Multiplier(int tube, UINT32 M)
{
  char control[3] = "M ";
  control[1] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, (double)M / 10);
}

void GUI_DilXcount(int tube, int X)
{
  char control[3] = "X ";
  control[1] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, X);
}

void GUI_CurrentConcentration(int tube, UINT32 c)
{
  char control[3] = "C ";
  control[1] = '1' + tube;
  mobMainControlPanel.SetText(control, c);
}

void GUI_TubeConcentration(int tube, UINT32 c)
{
  char control[4] = "TC ";
  control[2] = '1' + tube;
  mobMainControlPanel.SetText(control, c);
}

void GUI_ManualConcentration(int tube, UINT32 c)
{
  char control[9] = "ManualC ";
  control[7] = '1' + tube;
  mobMainControlPanel.SetNumber(control, c);
}

void GUI_ManualDilutionParams(int tube, UINT32 v, UINT32 c)
{
  char control[11] = "Dilution  ";
  control[9] = '1' + tube;
  control[8] = 'V';
  mobMainControlPanel.SetNumber(control, (double)v / 10);
  control[8] = 'C';
  mobMainControlPanel.SetNumber(control, c);
}

void GUI_InitConcentration(int tube, UINT32 c)
{
  char control[7] = "InitC ";
  control[5] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, c);
}

void GUI_Zone4Concentration(int tube, UINT32 c)
{
  char control[8] = "Zone4C ";
  control[6] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(control, c);
}

void GUI_Zone4FixedConcentration(int tube, BOOL fixed)
{
  char control[13] = "Zone4FixedC ";
  control[11] = '1' + tube;
  mobExperimentSettingsPanel.SetCheck(control, fixed);
}

void GUI_InConcentrationL(UINT32 c1) {mobExperimentSettingsPanel.SetNumber("CLOW",  c1);}
void GUI_InConcentrationH(UINT32 c2) {mobExperimentSettingsPanel.SetNumber("CHIGH", c2);}
void GUI_Vtube           (UINT32 V)  {mobExperimentSettingsPanel.SetNumber("V", (double)V / 10);}

void GUI_SetValve(int tube, int high, BOOL on)
{
  char name[8] = "Valve  ";
  
  name[5] = '1' + tube;
  name[6] = '1' + high;
  gui_SetButtonState(mobDebugPanel, name, on);
  gui_SetButtonState(mobMainControlPanel, name, on);
}

void GUI_SetPump(int high, BOOL on)
{
  char name[6] = "Pump ";
  
  name[4] = '1' + high;
  gui_SetButtonState(mobDebugPanel, name, on);
  gui_SetButtonState(mobMainControlPanel, name, on);
}

void GUI_SetAir(BOOL on)
{
  char name[8] = "AirPump";
  gui_SetButtonState(mobDebugPanel, name, on);
  gui_SetButtonState(mobMainControlPanel, name, on);
}

void GUI_SetHeater(BOOL on)
{
  char name[7] = "Heater";
  gui_SetButtonState(mobDebugPanel, name, on);
  gui_SetButtonState(mobMainControlPanel, name, on);
}

void GUI_GrowthCorrectionEod(int tube, int e)
{
  char name[5] = "EodX";
  name[3] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(name, (double)e / 1000);
}

void GUI_GrowthCorrectionEur(int tube, int e)
{
  char name[5] = "EurX";
  name[3] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(name, (double)e / 1000);
}

void GUI_GrowthCorrectionEuf(int tube, int e)
{
  char name[5] = "EufX";
  name[3] = '1' + tube;
  mobExperimentSettingsPanel.SetNumber(name, (double)e / 1000);
}

void GUI_DilPeriodUpper(int secs)
{
  mobExperimentSettingsPanel.SetNumber("UpperInterval", secs);
}

void GUI_CalibrationC(int tube, float c)
{
  char name[3] = "CX";
  
  name[1] = '1' + tube;
  mobCalibrationPanel.SetNumber(name, c);
}

void GUI_CalibrationV(int tube, float v)
{
  char name[3] = "VX";
  
  name[1] = '1' + tube;
  mobCalibrationPanel.SetNumber(name, v);
}

void GUI_CalibrationL(int tube, UINT32 l)
{
  char name[3] = "LX";
  
  name[1] = '1' + tube;
  mobCalibrationPanel.SetNumber(name, l);
}

void GUI_EnableManualDilution(int tube, BOOL enable)
{
  char name[15] = "Button_ManDilX";

  name[13] = '1' + tube;
  if (enable)
  {
    mobMainControlPanel.EnableControl(name);
  }
  else
  {
    mobMainControlPanel.DisableControl(name);
    OS_MessageAfterTimer(TASK_GUI, GUIMSG_ENABLE_MAN_DIL | ((UINT32)tube << 16),
	                     MAN_DIL_DISABLE_DELAY * 1000);
  }
}

void GUI_Init(void)
{
  OS_MessageEveryTimer(TASK_GUI, GUIMSG_HEARTBEAT, 500);
}

void GUI_Task(UINT32 msg)
{
#define LED_PATTERN 0b0001100110010101
  static int state;
  int tube;

  switch (msg & 0xFFFF)
  {
    case GUIMSG_HEARTBEAT:
      LL_LED((LED_PATTERN >> state) & 1);
      if (++state == 16) state = 0;
      GUI_Heartbeat();
      break;

    case GUIMSG_ENABLE_MAN_DIL:
      tube = msg >> 16;
      GUI_EnableManualDilution(tube, TRUE);
      break;
  }
}
