/* COPYRIGHT (c) 2019 DIVO Systems, Inc.
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

void Cmd_SetOdMeasureInterval(CommandParameter &Parameters)
{
  int secs = Parameters.NextParameterAsInteger();
  CTL_SetOdTimer(secs * 1000);
}

void Cmd_SetTubeParams(CommandParameter &Parameters)
{
  int tube = Parameters.NextParameterAsInteger();
  UINT32 lt = (Parameters.NextParameterAsDouble() + 0.0005) * 1000;
  UINT32 dt = (Parameters.NextParameterAsDouble() + 0.0005) * 1000;
  UINT32 ut = (Parameters.NextParameterAsDouble() + 0.0005) * 1000;
  int tbl = Parameters.NextParameterAsInteger();
  int tal = Parameters.NextParameterAsInteger();
  UINT32 dV = (Parameters.NextParameterAsDouble() + 0.05) * 10;

  CTL_SetThresholdLow    (tube, lt);
  CTL_SetThresholdDrug   (tube, dt);
  CTL_SetThresholdUpper  (tube, ut);
  CTL_SetDilPeriodBelowLT(tube, tbl);
  CTL_SetDilPeriodAboveLT(tube, tal);
  CTL_SetAddingVolume    (tube, dV);
}

void Cmd_SetConcentrationAndVolume(CommandParameter &Parameters)
{
  UINT32 c  =  Parameters.NextParameterAsInteger();
  UINT32 c1 =  Parameters.NextParameterAsInteger();
  UINT32 c2 =  Parameters.NextParameterAsInteger();
  UINT32 M  = (Parameters.NextParameterAsDouble() + 0.05) * 10;
  int    X  =  Parameters.NextParameterAsInteger();
  UINT32 V  =  Parameters.NextParameterAsInteger();

  CTL_SetConcentration(c);
  CTL_SetInConcentrationL(c1);
  CTL_SetInConcentrationH(c2);
  CTL_SetMultiplier(M);
  CTL_SetDilXcount(X);
  CTL_SetVtube(V);
}

void Cmd_SetGrowthCorrections(CommandParameter &Parameters)
{
  float e;
  int ei;

  e = Parameters.NextParameterAsDouble();
  ei = ((e > 0) ? (e + 0.0005) : (e - 0.0005)) * 1000;
  CTL_SetGrowthCorrectionEod(ei);
  e = Parameters.NextParameterAsDouble();
  ei = ((e > 0) ? (e + 0.0005) : (e - 0.0005)) * 1000;
  CTL_SetGrowthCorrectionEur(ei);
  e = Parameters.NextParameterAsDouble();
  ei = ((e > 0) ? (e + 0.0005) : (e - 0.0005)) * 1000;
  CTL_SetGrowthCorrectionEuf(ei);
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
}

void Cmd_SetMode(CommandParameter &Parameters)
{ 
  int mode = Parameters.NextParameterAsInteger();

  CTL_SetMode(mode);
}

void Cmd_SetCalibrationParams(CommandParameter &Parameters)
{
  int tube = Parameters.NextParameterAsInteger();
  double c = Parameters.NextParameterAsDouble();
  double v = Parameters.NextParameterAsDouble();
  UINT32 l = Parameters.NextParameterAsInteger();

  CTL_SetOdC(tube, c);
  CTL_SetOdV(tube, v);
  CTL_SetLaserStrength(tube, l);
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
  Serial.begin(115000);

  UART_PrintfMsg(" ");
  UART_PrintfMsg("=====================================");
  UART_PrintfMsg("           Morbidostat");
  UART_PrintfMsg(" Ver. %02d.%02d %s %s", MOB_VER_MAJOR, MOB_VER_MINOR, __DATE__, __TIME__);
  UART_PrintfMsg("COPYRIGHT (c) 2019 DIVO Systems, Inc.");
  UART_PrintfMsg("=====================================");
  UART_PrintfMsg(" ");

  UART_PrintfOdMsg("#");
  UART_PrintfOdMsg("# =====================================");
  UART_PrintfOdMsg("#            Morbidostat");
  UART_PrintfOdMsg("#  Ver. %02d.%02d %s %s", MOB_VER_MAJOR, MOB_VER_MINOR, __DATE__, __TIME__);
  UART_PrintfOdMsg("# COPYRIGHT (c) 2019 DIVO Systems, Inc.");
  UART_PrintfOdMsg("# =====================================");
  UART_PrintfOdMsg("#");

  UART_PrintfPumpMsg("#");
  UART_PrintfPumpMsg("# =====================================");
  UART_PrintfPumpMsg("#            Morbidostat");
  UART_PrintfPumpMsg("#  Ver. %02d.%02d %s %s", MOB_VER_MAJOR, MOB_VER_MINOR, __DATE__, __TIME__);
  UART_PrintfPumpMsg("# COPYRIGHT (c) 2019 DIVO Systems, Inc.");
  UART_PrintfPumpMsg("# =====================================");
  UART_PrintfPumpMsg("#");

  // Note that each command name is inside F("").
  // This places the command text in program memory to save RAM. 
  SerialCommandHandler.AddCommand(F("SetOdMeasureInterval"), Cmd_SetOdMeasureInterval);
  SerialCommandHandler.AddCommand(F("PrintOsStats"), Cmd_PrintOsStats);
  SerialCommandHandler.AddCommand(F("PrintExperimentParams"), Cmd_PrintExperimentParams);
  SerialCommandHandler.AddCommand(F("SetTubeParams"), Cmd_SetTubeParams);
  SerialCommandHandler.AddCommand(F("SetConcentrationAndVolume"), Cmd_SetConcentrationAndVolume);
  SerialCommandHandler.AddCommand(F("SetGrowthCorrections"), Cmd_SetGrowthCorrections);
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
  SerialCommandHandler.AddCommand(F("SetUpperInterval"), Cmd_SetUpperInterval);
  SerialCommandHandler.AddCommand(F("SetMode"), Cmd_SetMode);
  SerialCommandHandler.AddCommand(F("SetCalibrationParams"), Cmd_SetCalibrationParams);
#if MORB_SIMULATE
  SerialCommandHandler.AddCommand(F("sod"), Cmd_SetSimulateOd);
#endif
  SerialCommandHandler.SetDefaultHandler(Cmd_Unknown);
}

void CMD_Task()
{
  SerialCommandHandler.Process();
}