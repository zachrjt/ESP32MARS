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

uint8_t Time0 = 76;
uint8_t Time1 = 32;
uint8_t Time2 = 64;

uint8_t AlarmFlag;

void sendTimetoPIC1(void)
{
  digitalWrite(PIC1_SPICS, LOW);
  PIC1_SPI.beginTransaction(PICSPISettings);

  PIC1_SPI.transfer(Time0);
  PIC1_SPI.transfer(Time1);
  PIC1_SPI.transfer(Time2);

  Serial.println(Time0);
  Serial.println(Time1);
  Serial.println(Time2);

  PIC1_SPI.endTransaction();
  digitalWrite(PIC1_SPICS, HIGH);
}



void  getTimefromPIC1(void)
{
  digitalWrite(PIC1_SPICS, LOW);
  PIC1_SPI.beginTransaction(PICSPISettings);

  Time0 = PIC1_SPI.transfer(1);
  Time1 = PIC1_SPI.transfer(2);
  Time2 = PIC1_SPI.transfer(2);

  Serial.println(Time0);
  Serial.println(Time1);
  Serial.println(Time2);

  PIC1_SPI.endTransaction();
  digitalWrite(PIC1_SPICS, HIGH);
}



void sendAlarmFlagtoPIC2(void)
{
  digitalWrite(PIC2_SPICS, LOW);
  PIC1_SPI.beginTransaction(PICSPISettings);

  PIC1_SPI.transfer(AlarmFlag);

  PIC1_SPI.endTransaction();
  digitalWrite(PIC2_SPICS, HIGH);
}



//Button Interrupts (?) for features, for some reason these are each triggered once during Start up so AlarmFlag is default off
void pressed(Button2& btn) {

  if(btn ==  btn1)
  {
    Serial.println("Snooze if Alarm flag is on, do nothing otherwise");
    if (AlarmFlag)
    {
      AlarmFlag = 0;
      sendAlarmFlagtoPIC2();

      //Set up new Alarm event 5 minutes from now here
    }
    else      ///This is just for debugging
    {
      AlarmFlag = 1;
      sendAlarmFlagtoPIC2();
    }
  }
  else if (btn ==  btn2)
  {
    Serial.println("Turn off Alarm if its on");
    if (AlarmFlag)
    {
      AlarmFlag = 0;
      sendAlarmFlagtoPIC2();

      //Update event(?) here (not sure if needed)
    }
  }
}