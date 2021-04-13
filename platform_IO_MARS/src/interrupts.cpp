#include "interrupts.h"

volatile int TMRF;
volatile int DISPLAYTMRF;
hw_timer_t * timer = NULL;
hw_timer_t * timer2 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

extern int Hours;
extern int Minutes;
extern int Seconds;

void setUpInterrupts()
{
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000, true);
    timerAlarmEnable(timer);

    timer2 = timerBegin(1, 80, true);
    timerAttachInterrupt(timer2, &displayTimer, true);
    timerAlarmWrite(timer2, 10000000, true);
    timerAlarmEnable(timer2);
}

void IRAM_ATTR onTimer() 
{
  portENTER_CRITICAL_ISR(&timerMux);
  TMRF++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR displayTimer() 
{
  portENTER_CRITICAL_ISR(&timerMux);
  DISPLAYTMRF++;
  portEXIT_CRITICAL_ISR(&timerMux);
}