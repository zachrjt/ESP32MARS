#ifndef ICAL_LIBARY_H
    #define ICAL_LIBARY_H
    //https://www.youtube.com/watch?v=y8OnoxKotPQ is best way I can sum up the .ical libary 




//DEFINE STATEMENT SECTION START---------------------------------------------------------------------------------------------------------------------------------------------------
    //#define PRINTOUTDEBUG          //Used so the printing function will replace whitespace with identifiers to make string output debug easier, comment out for no debug, 

    #define NEF 9999                //No-more-events is a return long value for when no events could be found that match the timestamp and tolerance given, different than EOF 

    #define MAX_NAME_SIZE 77        //The maximimum array length of a character array containing a name
    #define MAX_SUMMARY_SIZE 154    //The maximimum array length of a character array containing a event summary
    #define MAX_LOCATION_SIZE 154   //The maximimum array length of a character array containing a location name
    #define MAX_TZ_SIZE 35          //The maximum array length of a character array containing time-zone property strings, dont need to be as long

    #define ICALMODERTN             //Just a nice way of indicating if an argument or parameter is mode of return value
    #define ICALMODEOPR             //Just a nice way of indicating if an argument or parameter is mode of operation to find the return value
    #define ICALOFFSET              //Just a nice way of indicating if an argument or parameter is a byte offset
    
    #define EVENTSTACKSIZE  8       //The number of events being managed by the calendar at any given time
    #define SECTORTABLESIZE 512     //The number of elements within the sector_table, # of pairs = 1/2 of this number
    
    #define NEXTLINE 0x00           //Used for the keyword finding functions to return the byteoffset of the first char of the next line after the keyword
    #define FIRSTCHAR 0xFF          //Used for the keyword finding functions to return the byteoffset of the first char of the keyword
    #define NEXTCHAR 0x11           //Used for the keyword finding functions to return the byteoffset of the next char after the keyword's last char

    #define ONELINE 0x00            //Used for the parse_data function to allow the parse function to only return a oneline string
    #define MULTILINE 0xFF          //Used for the parse_data function to allow the parse function to return a multi-line string, (no newlines added)

    #define NOEND -1                //Used for the parse_data function to indicate that a parse function will have no limit how far it searches in the file
//DEFINE STATEMENT SECTION END-----------------------------------------------------------------------------------------------------------------------------------------------------




//CLASS/STRUCT DECLARATION SECTION START-------------------------------------------------------------------------------------------------------------------------------------------
    typedef struct Event
    {
        //Sequence of events isn't considered, simply the order inwhich they are present within the .ical file is used for sequence, re-occuring events are not implemented

        char event_summary[MAX_SUMMARY_SIZE];   //The summary of the event

        char event_location[MAX_LOCATION_SIZE]; //The location of the event

        long start_byte_offset;     //The byte offset of the event's BEGIN:VEVENT
        long end_byte_offset;       //The byte offset of first char of the line following the event's END:VEVENT

        int event_start_date_code;  //Start date of event in utc parts: 20210406 (used for internal comparison)
        int event_start_year;       //Start date year
        int event_start_month;      //Start date month
        int event_start_day;        //Start date day

        int event_start_time_code;  //Start time of event in utc parts: 032743 (used for internal comparison)
        int event_start_hour;       //Start time hour
        int event_start_minute;     //Start date minute
        int event_start_second;     //Start date seconds

        byte alarm_status = 0;    //If the end and start time are the same we consider it to be an alarm and set this byte high
                                  //or if there is no end time specified in the considered formats (2 edges cases present within the u of c calendar)

        int event_end_date_code;  //End date of event in utc parts: 2021 04 06(used for internal comparison)
        int event_end_year;       //End date year
        int event_end_month;      //End date month
        int event_end_day;        //End date day

        int event_end_time_code;  //End time of event in utc parts: 03 27 43 (used for internal comparison)
        int event_end_hour;       //End time hour
        int event_end_minute;     //End date minute
        int event_end_second;     //End date seconds

    } CalendarEvent;

    typedef struct TimeZoneProperties
    {
        char time_zone_id[MAX_NAME_SIZE];       //contains the time-zone id of the calendar, no standard used but, some server-side libaries exist for use of this

        byte daylight_status;                   //If 1 then daylight saving occurs, if 0 no daylight savings
        byte daylight_mode;                     //Determines if we are in daylight or not, must be externally updated based on some webserver time or by hand, 1 for daylight savings 0 for no savings
        
        char daylight_time_zone [MAX_TZ_SIZE];  //Time zone name (MST, PST, etc) of the daylight savings timezone
        int daylight_offset;                    //The numerical time offset, from UTC time, that daylight savings is in -0700 would be 7 hours behind utc
        
        char standard_time_zone [MAX_TZ_SIZE];  //Time zone name (MST, MDT, etc) of the non-daylight savings timezone
        int standard_offset;                    //The numerical time offset, from UTC time, that standard time is in

        //Soooo, an excercise left to the reader: Try parsing "RRULE:FREQ=YEARLY;BYMONTH=11;BYDAY=1SU" in a function that can accurately change the time for daylight savings
    } TimeZoneInfo;

    typedef struct UserCalendar
    {
        char agenda_name[MAX_NAME_SIZE];                     //The name of the calendar, like who tf needs a multi-line calendar name

        TimeZoneInfo timezone;                               //A struc containing the timezone information about the Calendar

        int event_precedence[EVENTSTACKSIZE];                //The ints within the array related to the indices of the events, this is the order in which the events occur

        byte event_intialization = 0;                        //Indicates if events are present within the jobs array

        CalendarEvent *jobs[EVENTSTACKSIZE];                 //An array of CalendarEvent pointers that point to dynamically allocated or statically allocated CalendarEvent strucs

    } Calendar;

//CLASS/STRUCT DECLARATION SECTION END---------------------------------------------------------------------------------------------------------------------------------------------




//FUNCTION DECLARATION SECTION START-----------------------------------------------------------------------------------------------------------------------------------------------
    char *parse_data_string(File *file, const long file_byte_offset, long max_byte_offset, ICALMODERTN const byte return_string_mode);
    /* 
    REQUIRES:
        -A file_byte_offset which is the byte location/offset within the SD card File
        -A SD card class file address which is initialized and opened
        -A return_string_mode byte, most common is 0xFF indicating:
            -0xFF(MULTILINE): Mode is till end of string so it can return multi-line strings
            -0x00(ONELINE): Mode is till end of the line (CR-LF sequence) 
        -A long containing the max byte offset we search to, if not appliacable use -1 or NOEND
    PROMISES:
        -Upon sucess to return a pointer to a heap allocated character array which contains the string or multi-line string that is on the line/lines following 
        the file_byte_offset given
        -Upon failure to return a NULL pointer and to have free any heap space that might of been allocated
            -Possible failures include 
                -Insufficent heap space for allocation of a text buffer
                -Invaild file_byte_offset
                -Bad SD card read
                -End of file before end of a line (a bad ical file)
        -THREAD SAFE WITH pvPortMalloc() / vPortFree()
    */


    long find_next_keyword(File *file, const char *keyword, const long file_byte_offset, long max_byte_offset, ICALMODERTN const byte return_offset_mode);
    /* 
    REQUIRES:
        -A file_byte_offset which is the byte location/offset within the SD card File
        -A max_byte_offset which is the maximum file byte offset to be searched up to, 
            -(-1) or NOEND, for no max offset to search to
        -A SD card class file address which is initialized and opened
        -The address to string constant or char array that is NULL TERMINATED and contains the desired keyword to be found
        -A return_offset_mode byte, most common is 0xFF, indicating:
            -0xFF(FIRSTCHAR): Mode is start of keyword, means return value byte-offset is the first byte of the sequence of the keyword
            -0x11(NEXTCHAR): Mode is end of keyword, means return value byte-offset is the first byte after the end of the keyword
            -0x00(NEXTLINE): Mode is next line after keyword, means return value byte-offset is the first byte of the next line after the keyword occurance line
    PROMISES:
        -Upon sucess to return a long containing the file_byte_offset of the first char of the keyword specified that 
        occurs first after the input file_byte_offset
        -Upon error to return EOF
            -Possible failures include:
            -Invaild file_byte_offset
            -End of file before occurance of the keyword
        -Upon failure to find keyword within the max_byte_offset and the file_byte_offset
            -Return -2
    */


    //legacy function may not be updated to the same functionality of find_next_keyword
    long find_previous_keyword(File *file, const char *keyword, const long file_byte_offset, long min_byte_offset, ICALMODERTN const byte return_offset_mode);
    /* 
    REQUIRES:
        -A file_byte_offset which is the byte location/offset within the SD card File that the function will read backwards from
        -A min_byte_offset which is the minimum file byte offset to be searched back up to, 
            -PASS -1(NOEND) IF NO MIN OFFSET DESIRED
        -A SD card class file address which is initialized and opened
        -The address to string constant or char array that is NULL TERMINATED and contains the desired keyword to be found
        -A return_offset_mode byte, most common is 0xFF, indicating:
            -0xFF(FIRSTCHAR): Mode is start of keyword, means return value byte-offset is the first byte of the sequence of the keyword
            -0x11(NEXTCHAR): Mode is end of keyword, means return value byte-offset is the first byte after the end of the keyword
            -0x00(NEXTLINE): Mode is next line after keyword, means return value byte-offset is the first byte of the next line after the keyword occurance line
    PROMISES:
        -Upon sucess to return a long containing the file_byte_offset of byte per the specified mode
        -Upon Error to return EOF
            -Possible failures include:
            -Invaild file_byte_offset
            -Begining of file before occurance of the keyword
        -Upon failure to find keyword after the min_byte_offset
            =Return -2
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


    byte initialize_event(File *file, Calendar *user_calendar, CalendarEvent *user_event, ICALOFFSET const long event_byte_offset);
    /* 
    REQUIRES:
        -A SD card class file address which is initialized and opened
        -A pointer to an initialized calendar struct
        -A pointer to a created CalendarEvent
        -An byte-offset value for which the event begins at: "BEGIN:VEVENT" within the SD card file
    PROMISES:
        -To attempt to fill in the CalendarEvent's
            -Summary
            -Location
            -Time/date format status
            -Event start info(we assume start is when the alarm is, at least for the prototype)
            -Event end info
        -Returns 0 if success, -1 otherwise
        -THREAD SAFE WITH pvPortMalloc() / vPortFree(), (used internally with the function)
    */


    byte find_event(File *file, Calendar *user_calendar, long *sector_table, long *destination_byte_offset, const int current_datestamp, const int current_timestamp, byte update_sectors);
    /* 
    REQUIRES:
        -A SD card class file address which is initialized and opened
        -A sector table to look for relevant events
        -An initialized calendar
        -A destination byte offset for which to place the found event byte offset into
        -A int-utc-date code for which the event should start on/after 
        -A int-utc-time code for which the event should start on/after within the provided tolerance after
        -A byte for the update_sectors option, this will remove the event from the sector table before returning it so that the event_stack can be filled, 0x00 for no, 0xFF for yes
    PROMISES:
        -Upon success to return 0
        -Upon failure to return -1
    */


   byte fetch_event_time(File *file, Calendar *user_calendar, const long event_byte_offset, int *event_start_date, int *event_start_time, int *event_end_date, int *event_end_time);
    /* 
    REQUIRES:
        -A SD card class file address which is initialized and opened
        -An initializaed Calendar struct
        -A event_byte_offset pointing the first char of an BEGIN:VEVENT
        -A pointer to a int for the start date
        -A pointer to a int for the start time
        -A pointer to a int for the end date
        -A pointer to a int for the end time
    PROMISES:
        -Upon success to return 0
        -Upon failure to return -1
    */


   byte initialize_sector_table(File *file, Calendar *user_calendar, const int current_date_code, const int current_time_code, long *sector_table);
    /* 
    REQUIRES:
        -A SD card class file address which is initialized and opened
        -A sector table to add offsets to
        -A calendar that is intialized
        =A current date code: 20210407 for instance
    PROMISES:
        -Upon success to return 0
        -Upon failure to return -1
    */


   void update_sector_table(long *sector_table, const long event_start, const long event_end);
    /* 
    REQUIRES:
        -A sector table to edit
        -A event_start byte offset and a related event_end byte offset
        -The event offsets specified to belong to an event that we are flushing since it has passed
    PROMISES:
        -To update the sector table to be continous relevant sectors of SD card data
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


   void calendar_str_to_int(const char * str_num, int num_length, const char end_char, int *int_pointer);
    /* 
    REQUIRES:
        -A pointer to a charcter array that is numerical, a NO CHECK IS PERFORMED
        -A length of the number that starts at the address passed as a pointer
        -end_char is the basically a replacement for a null-termination so if you want to print utc timestamps like 202012T22123Z, it easier
        -Set end char to '\0' other wise
        -IF num_length is -1 then the string will be read until it's nulltermination or end_char if applicable
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


    void print_event(CalendarEvent *user_event);
    /* 
    REQUIRES:
        -A pointer to an initialized Event
    PROMISES:
        -To Serial print out information about the event
    */


   void print_calendar(Calendar *user_calendar);
    /* 
    REQUIRES:
        -A pointer to an initialized Calendar 
    PROMISES:
        -To Serial print out information about the calendar
    */

   void update_calendar_event(File *file, Calendar *user_calendar, long *sector_table, const int date_stamp, const int time_stamp);
    /* 
    REQUIRES:
        -A SD card class file address which is initialized and opened
        -A pointer to an initialized Calendar with initialized events in it's job array
        -A pointer to the current sector table for the SD card ical file
        -The current date and time stamps in UTC
    PROMISES:
        -To refresh the next occuring/occured event from the Calendar jobs
        -To reorder the event precedence array
        -If no more events can be found in the ical file/sector table then the jobs array pointer will be NULL
            -And the event_precedence array with have a -1 at the element spot
    */

   byte fill_calendar_events(File *file, Calendar *user_calendar, long *sector_table, const int date_stamp, const int time_stamp);
    /* 
    REQUIRES:
        -A SD card class file address which is initialized and opened
        -A pointer to an initialized Calendar with allocated jobs in
        -A pointer to the current sector table for the SD card ical file
    PROMISES:
        -To fill the event
    */
//FUNCTION DECLARATION SECTION END-------------------------------------------------------------------------------------------------------------------------------------------------




#endif  // #ifndef ICAL_LIBARY_H