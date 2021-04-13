#include <Arduino.h>
#include "function_macros.h"

#ifndef INTERRUPTS_H
    #define INTERRUPTS_H
    
    void setUpInterrupts();
    void IRAM_ATTR onTimer();
    void IRAM_ATTR displayTimer();
#endif