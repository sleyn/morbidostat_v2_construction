/* COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include "common.h"
#include <math.h>
#include <string.h>

#define CTL_THRSHLD_HYST 2 // +/- %
#define PUMP_SPEED 3333L // sec/L
#define INVALID_TUBE (-1)

#define DILMSG_TIMER 0
#define DILMSG_NEW   1

#define ODMSG_TIMER 0
#define ODMSG_MEAS  1

typedef enum {
  TUBST_Lowest,
  TUBST_Low,
  TUBST_Drug,
  TUBST_Upper,
} TubeState_t;

static UINT32 AddingVolume[NUM_TUBES]; // 0.1 mL
static UINT32 DilPeriodBelowLT[NUM_TUBES]; // Ticks
static UINT32 DilPeriodAboveLT[NUM_TUBES]; // Ticks
static UINT32 DilPeriodUpper; // Ticks
static UINT32 Concentration[NUM_TUBES];
static float OdC[NUM_TUBES];
static float OdV[NUM_TUBES];
static UINT32 LaserStrength[NUM_TUBES];
static BOOL TubeOn[NUM_TUBES];
static UINT32 ThrhldLow[NUM_TUBES], ThrhldDrug[NUM_TUBES], ThrhldUpper[NUM_TUBES];
static UINT32 ThrhldLowL[NUM_TUBES]; // 0.001
static UINT32 ThrhldLowH[NUM_TUBES];
static UINT32 ThrhldDrugL[NUM_TUBES];
static UINT32 ThrhldDrugH[NUM_TUBES];
static UINT32 InitConcentration[NUM_TUBES];
static UINT32 Zone4Concentration[NUM_TUBES];
static BOOL   Zone4FixedConcentration[NUM_TUBES];
static UINT32 InConcentrationL;
static UINT32 InConcentrationH;
static UINT32 Multiplier[NUM_TUBES]; // in 0.1 units
static UINT32 Vtube; // 0.1 mL
static int DilXcount[NUM_TUBES];
static int GrowthCorrectionEod[NUM_TUBES]; // in 0.001
static int GrowthCorrectionEur[NUM_TUBES]; // in 0.001
static int GrowthCorrectionEuf[NUM_TUBES]; // in 0.001
static BOOL DilOn;
static BOOL OdOn;
static UINT32 AirDelay; // mSec
static BOOL AirOn;
static int CleaningCycles;
static int morbMode = eMode_Off;

static UINT32 LastDilusionTicks[NUM_TUBES];
static TubeState_t TubeState[NUM_TUBES];
static int lastMode[NUM_TUBES];
static INT32 LastRate[NUM_TUBES];
static BOOL DilDrug[NUM_TUBES];
static UINT32 LastOdAvg[NUM_TUBES];
static UINT32 OdAvg[NUM_TUBES];
static int OdCount[NUM_TUBES];
static UINT32 TubeConcentration[NUM_TUBES]; // 0.1
static int DilTube = INVALID_TUBE;
static int CleaningCyclesLeft;
static UINT32 DilTaskTimer;
static BOOL cleaning, cleaningPump;
static UINT32 odMeasTimer;
static char zoneName[][7] = {"Lowest", "Low", "Drug", "Upper"};

#if MORB_SIMULATE
static double sim_od[NUM_TUBES];
#endif

struct {
  UINT32 timeL;
  UINT32 timeH;
  UINT32 concentration;
} DilParams[NUM_TUBES];

void ctl_AdjustConcentration(int tube, UINT32 vol, UINT32 con);
void ctl_OD(int tube, UINT32 od);
void ctl_Dilution(int tube, UINT32 volume, UINT32 concentration);
void ctl_StopDilutions(void);
void ctl_SetCleaningDil(void);
void ctl_ResetMorbidostatParams(void);

void CTL_Init(void)
{
  int i, a;
  float f;
  UINT32 l;
  BOOL b;
  static const float def_OdC[NUM_TUBES] = {-0.57, -0.50, -0.55, -0.51, -0.53, -0.47};
  static const float def_OdV[NUM_TUBES] = { 3.11,  2.75,  3.01,  2.75,  2.91,  2.62};
  
  for (i = 0; i < NUM_TUBES; i++)
  {
    if (NVS_ReadTag(NVTAG_OdC + i, &f, sizeof(f)) != sizeof(f)) f = def_OdC[i];
    CTL_SetOdC(i, f);  
    if (NVS_ReadTag(NVTAG_OdV + i, &f, sizeof(f)) != sizeof(f)) f = def_OdV[i];
    CTL_SetOdV(i, f);  
    if (NVS_ReadTag(NVTAG_LaserStrength + i, &l, sizeof(l)) != sizeof(l)) l = 4;
    CTL_SetLaserStrength(i, l);  
    if (NVS_ReadTag(NVTAG_ThresholdLow + i, &l, sizeof(l)) != sizeof(l)) l = 200;
    CTL_SetThresholdLow(i, l);
    if (NVS_ReadTag(NVTAG_ThresholdDrug + i, &l, sizeof(l)) != sizeof(l)) l = 400;
    CTL_SetThresholdDrug(i, l);
    if (NVS_ReadTag(NVTAG_ThresholdUpper + i, &l, sizeof(l)) != sizeof(l)) l = 800;
    CTL_SetThresholdUpper(i, l);
    if (NVS_ReadTag(NVTAG_DilPeriodBelowLT + i, &a, sizeof(a)) != sizeof(a)) a = 60;
    CTL_SetDilPeriodBelowLT(i, a);
    if (NVS_ReadTag(NVTAG_DilPeriodAboveLT + i, &a, sizeof(a)) != sizeof(a)) a = 15;
    CTL_SetDilPeriodAboveLT(i, a);
    if (NVS_ReadTag(NVTAG_AddingVolume + i, &l, sizeof(l)) != sizeof(l)) l = 20;
    CTL_SetAddingVolume(i, l);
    if (NVS_ReadTag(NVTAG_TubeOn + i, &b, sizeof(b)) != sizeof(b)) b = TRUE;
    CTL_SetTubeOn(i, b);
    if (NVS_ReadTag(NVTAG_Multiplier + i, &l, sizeof(l)) != sizeof(l)) l = 15;
    CTL_SetMultiplier(i, l);
    if (NVS_ReadTag(NVTAG_DilXcount + i, &a, sizeof(a)) != sizeof(a)) a = 3;
    CTL_SetDilXcount(i, a);
    if (NVS_ReadTag(NVTAG_InitConcentration + i, &l, sizeof(l)) != sizeof(l)) l = 100;
    CTL_SetInitConcentration(i, l);
    if (NVS_ReadTag(NVTAG_Zone4Concentration + i, &l, sizeof(l)) != sizeof(l)) l = 100;
    CTL_SetZone4Concentration(i, l);
    if (NVS_ReadTag(NVTAG_Zone4FixedConcentration + i, &b, sizeof(b)) != sizeof(b)) b = FALSE;
    CTL_SetZone4FixedConcentration(i, b);
    if (NVS_ReadTag(NVTAG_GrowthCorrectionEod + i, &a, sizeof(a)) != sizeof(a)) a = 0;
    CTL_SetGrowthCorrectionEod(i, a);
    if (NVS_ReadTag(NVTAG_GrowthCorrectionEur + i, &a, sizeof(a)) != sizeof(a)) a = 0;
    CTL_SetGrowthCorrectionEur(i, a);
    if (NVS_ReadTag(NVTAG_GrowthCorrectionEuf + i, &a, sizeof(a)) != sizeof(a)) a = 0;
    CTL_SetGrowthCorrectionEuf(i, a);
    GUI_ManualConcentration(i, 0);
    GUI_ManualDilutionParams(i, 0, 0);
    GUI_EnableManualDilution(i, TRUE);
  }
  if (NVS_ReadTag(NVTAG_DilPeriodUpper, &a, sizeof(a)) != sizeof(a)) a = 120;
  CTL_SetDilPeriodUpper(a);
  if (NVS_ReadTag(NVTAG_InConcentrationL, &l, sizeof(l)) != sizeof(l)) l = 0;
  CTL_SetInConcentrationL(l);
  if (NVS_ReadTag(NVTAG_InConcentrationH, &l, sizeof(l)) != sizeof(l)) l = 1000;
  CTL_SetInConcentrationH(l);
  if (NVS_ReadTag(NVTAG_AirDelay, &l, sizeof(l)) != sizeof(l)) l = 5000;
  CTL_SetAirDelay(l);
  if (NVS_ReadTag(NVTAG_AirOn, &b, sizeof(b)) != sizeof(b)) b = FALSE;
  CTL_SetAirOn(b);
  if (NVS_ReadTag(NVTAG_OdTimer, &l, sizeof(l)) != sizeof(l)) l = 7000;
  CTL_SetOdTimer(l);
  if (NVS_ReadTag(NVTAG_Vtube, &l, sizeof(l)) != sizeof(l)) l = 200;
  CTL_SetVtube(l);
  if (NVS_ReadTag(NVTAG_CleanCycles, &a, sizeof(a)) != sizeof(a)) a = 1;
  CTL_SetCleanCycles(a);
  CTL_SetMode(eMode_Off);
  ctl_ResetMorbidostatParams();
}

void ctl_AdjustConcentration(int tube, UINT32 vol, UINT32 con)
{
  TubeConcentration[tube] = 
    (TubeConcentration[tube] * (Vtube - vol) + con * 10 * vol + Vtube / 2) / Vtube;
  GUI_TubeConcentration(tube, (TubeConcentration[tube] + 5) / 10);
}

void ctl_StopDilutions(void)
{
  int i, j;

  OS_CancelTimer(DilTaskTimer);
  memset(DilParams, 0, sizeof(DilParams));
  DilTube = INVALID_TUBE;
  HWC_SetPump(PUMP_LOW, OFF);
  HWC_SetPump(PUMP_HIGH, OFF);
  HWC_SetAir(AirOn);
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < NUM_TUBES; j++)
    {
      HWC_SetValve(j, i, OFF);
    }
  }
}

void ctl_OD(int tube, UINT32 od)
{
  TubeState_t oldState = TubeState[tube];
  INT32 rate;
  static int DilCount[NUM_TUBES];

  switch (oldState)
  {
    case TUBST_Lowest:
      if (od >= ThrhldUpper[tube]) TubeState[tube] = TUBST_Upper;
      else if (od >= ThrhldDrugH[tube]) TubeState[tube] = TUBST_Drug;
      else if (od >= ThrhldLowH[tube]) TubeState[tube] = TUBST_Low;
      break;

    case TUBST_Low:
      if (od >= ThrhldUpper[tube]) TubeState[tube] = TUBST_Upper;
      else if (od >= ThrhldDrugH[tube]) TubeState[tube] = TUBST_Drug;
      else if (od < ThrhldLowL[tube]) TubeState[tube] = TUBST_Lowest;
      break;

    case TUBST_Drug:
      if (od >= ThrhldUpper[tube]) TubeState[tube] = TUBST_Upper;
      else if (od < ThrhldLowL[tube]) TubeState[tube] = TUBST_Lowest;
      else if (od < ThrhldDrugL[tube]) TubeState[tube] = TUBST_Low;
      break;

    case TUBST_Upper:
      if (od < ThrhldLowL[tube]) TubeState[tube] = TUBST_Lowest;
      else if (od < ThrhldDrugL[tube]) TubeState[tube] = TUBST_Low;
      break;
  }

  if (TubeState[tube] != oldState)
  {
    UART_PrintfMsg("Tube %d: Entered Zone %d %s, OD %ld.%03ld",
                   tube + 1, TubeState[tube] + 1, zoneName[TubeState[tube]], od / 1000, od % 1000);
  }

  OdAvg[tube] += od;
  OdCount[tube]++;

  switch (TubeState[tube])
  {
    case TUBST_Lowest:
      if (gTimerTicks >= LastDilusionTicks[tube] + DilPeriodBelowLT[tube])
      {
        ctl_Dilution(tube, AddingVolume[tube], InConcentrationL);
        DilDrug[tube] = FALSE;
        UART_PrintfMsg("Tube %d: Dilution Zone 1 Lowest, P1 only", tube + 1);
        if (OdCount[tube]) OdAvg[tube] /= OdCount[tube];
        LastRate[tube] = OdAvg[tube] - LastOdAvg[tube];
        LastOdAvg[tube] = OdAvg[tube];
        OdAvg[tube] = 0;
        OdCount[tube] = 0;
      }
      lastMode[tube] = 0;
      break;

    case TUBST_Low:
      if (gTimerTicks >= LastDilusionTicks[tube] + DilPeriodAboveLT[tube])
      {
        ctl_Dilution(tube, AddingVolume[tube], InConcentrationL);
        DilDrug[tube] = FALSE;
        UART_PrintfMsg("Tube %d: Dilution Zone 2 Low, P1 only", tube + 1);
        if (OdCount[tube]) OdAvg[tube] /= OdCount[tube];
        LastRate[tube] = OdAvg[tube] - LastOdAvg[tube];
        LastOdAvg[tube] = OdAvg[tube];
        OdAvg[tube] = 0;
        OdCount[tube] = 0;
      }
      lastMode[tube] = 0;
      break;

    case TUBST_Upper:
      if (oldState != TUBST_Upper)
      {
        Concentration[tube] = (Concentration[tube] * Multiplier[tube] + 5) / 10;
        if (Concentration[tube] > InConcentrationH) Concentration[tube] = InConcentrationH;
        else if (Concentration[tube] < InConcentrationL) Concentration[tube] = InConcentrationL;
        GUI_CurrentConcentration(tube, Concentration[tube]);
        UART_PrintfMsg("Tube %d: Concentration changed to %ld", tube + 1, Concentration[tube]);
      }
      if (gTimerTicks >= LastDilusionTicks[tube] + DilPeriodUpper)
      {
        ctl_Dilution(tube, AddingVolume[tube], 
          Zone4FixedConcentration[tube] ? Zone4Concentration[tube] : InConcentrationL);
        DilDrug[tube] = FALSE;
        UART_PrintfMsg("Tube %d: Dilution Zone 4 Upper, %s C=%ld", tube + 1, 
          Zone4FixedConcentration[tube] ? "Fixed" : "P1 only",
          Zone4FixedConcentration[tube] ? Zone4Concentration[tube] : InConcentrationL);
        if (OdCount[tube]) OdAvg[tube] /= OdCount[tube];
        LastRate[tube] = OdAvg[tube] - LastOdAvg[tube];
        LastOdAvg[tube] = OdAvg[tube];
        OdAvg[tube] = 0;
        OdCount[tube] = 0;
      }
      lastMode[tube] = 0;
      break;

    case TUBST_Drug:
      if (gTimerTicks >= LastDilusionTicks[tube] + DilPeriodAboveLT[tube])
      {
        if (OdCount[tube]) OdAvg[tube] /= OdCount[tube];
        rate = OdAvg[tube] - LastOdAvg[tube];

        if (rate > LastRate[tube] + (rate > 0 ? GrowthCorrectionEur[tube] : GrowthCorrectionEuf[tube]))
        { // Mode 1
          ctl_Dilution(tube, AddingVolume[tube], Concentration[tube]);
          DilDrug[tube] = TRUE;
          UART_PrintfMsg("Tube %d: Dilution Zone 3 Drug, Mode 1, OD %ld.%03ld, P1+P2 Concentration %ld",
                         tube + 1, OdAvg[tube] / 1000, OdAvg[tube] % 1000, Concentration[tube]);
          lastMode[tube] = 1;
        }
        else if (OdAvg[tube] > LastOdAvg[tube] + GrowthCorrectionEod[tube])
        { // Mode 2
          if (lastMode[tube] != 2)
            DilCount[tube] = DilDrug[tube] ? DilXcount[tube] - 1 : 0;
          
          if (DilCount[tube])
          {
            ctl_Dilution(tube, AddingVolume[tube], InConcentrationL);
            DilDrug[tube] = FALSE;
            UART_PrintfMsg("Tube %d: Dilution Zone 3 Drug, Mode 2, OD %ld.%03ld, P1 only",
                           tube + 1, OdAvg[tube] / 1000, OdAvg[tube] % 1000);
            DilCount[tube]--;
          }
          else
          {
            ctl_Dilution(tube, AddingVolume[tube], Concentration[tube]);
            DilDrug[tube] = TRUE;
            UART_PrintfMsg("Tube %d: Dilution Zone 3 Drug, Mode 2, OD %ld.%03ld, P1+P2 Concentration %ld",
                           tube + 1, OdAvg[tube] / 1000, OdAvg[tube] % 1000, Concentration[tube]);
            DilCount[tube] = DilXcount[tube] - 1;
          }
          lastMode[tube] = 2;
        }
        else
        { // Mode 3
          ctl_Dilution(tube, AddingVolume[tube], InConcentrationL);
          DilDrug[tube] = FALSE;
          UART_PrintfMsg("Tube %d: Dilution Zone 3 Drug, Mode 3, OD %ld.%03ld, P1 only",
                         tube + 1, OdAvg[tube] / 1000, OdAvg[tube] % 1000);
          lastMode[tube] = 3;
        }

        LastOdAvg[tube] = OdAvg[tube];
        LastRate[tube] = rate;
        OdAvg[tube] = 0;
        OdCount[tube] = 0;
      }
      break;
  }
}

void ctl_SetCleaningDil(void)
{
  int i;
  UINT32 time;
  
  for (i = 0; i < NUM_TUBES; i++)
  {
    if (TubeOn[i])
    {
      time = (PUMP_SPEED * AddingVolume[i] + 5) / 10;
      if (cleaningPump == PUMP_HIGH)
      {
        DilParams[i].timeL = 0; 
        DilParams[i].timeH = time; 
      }
      else
      {
        DilParams[i].timeL = time; 
        DilParams[i].timeH = 0; 
      }
    }
  }
}

void ctl_Dilution(int tube, UINT32 volume, UINT32 concentration)
{
  UINT32 time;

  if (!DilParams[tube].timeL && !DilParams[tube].timeH)
  { // Skip this dilution if another one is in progress
    if (concentration < InConcentrationL) concentration = InConcentrationL;
    else if (concentration > InConcentrationH) concentration = InConcentrationH;
    time = (PUMP_SPEED * volume + 5) / 10; 
    DilParams[tube].timeH = (time * (concentration - InConcentrationL) + 
                             (InConcentrationH - InConcentrationL) / 2) /
                             (InConcentrationH - InConcentrationL);
    DilParams[tube].timeL = time - DilParams[tube].timeH;
    DilParams[tube].concentration = concentration;
    LastDilusionTicks[tube] = gTimerTicks;
    OS_Message(TASK_DIL, DILMSG_NEW);
  }
}

void ctl_ResetMorbidostatParams(void)
{
  int i;

  for (i = 0; i < NUM_TUBES; i++)
  {
    DilParams[i].timeL = 0;
    DilParams[i].timeH = 0;
    TubeState[i] = TUBST_Lowest;
    LastDilusionTicks[i] = gTimerTicks;
    Concentration[i] = InitConcentration[i];
    GUI_CurrentConcentration(i, Concentration[i]);
    TubeConcentration[i] = InConcentrationL * 10;
    GUI_TubeConcentration(i, InConcentrationL);
    LastOdAvg[i] = 0;
    OdAvg[i] = 0;
    OdCount[i] = 0;
    lastMode[i] = 0;
    LastRate[i] = 0;
    DilDrug[i] = FALSE;
  }
}

void CTL_DilTask(UINT32 msg)
{
  static enum {
    DILST_Low,
    DILST_High,
    DILST_AirOff,
    DILST_AirOn
  } DilState;
  BOOL nextTube = FALSE;
  int i;

  switch (msg)
  {
    case DILMSG_TIMER: 
      switch (DilState)
      {
        case DILST_Low:
          DilParams[DilTube].timeL = 0;
          HWC_SetPump(PUMP_LOW, OFF);
          HWC_SetValve(DilTube, VAL_LOW, OFF);
          if (DilParams[DilTube].timeH)
          {
            HWC_SetValve(DilTube, VAL_HIGH, ON);
            HWC_SetPump(PUMP_HIGH, ON);
            DilTaskTimer = OS_MessageAfterTimer(TASK_DIL, DILMSG_TIMER, DilParams[DilTube].timeH);
            DilState = DILST_High;
          }
          else
          {
            DilTaskTimer = OS_MessageAfterTimer(TASK_DIL, DILMSG_TIMER, AirDelay);
            DilState = DILST_AirOff;
          }
          break;

        case DILST_High:
          DilParams[DilTube].timeH = 0;
          HWC_SetPump(PUMP_HIGH, OFF);
          HWC_SetValve(DilTube, VAL_HIGH, OFF);
          DilTaskTimer = OS_MessageAfterTimer(TASK_DIL, DILMSG_TIMER, AirDelay);
          DilState = DILST_AirOff;
          break;

        case DILST_AirOff:
          if (AirOn) HWC_SetAir(ON);
          DilTaskTimer = OS_MessageAfterTimer(TASK_DIL, DILMSG_TIMER, 1000);
          DilState = DILST_AirOn;
          break;

        case DILST_AirOn:
          nextTube = TRUE;
          if (cleaning)
            UART_PrintfMsg("Finished cleaning Tube %d Cycles left %d", DilTube + 1, CleaningCyclesLeft);
          break;
      }
      break;

    case DILMSG_NEW:
      if (DilTube == INVALID_TUBE) nextTube = TRUE;
      break;
  }
  
  if (nextTube)
  {
    for (i = 0; i < NUM_TUBES; i++)
    {
      if (++DilTube == NUM_TUBES) DilTube = 0;
      if ((DilParams[DilTube].timeL || DilParams[DilTube].timeH) && !cleaning)
      {
        ctl_AdjustConcentration(DilTube, AddingVolume[DilTube], DilParams[DilTube].concentration);
        UART_PrintfPumpMsg("Tube%d\t%ld\t%ld\t%d\t%d\t%ld.%03ld\t"
                           "%ld\t%ld\t%ld\t%ld.%03ld\t%ld.%03ld\t%ld.%03ld",
          DilTube + 1, DilParams[DilTube].timeL, DilParams[DilTube].timeH,
          TubeState[DilTube] + 1, lastMode[DilTube],
          LastOdAvg[DilTube] / 1000, LastOdAvg[DilTube] % 1000,
          (TubeConcentration[DilTube] + 5) / 10, InConcentrationL, InConcentrationH,
          ThrhldLow[DilTube] / 1000, ThrhldLow[DilTube] % 1000, 
          ThrhldDrug[DilTube] / 1000, ThrhldDrug[DilTube] % 1000, 
          ThrhldUpper[DilTube] / 1000, ThrhldUpper[DilTube] % 1000);
      }
      if (DilParams[DilTube].timeL)
      {
        HWC_SetAir(OFF);
        HWC_SetValve(DilTube, VAL_LOW, ON);
        HWC_SetPump(PUMP_LOW, ON);
        DilTaskTimer = OS_MessageAfterTimer(TASK_DIL, DILMSG_TIMER, DilParams[DilTube].timeL);
        DilState = DILST_Low;
        break;
      }
      else if (DilParams[DilTube].timeH)
      {
        HWC_SetAir(OFF);
        HWC_SetValve(DilTube, VAL_HIGH, ON);
        HWC_SetPump(PUMP_HIGH, ON);
        DilTaskTimer = OS_MessageAfterTimer(TASK_DIL, DILMSG_TIMER, DilParams[DilTube].timeH);
        DilState = DILST_High;
        break;
      }
    }
    if (i == NUM_TUBES)
    {
     if (cleaning)
     {
       GUI_RemainingCycles(CleaningCyclesLeft);
       if (CleaningCyclesLeft--)
       {
         ctl_SetCleaningDil();
         OS_Message(TASK_DIL, DILMSG_NEW);
       }
       else
       {
         CTL_SetMode(eMode_Off); 
       }
     }
     DilTube = INVALID_TUBE;
    }
  }
}

void CTL_OdTask(UINT32 msg)
{
  int i, k;
  UINT32 od[NUM_TUBES];
  double out;
  UINT32 iOut;
  
  switch (msg)
  {
    case ODMSG_TIMER:
      for (i = 0; i < NUM_TUBES; i++) LL_Laser(i, LaserStrength[i]);
      odMeasTimer = OS_MessageAfterTimer(TASK_OD, ODMSG_MEAS, 100);
      break;
      
    case ODMSG_MEAS:
      for (i = 0; i < NUM_TUBES; i++) od[i] = 0;
      for (k = 0; k < 300; k++)
      {
        for (i = 0; i < NUM_TUBES; i++) od[i] += LL_ReadOd(i);
      }
      for (i = 0; i < NUM_TUBES; i++)
      {
        LL_Laser(i, 0);
        if (OdOn && TubeOn[i] &&
            DilTube != i && !DilParams[i].timeL && !DilParams[i].timeH) // Ignore OD while diluting
        {
#if MORB_SIMULATE
          if (sim_od[i] > 0) out = sim_od[i];
          else
#endif
          out = OdC[i] * log10(od[i]) + OdV[i];
          if (out < 0) out = 0;
          POD_SendData(i, out);
          iOut = (out + 0.0005) * 1000;
          UART_PrintfOdMsg("Tube%d\t%ld.%03ld", i + 1, iOut / 1000, iOut % 1000);
          if (DilOn) ctl_OD(i, iOut);
        }
      }
      break;
  }
}

void CTL_SetOdTimer(UINT32 period)
{
  static UINT32 odTimer, odPeriod;

  if (odPeriod != period)
  {
    OS_CancelTimer(odTimer);
    odPeriod = period;
    odTimer = OS_MessageEveryTimer(TASK_OD, ODMSG_TIMER, odPeriod);
    GUI_MeasureInterval(period / 1000);
    UART_PrintfMsg("Set OD Period %ld.%03ld", period / 1000, period % 1000);
  }
}

void CTL_SetThresholdLow(int tube, UINT32 threshold)
{
  ThrhldLow[tube] = threshold;
  ThrhldLowL[tube] = threshold * (100 - CTL_THRSHLD_HYST) / 100;
  ThrhldLowH[tube] = (threshold * (100 + CTL_THRSHLD_HYST) + 99) / 100;
  GUI_ThresholdLow(tube, threshold);
  UART_PrintfMsg("Set Tube %d Threshold Low %ld.%03ld", tube + 1, threshold / 1000, threshold % 1000);
}

void CTL_SetThresholdDrug(int tube, UINT32 threshold)
{
  ThrhldDrug[tube] = threshold;
  ThrhldDrugL[tube] = threshold * (100 - CTL_THRSHLD_HYST) / 100;
  ThrhldDrugH[tube] = (threshold * (100 + CTL_THRSHLD_HYST) + 99) / 100;
  GUI_ThresholdDrug(tube, threshold);
  UART_PrintfMsg("Set Tube %d Threshold Drug %ld.%03ld", tube + 1, threshold / 1000, threshold % 1000);
}

void CTL_SetThresholdUpper(int tube, UINT32 threshold)
{
  ThrhldUpper[tube] = threshold;
  GUI_ThresholdUpper(tube, threshold);
  UART_PrintfMsg("Set Tube %d Threshold Upper %ld.%03ld", tube + 1, threshold / 1000, threshold % 1000);
}

void CTL_SetInConcentrationL(UINT32 concentration)
{
  InConcentrationL = concentration;
  GUI_InConcentrationL(concentration);
  UART_PrintfMsg("Set Concentration Low %ld", concentration);
}

void CTL_SetInConcentrationH(UINT32 concentration)
{
  InConcentrationH = concentration;
  GUI_InConcentrationH(concentration);
  UART_PrintfMsg("Set Concentration High %ld", concentration);
}

void CTL_SetMultiplier(int tube, UINT32 mult)
{
  Multiplier[tube] = mult;
  GUI_Multiplier(tube, mult);
  UART_PrintfMsg("Set Tube %d Multiplier %ld.%ld", tube + 1, mult / 10, mult % 10);
}

void CTL_SetDilXcount(int tube, int x)
{
  DilXcount[tube] = x;
  GUI_DilXcount(tube, x);
  UART_PrintfMsg("Set Tube %d X %d", tube + 1, x);
}

void CTL_SetGrowthCorrectionEod(int tube, int e)
{
  GrowthCorrectionEod[tube] = e;
  GUI_GrowthCorrectionEod(tube, e);
  if (e < 0) e = -e;
  UART_PrintfMsg("Set Tube %d Eod %c%d.%03d", tube + 1,
                 (GrowthCorrectionEod[tube] < 0) ? '-' : ' ', e / 1000, e % 1000);
}

void CTL_SetGrowthCorrectionEur(int tube, int e)
{
  GrowthCorrectionEur[tube] = e;
  GUI_GrowthCorrectionEur(tube, e);
  if (e < 0) e = -e;
  UART_PrintfMsg("Set Tube %d Eur %c%d.%03d", tube + 1,
                 (GrowthCorrectionEur[tube] < 0) ? '-' : ' ', e / 1000, e % 1000);
}

void CTL_SetGrowthCorrectionEuf(int tube, int e)
{
  GrowthCorrectionEuf[tube] = e;
  GUI_GrowthCorrectionEuf(tube, e);
  if (e < 0) e = -e;
  UART_PrintfMsg("Set Tube %d Euf %c%d.%03d", tube + 1,
                 (GrowthCorrectionEuf[tube] < 0) ? '-' : ' ', e / 1000, e % 1000);
}

void CTL_SetCleanCycles(int cycles)
{
  CleaningCycles = cycles;
  GUI_NumCleanCycles(CleaningCycles);
  UART_PrintfMsg("Set Cleaning Cycles %d", CleaningCycles);
}

void CTL_SetMode(int mode)
{
  if (morbMode == eMode_Off || mode == morbMode || mode == eMode_Off) 
  {
    if (mode == morbMode) mode = eMode_Off;
    if (DilOn || cleaning) ctl_StopDilutions();
    switch (mode)
    {
      case eMode_Off:
        OdOn = FALSE;
        DilOn = FALSE;
        cleaning = FALSE;
        UART_PrintfMsg("Set Mode Off");
        break;

      case eMode_OdReadings:
        OdOn = TRUE;
        DilOn = FALSE;
        cleaning = FALSE;
        UART_PrintfMsg("Set Mode OD Reading");
        break;

      case eMode_MorbidostatRestart:
        ctl_ResetMorbidostatParams();
        // Fall through

      case eMode_MorbidostatRun:
        OdOn = TRUE;
        DilOn = TRUE;
        cleaning = FALSE;
        UART_PrintfMsg("Set Mode Morbidostat %s", mode == eMode_MorbidostatRestart ?
                                              "Restart" : "Continue");
        CTL_PrintAllParams();
        mode = eMode_MorbidostatRun;
        break;

      case eMode_CleanPump1:
        OdOn = FALSE;
        DilOn = FALSE;
        cleaning = TRUE;
        cleaningPump = PUMP_LOW;
        UART_PrintfMsg("Set Mode Cleaning Pump 1");
        break;

      case eMode_CleanPump2:
        OdOn = FALSE;
        DilOn = FALSE;
        cleaning = TRUE;
        cleaningPump = PUMP_HIGH;
        UART_PrintfMsg("Set Mode Cleaning Pump 2");
        break;
    }
    morbMode = mode;
    GUI_Mode(mode);
    GUI_Pause(mode == eMode_MorbidostatRun, FALSE);
    if (cleaning)
    {
      CleaningCyclesLeft = CleaningCycles;
      OS_Message(TASK_DIL, DILMSG_NEW);
    }
    else
    {
      CleaningCyclesLeft = 0;
    }
    GUI_RemainingCycles(CleaningCyclesLeft);
  }
}

void CTL_ToggleMorbidostatPause(void)
{
  if (morbMode == eMode_MorbidostatRun)
  {
    OdOn = !OdOn;
    UART_PrintfMsg("Morbidostat %s", OdOn ? "Resumed" : "Paused");
    GUI_Pause(TRUE, !OdOn);
  }
}

void CTL_SetAirOn(BOOL on)
{
  AirOn = on;
  GUI_AirOn(on);
  HWC_SetAir(AirOn);
  UART_PrintfMsg("Set Air %s", on ? "ON" : "OFF");
}

void CTL_ToggleAirOn(void)
{
  CTL_SetAirOn(!AirOn);
  NVS_WriteTag(NVTAG_AirOn, &AirOn, sizeof(AirOn));
}

void CTL_SetVtube(UINT32 dmL)
{
  Vtube = dmL;
  GUI_Vtube(dmL);
  UART_PrintfMsg("Set Tube Volume %ld.%1ld", dmL / 10, dmL % 10);
}

void CTL_SetAddingVolume(int tube, UINT32 dmL)
{
  AddingVolume[tube] = dmL;
  GUI_AddingVolume(tube, dmL);
  UART_PrintfMsg("Set Tube %d Adding Volume %ld.%1ld", tube + 1, dmL / 10, dmL % 10);
}

void CTL_SetAirDelay(UINT32 mSec)
{
  AirDelay = mSec;
  GUI_AirDelay(mSec);
  UART_PrintfMsg("Set Air Delay %ld.%03ld", mSec / 1000, mSec % 1000);
}

void CTL_SetDilPeriodBelowLT(int tube, int min)
{
  DilPeriodBelowLT[tube] = min * MINUTE_TICKS;
  GUI_DilPeriodBelowLT(tube, min);
  UART_PrintfMsg("Set Tube %d Period BL %d", tube + 1, min);
}

void CTL_SetDilPeriodAboveLT(int tube, int min)
{
  DilPeriodAboveLT[tube] = min * MINUTE_TICKS;
  GUI_DilPeriodAboveLT(tube, min);
  UART_PrintfMsg("Set Tube %d Period AL %d", tube + 1, min);
}

void CTL_SetDilPeriodUpper(int sec)
{
  DilPeriodUpper = sec * SEC_TICKS;
  GUI_DilPeriodUpper(sec);
  UART_PrintfMsg("Set Period Upper %d", sec);
}

void CTL_SetInitConcentration(int tube, UINT32 c)
{
  InitConcentration[tube] = c;
  GUI_InitConcentration(tube, c);
  UART_PrintfMsg("Set Tube %d Initial Concentration %ld", tube + 1, c);
}

void CTL_SetZone4Concentration(int tube, UINT32 c)
{
  Zone4Concentration[tube] = c;
  GUI_Zone4Concentration(tube, c);
  UART_PrintfMsg("Set Tube %d Zone4 Concentration %ld", tube + 1, c);
}

void CTL_SetZone4FixedConcentration(int tube, BOOL fixed)
{
  Zone4FixedConcentration[tube] = fixed;
  GUI_Zone4FixedConcentration(tube, fixed);
  UART_PrintfMsg("Set Tube %d Zone4 Fixed Concentration %s", tube + 1, fixed ? "on" : "off");
}

void CTL_SetCurrentConcentration(int tube, UINT32 c)
{
  Concentration[tube] = c;
  GUI_CurrentConcentration(tube, c);
  UART_PrintfMsg("Set Tube %d Current Concentration %ld", tube + 1, c);
}

void CTL_ManualDilution(int tube, UINT32 v, UINT32 c)
{
  ctl_AdjustConcentration(tube, v, c);
  GUI_ManualDilutionParams(tube, v, c);
  UART_PrintfMsg("Manual Dilution Tube %d: Volume %ld.%0ld, Concentration %ld",
                 tube + 1, v / 10, v % 10, c);
  UART_PrintfPumpMsg("Manual Dilution\t%d\t%ld.%0ld\t%ld",
                 tube + 1, v / 10, v % 10, c);
}

void CTL_SetOdC(int tube, float c)
{
  int a = ((c > 0 ? c : -c) + 0.005) * 100;
   
  OdC[tube] = c;
  GUI_CalibrationC(tube, c);
  UART_PrintfMsg("Set Tube %d Calibration C %c%d.%02d",
                 tube + 1, OdC[tube] < 0 ? '-' : ' ', a / 100, a % 100);
}

void CTL_SetOdV(int tube, float v)
{
  int a = ((v > 0 ? v : -v) + 0.005) * 100;
   
  OdV[tube] = v;
  GUI_CalibrationV(tube, v);
  UART_PrintfMsg("Set Tube %d Calibration V %c%d.%02d",
                 tube + 1, OdV[tube] < 0 ? '-' : ' ', a / 100, a % 100);
}

void CTL_SetLaserStrength(int tube, UINT32 l)
{
  LaserStrength[tube] = l;
  GUI_CalibrationL(tube, l);
  UART_PrintfMsg("Set Tube %d Calibration Laser %ld", tube + 1, l);
}

void CTL_SetTubeOn(int tube, BOOL on)
{
  TubeOn[tube] = on;
  GUI_TubeOn(tube, on);
  UART_PrintfMsg("Set Tube %d %s", tube + 1, on ? "ON" : "OFF");
}

void CTL_ToggleTubeOn(int tube)
{
  CTL_SetTubeOn(tube, !TubeOn[tube]);
  NVS_WriteTag(NVTAG_TubeOn + tube, &TubeOn[tube], sizeof(TubeOn[tube]));
}

void CTL_PrintAllParams(void)
{
  int i;
  int eod, eur, euf;

  UART_PrintfMsg("                 Basic Parameters");
  UART_PrintfMsg("====================================================");
  UART_PrintfMsg("tube    LT     DT     UT     TBL    TAL    dV     C ");
  UART_PrintfMsg("                            (min)  (min)  (mL) Init ");
  UART_PrintfMsg("====================================================");

  for (i = 0; i < NUM_TUBES; i++)
  {
    UART_PrintfMsg("%2d   %2ld.%03ld %2ld.%03ld %2ld.%03ld  %5ld  %5ld %4ld.%1ld %4ld",
                   i + 1,
                   ThrhldLow[i] / 1000, ThrhldLow[i] % 1000,
                   ThrhldDrug[i] / 1000, ThrhldDrug[i] % 1000,
                   ThrhldUpper[i] / 1000, ThrhldUpper[i] % 1000,
                   DilPeriodBelowLT[i] / MINUTE_TICKS,
                   DilPeriodAboveLT[i] / MINUTE_TICKS,
                   AddingVolume[i] / 10, AddingVolume[i] % 10,
                   InitConcentration[i]);
  }
  UART_PrintfMsg("====================================================");

  UART_PrintfMsg("                 Advanced Parameters");
  UART_PrintfMsg("====================================================");
  UART_PrintfMsg("tube  M   X  Zone4    Zone4      Eod     Eur     Euf");
  UART_PrintfMsg("                 C  Fixed C");
  UART_PrintfMsg("====================================================");

  for (i = 0; i < NUM_TUBES; i++)
  {
    eod = GrowthCorrectionEod[i];
    eur = GrowthCorrectionEur[i];
    euf = GrowthCorrectionEuf[i];

    if (eod < 0) eod = -eod;
    if (eur < 0) eur = -eur;
    if (euf < 0) euf = -euf;

    UART_PrintfMsg("%2d  %2ld.%0ld %2d   %4ld        %c   %c%d.%03d  %c%d.%03d  %c%d.%03d",
                   i + 1,
                   Multiplier[i] / 10, Multiplier[i] % 10,
                   DilXcount[i], 
                   Zone4Concentration[i],
                   Zone4FixedConcentration[i] ? 'Y':'-',
                   GrowthCorrectionEod[i] < 0 ? '-' : ' ', eod / 1000, eod % 1000,
                   GrowthCorrectionEur[i] < 0 ? '-' : ' ', eur / 1000, eur % 1000,
                   GrowthCorrectionEuf[i] < 0 ? '-' : ' ', euf / 1000, euf % 1000
                 );
  }
  UART_PrintfMsg("====================================================");
  UART_PrintfMsg(" C1 = %ld, C2 = %ld, V = %ld.%0ld",
                 InConcentrationL, InConcentrationH, Vtube / 10, Vtube % 10);
  UART_PrintfMsg(" Tupper = %ld", DilPeriodUpper / SEC_TICKS);
  UART_PrintfMsg(" Dilutions - %c, OdReadings - %c, Air - %c, Air Delay - %ld.%01ld",
                 DilOn ? 'Y' : 'N', OdOn ? 'Y' : 'N', AirOn ? 'Y' : 'N',
                 AirDelay / 1000, (AirDelay % 1000) / 100);
  UART_PrintfMsg("============================================================");
}

void CTL_PrintRunStatus(void)
{
  int i;

  if (morbMode == eMode_MorbidostatRun)
  {
    UART_PrintfMsg("==================================");
    UART_PrintfMsg("tube  C    C     OD   Zone    Mode");
    UART_PrintfMsg("     dil tube");
    UART_PrintfMsg("==================================");

    for (i = 0; i < NUM_TUBES; i++)
    {
      UART_PrintfMsg("%2d  %4ld %4ld %3ld.%03ld %d-%-6s %d",
                     i + 1, Concentration[i], (TubeConcentration[i] + 5) / 10,
                     LastOdAvg[i] / 1000, LastOdAvg[i] % 1000,
                     TubeState[i] + 1, zoneName[TubeState[i]], lastMode[i]);
    }
    UART_PrintfMsg("==================================");
  }
}

#if MORB_SIMULATE
void CTL_SetSimulateOd(int tube, double od)
{
  int a = (od + 0.0005) * 1000;

  sim_od[tube] = od;
  UART_PrintfMsg("Set Tube %d Simulated OD %d.%03d", tube + 1, a / 1000, a % 1000);
}
#endif
