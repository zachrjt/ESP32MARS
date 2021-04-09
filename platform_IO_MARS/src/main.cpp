#include <Arduino.h>
#include <TFT_eSPI.h>     //TFT display libary
#include <SPI.h>          //SPI Libary
#include <Button2.h>      //Button libary
#include <SD.h>           //SD card library

#include "task_macros.h"            //task marcos for priority, cores, and stack sizes
#include "function_macros.h"        //function macros and general typedefs used for development
#include "Pinout_macros.h"          //pintout macros

#include "peripheral_initialize.h"  //other functions header file
#include "SPI_functions.h"          //SPI functions header file
#include "display_functions.h"

Button2 btn1(BUTTON_1_PIN);                 //creating button object for usage
Button2 btn2(BUTTON_2_PIN);                 //creating button object for usage

SPIClass SDSPI(HSPI); //defines the spi bus for use with the SD card
SPIClass PIC1_SPI;    //defines the spi bus for use with the PIC with clock display
SPIClass PIC2_SPI;    //defines the spi bus for use with the PIC with alarm capabilities

TFT_eSPI tft = TFT_eSPI();  //create tft_display object for usage

String Event1 = "Pass out and sleep";   //Name of the next event

void setup()
{
  Serial.begin(115200);

  if (!setup_functions())
  {
    //record error message
  }
  
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  pinMode(PIC_MISO, INPUT);
  pinMode(PIC1_SPICS, OUTPUT);
  digitalWrite(PIC1_SPICS, HIGH);
  pinMode(PIC2_SPICS, OUTPUT);
  digitalWrite(PIC2_SPICS, HIGH);
  PIC1_SPI.begin(PIC_SPICLK, PIC_MISO, PIC_MOSI, PIC1_SPICS);
  PIC2_SPI.begin(PIC_SPICLK, PIC_MISO, PIC_MOSI, PIC2_SPICS);

  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);
  btn1.setPressedHandler(pressed);
  btn2.setPressedHandler(pressed);
}

void loop()
{
  btn1.loop();
  btn2.loop();
  printNextEvent();
}

//Need webserver update task

//need alarm interrupt
