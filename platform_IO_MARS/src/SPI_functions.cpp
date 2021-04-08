#include "Pinout_macros.h"          //pintout macros
#include "SPI_functions.h"          //SPI functions header file
#include <Arduino.h>
#include <TFT_eSPI.h>     //TFT display libary
#include <SPI.h>          //SPI Libary
#include <Button2.h>      //Button libary
#include <SD.h>           //SD card library

extern Button2 btn1;                 //creating button object for usage
extern Button2 btn2;                 //creating button object for usage

extern SPIClass SDSPI; //defines the spi bus for use with the SD card
extern SPIClass PIC1_SPI;    //defines the spi bus for use with the PIC with clock display
extern SPIClass PIC2_SPI;    //defines the spi bus for use with the PIC with alarm capabilities

SPISettings PICSPISettings =  {
                              5000000, 
                              MSBFIRST,
                              SPI_MODE0
                              };

uint8_t Time0;
uint8_t Time1;
uint8_t Time2;

int Hours = 0;
int Minutes = 51;
int Seconds = 55;

uint8_t AlarmFlag;

void sendTimetoPIC1(void)
{
    Time0 = 0;
    Time1 = 0;
    Time2 = 0;

    if(Hours > 12)      //Conversions from ESP32 time to PIC display time vars (Hope this doesnt take too long in processing time)
    {
        Time2 += 0b00001000;
        Time0 += (uint8_t)(((Hours - 12) / 10) << 7);
        Serial.println(Time0);
        Time0 += (uint8_t)(((Hours - 12) % 10) << 3);
        Serial.println(Time0);
    }
    else if(Hours)
    {
        Time0 += (uint8_t)((Hours / 10) << 7);
        Time0 += (uint8_t)((Hours % 10) << 3);
    }
    else 
    {
        Time0 += 0b10010000;
    }

    Time0 += (uint8_t)(Minutes / 10);
    Serial.println(Time0);
    Time1 += (uint8_t)((Minutes % 10) << 4);
    Serial.println(Time1);

    Time1 += (uint8_t)(Seconds / 10);
    Time2 += (uint8_t)((Seconds % 10) << 4);

    digitalWrite(PIC1_SPICS, LOW);
    PIC1_SPI.beginTransaction(PICSPISettings);

    PIC1_SPI.transfer(Time0);
    PIC1_SPI.transfer(Time1);
    PIC1_SPI.transfer(Time2);

    PIC1_SPI.endTransaction();
    digitalWrite(PIC1_SPICS, HIGH);
}



void  getTimefromPIC1(void)                     //Dont pull from PIC more than once a second or you will get erroneous values for Time2
{
    digitalWrite(PIC1_SPICS, LOW);
    PIC1_SPI.beginTransaction(PICSPISettings);

    Time2 = PIC1_SPI.transfer(FIRST_TIME_REQUEST);
    Time1 = PIC1_SPI.transfer(SUBSEQUENT_TIME_REQUEST);
    Time0 = PIC1_SPI.transfer(SUBSEQUENT_TIME_REQUEST);

    PIC1_SPI.endTransaction();
    digitalWrite(PIC1_SPICS, HIGH);

    Hours = 0;
    Minutes = 0;
    Seconds = 0;
                                                    //Hope this doesnt take too long in processing time
    Hours += (int)((Time0 >> 7) * 10);              //bitmasking and shifting according to PIC time display standars
    Hours += (int)((Time0 & 0b01111000) >> 3);

    Minutes += (int)(Time0 & 0b00000111) * 10;
    Minutes += (int)((Time1 & 0b11110000) >> 4);

    Seconds += (int)(Time1 & 0b00001111) * 10;
    Seconds += (int)((Time2 & 0b11110000) >> 4);


    if((Time2 & 0b00001000) == 0b00001000)
    {
        Hours += 12;                                //24 hour clock
    }
    else
    {
        Hours %= 12;                                //make it 0:00 am rather than 12:00 am
    }
}



void sendAlarmFlagtoPIC2(void)
{
    digitalWrite(PIC2_SPICS, LOW);
    PIC2_SPI.beginTransaction(PICSPISettings);

    PIC2_SPI.transfer(AlarmFlag);

    PIC2_SPI.endTransaction();
    digitalWrite(PIC2_SPICS, HIGH);
}



//Button Interrupts (?) for features, for some reason these are each triggered once during Start up so AlarmFlag is default off
void pressed(Button2& btn) {

    if(btn ==  btn1)
    {
        if (AlarmFlag)  //Snooze Alarm if its on
        {
        AlarmFlag = ALARM_OFF;
        sendAlarmFlagtoPIC2();

        //Set up new Alarm event 5 minutes from now here
        }
        else      //for testing
        {
            AlarmFlag = ALARM_ON;
            sendAlarmFlagtoPIC2();
        }
        //getTimefromPIC1();  //for testing
    }
    else if (btn ==  btn2)
    {
        if (AlarmFlag)  //Turn off Alarm if its on
        {
            AlarmFlag = ALARM_OFF;
            sendAlarmFlagtoPIC2();

            //Update event(?) here (not sure if needed)
        }

        sendTimetoPIC1();      //for testing
    }
}