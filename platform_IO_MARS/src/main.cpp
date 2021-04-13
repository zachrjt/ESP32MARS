#include <Arduino.h>
#include <TFT_eSPI.h>     //TFT display libary
#include <SPI.h>          //SPI Libary
#include <Button2.h>      //Button libary
#include <SD.h>           //SD card library
#include <WiFi.h>         //Libary to connect to WiFI
#include <HTTPClient.h>   //Libary to pull http sites
#include <ical_libary.h>  //Ical file manager libary

#include "task_macros.h"            //task marcos for priority, cores, and stack sizes
#include "function_macros.h"        //function macros and general typedefs used for development
#include "Pinout_macros.h"          //pintout macros

#include "peripheral_initialize.h"  //other functions header file
#include "SPI_functions.h"          //SPI functions header file
#include "display_functions.h"
#include "interrupts.h"
#include "ical.h"   //ical setup 


TFT_eSPI tft = TFT_eSPI();  //create tft_display object for usage

Button2 btn1(BUTTON_1_PIN);                 //creating button object for usage
Button2 btn2(BUTTON_2_PIN);                 //creating button object for usage
Button2 btn3(BUTTON_3_PIN);                 //creating button object for usage

SPIClass SDSPI(HSPI); //defines the spi bus for use with the SD card                    ///////////////not sure how to use with the ical setup cpp since i defined it there
SPIClass PIC1_SPI;    //defines the spi bus for use with the PIC with clock display
SPIClass PIC2_SPI;    //defines the spi bus for use with the PIC with alarm capabilities

extern Calendar myCalendar;
extern long sector_table;

extern int TMRF;
extern int DISPLAYTMRF;
extern int snoozeFlag;
extern int PomodoroFlag;
extern int ALARMSTAMP;
extern int TIMESTAMP;
extern uint8_t AlarmFlag;

void setup()
{
  Serial.begin(115200);

  if (!setup_functions())
  {
    //record error message
  }
  displaySetup();   //will put in set up initialize eventually (maybe)
  PICSPISetup();
  setUpInterrupts();
  icalLibarySetup();
  clockButtonsSetup();

  printNextEvent();
  printTemperature();
}

void loop()
{
  btn1.loop();
  btn2.loop();
  btn3.loop();

  if (TMRF >= TIME_REQUEST_INTERVAL)  // gets time from PIC every 1 second
  {
    TMRF = 0;
    getTimefromPIC1();
    if(snoozeFlag)
    {
      checkSnooze();
    }
    if(PomodoroFlag)
    {
      updatePomodoroTime();
      displayPomodoro();  
    }
  }


  if(DISPLAYTMRF)   //updates display for non-pomodoro mode
  {
    if (!PomodoroFlag)
    {
      updateDisplay();
    }
    DISPLAYTMRF = 0;
  }



  /*if (ALARMSTAMP == TIMESTAMP) //checks for alarm from calendar event
  {
    if(!AlarmFlag)
    {
      AlarmFlag = ALARM_ON;
      sendAlarmFlagtoPIC2();
      updateEvents();         //it probably works
    }
  }*/
}
