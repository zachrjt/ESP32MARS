#include "ical.h"
#include <Arduino.h>
#include <SPI.h>          //SPI Libary
#include <SD.h>           //SD Card Libary
#include <ical_libary.h>    //actual libary

extern SPIClass SDSPI; //defines the spi bus for use with the SD card

Calendar myCalendar;

//below are utc time stamps that the ical libary uses, there is no updating function within thats left up to the user/zach tom.

int DATESTAMP = 20210409;

int TIMESTAMP = 133000; //730am local?

long sector_table[SECTORTABLESIZE];//this must remain allocated so that SD card can be reopened later if needed

void icalLibarySetup()
{
    SDSPI.begin(SD_SPICLK, SD_MISO, SD_MOSI, SD_SPICS);//begin sdspi connecition with sd card
    if (!SD.begin(SD_SPICS,  SDSPI))
    {
        if(Serial)
        {
            Serial.println("Could not mount SD card!");
            Serial.println("Try re-inserting the SD card and hitting the side reset button");
        }
        return;
    }
    if(Serial)
    {
        Serial.println("Mounted SD card");
    }

    File sdcard_calendar = SD.open("/ils_calendar.ics");
    if (!sdcard_calendar)
    {
        if(Serial)
        {
            Serial.println("Could not open file, make sure you have the ils_calendar.ical file");
        }
        return;
    }

    if(!initialize_calendar(&sdcard_calendar, &myCalendar))
    {
        if(Serial)
        {
            print_calendar(&myCalendar);
        }
        myCalendar.timezone.daylight_mode = 1;//We are in daylight savings time
    }
    
    if(!initialize_sector_table(&sdcard_calendar, &myCalendar, DATESTAMP, TIMESTAMP, sector_table))
    {
        if(Serial)
        {
            Serial.println("Sector Table: ");
            for(int i = 0; i < SECTORTABLESIZE && sector_table[i]!=0 ; i++)
            {
                Serial.print("\t");
                Serial.println(sector_table[i], HEX);
            }
        }
    }
    else
    {
        if(Serial)
        {
            Serial.println("\nERROR WHILE INITIALIZING SECTOR TABLE\n");
        }
    }
    //adding events onto calendar, 8 of them
    for(int i = 0; i < EVENTSTACKSIZE; i++)
    {
        myCalendar.jobs[i] = (CalendarEvent *)(pvPortMalloc(sizeof(CalendarEvent)));
        long next_event = 0;
        if(!find_event(&sdcard_calendar, &myCalendar, sector_table, &next_event, DATESTAMP, TIMESTAMP, 0xFF))
        {
            if(!initialize_event(&sdcard_calendar, &myCalendar, myCalendar.jobs[i], next_event))
            {
                myCalendar.event_intialization = 1;//We have initializated an event within the calendar
                myCalendar.event_precedence[i] = i;
                if(Serial)//prob dont need to check serial for print function in ical libary but im too tired to check
                {
                    print_event(myCalendar.jobs[i]);
                }
            }
            else
            {
                if (Serial)
                {
                    Serial.println("Could not initlize an event");
                }
            }
        }
        else
        {
            if(Serial)
            {
                Serial.println("Could not find an event");
            }
        }
    }
    sdcard_calendar.close();//closing serial, update cant be made but sector
    SD.end();

}