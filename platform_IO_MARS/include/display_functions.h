#include <Arduino.h>
#include <SPI.h>          //SPI Libary
#include <TFT_eSPI.h>     //TFT display libary

#ifndef DISPLAY_FUNCTIONS_H
    #define DISPLAY_FUNCTIONS_H
    
    void displaySetup();
    void printSplitString(String text);
    void printNextEvent(void);
    void printTemperature(void);
    void updateDisplay(void);
    void displayPomodoro(void);
#endif