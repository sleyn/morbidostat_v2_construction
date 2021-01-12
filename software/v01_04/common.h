/* COPYRIGHT (c) 2019 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#ifndef COMMON_H
#define COMMON_H


#define MOB_VER_MAJOR 1
#define MOB_VER_MINOR 4

#define MORB_SIMULATE 1

#define DEBUG 1
#if DEBUG
#define DPRINTF(...) UART_Printf(__VA_ARGS__)
#else
#define DPRINTF(...)
#endif

typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned long  UINT32;
typedef signed char    INT8;
typedef signed short   INT16;
typedef signed long    INT32;
typedef int BOOL;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif
#ifndef NULL
#define NULL ((void *)0)
#endif

#define TIMER_TICK_MS  10
#define SEC_TICKS      (1000L / TIMER_TICK_MS)
#define MINUTE_TICKS   (1000L * 60 / TIMER_TICK_MS)
#define NUM_TUBES 6

#define PUMP_LOW  0
#define PUMP_HIGH 1
#define VAL_LOW   0
#define VAL_HIGH  1
#define OFF       FALSE
#define ON        TRUE

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN UINT32 gTimerTicks;

enum
{
  TASK_OS_TIMER,
  TASK_DIL,
  TASK_OD,
  TASK_TEMP,
  TASK_LED,
  NUM_TASKS
};

enum
{
  eMode_Off = 0,
  eMode_OdReadings,
  eMode_Morbidostat,
  eMode_CleanPump1,
  eMode_CleanPump2,
  eMode_Max
};

void UART_Printf(const char *format, ...);
void UART_PrintfMsg(const char *format, ...);
void UART_PrintfOdMsg(const char *format, ...);
void UART_PrintfPumpMsg(const char *format, ...);

extern void POD_Init(void);
extern void POD_SendData(int od_index, double data);

void OS_TasksScheduler(void);
BOOL OS_Message(UINT32 taskId, UINT32 msg);
UINT32 OS_MessageAfterTimer(UINT32 taskId, UINT32 msg, UINT32 delay);
UINT32 OS_MessageEveryTimer(UINT32 taskId, UINT32 msg, UINT32 period);
void OS_CancelTimer(UINT32 timerId);
void OS_TimerTask(UINT32 msg);
void OS_PrintStats(void);
void OS_Init(void);

void LL_Init(void);
void LL_Pump(int high, BOOL on);
void LL_Valve(int tube, int high, BOOL on);
void LL_Air(BOOL on);
INT16 LL_Temp(void);
void LL_LED(BOOL on);
void LL_Heater(BOOL on);
int LL_ReadOd(int tube);
void LL_Laser(int tube, int strength);

void CTL_Init(void);
void CTL_OdTask(UINT32 msg);
void CTL_DilTask(UINT32 msg);
void CTL_SetOdTimer(UINT32 period);
void CTL_SetThresholdLow(int tube, UINT32 threshold);
void CTL_SetThresholdDrug(int tube, UINT32 threshold);
void CTL_SetThresholdUpper(int tube, UINT32 threshold);
void CTL_SetInConcentrationL(UINT32 concentration);
void CTL_SetInConcentrationH(UINT32 concentration);
void CTL_SetMultiplier(UINT32 mult);
void CTL_SetDilXcount(int x);
void CTL_SetGrowthCorrectionEod(int e);
void CTL_SetGrowthCorrectionEur(int e);
void CTL_SetGrowthCorrectionEuf(int e);
void CTL_SetMode(int mode);
void CTL_SetAirOn(BOOL on);
void CTL_SetVtube(UINT32 mL);
void CTL_SetAddingVolume(int tube, UINT32 mL);
void CTL_SetAirDelay(UINT32 mSec);
void CTL_SetDilPeriodBelowLT(int tube, int min);
void CTL_SetDilPeriodAboveLT(int tube, int min);
void CTL_SetDilPeriodUpper(int sec);
void CTL_SetConcentration(UINT32 c);
void CTL_SetOdC(int tube, float c);
void CTL_SetOdV(int tube, float v);
void CTL_SetLaserStrength(int tube, UINT32 l);
void CTL_SetTubeOn(int tube, BOOL on);
void CTL_ToggleTubeOn(int tube);
void CTL_ToggleAirOn(void);
void CTL_PrintAllParams(void);
#if MORB_SIMULATE
void CTL_SetSimulateOd(int tube, double od);
#endif

void TEMP_Init(void);
void TEMP_Task(UINT32 msg);
void TEMP_SetTempOn(BOOL on);
void TEMP_ResetTempMax(void);
void TEMP_ResetTempMin(void);
void TEMP_SetTemp(INT16 temp);
void TEMP_ToggleTempOn(void);

void CMD_Init(void);
void CMD_Task(void);

void GUI_Mode(int mode);
void GUI_AirOn(BOOL on);
void GUI_MeasureInterval(int secs);
void GUI_ThresholdLow    (int tube, UINT32 lt);
void GUI_ThresholdDrug   (int tube, UINT32 dt);
void GUI_ThresholdUpper  (int tube, UINT32 ut);
void GUI_DilPeriodBelowLT(int tube, int tbl);
void GUI_DilPeriodAboveLT(int tube, int tal);
void GUI_AddingVolume    (int tube, UINT32 dV);
void GUI_Concentration(UINT32 c);
void GUI_InConcentrationL(UINT32 c1);
void GUI_InConcentrationH(UINT32 c2);
void GUI_Multiplier(UINT32 M);
void GUI_DilXcount(int X);
void GUI_Vtube(UINT32 V);
void GUI_SetValve(int tube, int high, BOOL on);
void GUI_SetPump(int high, BOOL on);
void GUI_SetAir(BOOL on);
void GUI_AirDelay(UINT32 msecs);
void GUI_SetHeater(BOOL on);
void GUI_TempValues(INT16 temp, INT16 minTemp, INT16 maxTemp);
void GUI_SetTempOn(BOOL on);
void GUI_SetTempValue(INT16 temp);
void GUI_TubeOn(int tube, BOOL on);
void GUI_GrowthCorrectionEod(int e);
void GUI_GrowthCorrectionEur(int e);
void GUI_GrowthCorrectionEuf(int e);
void GUI_DilPeriodUpper(int secs);
void GUI_CalibrationC(int tube, float c);
void GUI_CalibrationV(int tube, float v);
void GUI_CalibrationL(int tube, UINT32 l);
void GUI_Heartbeat(void);
void GUI_FwVersion(void);

void HWC_SetValve(int tube, int high, BOOL on);
void HWC_SetPump(int high, BOOL on);
void HWC_SetAir(BOOL on);
void HWC_SetHeater(BOOL on);
void HWC_ToggleValve(int tube, int high);
void HWC_TogglePump(int high);
void HWC_ToggleAir(void);
void HWC_ToggleHeater(void);
void HWC_Init(void);

void MAIN_Init(void);
void LED_Task(UINT32 msg);
#endif
