#include <Arduino.h>
#include <TFT_eSPI.h>     //TFT display libary
#include <SPI.h>          //SPI Libary
#include <Button2.h>      //Button libary
#include <SD.h>           //SD card library

#include "task_macros.h"            //task marcos for priority, cores, and stack sizes
#include "function_macros.h"        //function macros and general typedefs used for development 

#include "peripheral_initialize.h"  //other functions header file

#define BUTTON_1_PIN        36
#define BUTTON_2_PIN        37

#define SD_MOSI 25
#define SD_MISO 33
#define SD_SPICLK 32
#define SD_SPICS 26

#define PIC_MOSI 12
#define PIC_MISO 13
#define PIC_SPICLK 15
#define PIC1_SPICS 17
#define PIC2_SPICS 22

void sendTimetoPIC1(void);        //Send time to PIC
void getTimefromPIC1(void);      //Get time from PIC
void sendAlarmFlagtoPIC2(void);   //If Alarm then alarm

void pressed(Button2& btn);       //Button Pressed event

SPIClass SDSPI(HSPI); //defines the spi bus for use with the SD card
SPIClass PIC1_SPI;    //defines the spi bus for use with the PIC with clock display
SPIClass PIC2_SPI;    //defines the spi bus for use with the PIC with alarm capabilities

SPISettings PICSPISettings =  {
                              500000, 
                              MSBFIRST,
                              SPI_MODE0
                              };

TFT_eSPI tft_display = TFT_eSPI(135, 240);  //creating display object for usage
Button2 btn1(BUTTON_1_PIN);                 //creating button object for usage
Button2 btn2(BUTTON_2_PIN);                 //creating button object for usage

uint8_t Time0 = 76;     //Times in PIC display format
uint8_t Time1 = 32;
uint8_t Time2 = 64;

uint8_t Time0R = 76;     //Used for debugging GetTimeFromPIC
uint8_t Time1R = 32;
uint8_t Time2R = 64;

uint8_t AlarmFlag = 0;

void setup()
{
  Serial.begin(115200);

  if (!setup_functions())
  {
    //record error message
  }

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
  //sendTimetoPIC1();
  sendAlarmFlagtoPIC2();
}

void sendTimetoPIC1(void)
{
  digitalWrite(PIC1_SPICS, LOW);
  PIC1_SPI.beginTransaction(PICSPISettings);

  Time0R = PIC1_SPI.transfer(Time0);
  Time1R = PIC1_SPI.transfer(Time1);
  Time2R = PIC1_SPI.transfer(Time2);

  Serial.println(Time0R);
  Serial.println(Time1R);
  Serial.println(Time2R);

  PIC1_SPI.endTransaction();
  digitalWrite(PIC1_SPICS, HIGH);
}

void  getTimefromPIC1(void)
{
  digitalWrite(PIC1_SPICS, LOW);
  PIC1_SPI.beginTransaction(PICSPISettings);

  //PIC1_SPI.transfer(0);

  //delay(1000);

  Time0R = PIC1_SPI.transfer(1);
  Time1R = PIC1_SPI.transfer(1);
  Time2R = PIC1_SPI.transfer(1);

  Serial.println(Time0R);
  Serial.println(Time1R);
  Serial.println(Time2R);

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

      //Set up new Alarm event 5 minutes from now here
      sendAlarmFlagtoPIC2();
      delay(1000);
      AlarmFlag = 1;
      sendAlarmFlagtoPIC2();
    }
    else      ///This is just for debugging
    {
      AlarmFlag = 1;
      sendTimetoPIC1();
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

    getTimefromPIC1();
  }
}

//Need websever update task

//need alarm interrupt