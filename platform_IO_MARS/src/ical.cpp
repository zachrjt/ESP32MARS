#include "ical.h"

extern SPIClass SDSPI; //defines the spi bus for use with the SD card

String weather_description;  //Pulls the weather description
String weather_value;  //Pulls the weather value like -4

Calendar myCalendar;

//below are utc time stamps that the ical libary uses, there is no updating function within thats left up to the user/zach tom.

int DATESTAMP = 20210412;
String DATEREQUEST;

int TIMESTAMP = 181500; //730am local?
String TIMEREQUEST;

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



    //WiFi setup-------------------------------------------------------------------------------------------------------------------------------------------------------------------

    WifiUpdate();

    /*WiFi.begin(NETWORKNAME, NETWORKPASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        if(Serial)
        {
            Serial.println("Connecting to WiFi network");
        }
    }
    if(Serial)
    {
        Serial.println("Connected to WiFi network!!!");
    }

    HTTPClient webserver;

    webserver.begin("http://50.66.76.111/Calgary");//Set up request to webserver host by zach
    int HTTPcode = webserver.GET();//send http request and take the return code

    if(0 < HTTPcode)    //ie no error
    {
        String request_data = webserver.getString();//grab content
        if(Serial)
        {
            Serial.println(HTTPcode);
            Serial.println(request_data);
        }
        int pattern_index = 0;
        char pattern_string[] = "<hi>Temperature ";
        int end_of_pattern = 0;
        for(int i = 0; request_data[i] != '\0'; i++)
        {
            if(request_data[i] == pattern_string[pattern_index])
            {
                pattern_index++;
                if(pattern_index == 15)
                {
                    end_of_pattern = i;
                }
            }
            else
            {
                pattern_index = 0;
            }
        }
        if(end_of_pattern != 0)
        {
           weather_description = request_data.substring((end_of_pattern+1), (end_of_pattern+11));
        }
        int end_of_usefull_string = 0;
        for(int i = 0; weather_description[i] != '<'; i++)
        {
            end_of_usefull_string++;
        }
        
        weather_description = weather_description.substring(0, end_of_usefull_string);
        if(Serial)
        {
            Serial.print("The weather is: ");
            Serial.println(weather_description);
        }

        pattern_index = 0;
        char pattern_string2[] = "<hi>UTCTIME:";
        end_of_pattern = 0;
        for(int i = 0; request_data[i] != '\0'; i++)
        {
            if(request_data[i] == pattern_string2[pattern_index])
            {
                pattern_index++;
                if(pattern_index == 12)
                {
                    end_of_pattern = i;
                }
            }
            else
            {
                pattern_index = 0;
            }
        }
        if(end_of_pattern != 0)
        {
           DATEREQUEST = (request_data.substring((end_of_pattern+1), (end_of_pattern+9)));
           TIMEREQUEST = (request_data.substring((end_of_pattern+10), (end_of_pattern+16)));
           DATESTAMP = DATEREQUEST.toInt();
           TIMESTAMP = TIMEREQUEST.toInt();
           sendTimetoPIC1();
        }
        if(Serial)
        {
            Serial.print("The time is: ");
            Serial.println(DATEREQUEST + " " + TIMEREQUEST);
        }

    }
    else
    {
        if(Serial)
        {
            Serial.println("ERROR while sending HTTP request:");
            Serial.println(HTTPcode);
        }
    }

    WiFi.disconnect();*/  //disconnect from WiFi

}

void WifiUpdate(void)
{
    WiFi.begin(NETWORKNAME, NETWORKPASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        if(Serial)
        {
            Serial.println("Connecting to WiFi network");
        }
    }
    if(Serial)
    {
        Serial.println("Connected to WiFi network!!!");
    }

    HTTPClient webserver;

    webserver.begin("http://50.66.76.111/Calgary");//Set up request to webserver host by zach
    int HTTPcode = webserver.GET();//send http request and take the return code

    if(0 < HTTPcode)    //ie no error
    {
        String request_data = webserver.getString();//grab content
        if(Serial)
        {
            Serial.println(HTTPcode);
            Serial.println(request_data);
        }
        int pattern_index = 0;
        char pattern_string[] = "<hi>Temperature ";
        int end_of_pattern = 0;
        for(int i = 0; request_data[i] != '\0'; i++)
        {
            if(request_data[i] == pattern_string[pattern_index])
            {
                pattern_index++;
                if(pattern_index == 15)
                {
                    end_of_pattern = i;
                }
            }
            else
            {
                pattern_index = 0;
            }
        }
        if(end_of_pattern != 0)
        {
           weather_description = request_data.substring((end_of_pattern+1), (end_of_pattern+11));
        }
        int end_of_usefull_string = 0;
        for(int i = 0; weather_description[i] != '<'; i++)
        {
            end_of_usefull_string++;
        }
        
        weather_description = weather_description.substring(0, end_of_usefull_string);
        if(Serial)
        {
            Serial.print("The weather is: ");
            Serial.println(weather_description);
        }

        pattern_index = 0;
        char pattern_string2[] = "<hi>UTCTIME:";
        end_of_pattern = 0;
        for(int i = 0; request_data[i] != '\0'; i++)
        {
            if(request_data[i] == pattern_string2[pattern_index])
            {
                pattern_index++;
                if(pattern_index == 12)
                {
                    end_of_pattern = i;
                }
            }
            else
            {
                pattern_index = 0;
            }
        }
        if(end_of_pattern != 0)
        {
           DATEREQUEST = (request_data.substring((end_of_pattern+1), (end_of_pattern+9)));
           TIMEREQUEST = (request_data.substring((end_of_pattern+10), (end_of_pattern+16)));
           DATESTAMP = DATEREQUEST.toInt();
           TIMESTAMP = TIMEREQUEST.toInt();
           sendTimetoPIC1();
        }
        if(Serial)
        {
            Serial.print("The time is: ");
            Serial.println(DATEREQUEST + " " + TIMEREQUEST);
        }

    }
    else
    {
        if(Serial)
        {
            Serial.println("ERROR while sending HTTP request:");
            Serial.println(HTTPcode);
        }
    }

    WiFi.disconnect();  //disconnect from WiFi

}