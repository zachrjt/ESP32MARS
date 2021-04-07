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

    File sdcard_calendar = SD.open("/uofc_calendar.ics");
    if (!sdcard_calendar)
    {
        Serial.println("Could not open file");
        return;
    }
    //6.5858 seconds to go through 150000 lines/ 600,730 byte
    long keyword_position = find_next_keyword(&sdcard_calendar, "6606-57709@d2l.ucalgary.ca",ICALOFFSET 0, ICALOFFSET -1, ICALMODERTN NEXTLINE);

    //0.27 seconds to find a previous keyword 
    keyword_position = find_previous_keyword(&sdcard_calendar, "BEGIN:VEVENT", ICALOFFSET keyword_position, ICALOFFSET -1, ICALMODERTN FIRSTCHAR);

    if (keyword_position == EOF)
    {
        Serial.println("An error was encountered");
    }
    else
    {
        char *data = parse_data_string(&sdcard_calendar, keyword_position, ICALOFFSET NOEND, ICALMODERTN MULTILINE);
        calendar_str_print(data);  //printing the string
        vPortFree(data);
    }
    Calendar myCalendar;
    if(!initialize_calendar(&sdcard_calendar, &myCalendar))
    {
        calendar_str_print(myCalendar.agenda_name);
        calendar_str_print(myCalendar.timezone.time_zone_id);

        calendar_str_print(myCalendar.timezone.daylight_time_zone);
        Serial.println(myCalendar.timezone.daylight_offset);

        calendar_str_print(myCalendar.timezone.standard_time_zone);
        Serial.println(myCalendar.timezone.standard_offset);
    }
    CalendarEvent myEvent;

    if(!initialize_event(&sdcard_calendar, &myEvent, keyword_position))
    {
        calendar_str_print(myEvent.event_summary);
        calendar_str_print(myEvent.event_location);

        calendar_str_print(myEvent.event_time_zone_id);

        Serial.println(myEvent.event_start_date_code);
        Serial.print("The event is on: ");
        Serial.print(myEvent.event_start_year);
        Serial.print("/");
        Serial.print(myEvent.event_start_month);
        Serial.print("/");
        Serial.println(myEvent.event_start_day);
        
        Serial.println(myEvent.event_start_time_code);
        Serial.print("At time: ");
        Serial.print(myEvent.event_start_hour);
        Serial.print(":");
        Serial.print(myEvent.event_start_minute);
        Serial.print(".");
        Serial.println(myEvent.event_start_minute);
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

