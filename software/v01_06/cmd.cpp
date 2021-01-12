/* COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include <string.h>

#define PGM_P char *  // Needed to remove warnings
#include "CommandHandler.h"

#include "common.h"

CommandHandler<32,80> SerialCommandHandler;

void Cmd_PrintOsStats(CommandParameter &Parameters)
{
  (void) Parameters;
  OS_PrintStats();
}

void Cmd_PrintExperimentParams(CommandParameter &Parameters)
{
  (void) Parameters;
  CTL_PrintAllParams();
}

void Cmd_PrintRunStatus(CommandParameter &Parameters)
{
  (void) Parameters;
  CTL_PrintRunStatus();
}

void Cmd_SetOdMeasureInterval(CommandParameter &Parameters)
{
  UINT32 secs = Parameters.NextParameterAsInteger() * 1000L;
  CTL_SetOdTimer(secs);
  NVS_WriteTag(NVTAG_OdTimer, &secs, sizeof(secs));
}

void Cmd_SetTubeParamsBasic(CommandParameter &Parameters)
{
  int tube = Parameters.NextParameterAsInteger();
  UINT32 lt = (Parameters.NextParameterAsDouble() + 0.0005) * 1000;
  UINT32 dt = (Parameters.NextParameterAsDouble() + 0.0005) * 1000;
  UINT32 ut = (Parameters.NextParameterAsDouble() + 0.0005) * 1000;
  int tbl = Parameters.NextParameterAsInteger();
  int tal = Parameters.NextParameterAsInteger();
  UINT32 dV = (Parameters.NextParameterAsDouble() + 0.05) * 10;
  UINT32 initC = Parameters.NextParameterAsInteger();

  CTL_SetThresholdLow(tube, lt);
  NVS_WriteTag(NVTAG_ThresholdLow + tube, &lt, sizeof(lt));
  CTL_SetThresholdDrug(tube, dt);
  NVS_WriteTag(NVTAG_ThresholdDrug + tube, &dt, sizeof(dt));
  CTL_SetThresholdUpper(tube, ut);
  NVS_WriteTag(NVTAG_ThresholdUpper + tube, &ut, sizeof(ut));
  CTL_SetDilPeriodBelowLT(tube, tbl);
  NVS_WriteTag(NVTAG_DilPeriodBelowLT + tube, &tbl, sizeof(tbl));
  CTL_SetDilPeriodAboveLT(tube, tal);
  NVS_WriteTag(NVTAG_DilPeriodAboveLT + tube, &tal, sizeof(tal));
  CTL_SetAddingVolume(tube, dV);
  NVS_WriteTag(NVTAG_AddingVolume + tube, &dV, sizeof(dV));
  CTL_SetInitConcentration(tube, initC);
  NVS_WriteTag(NVTAG_InitConcentration + tube, &initC, sizeof(initC));
}

void Cmd_SetTubeParamsAdvanced(CommandParameter &Parameters)
{
  int tube = Parameters.NextParameterAsInteger();
  UINT32 M = (Parameters.NextParameterAsDouble() + 0.05) * 10;
  int    X = Parameters.NextParameterAsInteger();
  UINT32 zone4C = Parameters.NextParameterAsInteger();
  BOOL   zone4FixedC = Parameters.NextParameterAsInteger();
  float e;
  int ei;

  CTL_SetMultiplier(tube, M);
  NVS_WriteTag(NVTAG_Multiplier + tube, &M, sizeof(M));
  CTL_SetDilXcount(tube, X);
  NVS_WriteTag(NVTAG_DilXcount + tube, &X, sizeof(X));
  CTL_SetZone4Concentration(tube, zone4C);
  NVS_WriteTag(NVTAG_Zone4Concentration + tube, &zone4C, sizeof(zone4C));
  CTL_SetZone4FixedConcentration(tube, zone4FixedC);
  NVS_WriteTag(NVTAG_Zone4FixedConcentration + tube, &zone4FixedC, sizeof(zone4FixedC));

  e = Parameters.NextParameterAsDouble();
  ei = ((e > 0) ? (e + 0.0005) : (e - 0.0005)) * 1000;
  CTL_SetGrowthCorrectionEod(tube, ei);
  NVS_WriteTag(NVTAG_GrowthCorrectionEod + tube, &ei, sizeof(ei));
  e = Parameters.NextParameterAsDouble();
  ei = ((e > 0) ? (e + 0.0005) : (e - 0.0005)) * 1000;
  CTL_SetGrowthCorrectionEur(tube, ei);
  NVS_WriteTag(NVTAG_GrowthCorrectionEur + tube, &ei, sizeof(ei));
  e = Parameters.NextParameterAsDouble();
  ei = ((e > 0) ? (e + 0.0005) : (e - 0.0005)) * 1000;
  CTL_SetGrowthCorrectionEuf(tube, ei);
  NVS_WriteTag(NVTAG_GrowthCorrectionEuf + tube, &ei, sizeof(ei));
}

void Cmd_SetTubeCurrentConcentration(CommandParameter &Parameters)
{
  int tube = Parameters.NextParameterAsInteger();
  UINT32 c = Parameters.NextParameterAsInteger();

  CTL_SetCurrentConcentration(tube, c);
}

void Cmd_SetTubeDilutionParams(CommandParameter &Parameters)
{
  int tube = Parameters.NextParameterAsInteger();
  UINT32 v  =  (Parameters.NextParameterAsDouble() + 0.05) * 10;
  UINT32 c = Parameters.NextParameterAsInteger();
  static UINT32 nextManDilAllowedTime[NUM_TUBES];

  if (gTimerTicks >= nextManDilAllowedTime[tube])
  {
    CTL_ManualDilution(tube, v, c);
    nextManDilAllowedTime[tube] = gTimerTicks + MAN_DIL_DISABLE_DELAY * SEC_TICKS;
    GUI_EnableManualDilution(tube, FALSE);
  }
}

void Cmd_SetConcentrationAndVolume(CommandParameter &Parameters)
{
  UINT32 c1 =  Parameters.NextParameterAsInteger();
  UINT32 c2 =  Parameters.NextParameterAsInteger();
  UINT32 V  =  (Parameters.NextParameterAsDouble() + 0.05) * 10;

  CTL_SetInConcentrationL(c1);
  NVS_WriteTag(NVTAG_InConcentrationL, &c1, sizeof(c1));
  CTL_SetInConcentrationH(c2);
  NVS_WriteTag(NVTAG_InConcentrationH, &c2, sizeof(c2));
  CTL_SetVtube(V);
  NVS_WriteTag(NVTAG_Vtube, &V, sizeof(V));
}

void Cmd_EnableAir(CommandParameter &Parameters)
{
  (void)Parameters; 
  CTL_ToggleAirOn();
}

void Cmd_SetValve(CommandParameter &Parameters)           
{ 
  int tube = Parameters.NextParameterAsInteger();
  int high = Parameters.NextParameterAsInteger();

  HWC_ToggleValve(tube, high);
}

void Cmd_SetPump(CommandParameter &Parameters)
{ 
  int high = Parameters.NextParameterAsInteger();
  
  HWC_TogglePump(high);
}

void Cmd_SetAir(CommandParameter &Parameters)
{ 
  (void)Parameters;
  HWC_ToggleAir();
}

void Cmd_SetAirDelay(CommandParameter &Parameters)
{ 
  UINT32 msecs = (UINT32)((Parameters.NextParameterAsDouble() + 0.05) * 10) * 100;

  CTL_SetAirDelay(msecs);
  NVS_WriteTag(NVTAG_AirDelay, &msecs, sizeof(msecs));
}

void Cmd_SetHeater(CommandParameter &Parameters)
{ 
  (void)Parameters;
  HWC_ToggleHeater();
}

void Cmd_SetTempOn(CommandParameter &Parameters)
{ 
  (void)Parameters;
  TEMP_ToggleTempOn();
}

void Cmd_SetTempValue(CommandParameter &Parameters)
{ 
  INT16 temp = (Parameters.NextParameterAsDouble() + 0.05) * 10;

  TEMP_SetTemp(temp);
  NVS_WriteTag(NVTAG_Temp, &temp, sizeof(temp));
}

void Cmd_TempMinReset(CommandParameter &Parameters)
{ 
  (void)Parameters; 
  TEMP_ResetTempMin();
}

void Cmd_TempMaxReset(CommandParameter &Parameters)
{ 
  (void)Parameters; 
  TEMP_ResetTempMax();
}

void Cmd_SetTubeOn(CommandParameter &Parameters)
{ 
  int tube = Parameters.NextParameterAsInteger();
  
  CTL_ToggleTubeOn(tube);
}

void Cmd_SetUpperInterval(CommandParameter &Parameters)
{ 
  int secs = Parameters.NextParameterAsInteger();

  CTL_SetDilPeriodUpper(secs);
  NVS_WriteTag(NVTAG_DilPeriodUpper, &secs, sizeof(secs));
}

void Cmd_SetMode(CommandParameter &Parameters)
{ 
  int mode = Parameters.NextParameterAsInteger();
  int num = Parameters.NextParameterAsInteger();

  if (mode == eMode_MorbidostatRun)
  {
    if (num) mode = eMode_MorbidostatRestart;
  }
  else if (mode == eMode_CleanPump1 || mode == eMode_CleanPump2)
  {
    CTL_SetCleanCycles(num);
    NVS_WriteTag(NVTAG_CleanCycles, &num, sizeof(num));
  }
  CTL_SetMode(mode);
  NVS_WriteTag(NVTAG_Mode, &mode, sizeof(mode));
}

void Cmd_MorbidostatPause(CommandParameter &Parameters)
{ 
  (void)Parameters; 
  CTL_ToggleMorbidostatPause();
}

void Cmd_SetCalibrationParams(CommandParameter &Parameters)
{
  int tube = Parameters.NextParameterAsInteger();
  float c = Parameters.NextParameterAsDouble();
  float v = Parameters.NextParameterAsDouble();
  UINT32 l = Parameters.NextParameterAsInteger();

  CTL_SetOdC(tube, c);
  NVS_WriteTag(NVTAG_OdC + tube, &c, sizeof(c));
  CTL_SetOdV(tube, v);
  NVS_WriteTag(NVTAG_OdV + tube, &v, sizeof(v));
  CTL_SetLaserStrength(tube, l);
  NVS_WriteTag(NVTAG_LaserStrength + tube, &l, sizeof(l));
}

void Cmd_FactoryReset(CommandParameter &Parameters)
{ 
  (void)Parameters;
  UART_Printf("Factory Reset (cleaning all non-volatile storage)\r\n");
  NVS_Clear();
  Reset();
}

#if MORB_SIMULATE
void Cmd_SetSimulateOd(CommandParameter &Parameters)
{
  int tube = Parameters.NextParameterAsInteger();
  double od = Parameters.NextParameterAsDouble();

  if (tube > 0 && tube <= NUM_TUBES)
    CTL_SetSimulateOd(tube - 1, od);
}
#endif

void Cmd_Unknown()
{
  UART_Printf("UNKNOWN COMMAND\r\n");
}

void CMD_Init()
{
  Serial.begin(115200);

  UART_PrintfMsg(" ");
  UART_PrintfMsg("============================================");
  UART_PrintfMsg("              Morbidostat");
  UART_PrintfMsg("    Ver. %02d.%02d %s %s", MOB_VER_MAJOR, MOB_VER_MINOR, __DATE__, __TIME__);
  UART_PrintfMsg("COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.");
  UART_PrintfMsg("============================================");
  UART_PrintfMsg(" ");

  UART_PrintfOdMsg("#");
  UART_PrintfOdMsg("# ============================================");
  UART_PrintfOdMsg("#               Morbidostat");
  UART_PrintfOdMsg("#     Ver. %02d.%02d %s %s", MOB_VER_MAJOR, MOB_VER_MINOR, __DATE__, __TIME__);
  UART_PrintfOdMsg("# COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.");
  UART_PrintfOdMsg("# ============================================");
  UART_PrintfOdMsg("#");

  UART_PrintfPumpMsg("#");
  UART_PrintfPumpMsg("# ============================================");
  UART_PrintfPumpMsg("#               Morbidostat");
  UART_PrintfPumpMsg("#     Ver. %02d.%02d %s %s", MOB_VER_MAJOR, MOB_VER_MINOR, __DATE__, __TIME__);
  UART_PrintfPumpMsg("# COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.");
  UART_PrintfPumpMsg("# ============================================");
  UART_PrintfPumpMsg("#");
  UART_PrintfPumpMsg("#Tube\tTimeL\tTimeH\tState\tMode\tOD\tTubeC\tC1\tC2\tLT\tDT\tUT");

  // Note that each command name is inside F("").
  // This places the command text in program memory to save RAM. 
  SerialCommandHandler.AddCommand(F("SetOdMeasureInterval"), Cmd_SetOdMeasureInterval);
  SerialCommandHandler.AddCommand(F("PrintOsStats"), Cmd_PrintOsStats);
  SerialCommandHandler.AddCommand(F("PrintExperimentParams"), Cmd_PrintExperimentParams);
  SerialCommandHandler.AddCommand(F("PrintRunStatus"), Cmd_PrintRunStatus);
  SerialCommandHandler.AddCommand(F("SetTubeParamsBasic"), Cmd_SetTubeParamsBasic);
  SerialCommandHandler.AddCommand(F("SetTubeParamsAdvanced"), Cmd_SetTubeParamsAdvanced);
  SerialCommandHandler.AddCommand(F("SetTubeCurrentConcentration"), Cmd_SetTubeCurrentConcentration);
  SerialCommandHandler.AddCommand(F("SetTubeDilutionParams"), Cmd_SetTubeDilutionParams);
  SerialCommandHandler.AddCommand(F("SetConcentrationAndVolume"), Cmd_SetConcentrationAndVolume);
  SerialCommandHandler.AddCommand(F("SetAir"), Cmd_EnableAir);
  SerialCommandHandler.AddCommand(F("SetAirDelay"), Cmd_SetAirDelay);
  SerialCommandHandler.AddCommand(F("SetValve"), Cmd_SetValve);
  SerialCommandHandler.AddCommand(F("SetPump"), Cmd_SetPump);
  SerialCommandHandler.AddCommand(F("SetAirPump"), Cmd_SetAir);
  SerialCommandHandler.AddCommand(F("SetHeater"), Cmd_SetHeater);
  SerialCommandHandler.AddCommand(F("SetTempOn"), Cmd_SetTempOn);
  SerialCommandHandler.AddCommand(F("SetTempValue"), Cmd_SetTempValue);
  SerialCommandHandler.AddCommand(F("TempMinReset"), Cmd_TempMinReset);
  SerialCommandHandler.AddCommand(F("TempMaxReset"), Cmd_TempMaxReset);
  SerialCommandHandler.AddCommand(F("SetTubeOn"), Cmd_SetTubeOn);
  SerialCommandHandler.AddCommand(F("Pause"), Cmd_MorbidostatPause);
  SerialCommandHandler.AddCommand(F("SetUpperInterval"), Cmd_SetUpperInterval);
  SerialCommandHandler.AddCommand(F("SetMode"), Cmd_SetMode);
  SerialCommandHandler.AddCommand(F("SetCalibrationParams"), Cmd_SetCalibrationParams);
  SerialCommandHandler.AddCommand(F("FactoryReset"), Cmd_FactoryReset);
#if MORB_SIMULATE
  SerialCommandHandler.AddCommand(F("sod"), Cmd_SetSimulateOd);
#endif
  SerialCommandHandler.SetDefaultHandler(Cmd_Unknown);
}

void CMD_Task()
{
  SerialCommandHandler.Process();
}