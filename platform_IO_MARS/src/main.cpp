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

    long keyword_position = find_keyword(&sdcard_calendar, "DESCRIPTION:print ", 0, ICALMODE 0xFF);
    if (keyword_position == EOF)
    {
        Serial.println("Could not find specified keyvalue within the file");
    }
    else
    {
        Serial.println("THE KEYWORD IS AT THIS POS:-------------------------------------------------------------------");
        Serial.println(keyword_position);
        Serial.println("----------------------------------------------------------------------------------------------");

        char *data = parse_data_string(&sdcard_calendar, keyword_position, ICALMODE 0xFF);
        for (int i = 0; data[i] != '\0' ; i++)
        {
            if(data[i] == '\t')//Replacing real text horizontal tabs with \t sequence to better recognize function return strings
            {
                Serial.print('\\');
                Serial.print('t');
            }
            else if(data[i] == ' ')//Replacing real text spaces with underscores to better recognize function return strings
            {
                Serial.print('_');
            }
            else if(data[i] == '\\' && data[1 + i] == 'n' && 0)//Replacing intext newline \n with actual newlines, DISABLED CURRENTLY
            {
                Serial.println();
                i++;
            }
            else
            {
                Serial.print(data[i]);
            }
        }
        Serial.print('\n');
        vPortFree(data);
    }

    //Calendar myCalendar;
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

