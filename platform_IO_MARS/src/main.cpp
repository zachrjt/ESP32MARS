#include <Arduino.h>
#include <TFT_eSPI.h>     //TFT display libary
#include <SPI.h>          //SPI Libary
#include <Button2.h>      //Button libary

#include "task_macros.h"            //task marcos for priority, cores, and stack sizes
#include "function_macros.h"        //function macros and general typedefs used for development 

#include "peripheral_initialize.h"  //other functions header file

#define BUTTON_1_PIN        35
#define BUTTON_2_PIN        0

void getTimefromPIC(void);

SPIClass PICSPI;
SPISettings PICSPISettings =  {
                              5000000,
                              MSBFIRST,
                              SPI_MODE0
                              };

uint8_t Time0 = 77;
uint8_t Time1 = 32;
uint8_t Time2 = 64;

#define MOSI 12
#define MISO 13
#define SPICLK 15
#define SPICS 17

TFT_eSPI tft_display = TFT_eSPI(135, 240);  //creating display object for usage
Button2 btn1(BUTTON_1_PIN);                 //creating button object for usage
Button2 btn2(BUTTON_2_PIN);                 //creating button object for usage


void setup()
{
  if (!setup_functions())
  {
    //record error message
  }
  PICSPI.begin(SPICLK, MISO, MOSI, SPICS);
}

void loop()
{
  delay(10000);
  getTimefromPIC();
}

//Need websever update task

//need alarm interrupt
//need button interrupts for features


void getTimefromPIC(void)
{
  PICSPI.beginTransaction(PICSPISettings);
  Time0 = PICSPI.transfer(Time0);
  Serial.print(Time0);
  Time1 = PICSPI.transfer(Time1);
  Serial.print(Time1);
  Time2 = PICSPI.transfer(Time2);
  Serial.print(Time2);
  PICSPI.endTransaction();
}