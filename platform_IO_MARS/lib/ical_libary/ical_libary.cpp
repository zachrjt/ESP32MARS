#include <Arduino.h>      //Arduino Stuff
#include <SPI.h>          //SPI Libary
#include <SD.h>           //SD Card libary

#include "ical_libary.h"  //Associated header file

char *parse_data_string(File *file, const long file_byte_offset, long max_byte_offset, ICALMODERTN const byte return_string_mode)
{
    long current_file_byte_offset = file_byte_offset;
    int max_text_buffer_index = 77;
    int text_buffer_index = 0;
    char *text_buffer = (char *)(pvPortMalloc(sizeof(char) * (max_text_buffer_index))); //the maximum length of a line in a ical file is 77 with the CR-LF sequence, 
    if (file->position() != file_byte_offset)//Are already at the specified posistion?
    {
        if (!file->seek(file_byte_offset)) //going to the specified position
        {
            return NULL; //specifed position could not be seeked out
        }
    }
    char buffer_byte = '\0';

    if (max_byte_offset == -1)   //pass -1 if we dont want to specifiy A maximum
    {
        max_byte_offset = 2147483647;   //maximum value possible
    }

    while ((text_buffer_index < max_text_buffer_index) && (current_file_byte_offset < max_byte_offset))
    {
        if ((ICALMODERTN return_string_mode == 0xFF) && (text_buffer_index == (max_text_buffer_index - 1)))  //text buffer may need to be larger for multi-line strings
        {                                                                                                 //We check the ICALMODE since this is only needed for mode 0xFF
            char *temp_buffer = (char *)(pvPortMalloc((sizeof(char) * (75 + max_text_buffer_index))));
            for (int i = 0; i <= text_buffer_index; i++)
            {
                temp_buffer[i] = text_buffer[i];
            }
            vPortFree(text_buffer);
            text_buffer = temp_buffer;
            max_text_buffer_index += 75;
        }

        buffer_byte = file->read();
        if (buffer_byte == -1) //read returns -1 if EOF or other error
        {
            vPortFree(text_buffer); //if EOF bad .ical format since all lines
            return NULL;
        }
        else
        {
            current_file_byte_offset++; //incrementing the byte offset to match the position
            text_buffer[text_buffer_index] = buffer_byte;
            if (buffer_byte == '\n') //If the current buffer byte is LF
            {
                if (text_buffer[(-1 + text_buffer_index)] == '\r')   //If the last byte was CR
                {
                    //Means we encountered a CR-LF sequence
                    if((ICALMODERTN return_string_mode == 0xFF) && ((file->peek() == '\t') || (file->peek() == ' ')))//if the next byte is a whitespace character
                    {                                                                                             //it means the string is multi-line(if mode 0xFF)
                        file->read();   //discard white space and allow for the function to continue reading data
                        current_file_byte_offset++; //increment file byte offset
                        text_buffer_index = text_buffer_index - 2;    //rewrite over the CR-LF sequence with the upcoming string data, -2 since the next line increments by 1 anyways
                    }
                    else //the next character was not a whitespace so end of string, regardless of mode
                    {
                        //Serial.print("found end of line at ");       //For debugging purposes
                        //Serial.println(current_file_byte_offset);    //For debugging purposes
                        text_buffer_index = text_buffer_index - 1;    //rewrite over the CR-LF sequence with null characters ending string 
                        break;  //breaking to null terminate string and return it 
                    }
                }
            }
            text_buffer_index++;//incrementing text_buffer index
        }
    }
    text_buffer[text_buffer_index++] = '\0';  //adding null character to terminate string
    //Serial.println(max_text_buffer_index);  //For debugging purposes
    return text_buffer;
}


long find_next_keyword(File *file, const char *keyword, const long file_byte_offset, long max_byte_offset, ICALMODERTN const byte return_offset_mode)
{
    long current_byte_offset = file_byte_offset; //The byte offset to returned, of the first character of the keyword we're looking for

    if (max_byte_offset == -1)   //pass -1 if we dont want to specifiy A maximum
    {
        max_byte_offset = 2147483647;   //maximum value possible for a long
    }

    int keyword_index = 0;                       //Pointer index of our keyword string
    int keyword_length = 0;                      //The length of the keyword
    for (int i = 0; keyword[i] != '\0'; i++, keyword_length++); //Finding the length of keyword passed, I know C++ has strings but, I dont feel like learning them

    char buffer_byte = '\0';                    //Initializing a buffer byte to read bytes in from the file

    if (file->position() != file_byte_offset)//Are already at the specified posistion? 
    {
        if (!file->seek(file_byte_offset)) //Going to the specified position within the file
        {
            current_byte_offset = EOF; //Specifed position could not be seeked out since invalid, returning EOF
            return current_byte_offset;
        }
    } 

    while (1)
    {
        if (current_byte_offset >= max_byte_offset)//Are we past the maximum alloated byte offset
        {
            current_byte_offset = -2;      //Return -2 indicating error not within maximum byte offset
            return current_byte_offset;
        }
        buffer_byte = file->read();     

        if (buffer_byte == -1)             //read() method of File will return -1/EOF if no more characters could be read, ie EOF
        {
            current_byte_offset = EOF;     //Return EOF indicating error during keyword parsing
            return current_byte_offset;
        }
        else                               //Not an error nor a CR so a regular char byte
        {
            current_byte_offset++;
            if(buffer_byte == keyword[keyword_index++])    //If the current byte matches the first/next character in the keyword
            {   
                if (keyword_index == keyword_length)        //If a keyword match was found, ie reached end of keyword string;
                {
                    if (ICALMODERTN return_offset_mode == 0x00)
                    {
                        //Could implement a recursive call to itself with a different mode to find the CR-LF sequence in mode 0x11 but, that would 
                        //take up more stack space and instructions
                        while(1)
                        {
                            if (current_byte_offset >= max_byte_offset)//Are we past the maximum alloated byte offset
                            {
                                current_byte_offset = -2;      //Return -2 indicating error not within maximum byte offset
                                return current_byte_offset;
                            }
                            buffer_byte = file->read();
                            if (buffer_byte == -1)             //read() method of File will return -1/EOF if no more characters could be read, ie EOF
                            {
                                current_byte_offset = EOF;     //Return EOF indicating error during keyword parsing
                                return current_byte_offset;
                            }
                            else
                            {
                                current_byte_offset++;             //successful read increment byte offset
                                if (buffer_byte == '\r')
                                {
                                    if (file->peek() == '\n')      //Check if the next byte is LF...
                                    {
                                        current_byte_offset++;     //increment the byte offset and break so it can be returned
                                        break;                     //return to keyword return section
                                    }
                                }
                            }
                        }
                    }
                    else if (ICALMODERTN return_offset_mode == 0x11)
                    {
                        //Returning the byte offset of the first byte after the keyword (mode 0x11), dont have to do anything
                    }
                    else //ICALMODE return_offset_mode should have a value of 0xFF since it can only have 3 modes but we'll ignore that
                    {
                        current_byte_offset -= keyword_index;   //Returning the byte offset for the first byte of the keyword (mode 0xFF)
                    }
                    break;              
                }
            }
            else
            {
                keyword_index = 0;  //Keyword pattern broken or not even found yet
            }
        }
    }
    return current_byte_offset; //Since no key value was found in the entire file return EOF
}


long find_previous_keyword(File *file, const char *keyword, const long file_byte_offset, long min_byte_offset, ICALMODERTN const byte return_offset_mode)
{
    long current_byte_offset = file_byte_offset; //The byte offset to returned, of the first character of the keyword we're looking for

    if (min_byte_offset == -1)   //pass -1 if we dont want to specifiy a minimum
    {
        min_byte_offset = 0;   //minimum value possible
    }

    int keyword_length = 0;                      //The length of the keyword
    for (int i = 0; keyword[i] != '\0'; i++, keyword_length++); //Finding the length of keyword passed
    int keyword_index = keyword_length-1;         //Pointer index of our keyword string, in mode 0x00 (reverse) starts at max and decreases

    char buffer_byte = '\0';                    //Initializing a buffer byte to read bytes in from the file
    if (file->position() != file_byte_offset)//Are already at the specified posistion?
    {
        if (!file->seek(file_byte_offset)) //Going to the specified position within the file
        {
            current_byte_offset = EOF; //Specifed position could not be seeked out since invalid, returning EOF
            return current_byte_offset;
        }
    }

    while (1)
    {
        if (current_byte_offset <= min_byte_offset)//Are we below the minimum byte offset
        {
            current_byte_offset = -2;      //Return -2 indicating error not within minimum byte offset
            return current_byte_offset;
        }
        file->seek(current_byte_offset);   //Need to read backwards seek previous byte, there is no way to use read and go backwards

        buffer_byte = file->read();
        if (buffer_byte == -1)             //read() method of File will return -1/EOF if no more characters could be read, ie EOF
        {
            current_byte_offset = EOF;     //Return EOF indicating error during keyword parsing
            return current_byte_offset;
        }
        else                               //Not an error nor a CR so a regular char byte         
        {
            current_byte_offset--;         //Success so we decrement the current_byte_offset so next time we read the previous byte
            if(buffer_byte == keyword[keyword_index--]) //Does the current byte match the current on in the keyword pattern
            {
                if (keyword_index == 0)        //If a keyword match was found, ie reached end of keyword string;
                {
                    if (ICALMODERTN return_offset_mode == 0x00)//Returning the byte offset of the first line after keyword (mode 0x00)
                    {
                        while(1)
                        {
                            buffer_byte = file->read();
                            if (buffer_byte == -1)             //read() method of File will return -1/EOF if no more characters could be read, ie EOF
                            {
                                current_byte_offset = EOF;     //Return EOF indicating error during keyword parsing
                                return current_byte_offset;
                            }
                            else
                            {
                                current_byte_offset++;             //successful read increment byte offset
                                if (buffer_byte == '\r')           //Checking if current byte is CR
                                {
                                    if (file->peek() == '\n')      //Check if the next byte is LF
                                    {
                                        current_byte_offset +=2;   //return byte after CR-LF sequence
                                        break;                     //return to keyword return section
                                    }
                                }
                            }
                        }
                    }
                    else if (ICALMODERTN return_offset_mode == 0x11)
                    {
                        current_byte_offset += keyword_length;//Returning the byte offset of the first byte after the keyword (mode 0x11)
                    }
                    else //ICALMODE return_offset_mode should have a value of 0xFF since it can only have 3 modes but we'll ignore that
                    {
                        //Returning the byte offset for the first byte of the keyword (mode 0xFF), so don't need to do anything
                    }
                    break;   
                }
            }
            else
            {
                keyword_index = keyword_length - 1;     //Either the keyword pattern streak was lost or no match to the keyword yet
            }
        }
    }
    return current_byte_offset;            //Since no key value was found in the entire file return EOF
}


byte initialize_calendar(File *file, Calendar *user_calendar)
{
    //This function just needs some basic text parsing for the name and time zone id calling the parse string and find keyword funcs
    //Grabing name section---------------------------------------------------------------------------------------------------------------------------------------------------------
    long working_byte_offset = 0;
    const long max_cal_byte_offset = find_next_keyword(file, "BEGIN:VEVENT", ICALOFFSET 0, ICALOFFSET -1, ICALMODERTN FIRSTCHAR);//Find where the first event starts
                                                                                                                  //Which is the end of revelant calendar settings
    
    working_byte_offset = find_next_keyword(file, "X-WR-CALNAME:", ICALOFFSET working_byte_offset, ICALOFFSET max_cal_byte_offset, ICALMODERTN NEXTCHAR); //Find byte-offset of start of calendar name
    if(working_byte_offset == EOF)
    {
        return -1;  //Failure could not find calendar name keyword
    }
    else if(working_byte_offset == -2)  //could not fine name property within start and first event
    {
        calendar_char_copy("Unknown", user_calendar->agenda_name); //since the calendar name wasnt specified setting it to "unknown"
    }
    char *working_string_pointer = NULL;
    working_string_pointer = parse_data_string(file, working_byte_offset, NOEND, ONELINE);    //Parse name line for name
    if (working_string_pointer == NULL)
    {
        return -1;  //Failure could not parse calendar name
    }
    calendar_char_copy(working_string_pointer, user_calendar->agenda_name); //copying heap string into agenda name of struc
    vPortFree(working_string_pointer);  //freeing name from heap
    //Grabing name section---------------------------------------------------------------------------------------------------------------------------------------------------------

    //Grabing timezone section-----------------------------------------------------------------------------------------------------------------------------------------------------
    //Timezone id section
    working_byte_offset = find_next_keyword(file, "TZID:", ICALOFFSET 0, ICALOFFSET max_cal_byte_offset, ICALMODERTN NEXTCHAR);
    if(working_byte_offset == -2) //-2 implies the function could not find the keyword between 0 and the first event, so not specified
    {
        calendar_char_copy("Unknown", user_calendar->timezone.time_zone_id); //No timezone id string specified so setting to "Unknown"
    }
    else if(working_byte_offset == EOF)
    {
        return -1;  //Error occurred while looking so quit
    }
    else
    {
        working_string_pointer = parse_data_string(file, working_byte_offset, NOEND, ONELINE);    //Parse for time zone id
        if (working_string_pointer == NULL)
        {
            return -1;  //Failure could not parse timezone id string data
        }
        calendar_char_copy(working_string_pointer, user_calendar->timezone.time_zone_id); //copying heap string into agenda name of struc
        vPortFree(working_string_pointer);  //freeing time zone id from heap
    }

    //Daylight savings section
    working_byte_offset = find_next_keyword(file, "BEGIN:DAYLIGHT", ICALOFFSET 0, ICALOFFSET max_cal_byte_offset, ICALMODERTN NEXTLINE);    //looking for beginning of daylight info
    if(working_byte_offset == -2) //-2 implies the function could not find the keyword between 0 and the first event, so not specified
    {
        user_calendar->timezone.daylight_status = 0; //No daylight savings found setting byte to 0
    }
    else if(working_byte_offset == EOF)
    {
        return -1;  //Error occurred while looking so quit
    }
    else    //The daylight saving info beginning was found
    {
        const long daylight_start = working_byte_offset;   //setting the starting byte offset to the BEGIN:DAYLIGHT
        const long daylight_end = find_next_keyword(file, "END:DAYLIGHT", ICALOFFSET daylight_start, ICALOFFSET max_cal_byte_offset, ICALMODERTN FIRSTCHAR); //finding the end of the daylight savings
        user_calendar->timezone.daylight_status = 1; //Daylight savings found setting byte to enable daylight savings

        //looking for the daylight savings numerical offset
        working_byte_offset = find_next_keyword(file, "TZOFFSETTO:", ICALOFFSET daylight_start, ICALOFFSET daylight_end, ICALMODERTN NEXTCHAR); //looking for the daylight savings
        if(working_byte_offset < 0)                                                                                                         //offset value
        {
            return -1;  //error daylight savings but no specified offset
        }                                                                                           
        working_string_pointer = parse_data_string(file, working_byte_offset, NOEND,  NEXTLINE);    //parse the time zone offset
        if(working_string_pointer == NULL)
        {
            return -1;  //error during parsing daylight saving offset value
        }
        calendar_str_to_int(working_string_pointer, -1, '\0', &user_calendar->timezone.daylight_offset);  //put the timezone offset string data into the calendar.timezone struct as an int
        vPortFree(working_string_pointer);  //freeing the heap offset data

        //looking for the time zone string id of daylight savings time
        working_byte_offset = find_next_keyword(file, "TZNAME:", ICALOFFSET daylight_start, ICALOFFSET daylight_end, ICALMODERTN NEXTCHAR);
        if(working_byte_offset == -1)//error during search
        {
            return -1;  //error during search
        }
        else if(working_byte_offset == -2)//could not find within daylight, ie not specified
        {
            calendar_char_copy("Unknown", user_calendar->timezone.daylight_time_zone); //since the daylight savings time zone id wasnt specified setting it to unknown
        }
        else    //found timezone name parsing data
        {
            working_string_pointer = parse_data_string(file, working_byte_offset, NOEND, ONELINE);    //parsing the string data of the timezone id for daylight savings
            if(working_string_pointer == NULL)
            {
                return -1;  //error during parsing daylight time zone id
            }
            calendar_char_copy(working_string_pointer, user_calendar->timezone.daylight_time_zone); //coping data into the struc
            vPortFree(working_string_pointer);  //freeing daylight savings id heap string data
        }
    }
    
    //Standard (non-daylight savings section
    working_byte_offset = find_next_keyword(file, "BEGIN:STANDARD", ICALOFFSET 0, ICALOFFSET max_cal_byte_offset, ICALMODERTN NEXTLINE);    //looking for beginning of daylight info
    if(working_byte_offset == -2) //-2 implies the function could not find the keyword between 0 and the first event, so not specified
    {
        user_calendar->timezone.daylight_status = 0; //No standard found daylight savings setting byte to 0
    }
    else if(working_byte_offset == EOF)
    {
        return -1;  //Error occurred while looking so quit
    }
    else    //The standard info beginning was found
    {
        const long standard_start = working_byte_offset;   //setting the starting byte offset to the BEGIN:STANDARD
        const long standard_end = find_next_keyword(file, "END:STANDARD", ICALOFFSET standard_start, ICALOFFSET max_cal_byte_offset, ICALMODERTN FIRSTCHAR); //finding the end of the standard 
        user_calendar->timezone.daylight_status = 1; //Standard found setting byte to enable daylight savings

        //looking for the daylight savings numerical offset
        working_byte_offset = find_next_keyword(file, "TZOFFSETTO:", ICALOFFSET standard_start, ICALOFFSET standard_end, ICALMODERTN NEXTCHAR); //looking for the daylight savings
        if(working_byte_offset < 0)                                                                                                         //offset value
        {
            return -1;  //error standard time present but no specified offset
        }                                                                                           
        working_string_pointer = parse_data_string(file, working_byte_offset, NOEND, NEXTLINE);    //parse the time zone offset
        calendar_str_to_int(working_string_pointer, -1, '\0', &user_calendar->timezone.standard_offset);  //put the timezone offset string data into the calendar.timezone struct as an int
        vPortFree(working_string_pointer);  //freeing the heap offset data

        //looking for the time zone string id of standard time
        working_byte_offset = find_next_keyword(file, "TZNAME:", ICALOFFSET standard_start, ICALOFFSET standard_end, ICALMODERTN NEXTCHAR);
        if(working_byte_offset == -1)//error during search
        {
            return -1;  //error during search
        }
        else if(working_byte_offset == -2)//could not find within daylight, ie not specified
        {
            calendar_char_copy("Unknown", user_calendar->timezone.standard_time_zone); //since the standard time zone id wasnt specified setting it to unknown
        }
        else    //found timezone name parsing data
        {
            working_string_pointer = parse_data_string(file, working_byte_offset, NOEND, ONELINE);    //parsing the string data of the timezone of standard time
            if(working_string_pointer == NULL)
            {
                return -1;  //error during parsing standard time zone id
            }
            calendar_char_copy(working_string_pointer, user_calendar->timezone.standard_time_zone); //coping data into the struc
            vPortFree(working_string_pointer);  //freeing standard time zone id heap string data
        }
    }
    //Grabing timezone section-----------------------------------------------------------------------------------------------------------------------------------------------------
    
    return 0;
}


byte initialize_event(File *file, Calendar *user_calendar, CalendarEvent *user_event, ICALOFFSET const long event_byte_offset)
{
    //This function fills events that are passed into it from data found at the event_byte_offset
    if(event_byte_offset == EOF)
    {
        return -1;
    }
    const long max_event_byte_offset = find_next_keyword(file, "END:VEVENT", ICALOFFSET event_byte_offset, ICALOFFSET -1, ICALMODERTN NEXTLINE); //Find end of event
    if(max_event_byte_offset < 0)
    {
        return -1;  //Error or failure to find end of event, returning error code
    }
    //Grabing summary section------------------------------------------------------------------------------------------------------------------------------------------------------
    long working_byte_offset = 0;//Used to find keyword locally
    char *working_string_pointer = NULL;
    working_byte_offset = find_next_keyword(file, "SUMMARY:", ICALOFFSET event_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);  //looking for summary info
    if(working_byte_offset == EOF)
    {
        return -1;  //Error during search for event summary
    }
    else if(working_byte_offset == -2)  //could not find summary property within event byte offset range
    {
        calendar_char_copy("Event", user_event->event_summary); //since the event summary wasnt specified setting it to "Event"
    }
    else
    {
        working_string_pointer = parse_data_string(file, working_byte_offset, (working_byte_offset + 1 + MAX_SUMMARY_SIZE), MULTILINE);    //Parse summary line for summary string
        if (working_string_pointer == NULL)
        {
            return -1;  //Failure could not parse event summary
        }
        calendar_char_copy(working_string_pointer, user_event->event_summary); //copying heap string into event summary of struc
        vPortFree(working_string_pointer);  //freeing summary from heap
    }
    //Grabing summary section------------------------------------------------------------------------------------------------------------------------------------------------------
    
    //Grabing location section-----------------------------------------------------------------------------------------------------------------------------------------------------
    working_byte_offset = find_next_keyword(file, "LOCATION:", ICALOFFSET event_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);  //looking for location string
    if(working_byte_offset == EOF)
    {
        return -1;  //Error during search for event location
    }
    else if(working_byte_offset == -2)  //could not find location property within event byte offset range
    {
        calendar_char_copy("Everywhere", user_event->event_location); //since the calendar location wasnt specified setting it to "Everywhere"
    }
    else
    {
        working_string_pointer = parse_data_string(file, working_byte_offset, (working_byte_offset + 1 + MAX_LOCATION_SIZE), MULTILINE); //Parse line for location string
        if (working_string_pointer == NULL)
        {
            return -1;  //Failure could not parse event location 
        }
        calendar_char_copy(working_string_pointer, user_event->event_location); //copying heap string into location of struc
        vPortFree(working_string_pointer);  //freeing location from heap
    }
    //Grabing location section-----------------------------------------------------------------------------------------------------------------------------------------------------

    //Grabing times section--------------------------------------------------------------------------------------------------------------------------------------------------------

    if(fetch_event_time(file, user_calendar, event_byte_offset, &user_event->event_start_date_code, &user_event->event_start_time_code, &user_event->event_end_date_code, &user_event->event_end_time_code))
    {
        //Error could not parse times
        return -1;
    }
    else
    {
        //Checking for alarm status i.e. end time/date == start time/date
        if((user_event->event_start_time_code) == (user_event->event_end_time_code))
        {
            if((user_event->event_start_date_code) == (user_event->event_end_date_code))
            {
                user_event->alarm_status = 1;
            }
            else
            {
                user_event->alarm_status = 0;
            }
        }
        else
        {
            user_event->alarm_status = 0;
        }
        //Need to force the times into the individual ints
        int temp_code = user_event->event_start_time_code;

        user_event->event_start_second = (temp_code % 100);

        temp_code = (temp_code/100);
        user_event->event_start_minute = (temp_code % 100);

        temp_code = (temp_code/100);
        user_event->event_start_hour = temp_code;

        //now the date stamp
        temp_code = user_event->event_start_date_code;

        user_event->event_start_day = (temp_code % 100);

        temp_code = (temp_code/100);
        user_event->event_start_month = (temp_code % 100);

        temp_code = (temp_code/100);
        user_event->event_start_year = temp_code;

        //Now the end times/dates
        temp_code = user_event->event_end_time_code;

        user_event->event_end_second = (temp_code % 100);

        temp_code = (temp_code/100);
        user_event->event_end_minute = (temp_code % 100);

        temp_code = (temp_code/100);
        user_event->event_end_hour = temp_code;

        //now the date stamp
        temp_code = user_event->event_end_date_code;

        user_event->event_end_day = (temp_code % 100);

        temp_code = (temp_code/100);
        user_event->event_end_month = (temp_code % 100);

        temp_code = (temp_code/100);
        user_event->event_end_year = temp_code;


        //let us now update the event struc byte offset values incase we need them later
        user_event->start_byte_offset = event_byte_offset;
        user_event->end_byte_offset = max_event_byte_offset;
    }
    //Grabing times section--------------------------------------------------------------------------------------------------------------------------------------------------------
    
    return 0;
}


byte find_event(File *file, Calendar *user_calendar, long *sector_table, long *destination_byte_offset, int current_datestamp, int current_timestamp, byte update_sectors)
{
    //This needs some basic operations and calls to find_event_limits
    //First needs the creation/function to make a read_sector_table 
        //Read sector table basically is a table of byte offset ranges that are valid to read from, it makes read the file after intialization muchhhhh easier
            //We wont need to re-read 150000 lines and any lines/events that have already happened can be skipped over and dont need to be parsed

    long found_event_byte_offset = 0;//Storest the nearest event happining/ending soon currently
    int found_event_timing[4] = {100000000, 10000000, 100000000, 10000000};//Storest the time info: {start-date, start-time, end-date, end-time}

    long working_byte_offset = 0;//The active byte offset being looked at
    int max_sector_index = 0;//Finding the last index in the sector
    while(sector_table[max_sector_index] != 0)
    {
        max_sector_index++;
    }

    int temp_event_timing[4] = {0, 0, 0, 0};//The timing (start date, time ; end date, time) of a found event

    for(int i = 0; i < max_sector_index; i+=2)//Look through the entire sector table(All relevant events)
    {
        if(Serial && 0)
        {
            Serial.print("Looking in sector index: ");
            Serial.println(i);
            Serial.println(sector_table[i], HEX);
            Serial.println(sector_table[(1 + i)], HEX);
            Serial.print("\t");
        }

        working_byte_offset = sector_table[i];
        while(1)//In each one look for all the possible events
        {
            if(Serial && 0)
            {
                Serial.print(".");
            }
            working_byte_offset = find_next_keyword(file, "BEGIN:VEVENT", (working_byte_offset), sector_table[(1+ i)], FIRSTCHAR);//find an event 
            if(working_byte_offset == EOF)
            {
                return -1;///Error while parsing shouldn't happen
            }
            else if(working_byte_offset == -2)
            {
                if(Serial && 0)
                {
                    Serial.println("\tNo more events in this sector");
                }
                break;//Can not find anymore events within the current sector
            }
            else
            {
                //Found an event, checking if the timing is good
                if(!fetch_event_time(file, user_calendar, working_byte_offset, &(temp_event_timing[0]), &(temp_event_timing[1]), &(temp_event_timing[2]), &(temp_event_timing[3])))
                {
                    int tiny_event_state = 0;//1 for a zero-duration event, 0 for an event with a duration
                    int logic_state = 0;//used since it's easier to set a logic bit then structure a proper if else, continue structure
                    int ongoing_state = 0;//used to track if an event is ongoing based on its start/end time/date and the current time
                    if((temp_event_timing[1] == temp_event_timing[3]) && (temp_event_timing[0] == temp_event_timing[2]))
                    {
                        //The event ends/starts at the same time, perhaps because it's an alarm or if its a local date stamp only
                        tiny_event_state = 1;
                    }
                    if(!tiny_event_state)
                    {
                        //okay the event has a length and is not infinitesimal in duration
                        //harder to figure out since event that have started but haven't ended are still useful to know about
                        if(temp_event_timing[0] <= current_datestamp)//has the event date happened or is it now
                        {
                            //A little tangent you might think that we aren't considering the seperate case of the event starting today at some time 
                            //And that we should consider/compare that time to see if the event has started, 
                            //.....
                            //butttttt since the end time will always be greater than the start time it doesn't matter
                            //if the event starts later than the end time statements below wont matter since it'll pass them because end time > start, and if the event time has passed today then 
                            //the comparisons below will matter and would have been made anyways in a seperate case 

                            //okay event start date has occured/occuring but has the end date passed already?
                            if(temp_event_timing[2] < current_datestamp)
                            {
                                //end date has passed too
                                logic_state = 1;
                            }
                            else if(temp_event_timing[2] == current_datestamp)//is the end date today?
                            {
                                //okay the end date is today but, has the end time passed?
                                if(temp_event_timing[3] <= current_timestamp)
                                {
                                    logic_state = 1;//Event end time has passed today/rn :(
                                }
                                else
                                {
                                    //event end is today but event end time is still later today
                                    ongoing_state = 1;//event is ongoing rn
                                }
                            }
                            else
                            {
                                //event start is today/past but end dat is later on
                                ongoing_state = 1;//event is ongoing rn
                            }
                        }
                    }
                    else
                    {
                        //these events can be ongoing they are either done or not done
                        //the event is infinitesimal in duration
                        //only need to check the start time/date since they are the same
                        if(temp_event_timing[0] < current_datestamp)
                        {
                            //event start and end, both the same, has passed 
                            logic_state = 1;//
                        }
                        else if (temp_event_timing[0] == current_datestamp)
                        {
                            //Event is today, but what time today
                            if(temp_event_timing[1] <= current_timestamp)
                            {
                                //okay start/end time has happened already/rn
                                logic_state = 1;
                            }
                        }
                    }
                    if(!logic_state)
                    {
                        if(!tiny_event_state)
                        {
                            //event has some duration so the end times are important to compare
                            if(temp_event_timing[0] <= found_event_timing[0])//is the start date lower than the current lowest
                            {
                                //okay we need to see if the event is ongoing
                                if(!ongoing_state)
                                {
                                    //the event is ongoing so end dates are what we want to compare to figure out which event ends next, not which one start first
                                    if(temp_event_timing[2] < found_event_timing[2])//is the ongoing event end date earlier than the lowest event found
                                    {
                                        //okay this event ends earlier so it is more relevant
                                        //since we already checked the all the event times this is the closed start date to now, new lowest
                                        found_event_timing[0] = temp_event_timing[0];
                                        found_event_timing[1] = temp_event_timing[1];
                                        found_event_timing[2] = temp_event_timing[2];
                                        found_event_timing[3] = temp_event_timing[3];
                                        found_event_byte_offset = working_byte_offset; 
                                    }
                                    else if(temp_event_timing[2] == found_event_timing[2])//is the event end the same as the current lowest
                                    {
                                        //since the end dates are the same need to compare the end times
                                        if(temp_event_timing[3] < found_event_timing[3])
                                        {
                                            //event is ongoing and ends earlier the same day as the earliers day
                                            found_event_timing[0] = temp_event_timing[0];
                                            found_event_timing[1] = temp_event_timing[1];
                                            found_event_timing[2] = temp_event_timing[2];
                                            found_event_timing[3] = temp_event_timing[3];
                                            found_event_byte_offset = working_byte_offset; 
                                        }
                                    }
                                }
                                else//not an ongoing event
                                {
                                    //the event is not ongoing so that means that the start date was the same as the lowest so only need to check the start time
                                    if (temp_event_timing[0] < found_event_timing[0])//we already know that the event date is <= so if less that the lowest than easy
                                    {

                                    }
                                    else if(temp_event_timing[1] < found_event_timing[1])//since its not les it must be the same so check the start time of the event
                                    {
                                        //event start is earlier than the lowest event during the lowest date
                                        found_event_timing[0] = temp_event_timing[0];
                                        found_event_timing[1] = temp_event_timing[1];
                                        found_event_timing[2] = temp_event_timing[2];
                                        found_event_timing[3] = temp_event_timing[3];
                                        found_event_byte_offset = working_byte_offset; 
                                    }
                                }
                            }
                        }
                        else
                        {
                            //event has no duration only need to compare start times since it also the end time
                            if((temp_event_timing[0] < found_event_timing[0]) || (temp_event_timing[0] < found_event_timing[2]))//is the start date lower than the current lowest start or an ongoing events end 
                            {
                                //since we already checked the all the event times this is the closed start date to now, new lowest
                                found_event_timing[0] = temp_event_timing[0];
                                found_event_timing[1] = temp_event_timing[1];
                                found_event_timing[2] = temp_event_timing[2];
                                found_event_timing[3] = temp_event_timing[3];
                                found_event_byte_offset = working_byte_offset; 
                            }
                            else if((temp_event_timing[0] == found_event_timing[0] )|| (temp_event_timing[0] == found_event_timing[2]))//okay the start/end dates are the same what about the start times?
                            {
                                if((temp_event_timing[1] < found_event_timing[1]) || (temp_event_timing[1] < found_event_timing[3])) //checking if the start time is lower than the lowest start/end
                                {
                                    //okay found new lowest, we dont compare if they are equal since order of occurance within the file is used the sequence of same event time events
                                    found_event_timing[0] = temp_event_timing[0];
                                    found_event_timing[1] = temp_event_timing[1];
                                    found_event_timing[2] = temp_event_timing[2];
                                    found_event_timing[3] = temp_event_timing[3];
                                    found_event_byte_offset = working_byte_offset; 
                                }
                            }
                        }
                    }
                    else
                    {
                        if(Serial)
                        {
                            Serial.print("The event at: ");
                            Serial.println(working_byte_offset, HEX);
                            Serial.print("With a start date / time stamp of: ");
                            Serial.print(temp_event_timing[0]);
                            Serial.print(" / ");
                            Serial.println(temp_event_timing[1]);

                            Serial.println("----------------------------------------------------");

                            Serial.print("With an end date / time stamp of: ");
                            Serial.print(temp_event_timing[2]);
                            Serial.print(" / ");
                            Serial.println(temp_event_timing[3]);
                        }
                        //These events were on the sector table but no longer are relevant, could happen if the time stamp is updated before find event after sector table initialization
                        //Or if the event stack size isnt larger enough so like 100 events are scheduled to start/end at a given time and we can only "recognize/acknowledge" EVENTSTACKSIZE number of them
                        //THen the end time around when removing these event we would arrive here finding event that have finished but are still present within the sector table
                        long event_end = find_next_keyword(file, "END:VEVENT", found_event_byte_offset, NOEND, NEXTLINE);
                        update_sector_table(sector_table, found_event_byte_offset, event_end);
                    }
                }
                else
                {
                    return -1; //Could not parse the timing of the event, despite being within the sector table implying it's safe to parse
                }
            }
            working_byte_offset++;//increment this so we dont get stuck on first event
        }
        if(Serial && 0)
        {
            Serial.println();
        }
    }
    if(Serial && 0)
    {
        Serial.println(found_event_byte_offset, HEX);
    }
    if(update_sectors == 0xFF)
    {
        //grabbing the event end to update sector table
        long event_end = find_next_keyword(file, "END:VEVENT", found_event_byte_offset, NOEND, NEXTLINE);
        update_sector_table(sector_table, found_event_byte_offset, event_end);
    }

    *destination_byte_offset = found_event_byte_offset;//Setting the event to the nearest event, finished the first

    return 0;//Success
}

byte fetch_event_time(File *file, Calendar *user_calendar, const long event_byte_offset, int *event_start_date, int *event_start_time, int *event_end_date, int *event_end_time)
{
    long working_byte_offset = 0;//Used to find keyword locally
    long extra_byte_offset = 0;//An extra working byte offset to use

    long event_start_byte_offset = event_byte_offset;//THe  event start byte, first char of BEGIN:VEVENT
    long event_end_byte_offset = 0;//The  event end byte, the line after END:VEVENT

    int event_date_code = 0;//The start date code of the event currently being looked at (UTC)
    int event_time_code = 0;//The end time code of the event currently being looked at (UTC)

    int event_date_end_code = 0;//The end date code of the event UTC
    int event_time_end_code = 0;//The end time code of the event UTC

    char * working_string_pointer = NULL;//Used to parse string content from file

    event_end_byte_offset = find_next_keyword(file, "END:VEVENT", event_start_byte_offset, NOEND, FIRSTCHAR);//Find end of event

    working_byte_offset = find_next_keyword(file, "DTSTART", event_start_byte_offset, event_end_byte_offset, NEXTCHAR);//Find the start date
    
    if(working_byte_offset < 0)
    {
        return -1;  //Critical Error during search for DTSTART this is bad since all events must have a start time, bad ICAL file
    }
    else
    {
        char char_temp = file->read();

        //UTC TIMESTAMP------------------------------------------------------------------------------------------------------------------------------------------------------------
        if (char_temp == ':')//Check if the start time is in a UTC time stamp
        {
            working_byte_offset++;  //incrementing past colon
            working_string_pointer = parse_data_string(file, working_byte_offset, event_end_byte_offset, ONELINE); //Parse line for DTSTART utc string
            if (working_string_pointer == NULL)
            {
                return -1; //Could not parse UTC time stamp
            }
            //transfering the start date into an int event date
            calendar_str_to_int(working_string_pointer, -1,'T', &event_date_code); //transfering the utc year|month|day into the event date code int
            //transfering the start time into an int event time
            calendar_str_to_int(&(working_string_pointer[9]), -1, 'Z', &event_time_code); //transfering the utc time code into the event date code int
            vPortFree(working_string_pointer);

            working_byte_offset = find_next_keyword(file, "DTEND:", working_byte_offset, event_end_byte_offset, NEXTCHAR);//Getting the end time
            if(working_byte_offset == EOF)
            {
                return -1;//Error during parsing of end time
            }
            else if(working_byte_offset == -2)
            {
                //WE could not find an end date for the event, edge case of 2 events within ical, event has zero-width same end as start
                event_date_end_code = event_date_code;
                event_time_end_code = event_time_code;
            }
            else
            {
                //Found a utc end time for the event
                working_string_pointer = parse_data_string(file, working_byte_offset, event_end_byte_offset, ONELINE); //Parse line for DTEND: utc stamp
                if (working_string_pointer == NULL)
                {
                    return -1; //Found but could not parse the utc end time stamp
                }
                calendar_str_to_int(working_string_pointer, -1,'T', &event_date_end_code); //transfering the end utc year|month|day into the event end date code
                calendar_str_to_int(&(working_string_pointer[9]), -1, 'Z', &event_time_end_code);//transfering the end utc time into the event date code
                vPortFree(working_string_pointer);//Free utc end time stamp off heap
            }
        }
        //UTC TIMESTAMP------------------------------------------------------------------------------------------------------------------------------------------------------------

        //TZID/LOCAL TIMESTAMP-----------------------------------------------------------------------------------------------------------------------------------------------------
        else if (char_temp == ';')   //local time stamp or TZID time stamp
        {
            int time_offset = 0; //offset of local time compared to utc, we use utc timestamps to figure out 
            if (user_calendar->timezone.daylight_status == 1)//Does the calendar even have daylight savings info?
            {
                if(user_calendar->timezone.daylight_mode == 1)//Are we in daylight savings?
                {
                    time_offset = user_calendar->timezone.daylight_offset;//Yes we are, 
                }
                else
                {
                    time_offset = user_calendar->timezone.standard_offset;//No, or daylight savings mode was never specified hence the else and not else if ... 
                }
            }
            time_offset *=100; //since timeoffset is in hours, minutes but utc stamps are hours, minute, seconds need to shift two places left

            char_temp = file->read();//Look for either ;VALUE=... or ;TZID=....
            //LOCAL TIMESTAMP------------------------------------------------------------------------------------------------------------------------------------------------------
            if(char_temp == 'V')
            {
                //Event has a local time must add offset to get UTC time
                working_byte_offset+=12; //Moving from the ; in DTSTART;VALUE=DATE:1.. to the 1
                working_string_pointer = parse_data_string(file, working_byte_offset, event_end_byte_offset, ONELINE); //Parse line for the start time stamp 
                if (working_string_pointer == NULL)
                {
                    return -1;//Error during search start time parse
                }
                //This format of datestamp only had a date no time, used for edge 248 cases
                calendar_str_to_int(working_string_pointer, -1, '\0', &event_date_code); //transfering the datestamp year|month|day into event date stamp
                event_time_code = 0;//No time stamp with this format set to 0
                vPortFree(working_string_pointer); //freeing the start date stamp

                //Note no time stamp, we assume time = 00:00.00 plus the offset in regards to utc, so same day by ahead if local is behind, behind if local is ahead
                if (time_offset <= 0)//Check if negative
                {
                    event_time_code = (-1 * time_offset);//Local time is behind so utc equivalent is offset ahead
                }
                else
                {
                    event_date_code -=1;    //subtract a day since local time is ahead of utc
                    event_time_code = 235959 - time_offset;
                }

                working_byte_offset = find_next_keyword(file, "DTEND;VALUE=DATE:", working_byte_offset, event_end_byte_offset, NEXTCHAR);//Grab end date
                if(working_byte_offset == EOF)
                {
                    return -1;//Error during search for start time
                }
                else if(working_byte_offset == -2)
                {
                    //WE could not find an end date for the event, edge case of 2 events within ical, we same the event has zero-width
                    event_date_end_code = event_date_code;
                    event_time_end_code = event_time_code;
                }
                else
                {
                    //Found an end date for the event
                    working_string_pointer = parse_data_string(file, working_byte_offset, event_end_byte_offset, ONELINE); //Parse line for end date
                    if (working_string_pointer == NULL)
                    {
                        return -1;//Failure to parse end time
                    }
                    //transfering the end date into an int for comparison between now and the event date
                    calendar_str_to_int(working_string_pointer, -1, '\0', &event_date_end_code); //transfering the end datestamp year|month|day into event date stamp
                    event_time_end_code = 0;//No time stamp with this format set to 0
                    vPortFree(working_string_pointer);

                    //Note no time stamp, we assume time = 00:00.00 plus the offset in regards to utc, so same day by ahead if local is behind, behind if local is ahead
                    if (time_offset <= 0)//Check if negative
                    {
                        event_time_end_code = (-1 * time_offset);//Local time is behind so utc equivalent is offset ahead
                    }
                    else
                    {
                        event_date_end_code -=1;    //subtract a day since local time is ahead of utc
                        event_time_end_code = 235959 - time_offset;
                    }
                }
            }
            //LOCAL TIMESTAMP------------------------------------------------------------------------------------------------------------------------------------------------------

            //TZID TIMESTAMP-------------------------------------------------------------------------------------------------------------------------------------------------------
            else if (char_temp == 'T')//TZID, check if the same as calendar, add offset based on utc offset in calendar struc
            {
                working_byte_offset+=6;//Need to move byte offset from ; to Amer.. in DTSTART;TZID=AMERICA/EDMONTON:
                extra_byte_offset = find_next_keyword(file, ":", working_byte_offset, event_end_byte_offset, NEXTCHAR);
                if(extra_byte_offset < 0)
                {
                    return -1;//Failure to find TZID colon
                }
                else
                {
                    //grabbing time zone id string, extra is decremented one since it is the byte offset after the colon not at the colon
                    working_string_pointer = parse_data_string(file, working_byte_offset, (-1 + extra_byte_offset), ONELINE);  //looking for the tzid string in DTSTART;TZID= "America/Edmonton" :20210215T000000
                    if (working_string_pointer == NULL)
                    {
                        return -1;//Failure to parse TZID
                    }

                    for(int i = 0; working_string_pointer[i] != '\0'; i++)
                    {
                        if (working_string_pointer[i] != user_calendar->timezone.time_zone_id[i])
                        {
                            vPortFree(working_string_pointer);//freeing the TZID string found
                            return -1;//Timezone does not match the calendar timezone so no idea what to do with it
                        }
                    }
                    vPortFree(working_string_pointer);  //freeing the heap timezoneid

                    working_string_pointer = parse_data_string(file, extra_byte_offset, event_end_byte_offset, ONELINE);//parseing the timestamp after the :
                    if (working_string_pointer == NULL)
                    {
                        return -1;//Failure to parse event date stamp
                    }
                    //pulling out the date and time stuff
                    calendar_str_to_int(working_string_pointer, -1, 'T', &event_date_code);//grabbing date stamp code into year|month|day into event date stamp
                    calendar_str_to_int(&(working_string_pointer[9]), -1, '\0', &event_time_code); //transfering the time stamp hour|minute|second into event time stamp, it isnt Z terminated like UTC
                    vPortFree(working_string_pointer);  //freeing the heap timezoneid string

                    //Adjust time offset
                    if (time_offset <= 0)//Check if negative
                    {
                        event_time_code += (-1 * time_offset);//Local time is behind so utc equivalent is offset ahead
                    }
                    else
                    {
                        event_date_code -=1;    //subtract a day since local time is ahead of utc
                        event_time_code = 235959 - time_offset;
                    }
                    //Grabbing end time
                    working_byte_offset = find_next_keyword(file, "DTEND;TZID=", working_byte_offset, event_end_byte_offset, NEXTCHAR);//Check end date
                    if(working_byte_offset == EOF)
                    {
                        return -1;//Failure to find END time
                    }
                    else if(working_byte_offset == -2)
                    {
                        //WE could not find an end date for the event, edge case of 2 events within ical, we same the event has zero-width
                        event_date_end_code = event_date_code;
                        event_time_end_code = event_time_code;
                    }
                    else
                    {
                        working_byte_offset = find_next_keyword(file, ":", working_byte_offset, event_end_byte_offset, NEXTCHAR);//Jump to colon past know TZID
                        if (working_byte_offset < 0)
                        {
                            return -1; //Failure to find end time TZID stamp
                        }
                        //Found an end date for the event
                        working_string_pointer = parse_data_string(file, working_byte_offset, event_end_byte_offset, ONELINE); //Parse line for end date
                        if (working_string_pointer == NULL)
                        {
                            return -1;//Failure to parse end date string
                        }
                        //transfering the end date into end time/date ints
                        calendar_str_to_int(working_string_pointer, -1, 'T', &event_date_end_code);    //grabbing date stamp code into year|month|day into event end date stamp
                        //pulling out the time stuff
                        calendar_str_to_int(&(working_string_pointer[9]), -1, '\0', &event_time_end_code); //transfering the time stamp hour|minute|second into event end time stamp, it isnt Z terminated like UTC
                        vPortFree(working_string_pointer);

                        //Adjusting end time based on calendar timezone offsets
                        if (time_offset <= 0)//Check if negative
                        {
                            event_time_end_code += (-1 * time_offset);//Local time is behind so utc equivalent is offset ahead
                        }
                        else
                        {
                            event_date_end_code -=1;    //subtract a day since local time is ahead of utc
                            event_time_end_code = 235959 - time_offset;
                        }
                    }
                }
            }
            //TZID TIMESTAMP-------------------------------------------------------------------------------------------------------------------------------------------------------
            else
            {
                return -1; //Not local nor TZID who knows?
            }
            //TZID/LOCAL TIMESTAMP-----------------------------------------------------------------------------------------------------------------------------------------------------
        }
        else
        {   
            return -1; //Event dates are not parsable
        }
    }
    //Assigning values
    *event_start_date = event_date_code;
    *event_start_time = event_time_code;
    *event_end_date = event_date_end_code; 
    *event_end_time = event_time_end_code;

    return 0;//Success!
}


byte initialize_sector_table(File *file, Calendar *user_calendar, const int current_date_code, const int current_time_code, long *sector_table)
{
    byte return_value_code = 0;  //is the returned error value
    long working_byte_offset = 0;//Used to find keyword locally

    long event_start_byte_offset = 0;//THe focused event start byte, first char of BEGIN:VEVENT
    long event_end_byte_offset = 0;//The focused event end byte, the line after END:VEVENT

    int event_start_date = 0;//The date code of the event currently being looked at
    int event_start_time = 0;//The time code of the event currently being looked at

    int event_end_date = 0;//The date code of the event currently being looked at
    int event_end_time = 0;//The time code of the event currently being looked at


    int event_counter = 0;//Counts the events encountered//for debugging purposes
    int sector_count_max = SECTORTABLESIZE;//Maximum number of elements within the sector table
    int sector_count = 0;//The current element index of the sector table

    const long file_end = (long)(file->size()-14);//End of the file using size instead of keyword parse 1000X faster, -14 since END:VCALENDAR is 14 in length with CR-LF sequence


    //Need to wipe values of sector_table and write 0's
    for(int i = 0; i < SECTORTABLESIZE; i++)
    {
        sector_table[i] = 0;//Setting all sector table values to zero
    }

    while(sector_count < sector_count_max)
    {
        working_byte_offset = find_next_keyword(file, "BEGIN:VEVENT", event_end_byte_offset, file_end, FIRSTCHAR);
        if(working_byte_offset < 0)//Return of error or -1 implies end of files or no event left in file
        {
            break;//Found all events
        }
        else
        {
            //found event parse end
            event_start_byte_offset = working_byte_offset; //setting current event start to the found event start in working_byte_offset
            event_end_byte_offset = find_next_keyword(file, "END:VEVENT", event_start_byte_offset, file_end, NEXTLINE);//find end of event
            if(event_end_byte_offset < 0)
            {
                return -1; //error during parsing of event end
            }

            if(!fetch_event_time(file, user_calendar, event_start_byte_offset, &event_start_date, &event_start_time, &event_end_date, &event_end_time))//get the event times
            {
                int tiny_event_state = 0;//1 for a zero-duration event, 0 for an event with a duration
                int logic_state = 0;//used since it's easier to set a logic bit then structure a proper if else, continue structure
                if((event_start_time == event_end_time) && (event_start_date == event_end_date))
                {
                    //The event ends/starts at the same time, perhaps because it's an alarm or if its a local date stamp only
                    tiny_event_state = 1;
                }
                if(!tiny_event_state)
                {
                    //okay the event has a length and is not infinitesimal in duration
                    //harder to figure out since event that have started but haven't ended are still useful to know about
                    if(event_start_date < current_date_code)//has the event date happened or is it now
                    {
                        //okay event start date has occured/occuring but has the end date passed already?
                        if(event_end_date < current_date_code)
                        {
                            //end date has passed too
                            logic_state = 1;
                        }
                        else if(event_end_date == current_date_code)//is the end date today?
                        {
                            //okay the end date is today but, has the end time passed?
                            if(event_end_time <= current_time_code)
                            {
                                logic_state = 1;//Event end time has passed today/rn :(
                            }
                        }
                    }
                    else if(event_start_date == current_date_code)
                    {
                        if(event_start_time <= current_time_code)
                        {
                            if(event_end_date < current_date_code)
                            {
                                //end date has passed too
                                logic_state = 1;
                            }
                            else if(event_end_date == current_date_code)//is the end date today?
                            {
                                //okay the end date is today but, has the end time passed?
                                if(event_end_time <= current_time_code)
                                {
                                    logic_state = 1;//Event end time has passed today/rn :(
                                }
                            }
                        }
                    }
                }
                else if(tiny_event_state)
                {
                    //the event is infinitesimal in duration
                    //only need to check the start time/date since they are the same
                    if(event_start_date < current_date_code)
                    {
                        //event start and end, both the same, has passed 
                        logic_state = 1;//
                    }
                    else if (event_start_date == current_date_code)
                    {
                        //Event is today, but what time today
                        if(event_start_time <= current_time_code)
                        {
                            //okay start/end time has happened already/rn
                            logic_state = 1;
                        }
                    }
                }
                if(!logic_state)
                {
                    event_counter++;
                    //okay the event is relevant (hasnt begun or is ongoing currently)
                    //We must add it to the sector table

                    if (sector_count < (sector_count_max-2))//Is there more than 1 pair (2 elements) left in the sector table?
                    {
                        //checking if last sector offset is the begining of this sectors offset i.e. a continous region of byte offsets
                        if ((sector_count != 0) && (event_start_byte_offset == sector_table[(-1 + sector_count)]))
                        {
                            //The last sector's end is the same as this sector's start so we can replace the last sector's end with this sector's end since its a continous byte-offset region
                            sector_table[(-1 + sector_count)] = event_end_byte_offset;
                            //no need to increment the sector count since technically no new elements were added
                        }
                        else
                        {
                            sector_table[sector_count++] = event_start_byte_offset;//Set current sector table element to start of focused, relevant event
                            sector_table[sector_count++] = event_end_byte_offset;//Set related pair element to end of focused, relevant event
                        }

                    }
                    else//only one sector element pair left, we need to encompass rest of file incase there are more relevant events//please dont try using this since find_event must then handle bad dates (who knows)
                    {
                        if(event_start_byte_offset == sector_table[(-1 + sector_count)])//checking to see if the begining of this sector and end of the last sector is continous
                        {
                            sector_table[(-1 + sector_count)] = NOEND;//Since the start and end are the same 
                            sector_count+=2;// Adding 2 to sector count to prevent the function from writing sector byte offsets after NOEND element
                        }
                        sector_table[sector_count++] = event_start_byte_offset;
                        sector_table[sector_count++] = NOEND;   //Since there 1 pair left in sector table we set the last pair of sector bytes to the start of the current relevant event and the end of the file,
                                                                //Note that NOEND is that acutal end of file but, the find_keyword functions and the parsing functions treat NOEND as the literal end of 2GiB file but typicall they double check for EOF
                        break;
                    }
                }
            }
            else
            {
                continue;//event time could not be parsed we assume unrecognized formatting so irrelevant anyways, could be error tho
            }
        }
    }
    if(Serial)//debugging
    {
        Serial.print("Found this many events in the file that are relevant: ");
        Serial.println(event_counter);
    }
    return return_value_code;
}


void update_sector_table(long *sector_table, const long event_start, const long event_end)
{
    long temp_copy[SECTORTABLESIZE];//A copy of the original use for editing locally
    //zero out temp
    for(int i = 0; i < SECTORTABLESIZE; i++)
    {
        temp_copy[i] = 0;
    }

    int top_slice_index = 0;
    int bottom_slice_index = 0;
    for(int i = 0; i < (SECTORTABLESIZE/2);i+=2)//look for the sector slice that the event is in/bounded on
    {
        if((sector_table[i] <= event_start) && (sector_table[(i + 1)] >= event_end))
        {
            top_slice_index = i;
            bottom_slice_index = (i + 1);
        }
    }

    if(sector_table[top_slice_index] == event_start)
    {
        if(sector_table[bottom_slice_index] == event_end)
        {
            //The event start and end were already sector bounds, just need to remove them from array and shorten it
            for(int i = 0; i < top_slice_index; i++)
            {
                temp_copy[i] = sector_table[i];
            }
            for(int i = top_slice_index; i < (SECTORTABLESIZE-2); i++)
            {
                temp_copy[i] = sector_table[i+2];///since temp is now 2 elements shorter
            }
        }
        else
        {
            //Bottom slice is less than the next sector start bound so we can replace the starting sector bound with the event end sector bound
            for(int i = 0; i < top_slice_index; i++)
            {
                temp_copy[i] = sector_table[i];
            }
            temp_copy[top_slice_index] = event_end;//Copy event end into original sector start
            for(int i = bottom_slice_index; i < SECTORTABLESIZE; i++)
            {
                temp_copy[i] = sector_table[i];
            }
        }
    }
    else
    {
        if(sector_table[bottom_slice_index] == event_end)
        {
            //The event start was not a sector bound but the end was so replace original sector end with event start
            for(int i = 0; i < bottom_slice_index; i++)
            {
                temp_copy[i] = sector_table[i];
            }
            temp_copy[bottom_slice_index] = event_start;
            for(int i = (1 + bottom_slice_index); i < SECTORTABLESIZE; i++)
            {
                temp_copy[i] = sector_table[i];
            }
        }
        else
        {
            //Event is not bounded on any sector so a new sector must be made(basically cutting in half the old sector)
            for(int i = 0; i <= top_slice_index; i++)
            {
                temp_copy[i] = sector_table[i];
            }
            temp_copy[bottom_slice_index] = event_start;

            //making another element
            temp_copy[(1 + bottom_slice_index)] = event_end;//new sector start
            for(int i = (2 + bottom_slice_index); i < SECTORTABLESIZE; i++)
            {
                temp_copy[i] = sector_table[i-2];
            }
            //Data might be lost so checking end of sector_table to see if was full
            if(sector_table[SECTORTABLESIZE-1] == NOEND)
            {
                temp_copy[SECTORTABLESIZE-1] = NOEND; //setting the end to NOEND so even though a sector is lost we can still access all relevant data, altough slower
            }
        }
    }
    //copying temp into the actual sector table
    for(int i = 0; i < SECTORTABLESIZE; i++)
    {
        sector_table[i] = temp_copy[i];
    }
}


void calendar_str_print(const char * char_array)
{
    //This is used as a debugging function to print string data to the serial console to test features and debug processes
    if(!Serial)    //check if serial port is available, i.e. forgot to edit out debug code elsewhere and this code is called on un-plugged TTgo
    {
        return;
    }

    #ifdef PRINTOUTDEBUG
        for (int i = 0; char_array[i] != '\0'; i++)
        {
        
            if(char_array[i] == '\t')//Replacing real text horizontal tabs with \t sequence to better recognize function return strings
            {
                Serial.print('\\');
                Serial.print('t');
            }
            else if(char_array[i] == ' ')//Replacing real text spaces with underscores to better recognize function return strings
            {
                Serial.print('_');
            }
            else
            {
                Serial.print(char_array[i]);
            }
        }
        Serial.print('\n');
    #endif  // #ifdef ICAL_LIBARY_H

    #ifndef PRINTOUTDEBUG
        for (int i = 0; char_array[i] != '\0' ; i++)
        {
            Serial.print(char_array[i]);
        }
        Serial.print('\n');
    #endif  // #ifndef ICAL_LIBARY_H
}


void calendar_str_to_int(const char * str_num, int num_length,  const char end_char, int *int_pointer)
{
    char limiter = '\0';  //default null termination value
    if (num_length == -1)//if the num length is not specified then we assume that the length is the entire string until the null character or end_char is reached
    {
        if(end_char != '\0')
        {
            for(num_length = 0; str_num[num_length] != '\0'; num_length++)
            {
                if(str_num[num_length] == end_char)    //checking to see if the *end_char occurs before the string's null termination
                {
                    limiter = end_char; //setting limiter equal to the *end_char since it occurs before the Null termination
                    break;  
                }
            } 
        }
        else
        {
            for(num_length = 0; str_num[num_length] != '\0'; num_length++);
        }
    }
    int temp_num = 0;
    int temp_exponent = 0;
    int i = 0;
    if(str_num[0] == '-')
    {
        i++;
    }
    while((str_num[i] != limiter) && (i < num_length))  
    {

        temp_exponent = 1;
        for(int j = (i+1); j < num_length; j++)
        {
            temp_exponent*=10;   //This is for the base 10 conversion
        }
        temp_num += (temp_exponent * ((int)str_num[i] - 48));    //ascii offset of 48 needed
        i++;    //increment i
    }
    if(str_num[0] == '-')
    {
        temp_num *=-1;  //number was negative
    }

    *int_pointer = temp_num;  //Change the pointer's content 
}


void calendar_char_copy(const char * str, char *dest)
{
    //Used to stick a str into a character array
    int i = 0;
    while(str[i] != '\0')
    {
        dest[i] = str[i];
        i++;
    }
    dest[i] = '\0'; //Null terminating character array
}


void print_event(CalendarEvent *user_event)
{
    if (Serial)
    {
        calendar_str_print(user_event->event_summary);
        Serial.print("\tLocated at: ");
        calendar_str_print(user_event->event_location);
        Serial.print("\tWith an alarm status of: ");
        Serial.println(user_event->alarm_status);
        if ((user_event->alarm_status) == 0)
        {
            Serial.print("\tStart date code of: ");
            Serial.println(user_event->event_start_date_code); //Not useful for humans to read

            Serial.print("\tThe event is on: ");
            Serial.print(user_event->event_start_year, DEC);
            Serial.print("/");
            Serial.print(user_event->event_start_month, DEC);
            Serial.print("/");
            Serial.println(user_event->event_start_day, DEC);

            Serial.print("\tStart time code of: ");
            Serial.println(user_event->event_start_time_code);  //Not useful for humans to read

            Serial.print("\t\tAt time of: ");
            Serial.print(user_event->event_start_hour, DEC);
            Serial.print(":");
            Serial.print(user_event->event_start_minute, DEC);
            Serial.print(".");
            Serial.println(user_event->event_start_second, DEC);

            Serial.print("\tEnd date code of: ");
            Serial.println(user_event->event_end_date_code); //Not useful for humans to read

            Serial.print("\tThe event ends on: ");
            Serial.print(user_event->event_end_year, DEC);
            Serial.print("/");
            Serial.print(user_event->event_end_month, DEC);
            Serial.print("/");
            Serial.println(user_event->event_end_day, DEC);

            
            Serial.print("\tEnd time code of: ");
            Serial.println(user_event->event_end_time_code);  //Not useful for humans to read

            Serial.print("\t\tAt time of: ");
            Serial.print(user_event->event_end_hour, DEC);
            Serial.print(":");
            Serial.print(user_event->event_end_minute, DEC);
            Serial.print(".");
            Serial.println(user_event->event_end_second, DEC);
        }
        else
        {
            Serial.print("\tAlarm date code of: ");
            Serial.println(user_event->event_start_date_code); //Not useful for humans to read

            Serial.print("\tThe alarm is on: ");
            Serial.print(user_event->event_start_year, DEC);
            Serial.print("/");
            Serial.print(user_event->event_start_month, DEC);
            Serial.print("/");
            Serial.println(user_event->event_start_day, DEC);

            Serial.print("\tAlarm time code of: ");
            Serial.println(user_event->event_start_time_code);  //Not useful for humans to read

            Serial.print("\t\tAt time of: ");
            Serial.print(user_event->event_start_hour, DEC);
            Serial.print(":");
            Serial.print(user_event->event_start_minute, DEC);
            Serial.print(".");
            Serial.println(user_event->event_start_second, DEC);
        }
    }
}


void print_calendar(Calendar *user_calendar)
{
    if (Serial)
    {
        Serial.print("Calendar Name: ");
        calendar_str_print(user_calendar->agenda_name);
        Serial.print("\tTimezone of Calendar: ");
        calendar_str_print(user_calendar->timezone.time_zone_id);

        if(user_calendar->timezone.daylight_status == 1)//Do we have timezone information?
        {
            Serial.print("\tThe calendar's daylight savings is in timezone: ");
            calendar_str_print(user_calendar->timezone.daylight_time_zone);
            Serial.print("\tHaving an offset of: ");
            Serial.println(user_calendar->timezone.daylight_offset);

            Serial.print("\tThe calendar's standard time is in timezone: ");
            calendar_str_print(user_calendar->timezone.standard_time_zone);
            Serial.print("\tHaving an offset of: ");
            Serial.println(user_calendar->timezone.standard_offset);
        }

        if (user_calendar->event_intialization == 1)//Do we have events?
        {
            for(int i = 0; (i < EVENTSTACKSIZE) && (user_calendar->jobs[i] != NULL); i++)
            {
                Serial.print("\tWith an event of: ");
                print_event(user_calendar->jobs[i]);
            }
        } 

    }
}


void update_calendar_event(File *file, Calendar *user_calendar, long *sector_table, const int date_stamp, const int time_stamp)
{
    int focused_event_index = 0;
    for(int i = 0; i < EVENTSTACKSIZE; i++)
    {
        if(user_calendar->event_precedence[i] == 0) //Looking for the element with the minimum precedence (0)
        {
            focused_event_index = i;                //Grabbing the jobs index which has the event to remove/refresh
            break;
        }
    }
    
    long next_event = 0;    //byte offset value of the next event
    int logic_state = 0;    //To make coding easier for zach

    if(!find_event(file, user_calendar, sector_table, &next_event, date_stamp, time_stamp, 0xFF))
    {
        //need to initialize the event
        if(!initialize_event(file, user_calendar, user_calendar->jobs[focused_event_index], next_event))
        {
            logic_state = 1;    //Since we did find an event and we did initialze it were good
            if(Serial)
            {
                Serial.println("An event was replaced with a new event");
            }
        }
    }
    
    if(!logic_state)
    {
        vPortFree(user_calendar->jobs[focused_event_index]);            //We deallocate the heap space that the event is contained in (hopefull it was heap allocated)??  
        user_calendar->jobs[focused_event_index] = NULL;                //Since we could not find another event we clearly set the jobs element pointer to NULL
        user_calendar->event_precedence[focused_event_index] = -1;     //And we also set the event precendence to -1 since there are no more events to put here
    }

    for(int i = 0; i < EVENTSTACKSIZE; i++)
    {
        if(user_calendar->event_precedence[i] == 0)
        {
            user_calendar->event_precedence[i] == EVENTSTACKSIZE-1;
        }
        else if(user_calendar->event_precedence[i] == -1)
        {
            //Do nothing since this event can't be used anyway
        }
        else
        {
            user_calendar->event_precedence[i] = user_calendar->event_precedence[i] - 1;//event thing gets shift down one
        }
    }
}