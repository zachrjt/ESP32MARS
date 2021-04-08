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


int DATESTAMP = 20210407;  //The current year, month, day
int TIMESTAMP = 202930;    //The current time: hours (24 hours) minutes, seconds

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
    long keyword_position = 0;
    keyword_position = find_next_keyword(&sdcard_calendar, "6606-700320@d2l.ucalgary.ca", 0, NOEND, FIRSTCHAR);

    keyword_position = find_previous_keyword(&sdcard_calendar, "BEGIN:VEVENT", keyword_position, NOEND, FIRSTCHAR);


    Calendar myCalendar;
    if(!initialize_calendar(&sdcard_calendar, &myCalendar))
    {
        print_calendar(&myCalendar);
        myCalendar.timezone.daylight_mode = 1;//We are in daylight savings time
    }
    CalendarEvent myEvent;

    if(!initialize_event(&sdcard_calendar, &myCalendar, &myEvent, keyword_position))
    {
        print_event(&myEvent);
    }

    Serial.println();
    long sector_table[SECTORTABLESIZE];
    if(!initialize_sector_table(&sdcard_calendar, &myCalendar, 20210407, 141610, sector_table))
    {
        
        Serial.println("Sector Table: ");
        for(int i = 0; i < SECTORTABLESIZE; i++)
        {
            Serial.print("\t");
            Serial.println(sector_table[i], HEX);
        }
        
    }
    else
    {
        Serial.println("\nERROR WHILE INITIALIZING SECTOR TABLE\n");
    }

    /*
    for(int i = 0; i < EVENTSTACKSIZE; i++)
    {
        myCalendar.jobs[i] = (CalendarEvent *)(pvPortMalloc(sizeof(CalendarEvent)));
        long next_event = 0;
        if(!find_event(&sdcard_calendar, &myCalendar, sector_table, &next_event, DATESTAMP, TIMESTAMP))
        {
            if(!initialize_event(&sdcard_calendar, &myCalendar, myCalendar.jobs[i], next_event))
            {
                myCalendar.event_intialization = 1;//We have initializated an event within the calendar
                myCalendar.event_precedence[i] = i;
                print_event(myCalendar.jobs[i]);
                continue;//All good move onto next event
            }
            else
            {
                Serial.println("Could not initlize an event");
            }
        }
        else
        {
            Serial.println("Could not find an event");
        }
    }
    */
    long next_event = 0;
    CalendarEvent my_other_event;
    
    if(!find_event(&sdcard_calendar, &myCalendar, sector_table, &next_event, DATESTAMP, TIMESTAMP))
    {
        if(!initialize_event(&sdcard_calendar, &myCalendar, &my_other_event, next_event))
        {
            print_event(&my_other_event);
        }
        else
        {
            Serial.println("Error while trying to intialize event");
        }
    }
    else
    {
        Serial.println("Error finding event");
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

