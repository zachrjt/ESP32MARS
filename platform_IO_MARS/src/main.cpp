//INCLUDE SECTION START------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <SPI.h>          //SPI Libary
#include <Button2.h>      //Button Libary
#include <SD.h>           //SD Card Libary

#include <TFT_eSPI.h>               //TTgo TFT Display Libary
#include <ical_libary.h>            //Icalendar format file libary

#include "task_macros.h"            //task marcos for priority, cores, and stack sizes
#include "function_macros.h"        //function macros and general typedefs used for development 
#include "peripheral_initialize.h"  //other functions header file
//INCLUDE SECTION END--------------------------------------------------------------------------------------------------------------------------------------------------------------




//PERIPHERAL GPIO SETUP SECTION START----------------------------------------------------------------------------------------------------------------------------------------------
#define BUTTON_1_PIN        35
#define BUTTON_2_PIN        0

TFT_eSPI tft_display = TFT_eSPI(135, 240);  //creating display object for usage
Button2 btn1(BUTTON_1_PIN);                 //creating button object for usage
Button2 btn2(BUTTON_2_PIN);                 //creating button object for usage

SPIClass SDSPI(HSPI); //defines the spi bus for use with the SD card

#define SD_MOSI 26
#define SD_MISO 27
#define SD_SPICLK 25
#define SD_SPICS 33
//PERIPHERAL GPIO SETUP SECTION END------------------------------------------------------------------------------------------------------------------------------------------------


long DATESTAMP = 20210404;  //The current year, month, day
long TIMESTAMP = 010000;    //The current time: hours (24 hours) minutes, seconds

//SETUP FUNCTION START-------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
    if (!setup_functions())
    {
        //record error message
    }
    Serial.begin(115200);
    while (!Serial)
    {
        delay(100); //waiting for serial connection
    }
    delay(500);
    Serial.println("Serial connection sucessful....");

    Serial.println("Connecting to SD card");
    SDSPI.begin(SD_SPICLK, SD_MISO, SD_MOSI, SD_SPICS);
    if (!SD.begin(SD_SPICS,  SDSPI))
    {
        Serial.println("Could not mount SD card!");
        return;
    }
    Serial.println("Mounted SD card");

    File sdcard_calendar = SD.open("/ical_simple.ics");
    if (!sdcard_calendar)
    {
        Serial.println("Could not open file");
        return;
    }
    //6.5858 seconds to go through 150000 lines/ 600,730 byte
    long keyword_position = find_next_keyword(&sdcard_calendar, "-//D2L//NONSGML ", 0, ICALMODERTN 0x00);
    /*-A return_offset_mode byte, most common is 0xFF, indicating:
            -0xFF: Mode is start of keyword, means return value byte-offset is the first byte of the sequence of the keyword
            -0x11: Mode is end of keyword, means return value byte-offset is the first byte after the end of the keyword
            -0x00: Mode is next line after keyword, means return value byte-offset is the first byte of the next line after the keyword occurance line
    */
    //0.27 seconds to find a previous keyword 
    keyword_position = find_previous_keyword(&sdcard_calendar, "BEGIN", keyword_position, ICALMODERTN 0xFF);
    /*-A return_offset_mode byte, most common is 0xFF, indicating:
            -0xFF: Mode is start of keyword, means return value byte-offset is the first byte of the sequence of the keyword
            -0x11: Mode is end of keyword, means return value byte-offset is the first byte after the end of the keyword
            -0x00: Mode is next line after keyword, means return value byte-offset is the first byte of the next line after the keyword occurance line
    */
    if (keyword_position == EOF)
    {
        Serial.println("An error was encountered");
    }
    else
    {
        char *data = parse_data_string(&sdcard_calendar, keyword_position, ICALMODERTN 0xFF);
        /*
        -A return_string_mode byte, most common is 0xFF indicating:
            -0xFF: Mode is till end of string so it can return multi-line strings
            -0x00: Mode is till end of the line (CR-LF sequence) 
        */
        calendar_str_print(data);  //printing the string
        Serial.print('\n');
        vPortFree(data);
    }
    Calendar myCalendar;
    if(!intialize_calendar(&sdcard_calendar, &myCalendar))
    {
        calendar_str_print(myCalendar.agenda_name);  //printing the string
    }
    Serial.println("ENDING SERIAL CONNECTION");
    sdcard_calendar.close();
    SD.end();
}
//SETUP FUNCTION END---------------------------------------------------------------------------------------------------------------------------------------------------------------




//LOOP FUNCTION START--------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
    delay(10000);
}
//LOOP FUNCTION END----------------------------------------------------------------------------------------------------------------------------------------------------------------

