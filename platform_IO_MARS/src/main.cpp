#include <Arduino.h>
#include <TFT_eSPI.h>     //TFT display libary
#include <SPI.h>          //SPI Libary
#include <Button2.h>      //Button libary
#include <SD.h>           //SD card library
#include <ical_libary.h>

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

SPIClass SDSPI(HSPI); //defines the spi bus for use with the SD card                    ///////////////not sure how to use with the ical setup cpp since i defined it there
SPIClass PIC1_SPI;    //defines the spi bus for use with the PIC with clock display
SPIClass PIC2_SPI;    //defines the spi bus for use with the PIC with alarm capabilities

extern Calendar myCalendar;
extern long sector_table[SECTORTABLESIZE];

extern int TMRF;
extern int snoozeF;


String Event1 = "Pass out and sleep";   //Name of the next event

void setup()
{
  Serial.begin(115200);

  if (!setup_functions())
  {
    //record error message
  }
  icalLibarySetup();//zach added, just doing first since it takes like 15 seconds

  Event1 = myCalendar.jobs[0]->event_summary;

  displaySetup();   //will put in set up initialize eventually    ///should add something to disaply the first job within the calendar, it's summary as a string to tired
  PICSPISetup();
  clockButtonsSetup();
  setUpInterrupts();

}

void loop()
{
  btn1.loop();
  btn2.loop();

  if(TMRF)        //Snooze checking protocol
  {
    if(snoozeF)
    {
        checkSnooze();
    }
  }

  if (TMRF >= TIME_REQUEST_INTERVAL)  // gets time from PIC every 1 second
  {
    TMRF = 0;
    getTimefromPIC1();
  }

  printNextEvent();  //Prints the string on Event1 global var
}

//Need webserver update task

//need alarm interrupt
