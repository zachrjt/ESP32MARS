#ifndef ICAL_LIBARY_H
    #define ICAL_LIBARY_H




//DEFINE STATEMENT SECTION START---------------------------------------------------------------------------------------------------------------------------------------------------
    #define PRINTOUTDEBUG  //Used so the printing function will replace whitespace with identifiers to make string output debug easier, comment out for no debug, 

    #define NEF 9999  //No-more-events is a return long value for when no events could be found that match the timestamp and tolerance given, different than EOF 

    //77 is the maximum number of bytes/characters allowed in a single row of a .ical file, including the CR-LF terminating sequence
    #define MAX_NAME_SIZE 75    //The maximimum array length of a character array containing a name
    #define MAX_SUMMARY_SIZE 75 //The maximimum array length of a character array containing a event summary
    #define MAX_LOCATION_SIZE 75 //The maximimum array length of a character array containing a location name
    #define MAX_TZ_SIZE 35  //The maximum array length of a character array containing time-zone property strings, dont need to be as long

    #define ICALMODERTN    //Just a nice way of indicating if an argument or parameter is mode of return value
    #define ICALMODEOPR    //Just a nice way of indicating if an argument or parameter is mode of operation to find the return value
    #define ICALOFFSET     //Just a nice way of indicating if an argument or parameter is a byte offset


    #define NEXTLINE 0x00   //blah blah blah read below for what the modes mean
    #define FIRSTCHAR 0xFF
    #define NEXTCHAR 0x11

    #define ONELINE 0x00
    #define MULTILINE 0xFF
//DEFINE STATEMENT SECTION END-----------------------------------------------------------------------------------------------------------------------------------------------------




//CLASS/STRUCT DECLARATION SECTION START-------------------------------------------------------------------------------------------------------------------------------------------
    typedef struct Event
    {
        //Sequence of events isn't consider, simply the order inwhich they are present within the .ical file is used for sequence
        //Repeated/reoccuring events are implemented, perhaps if there is enough time
        char event_summary[MAX_SUMMARY_SIZE];   //The summary of the event

        char event_location[MAX_LOCATION_SIZE]; //The location of the event

        byte date_format;           //If 1 then alternate string date used, 0 normally
        int event_start_date;       //Start date of event
        int event_start_time;       //Start time, on the date, of the event

        int event_end_date;         //End date of event, typically same day as start
        int event_end_time;         //End time, on date of end, of the event

        int event_alarm_day;       //The alarm time of alarm/due date if used, otherwise begining of event
        int event_alarm_time;      //The end time of alarm/due date if used, otherwise begining of event
    } CalendarEvent;

    typedef struct TimeZoneProperties
    {
        char time_zone_id[MAX_NAME_SIZE];       //contains the time-zone id of the calendar, no standard used but, some server-side libaries exist for use of this

        byte daylight_status;                   //If 1 then daylight saving occurs, if 0 no daylight savings
        char daylight_time_zone [MAX_TZ_SIZE];  //Time zone name (MST, PST, etc) of the daylight savings timezone
        int daylight_offset;                    //The numerical time offset, from UTC time, that daylight savings is in -0700 would be 7 hours behind utc
        char standard_time_zone [MAX_TZ_SIZE];  //Time zone name (MST, MDT, etc) of the non-daylight savings timezone
        int standard_offset;                    //The numerical time offset, from UTC time, that standard time is in

        //In the actual produc there should be more info like when daylight savings happens but parsing and interpreting this other info is tediuous
        //Instead it's easier to upload this info to a server and have it grab some time, so this may or may not be implemented in the prototype
        //Soooo, an excercise left to the reader: Try parsing "RRULE:FREQ=YEARLY;BYMONTH=11;BYDAY=1SU" in a function that can accurately change the time for daylight savings
    } TimeZoneInfo;

    typedef struct UserCalendar
    {
        char agenda_name[MAX_NAME_SIZE];        //The name of the calendar,  like who tf needs a long ass calendar name 
        TimeZoneInfo timezone;                  //A struc containing the timezone information about the Calendar
        byte event_precedence[4];               //The ints within the array related to the indices of the events, this is the order in which the events occur
        CalendarEvent *jobs[4];                 //An array of CalendarEvent pointers that point to dynamically allocated or statically allocated CalendarEvent strucs

    } Calendar;
//CLASS/STRUCT DECLARATION SECTION END---------------------------------------------------------------------------------------------------------------------------------------------




//FUNCTION DECLARATION SECTION START-----------------------------------------------------------------------------------------------------------------------------------------------
    char *parse_data_string(File *file, const long file_byte_offset, ICALMODERTN const byte return_string_mode);
    /* 
    REQUIRES:
        -A file_byte_offset which is the byte location/offset within the SD card File
        -A SD card class file address which is initialized and opened
        -A return_string_mode byte, most common is 0xFF indicating:
            -0xFF: Mode is till end of string so it can return multi-line strings
            -0x00: Mode is till end of the line (CR-LF sequence) 
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


    long find_next_keyword(File *file, const char *keyword, const long file_byte_offset, long max_byte_offset, ICALMODERTN const byte return_offset_mode);
    /* 
    REQUIRES:
        -A file_byte_offset which is the byte location/offset within the SD card File
        -A max_byte_offset which is the maximum file byte offset to be searched up to, 
            -PASS -1 IF NO MAX OFFSET DESIRED
        -A SD card class file address which is initialized and opened
        -The address to string constant or char array that is NULL TERMINATED and contains the desired keyword to be found
        -A return_offset_mode byte, most common is 0xFF, indicating:
            -0xFF: Mode is start of keyword, means return value byte-offset is the first byte of the sequence of the keyword
            -0x11: Mode is end of keyword, means return value byte-offset is the first byte after the end of the keyword
            -0x00: Mode is next line after keyword, means return value byte-offset is the first byte of the next line after the keyword occurance line
    PROMISES:
        -Upon sucess to return a long containing the file_byte_offset of the first char of the keyword specified that 
        occurs first after the input file_byte_offset
        -Upon error to return EOF
                -Possible failures include:
                -Invaild file_byte_offset
                -End of file before occurance of the keyword
        -Upon failure to find keyword with the max_byte_offset
            =Return -2
    */


    long find_previous_keyword(File *file, const char *keyword, const long file_byte_offset, long min_byte_offset, ICALMODERTN const byte return_offset_mode);
    /* 
    REQUIRES:
        -A file_byte_offset which is the byte location/offset within the SD card File that the function will read backwards from
        -A min_byte_offset which is the minimum file byte offset to be searched back up to, 
            -PASS -1 IF NO MIN OFFSET DESIRED
        -A SD card class file address which is initialized and opened
        -The address to string constant or char array that is NULL TERMINATED and contains the desired keyword to be found
        -A return_offset_mode byte, most common is 0xFF, indicating:
            -0xFF: Mode is start of keyword, means return value byte-offset is the first byte of the sequence of the keyword
            -0x11: Mode is end of keyword, means return value byte-offset is the first byte after the end of the keyword
            -0x00: Mode is next line after keyword, means return value byte-offset is the first byte of the next line after the keyword occurance line
    PROMISES:
        -Upon sucess to return a long containing the file_byte_offset of byte per the specified mode
        -Upon Error to return EOF
                -Possible failures include:
                -Invaild file_byte_offset
                -Begining of file before occurance of the keyword
        -Upon failure to find keyword after the min_byte_offset
            =Return -2
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
            -1)The file_byte_offset of the event's BEGIN:VEVENT that is within the specified time range
            -2)The first byte of the line after the event's END:VEVENT of the event specified
        -Upon failure to return:
            -1)EOF in both the 1st and 2nd element of the array if failure
            -2)NEF in both the 1st and 2nd element if no event could be found with the specified arguments
    */


    byte initialize_calendar(File *file, Calendar *user_calendar);
    /* 
    REQUIRES:
        -A SD card class file address which is initialized and opened
        -A pointer to a created Calendar structure
    PROMISES:
        -To attempt to fill in the Calendar's:
            -Name
            -Timezone id
            -Day light savings crap
        -Returns 0 if success, -1 otherwise
        -THREAD SAFE WITH pvPortMalloc() / vPortFree(), (used internally with the function)
    */


    byte initialize_event(File *file, CalendarEvent *user_event, ICALOFFSET const long event_byte_offset);
    /* 
    REQUIRES:
        -A SD card class file address which is initialized and opened
        -A pointer to a created CalendarEvent
        -An byte-offset value for which the event begins at: "BEGIN:VEVENT" within the SD card file
    PROMISES:
        -To attempt to fill in the CalendarEvent's
            -Summary
            -Location
            -Event start info
            -Event end info
            -Event alarm info
        -Returns 0 if success, -1 otherwise
        -THREAD SAFE WITH pvPortMalloc() / vPortFree(), (used internally with the function)
    */


   void calendar_str_print(const char * char_array);
    /* 
    REQUIRES:
        -A pointer to a charcter array THAT IS NULL TERMINATED
        -A Serial port to be open, a check is performed however
    PROMISES:
        -To attempt to serial print the content within the character array up to the termination Null char
        -DEBUGMODE define will affect output
    */


   void calendar_str_to_int(const char * str_num, int num_length, int *int_pointer);
    /* 
    REQUIRES:
        -A pointer to a charcter array that is numerical, a NO CHECK IS PERFORMED
        -A length of the number that starts at the address passed as a pointer
        -IF num_length is -1 then the string will be read until it's nulltermination
        -A pointer to an int to fill
    PROMISES:
        -To attempt to convert the string within the character array to a base 10 int
    */


   void calendar_char_copy(const char * str, char *dest);
    /* 
    REQUIRES:
        -A pointer to a character array that is to be modified(dest), of large enough size to fit the intended string
        -A static string (str) to use
    PROMISES:
        -To fill the dest chararcter array with the provided string,
        -To Null terminate the string
    */
//FUNCTION DECLARATION SECTION END-------------------------------------------------------------------------------------------------------------------------------------------------




#endif  // #ifndef ICAL_LIBARY_H