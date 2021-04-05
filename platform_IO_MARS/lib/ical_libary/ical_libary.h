#ifndef ICAL_LIBARY_H
    #define ICAL_LIBARY_H

//FUNCTION DECLARATION SECTION START-----------------------------------------------------------------------------------------------------------------------------------------------
    char *parse_data_line(File *file, const long file_byte_offset);
    /* 
    REQUIRES:
        -A file_byte_offset which is the byte location/offset within the SD card File
        -A SD card class file address which is initialized and opened
    PROMISES:
        -Upon sucess to return a pointer to a heap allocated character array which contains the string or multi-line string that is on the line/lines following 
        the file_byte_offset given
        -Upon failure to return a NULL pointer and to have free any heap space that might of been allocated
            -Possible failures include 
                -Insufficent heap space for allocation of a text buffer
                -Invaild file_byte_offset
                -Bad SD card read
                -End of file before end of a line
        -THREAD SAFE WITH pvPortMalloc() / vPortFree()
    */


    long parse_keyword(File *file, const char *keyword, const long file_byte_offset);
    /* 
    REQUIRES:
        -A file_byte_offset which is the byte location/offset within the SD card File
        -A SD card class file address which is initialized and opened
        -The address to string constant or char array that is NULL TERMINATED and contains the desired keyword to be found
    PROMISES:
        -Upon sucess to return a long containing the file_byte_offset of the first char of the keyword specified that 
        occurs first after the input file_byte_offset
        -Upon failure to return EOF
                -Possible failures include:
                -Invaild file_byte_offset
                -Bad ical file which is not line terminated with CR-LF sequence
                -End of file before occurance of the keyword
        -THREAD SAFE WITH pvPortMalloc() / vPortFree()
    */

//FUNCTION DECLARATION SECTION END-------------------------------------------------------------------------------------------------------------------------------------------------




//CLASS/STRUCT DECLARATION SECTION START-------------------------------------------------------------------------------------------------------------------------------------------
    typedef struct Agenda
    {
        char agenda_name[32];       //The name of the agenda, im not going to use a pointer, like who tf needs a long ass agenda name 
        int timezone_properties;    //Tells us the timezone settings for the entire agenda
        int agenda_event_counter;   //Tells us how many events are present within the Agenda

        CalendarEvent *jobs[4];     //An array of points that must be intialized to point to dynamically allocated heap CalendarEvents

    } UserAgenda;


    typedef struct Event
    {
        int event_properties;       //Encodes properties such as alarm?.....local or UTC......priority....maybe other?
        char event_name[32];       //The event name 
        char event_summary[64];       //The summary of the event, might use description later, but summary is text only, where as description can have html 

        char event_location[32];

        int event_dmy;              //The day, month, year of the event in a certain encoding, can be bitwise accessed for parts
        int event_time[3];          //Event time specified: time-hour, time-minute, time-seconds, can be local or UTC 
        int event_alarm_time;       //The end time of alarm/due date if used; is an offset from the event-time
    } CalendarEvent;
//CLASS/STRUCT DECLARATION SECTION END---------------------------------------------------------------------------------------------------------------------------------------------
#endif  // #ifndef ICAL_LIBARY_H