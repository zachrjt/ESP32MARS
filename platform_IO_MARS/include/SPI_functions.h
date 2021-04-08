#include <Arduino.h>
#include <TFT_eSPI.h>     //TFT display libary
#include <SPI.h>          //SPI Libary
#include <Button2.h>      //Button libary
#include <SD.h>           //SD card library

#ifndef SPI_FUNCTIONS_H
    #define SPI_FUNCTIONS_H

    #define ALARM_ON    1
    #define ALARM_OFF   0

    #define FIRST_TIME_REQUEST      1
    #define SUBSEQUENT_TIME_REQUEST 2

    void sendTimetoPIC1(void);          //Send time to PIC
    void getTimefromPIC1(void);         //Get time from PIC
    void sendAlarmFlagtoPIC2(void);     //If Alarm then alarm
    void pressed(Button2& btn);       //Button Pressed event

#endif