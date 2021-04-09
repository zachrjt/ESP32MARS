#include "interrupts.h"

volatile int TMRF;
hw_timer_t * timer = NULL;
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
}

void IRAM_ATTR onTimer() 
{
  portENTER_CRITICAL_ISR(&timerMux);
  TMRF++;
  portEXIT_CRITICAL_ISR(&timerMux);
}