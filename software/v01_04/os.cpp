/* COPYRIGHT (c) 2019 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include "ArduinoTimer.h"

#include "common.h"

typedef struct
{
  UINT32 counter;
  UINT32 time;
  UINT32 period;
  UINT32 taskId;
  UINT32 msg;
} timer_t;

static timer_t sTimers[32];

typedef struct
{
  void (*pFunc)(UINT32 msg);
  UINT8  qSize;
  UINT8  qHead;
  UINT8  qTail;
  UINT8  qMax;
  UINT32 qOverflows;
  UINT32 *pQueue;
  char   *name;
} task_t;

static UINT32 timerQueue[20];
static UINT32 LEDQueue[20];
static UINT32 dilQueue[20];
static UINT32 odQueue[20];
static UINT32 tempQueue[20];

static UINT32 osMsecs, osMsecsFromLastTick;

#define QUEUE(_name) sizeof(_name##Queue)/sizeof(_name##Queue[0]), 0, 0, 0, 0, _name##Queue, (char *)#_name

static task_t sTasks[] =
{
  {OS_TimerTask, QUEUE(timer)},
  {CTL_DilTask,  QUEUE(dil)  },
  {CTL_OdTask,   QUEUE(od)   },
  {TEMP_Task,    QUEUE(temp) },
  {LED_Task,     QUEUE(LED)  },
};

#define OS_MAX_TIMERS      (int)(sizeof(sTimers)/sizeof(sTimers[0]))
#define OS_TIMERS_LIST_END (&sTimers[OS_MAX_TIMERS])
#define OS_TASKS_LIST_END  (&sTasks[NUM_TASKS])

void OS_Init(void)
{
  osMsecs = millis();
  osMsecsFromLastTick = 0;
}

void OS_TasksScheduler(void)
{
  task_t *pTask;
  UINT32 ticks = millis();

  osMsecsFromLastTick += ticks - osMsecs;
  osMsecs = ticks;
  if (osMsecsFromLastTick >= TIMER_TICK_MS)
  {
    gTimerTicks += osMsecsFromLastTick / TIMER_TICK_MS;
    osMsecsFromLastTick = osMsecsFromLastTick % TIMER_TICK_MS;
    OS_Message(TASK_OS_TIMER, 0);
  }

  for (pTask = sTasks; pTask < OS_TASKS_LIST_END; pTask++)
  {
    if (pTask->qHead != pTask->qTail)
    {
      pTask->pFunc(pTask->pQueue[pTask->qHead]);
      if (++(pTask->qHead) == pTask->qSize) pTask->qHead = 0;
      break;
    }
  }
}

void OS_TimerTask(UINT32 msg)
{
  timer_t *pTimer;

  (void) msg;
  for (pTimer = sTimers; pTimer < OS_TIMERS_LIST_END; pTimer++)
  {
    if (pTimer->time && gTimerTicks >= pTimer->time)
    {
      OS_Message(pTimer->taskId, pTimer->msg);
      pTimer->time = pTimer->period ? (pTimer->time + pTimer->period) : 0;
    }
  }
}

BOOL OS_Message(UINT32 taskId, UINT32 msg)
{
  task_t *pTask = &sTasks[taskId];
  UINT8 size;

  if ((!pTask->qHead && pTask->qTail != pTask->qSize - 1) ||
      (pTask->qHead && pTask->qHead != pTask->qTail + 1))
  {
    pTask->pQueue[pTask->qTail] = msg;
    if (++(pTask->qTail) == pTask->qSize) pTask->qTail = 0;
    size = pTask->qTail - pTask->qHead;
    if (pTask->qTail < pTask->qHead) size += pTask->qSize;
    if (size > pTask->qMax) pTask->qMax = size;
    return(TRUE);
  }
  else
  {
    pTask->qOverflows++;
    return(FALSE);
  }
}

UINT32 OS_MessageAfterTimer(UINT32 taskId, UINT32 msg, UINT32 delay)
{
  int i;

  for (i = 0; i < OS_MAX_TIMERS && sTimers[i].time; i++);

  if (i != OS_MAX_TIMERS)
  {
    sTimers[i].taskId = taskId;
    sTimers[i].msg = msg;
    sTimers[i].time = gTimerTicks + (delay + TIMER_TICK_MS / 2) / TIMER_TICK_MS;
    sTimers[i].period = 0;
    sTimers[i].counter++;
    return((sTimers[i].counter << 8) | (i + 1));
  }
  else
  {
    return(0);
  }
}

UINT32 OS_MessageEveryTimer(UINT32 taskId, UINT32 msg, UINT32 period)
{
  UINT32 id;

  id = OS_MessageAfterTimer(taskId, msg, period);
  if (id) sTimers[(id & 0xFF) - 1].period = (period + TIMER_TICK_MS / 2) / TIMER_TICK_MS;
  return(id);
}

void OS_CancelTimer(UINT32 timerId)
{
  if ((timerId & 0xFF) &&
      (timerId >> 8) == (sTimers[(timerId & 0xFF) - 1].counter & 0x00FFFFFF))
    sTimers[(timerId & 0xFF) - 1].time = 0;
}

void OS_PrintStats(void)
{
  task_t *pTask;
  int i;

  UART_Printf("Tasks\r\n");
  UART_Printf("     Task      qSize   qHead   qTail   qMax   qOfl\r\n");
  for (pTask = sTasks; pTask < OS_TASKS_LIST_END; pTask++)
  {
    UART_Printf("%10s%9d%8d%8d%7d%7d\r\n",
                pTask->name, pTask->qSize, pTask->qHead, pTask->qTail,
                pTask->qMax, pTask->qOverflows);
  }

  UART_Printf("\r\nTimers (current time: %ld ticks, %ld msecs)\r\n", gTimerTicks, millis());
  UART_Printf(" idx   counter   time  period  task\r\n");
  for (i = 0; i < OS_MAX_TIMERS; i++)
  {
    if (sTimers[i].time)
    {
      UART_Printf(" %2d %7ld   %7ld %7ld  %s\r\n",
        i, sTimers[i].counter, sTimers[i].time, sTimers[i].period, sTasks[sTimers[i].taskId].name);
    }
  }

}
