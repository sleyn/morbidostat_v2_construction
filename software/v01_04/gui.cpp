/* COPYRIGHT (c) 2019 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include <string.h>

#include "MegunoLink.h"

#include "common.h"

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
  static const char modeName[5][12] =
              {"Off", "ODReadings", "Morbidostat", "CleanPump1", "CleanPump2"};
  int i;

  for (i = 0; i < eMode_Max; i++)
  {
    gui_SetButtonState(mobMainControlPanel, (char *)modeName[i], i == mode);
  }
}

void GUI_SetTempOn(BOOL on)
{
  gui_SetButtonState(mobMainControlPanel, (char *)"SetTempOn", on);
}

void GUI_SetTempValue(INT16 temp)
{
  mobMainControlPanel.SetNumber("SetTemp", temp / 10.);
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

void GUI_FwVersion(void)
{
  char str[80];

  sprintf(str, "%02d.%02d", MOB_VER_MAJOR, MOB_VER_MINOR);
  mobMainControlPanel.SetText("FwVersion", str);
}

void GUI_DilPeriodBelowLT(int tube, int tbl)
{
  static char tbls[NUM_TUBES][5] = {"TBL1", "TBL2", "TBL3", "TBL4", "TBL5", "TBL6"};
  mobExperimentSettingsPanel.SetNumber(tbls[tube], tbl);
}

void GUI_DilPeriodAboveLT(int tube, int tal)
{
  static char tals[NUM_TUBES][5] = {"TAL1", "TAL2", "TAL3", "TAL4", "TAL5", "TAL6"};
  mobExperimentSettingsPanel.SetNumber(tals[tube], tal);
}

void GUI_ThresholdLow(int tube, UINT32 lt)
{
  static char lts[NUM_TUBES][4] = {"LT1", "LT2", "LT3", "LT4", "LT5", "LT6"};
  mobExperimentSettingsPanel.SetNumber(lts[tube], (double)lt / 1000);
}

void GUI_ThresholdDrug(int tube, UINT32 dt)
{
  static char dts[NUM_TUBES][4] = {"DT1", "DT2", "DT3", "DT4", "DT5", "DT6"};
  mobExperimentSettingsPanel.SetNumber(dts[tube], (double)dt / 1000);
}

void GUI_ThresholdUpper(int tube, UINT32 ut)
{
  static char uts[NUM_TUBES][4] = {"UT1", "UT2", "UT3", "UT4", "UT5", "UT6"};
  mobExperimentSettingsPanel.SetNumber(uts[tube], (double)ut / 1000);
}

void GUI_AddingVolume(int tube, UINT32 dV)
{
  static char dvs[NUM_TUBES][4] = {"DV1", "DV2", "DV3", "DV4", "DV5", "DV6"};
  mobExperimentSettingsPanel.SetNumber(dvs[tube], (double)dV / 10);
};

void GUI_Concentration    (UINT32 c)  { mobExperimentSettingsPanel.SetNumber("C",  c);   }
void GUI_InConcentrationL (UINT32 c1) { mobExperimentSettingsPanel.SetNumber("C1", c1); }
void GUI_InConcentrationH (UINT32 c2) { mobExperimentSettingsPanel.SetNumber("C2", c2); }
void GUI_Multiplier       (UINT32 M)  { mobExperimentSettingsPanel.SetNumber("M",  (double)M / 10);}
void GUI_DilXcount        (int X)     { mobExperimentSettingsPanel.SetNumber("X",  X);   }
void GUI_Vtube            (UINT32 V)  { mobExperimentSettingsPanel.SetNumber("V",  V);   }

void GUI_SetValve(int tube, int high, BOOL on)
{
  char name[8] = "ValveXX";
  
  name[5] = '1' + tube;
  name[6] = '1' + high;
  gui_SetButtonState(mobDebugPanel, name, on);
}

void GUI_SetPump(int high, BOOL on)
{
  char name[6] = "PumpX";
  
  name[4] = '1' + high;
  gui_SetButtonState(mobDebugPanel, name, on);
}

void GUI_SetAir(BOOL on)
{
  gui_SetButtonState(mobDebugPanel, (char *)"AirPump", on);
}

void GUI_SetHeater(BOOL on)
{
  gui_SetButtonState(mobDebugPanel, (char *)"Heater", on);
}

void GUI_GrowthCorrectionEod(int e)
{
  mobExperimentSettingsPanel.SetNumber("Eod", (double)e / 1000);
}

void GUI_GrowthCorrectionEur(int e)
{
  mobExperimentSettingsPanel.SetNumber("Eur", (double)e / 1000);
}

void GUI_GrowthCorrectionEuf(int e)
{
  mobExperimentSettingsPanel.SetNumber("Euf", (double)e / 1000);
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
