#ifndef ICAL_LIBARY_H
    #define ICAL_LIBARY_H




//DEFINE STATEMENT SECTION START---------------------------------------------------------------------------------------------------------------------------------------------------
    #define NEF 9999  //No-more-events is a return long value for when no events could be found that match the timestamp and tolerance given, different than EOF 

    //77 is the maximum number of bytes/characters allowed in a single row of a .ical file, including the CR-LF terminating sequence
    #define MAX_NAME_SIZE 77    //The maximimum array length of a character array containing a name
    #define MAX_SUMMARY_SIZE 77 //The maximimum array length of a character array containing a event summary
    #define MAX_LOCATION_SIZE 77 //The maximimum array length of a character array containing a event summary

    #define ICALMODE    //Just a nice way of indicating if an argument or parameter is not a value but a mode of operation for the intended function
//DEFINE STATEMENT SECTION END-----------------------------------------------------------------------------------------------------------------------------------------------------




//CLASS/STRUCT DECLARATION SECTION START-------------------------------------------------------------------------------------------------------------------------------------------
    typedef struct Event
    {
        int event_properties;       //Encodes properties such as alarm?.....local or UTC......priority....maybe other?
        char event_name[MAX_NAME_SIZE];        //The event name 
        char event_summary[MAX_SUMMARY_SIZE];     //The summary of the event

        char event_location[MAX_LOCATION_SIZE];

        int event_dmy;              //The day, month, year of the event in a certain encoding, can be bitwise accessed for parts
        int event_time[3];          //Event time specified: time-hour, time-minute, time-seconds, can be local or UTC 
        int event_alarm_0time;       //The end time of alarm/due date if used; is an offset from the event-time
    } CalendarEvent;

    typedef struct UserCalendar
    {
        char agenda_name[MAX_NAME_SIZE];       //The name of the calendar,  like who tf needs a long ass calendar name 
        long timezone_properties;    //Tells us the timezone settings for the entire agenda
        byte event_precedence[4];    //The ints within the array related to the indices of the events, this is the order in which the events occur

        CalendarEvent *jobs[4];     //An array of CalendarEvent pointers that point to dynamically allocated or statically allocated CalendarEvent strucs

    } Calendar;
//CLASS/STRUCT DECLARATION SECTION END---------------------------------------------------------------------------------------------------------------------------------------------




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


    long find_keyword(File *file, const char *keyword, const long file_byte_offset, ICALMODE const byte return_offset_mode);
    /* 
    REQUIRES:
        -A file_byte_offset which is the byte location/offset within the SD card File
        -A SD card class file address which is initialized and opened
        -The address to string constant or char array that is NULL TERMINATED and contains the desired keyword to be found
        -A return_offset_mode byte, most common is 0xFF, indicating:
            -0xFF: Mode is start of keyword, means return value byte-offset is the first byte of the sequence of the keyword
            -0x11: Mode is end of keyword, means return value byte-offset is the first byte after the end of the keyword
            -0x00: Mode is next line after keyword, means return value byte-offset is the first byte of the next line after the keyword occurance line
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


    int intialize_calendar(File *file, Calendar *user_calendar);
    /* 
    REQUIRES:
        -A SD card class file address which is initialized and opened
        -A pointer to a created Calendar structure
    PROMISES:
        -To attempt to fill in the Calendar's:
            -Name
            -Timezone id
        -Returns 0 if success, -1 otherwise
    */


   long *find_event(File *file, const long start_offset_byte, const long *read_sector_table, long date, long time, long tolerance);
    /* 
    REQUIRES:
        -A file_byte_offset which is the starting point to look for events
        -An address to a read_sector_table which contains revelant byte offset ranges to read from
        -A SD card class file address which is initialized and opened
        -A long-utc-date for which the event should start on/after 
        -A long-utc-time for which the event should start on/after within the provided tolerance after
        -A long tolerance which is for how long after this long time and date, in hours, the event should be within
    PROMISES:
        -Upon success to return a pointer to a 2-long array containing:
            =1)The file_byte_offset of the event's BEGIN:VEVENT that is within the specified time range
            -2)The first byte of the line after the event's END:VEVENT of the event specified
        -Upon failure to return:
            -1)EOF in both the 1st and 2nd element of the array if failure
            =2)NEF in both the 1st and 2nd element if no event could be found with the specified arguments
    */
//FUNCTION DECLARATION SECTION END-------------------------------------------------------------------------------------------------------------------------------------------------




#endif  // #ifndef ICAL_LIBARY_H