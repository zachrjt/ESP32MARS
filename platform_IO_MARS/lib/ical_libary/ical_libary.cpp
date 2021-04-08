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
        max_byte_offset = 2147483647;   //maximum value possible
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


byte initialize_event(File *file, CalendarEvent *user_event, ICALOFFSET const long event_byte_offset)
{
    //This function fills events that are passed into it from data found at the event_byte_offset
    if(event_byte_offset == EOF)
    {
        return -1;
    }
    const long max_event_byte_offset = find_next_keyword(file, "END:VEVENT", ICALOFFSET event_byte_offset, ICALOFFSET -1, ICALMODERTN FIRSTCHAR); //Find end of event
    if(max_event_byte_offset < 0)
    {
        return -1;  //Error or failure to find end of event, returning error code
    }
    //Grabing summary section------------------------------------------------------------------------------------------------------------------------------------------------------
    long working_byte_offset = 0;//Used to find keyword locally
    long extra_byte_offset = 0;//Extra one used to find keywords locally
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
    //Grabing the start
    working_byte_offset = find_next_keyword(file, "DTSTART:", ICALOFFSET event_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);  //looking for DTSTART string
    if(working_byte_offset == EOF)
    {
        return -1;  //Critical Error during search for DTSTART:
    }
    else if(working_byte_offset == -2)//could not find DTSTART:, likely because non-utc usage look for DTSTART;VALUE=DATE:
    {
        working_byte_offset = find_next_keyword(file, "DTSTART;VALUE=DATE:", ICALOFFSET event_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);//look for DTSTART;..
        if(working_byte_offset == EOF)
        {
            return -1;  //Critical Error during search for DTSTART;VALUE=DATE
        }
        else if(working_byte_offset == -2) //Could not find DTSTART;VALUE=, trying DTSTART;TZID=
        {
            working_byte_offset = find_next_keyword(file, "DTSTART;TZID=", ICALOFFSET event_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);  //looking for DTSTART;TZID= string
            if(working_byte_offset < 0)
            {
                return -1;  //Critical Error during search for DTSTART;TZID= or could not find, might be other format that I'm not considering
            }
            else
            {
                //So DTSTART;TZID=
                user_event->date_format = 2;    //Setting date format state to 2 since its a TZID time stamp
                extra_byte_offset = find_next_keyword(file, ":", ICALOFFSET working_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);
                if(extra_byte_offset < 0)
                {
                    return -1; //Error while trying to search for timezone id UTC stamp should be there
                }
                else
                {
                    //grabbing time zone id string, extra is decremented one since it is the byte offset after the colon not at the colon
                    working_string_pointer = parse_data_string(file, working_byte_offset, (-1 + extra_byte_offset), ONELINE);  //looking for the tzid string in DTSTART;TZID= "America/Edmonton" :20210215T000000
                    if (working_string_pointer == NULL)
                    {
                        return -1;  //Failure could not parse time zone id string
                    }
                    calendar_char_copy(working_string_pointer, user_event->event_time_zone_id);//copying the heap timezoneid into the struc timezone field
                    vPortFree(working_string_pointer);  //freeing the heap timezoneid

                    //grabbing the utc timestamp after the timezone id
                    working_string_pointer = parse_data_string(file, extra_byte_offset, NOEND, ONELINE);  //grabing the timestamp in DTSTART;TZID=America/Edmonton: 20210215T000000
                    if (working_string_pointer == NULL)
                    {
                        return -1;  //Failure could not parse timestamp from TZID line
                    }
                    //pulling out the date stuff
                    calendar_str_to_int(working_string_pointer, -1, 'T', &user_event->event_start_date_code);    //grabbing date stamp code into year|month|day into event struc
                    calendar_str_to_int(working_string_pointer, 4,'\0', &user_event->event_start_year); //transfering the year
                    calendar_str_to_int(&(working_string_pointer[4]), 2,'\0', &user_event->event_start_month); //transfering the month
                    calendar_str_to_int(&(working_string_pointer[6]), -1,'T', &user_event->event_start_day); //transfering the day

                    //pulling out the time stuff
                    calendar_str_to_int(&(working_string_pointer[9]), -1, '\0', &user_event->event_start_time_code); //transfering the time stamp hour|minute|second into event struc
                    calendar_str_to_int(&(working_string_pointer[9]), 2,'\0', &user_event->event_start_hour); //transfering the hour
                    calendar_str_to_int(&(working_string_pointer[11]), 2,'\0', &user_event->event_start_minute); //transfering the minute
                    calendar_str_to_int(&(working_string_pointer[13]), -1,'\0', &user_event->event_start_second); //transfering the second  UNLIKE UTC TIME STAMP NOT 'Z' terminated
                    vPortFree(working_string_pointer);  //freeing the heap timezoneid
                }   
            }
        }
        else
        {
            //So DTSTART;VALUE= we assume that the event starts on the date at 00:00.00 time
            user_event->date_format = 1;    //Setting date format state to 1
            calendar_char_copy("LOCAL", user_event->event_time_zone_id);//copying the mode 1 : "Date" into the struc timezone field
            working_string_pointer = parse_data_string(file, working_byte_offset, NOEND, ONELINE); //Parse line for the date stamp
            if (working_string_pointer == NULL)
            {
                return -1;  //Failure could not non-utc date stamp
            }
            //This format of datestamp only had a date no time, used for edge 248 cases
            calendar_str_to_int(working_string_pointer, -1, '\0', &user_event->event_start_date_code); //transfering the datestamp year|month|day into event struc
            calendar_str_to_int(working_string_pointer, 4,'\0', &user_event->event_start_year); //transfering the year into the event struc
            calendar_str_to_int(&(working_string_pointer[4]), 2,'\0', &user_event->event_start_month); //transfering the year into the event struc
            calendar_str_to_int(&(working_string_pointer[6]), -1,'\0', &user_event->event_start_day); //transfering the year into the event struc
            vPortFree(working_string_pointer); //freeing the date stamp off the heap
        }
    }
    else//DTSTART was UTC so we can parse info much more easily
    {
        user_event->date_format = 0;    //Normal utc code setting time format byte to 0
        calendar_char_copy("UTC", user_event->event_time_zone_id);    //Since UTC copying UTC string into the time zone id string
        working_string_pointer = parse_data_string(file, working_byte_offset, NOEND, ONELINE); //Parse line for DTSTART utc string
        if (working_string_pointer == NULL)
        {
            return -1;  //Failure could not parse event start utc string  20210326 T 211332 Z
        }
        //transfering the start date
        calendar_str_to_int(working_string_pointer, -1,'T', &user_event->event_start_date_code); //transfering the utc year|month|day into event struc
        calendar_str_to_int(working_string_pointer, 4,'\0', &user_event->event_start_year); //transfering the year
        calendar_str_to_int(&(working_string_pointer[4]), 2,'\0', &user_event->event_start_month); //transfering the month
        calendar_str_to_int(&(working_string_pointer[6]), -1,'T', &user_event->event_start_day); //transfering the day

        //transfering the start time
        calendar_str_to_int(&(working_string_pointer[9]), -1, 'Z', &user_event->event_start_time_code); //transfering the utc hour|minute|second into event struc
        calendar_str_to_int(&(working_string_pointer[9]), 2,'\0', &user_event->event_start_hour); //transfering the hour
        calendar_str_to_int(&(working_string_pointer[11]), 2,'\0', &user_event->event_start_minute); //transfering the minute
        calendar_str_to_int(&(working_string_pointer[13]), -1,'Z', &user_event->event_start_second); //transfering the second
        vPortFree(working_string_pointer);  //freeing DTSTART line from heap
    }

    //grabing the end
    working_byte_offset = find_next_keyword(file, "DTEND:", ICALOFFSET event_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);  //looking for DTEND string
    if(working_byte_offset == EOF)
    {
        return -1;  //Critical Error during search for DTEND:
    }
    else if(working_byte_offset == -2)//could not find DTEND:, likely because non-utc usage look for DTEND;VALUE=DATE:
    {
        working_byte_offset = find_next_keyword(file, "DTEND;VALUE=DATE:", ICALOFFSET event_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);//look for DTSTART;..
        if(working_byte_offset == EOF)
        {
            return -1;  //Critical Error during search for DTEND;VALUE=DATE
        }
        else if(working_byte_offset == -2) //Could not find DTEND;VALUE=, trying DTEND;TZID=
        {
            working_byte_offset = find_next_keyword(file, "DTEND;TZID=", ICALOFFSET event_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);  //looking for DTEND;TZID= string
            if(working_byte_offset == EOF)
            {
                return -1;  //Critical Error during search for DTEND;TZID= 
            }
            else if(working_byte_offset == -2)
            {
                //For some reason 2 events dont have a end time, if end time is zero consider it to be an event of infinsimal duration, and say the end time is the same as the start
                user_event->alarm_status = 1;
                //Copying the start times into the end times
                user_event->event_end_date_code = user_event->event_start_date_code;
                user_event->event_end_year = user_event->event_start_year;
                user_event->event_end_month = user_event->event_start_month;
                user_event->event_end_day = user_event->event_start_day;

                user_event->event_end_time_code = user_event->event_start_time_code;
                user_event->event_end_hour = user_event->event_start_hour;
                user_event->event_end_minute = user_event->event_start_minute;
                user_event->event_end_second = user_event->event_start_second;
            }
            else
            {
                //So DTEND;TZID=
                user_event->date_format = 2;    //Setting date format state to 2 since its a TZID time stamp
                extra_byte_offset = find_next_keyword(file, ":", ICALOFFSET working_byte_offset, ICALOFFSET max_event_byte_offset, ICALMODERTN NEXTCHAR);
                if(extra_byte_offset < 0)
                {
                    return -1; //Error while trying to search for timezone id UTC stamp should be there
                }
                else
                {
                    //grabbing time zone id string, extra is decremented one since it is the byte offset after the colon not at the colon
                    working_string_pointer = parse_data_string(file, working_byte_offset, (-1 + extra_byte_offset), ONELINE);  //looking for the tzid string in DTEND;TZID= "America/Edmonton" :20210215T000000
                    if (working_string_pointer == NULL)
                    {
                        return -1;  //Failure could not parse time zone id string
                    }
                    calendar_char_copy(working_string_pointer, user_event->event_time_zone_id);//copying the heap timezoneid into the struc timezone field
                    vPortFree(working_string_pointer);  //freeing the heap timezoneid

                    //grabbing the utc timestamp after the timezone id
                    working_string_pointer = parse_data_string(file, extra_byte_offset, NOEND, ONELINE);  //grabing the timestamp in DTEND;TZID=America/Edmonton: 20210215T000000
                    if (working_string_pointer == NULL)
                    {
                        return -1;  //Failure could not parse timestamp from TZID line
                    }
                    //pulling out the date stuff
                    calendar_str_to_int(working_string_pointer, -1, 'T', &user_event->event_end_date_code);    //grabbing date stamp code into year|month|day into event struc
                    calendar_str_to_int(working_string_pointer, 4,'\0', &user_event->event_end_year); //transfering the year
                    calendar_str_to_int(&(working_string_pointer[4]), 2,'\0', &user_event->event_end_month); //transfering the month
                    calendar_str_to_int(&(working_string_pointer[6]), -1,'T', &user_event->event_end_day); //transfering the day

                    //pulling out the time stuff
                    calendar_str_to_int(&(working_string_pointer[9]), -1, '\0', &user_event->event_end_time_code); //transfering the time stamp hour|minute|second into event struc
                    calendar_str_to_int(&(working_string_pointer[9]), 2,'\0', &user_event->event_end_hour); //transfering the hour
                    calendar_str_to_int(&(working_string_pointer[11]), 2,'\0', &user_event->event_end_minute); //transfering the minute
                    calendar_str_to_int(&(working_string_pointer[13]), -1,'\0', &user_event->event_end_second); //transfering the second  UNLIKE UTC TIME STAMP NOT 'Z' terminated
                    vPortFree(working_string_pointer);  //freeing the heap timezoneid
                }   
            }
        }
        else
        {
            //So DTSTART;VALUE= we assume that the event starts on the date at 00:00.00 time
            user_event->date_format = 1;    //Setting date format state to 1
            calendar_char_copy("LOCAL", user_event->event_time_zone_id);//copying the mode 1 : "Date" into the struc timezone field
            working_string_pointer = parse_data_string(file, working_byte_offset, NOEND, ONELINE); //Parse line for the date stamp
            if (working_string_pointer == NULL)
            {
                return -1;  //Failure could not non-utc date stamp
            }
            //This format of datestamp only had a date no time, used for edge 248 cases
            calendar_str_to_int(working_string_pointer, -1, '\0', &user_event->event_end_date_code); //transfering the datestamp year|month|day into event struc
            calendar_str_to_int(working_string_pointer, 4,'\0', &user_event->event_end_year); //transfering the year into the event struc
            calendar_str_to_int(&(working_string_pointer[4]), 2,'\0', &user_event->event_end_month); //transfering the year into the event struc
            calendar_str_to_int(&(working_string_pointer[6]), -1,'\0', &user_event->event_end_day); //transfering the year into the event struc
            vPortFree(working_string_pointer); //freeing the date stamp off the heap
        }
    }
    else//DTENDwas UTC so we can parse info much more easily
    {
        user_event->date_format = 0;    //Normal utc code setting time format byte to 0
        calendar_char_copy("UTC", user_event->event_time_zone_id);    //Since UTC copying UTC string into the time zone id string
        working_string_pointer = parse_data_string(file, working_byte_offset, NOEND, ONELINE); //Parse line for DTSTART utc string
        if (working_string_pointer == NULL)
        {
            return -1;  //Failure could not parse event start utc string  20210326 T 211332 Z
        }
        //transfering the start date
        calendar_str_to_int(working_string_pointer, -1,'T', &user_event->event_end_date_code); //transfering the utc year|month|day into event struc
        calendar_str_to_int(working_string_pointer, 4,'\0', &user_event->event_end_year); //transfering the year
        calendar_str_to_int(&(working_string_pointer[4]), 2,'\0', &user_event->event_end_month); //transfering the month
        calendar_str_to_int(&(working_string_pointer[6]), -1,'T', &user_event->event_end_day); //transfering the day

        //transfering the start time
        calendar_str_to_int(&(working_string_pointer[9]), -1, 'Z', &user_event->event_end_time_code); //transfering the utc hour|minute|second into event struc
        calendar_str_to_int(&(working_string_pointer[9]), 2,'\0', &user_event->event_end_hour); //transfering the hour
        calendar_str_to_int(&(working_string_pointer[11]), 2,'\0', &user_event->event_end_minute); //transfering the minute
        calendar_str_to_int(&(working_string_pointer[13]), -1,'Z', &user_event->event_end_second); //transfering the second
        vPortFree(working_string_pointer);  //freeing DTEND line from heap
    }

    //Grabing times section--------------------------------------------------------------------------------------------------------------------------------------------------------
    
    //Checking the start and end time section--------------------------------------------------------------------------------------------------------------------------------------
    if(user_event->alarm_status == 0)   //If our alarm state (End and start times are identical) isnt already 1
    {
        if ((user_event->event_end_time_code) == (user_event->event_start_time_code))//Is our time codes the same?
        {
            if ((user_event->event_end_date_code) == (user_event->event_start_date_code))//Is our date codes the same
            {
                user_event->alarm_status = 1; //Since the end and start times are identical we say that it's an alarm
            }
        }
    }
    //Checking the start and end time section--------------------------------------------------------------------------------------------------------------------------------------
    return 0;
}


long *find_event(File *file, const long start_offset_byte, const long *read_sector_table, long date, long time, long tolerance)
{
    //This needs some basic operations and calls to find_event_limits
    //First needs the creation/function to make a read_sector_table 
        //Read sector table basically is a table of byte offset ranges that are valid to read from, it makes read the file after intialization muchhhhh easier
            //We wont need to re-read 150000 lines and any lines/events that have already happened can be skipped over and dont need to be parsed
    return 0;
}


byte initialize_sector_table(File *file, Calendar *user_calendar, const int current_date_code, const int current_time_code, long *sector_table)
{
    //This function should parse through the entire sd card file
    //While doing so it should examine each event and look at its start date stamps
    //If the date/time stamp is less than a current date/time stamp it should consider the event(the byte offset range between the BEGIN and END) to be irrelevant
    //These irrelevant byte offset ranges wont be in the in the sector table
    //Such that the sector table contains all the RELEVANT ranges of byte offsets in order that contain data/events that are after, or on the current date 
    //unfortunalety any reoccuring events might be discarded since I never, and will not, implemented RRULE parsing for reoccurance of date stamps
    //The sector table element are ranges that are valid between partners so: between [0] and [1] are good, but not between [1] and [2]
    //but between [2] and [3], so there are an even number of sector table oofset
    //If there is not enough room the last element pair should contain the next relevant byte-offset to the end of the file
    //As events are completed the sector table should be updated to remove the region that the event was enclosed in
    

    byte return_value_code = 0;  //is the returned error value
    long working_byte_offset = 0;//Used to find keyword locally
    long extra_byte_offset = 0;//An extra working byte offset to use

    long focused_event_start_byte_offset = 0;//THe focused event start byte, first char of BEGIN:VEVENT
    long focused_event_end_byte_offset = 0;//The focused event end byte, the line after END:VEVENT

    int focused_event_date_code = 0;//The date code of the event currently being looked at
    int focused_event_time_code = 0;//The time code of the event currently being looked at

    int event_counter = 0;//Counts the events encountered//for debugging purposes
    int sector_count_max = SECTORTABLESIZE;//Maximum number of elements within the sector table
    int sector_count = 0;//The current element index of the sector table
    const long file_end = (long)(file->size()-14);//End of the file using size instead of keyword parse 1000X faster, -14 since END:VCALENDAR is 14 in length with CR-LF sequence

    char *working_string_pointer = NULL;//Used to parse string data for date relevance comparsions

    //Need to wipe values of sector_table and write 0's
    for(int i = 0; i < SECTORTABLESIZE; i++)
    {
        sector_table[i] = 0;//Setting all sector table values to zero
    }

    while(sector_count < sector_count_max)
    {
        working_byte_offset = find_next_keyword(file, "BEGIN:VEVENT", focused_event_end_byte_offset, file_end, FIRSTCHAR);
        if(working_byte_offset < 0)//Return of error or -1 implies end of files or no event left in file
        {
            break;//Found all events
        }
        else
        {
            event_counter++;
            focused_event_start_byte_offset = working_byte_offset;//Because an event was found it is "focused" since we are examining it's start time for relevance
            focused_event_end_byte_offset = find_next_keyword(file, "END:VEVENT", focused_event_start_byte_offset, file_end, NEXTLINE);//Find end of event to use as maximum in date relevance check

            working_byte_offset = find_next_keyword(file, "DTSTART", focused_event_start_byte_offset, focused_event_end_byte_offset, NEXTCHAR);//Find the start date for relevance check
            if(working_byte_offset < 0)
            {
                return_value_code = -1;
                break;  //Critical Error during search for DTSTART this is bad since all events must have a start time, bad ICAL file
            }
            else
            {
                char char_temp = file->read();
                if (char_temp == ':')    //Check if the start time is in a UTC time stamp
                {
                    working_byte_offset++;  //incrementing past colon
                    working_string_pointer = parse_data_string(file, working_byte_offset, focused_event_end_byte_offset, ONELINE); //Parse line for DTSTART utc string
                    if (working_string_pointer == NULL)
                    {
                       return_value_code = -1;
                       break;  //Failure could not parse event start utc string  20210326 T 211332 Z
                    }
                    //transfering the start date into an int for comparison between now and the event date
                    calendar_str_to_int(working_string_pointer, -1,'T', &focused_event_date_code); //transfering the utc year|month|day into the focused event date code int
                    //transfering the start time into an int for comparison between now and the event time
                    calendar_str_to_int(&(working_string_pointer[9]), -1, 'Z', &focused_event_time_code);
                    vPortFree(working_string_pointer);
                    
                    /*
                    //debugging stuff
                    Serial.print("An event start offset is: ");
                    Serial.println(focused_event_start_byte_offset, HEX);

                    Serial.print("The event end offset is: ");
                    Serial.println(focused_event_end_byte_offset, HEX);
                    Serial.print("\tEvent date: ");
                    Serial.print(focused_event_date_code);
                    Serial.print(", Current date: ");
                    Serial.println(current_date_code);

                    Serial.print("\tEvent time: ");
                    Serial.print(focused_event_time_code);
                    Serial.print(", Current time: ");
                    Serial.println(current_time_code);
                    */

                    if(focused_event_date_code <= current_date_code)//Is the event start date less than or the same as the current date
                    {
                        if(focused_event_time_code <= current_time_code)//Is the event start time less that or the same as the current date
                        {
                            working_byte_offset = find_next_keyword(file, "DTEND:", working_byte_offset, focused_event_end_byte_offset, NEXTCHAR);//Check utc end time to see if event is ongoing
                            if(working_byte_offset == EOF)
                            {
                                return_value_code = -1;//EOF means failure to search/read quit
                                break;
                            }
                            else if(working_byte_offset == -2)
                            {
                                //WE could not find an end date for the event, edge case of 2 events within ical, we same the event has zero-width
                                continue;//Since the start time is the end time for non-specified endtime event this event is not relevant
                            }
                            else
                            {
                                //Found a utc end time for the event
                                working_string_pointer = parse_data_string(file, working_byte_offset, focused_event_end_byte_offset, ONELINE); //Parse line for DTEND: utc string
                                if (working_string_pointer == NULL)
                                {
                                    return_value_code = -1;
                                    break;  //Failure could not parse event end utc string  20210326 T 211332 Z
                                }
                                //transfering the end date into an int for comparison between now and the event date
                                calendar_str_to_int(working_string_pointer, -1,'T', &focused_event_date_code); //transfering the end utc year|month|day into the focused event date code int
                                //transfering the end time into an int for comparison between now and the event time
                                calendar_str_to_int(&(working_string_pointer[9]), -1, 'Z', &focused_event_time_code);
                                vPortFree(working_string_pointer);

                                if(focused_event_date_code == current_date_code)//Is the event the same as the current date
                                {
                                    if(focused_event_time_code <= current_time_code)//Is the event end time less that or the same as the current date
                                    {
                                        continue;//Since the end date/time has already passed this event is not relevant
                                    }
                                }
                                else if(focused_event_date_code < current_date_code)//Is the end time less than the current time, ie event has finished already?
                                {
                                    continue;//End time has already passed too
                                }
                            }
                        }
                    }
                    //Since the else-if structure above was left without reaching a continue event is relevant and needs to be added to sector table
                    if (sector_count < (sector_count_max-2))//Is there more than 1 pair (2 elements) left in the sector table?
                    {
                        //checking if last sector offset is the begining of this sectors offset i.e. a continous region of byte offsets
                        if ((sector_count != 0) && (focused_event_start_byte_offset == sector_table[(-1 + sector_count)]))
                        {
                            //The last sector's end is the same as this sector's start so we can replace the last sector's end with this sector's end since it a continous byte-offset region
                            sector_table[(-1 + sector_count)] = focused_event_end_byte_offset;
                            //no need to increment the sector count since technically no new elements were added
                        }
                        else
                        {
                            sector_table[sector_count++] = focused_event_start_byte_offset;//Set current sector table element to start of focused, relevant event
                            sector_table[sector_count++] = focused_event_end_byte_offset;//Set related pair element to end of focused, relevant event
                        }
                        //Serial.print("\tSector Table Event Pair Added For Event # ");
                        //Serial.println(event_counter);
                    }
                    else
                    {
                        if(focused_event_start_byte_offset == sector_table[(-1 + sector_count)])//checking to see if the begining of this sector and end of the last sector is continous
                        {
                            sector_table[(-1 + sector_count)] = NOEND;//Since the start and end  is continous pl
                            sector_count+=2;// Adding 2 to sector count to prevent the function from writing sector byte offsets after NOEND element
                        }
                        sector_table[sector_count++] = focused_event_start_byte_offset;
                        sector_table[sector_count++] = NOEND;   //Since there 1 pair left in sector table we set the last pair of sector bytes to the start of the current relevant event and the end of the file,
                                                                //Note that NOEND is that acutal end of file but, the find_keyword functions and the parsing functions treat NOEND as the literal end of 2GiB file 
                        //Serial.print("\tEnd sector table pair added For Event # ");
                        //Serial.print(event_counter);
                        //Serial.println("-------------------------------------------------------------------------------------------------------------------");
                        break;
                    }
                }
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
                    if(char_temp == 'V')
                    {
                        //This mean event time is in a LOCAL time, adjust the time with using the offset
                        working_byte_offset+=12; //Moving from the ; in DTSTART;VALUE=DATE:1.. to the 1
                        working_string_pointer = parse_data_string(file, working_byte_offset, focused_event_end_byte_offset, ONELINE); //Parse line for the start time stamp 
                        if (working_string_pointer == NULL)
                        {
                                return_value_code = -1;//Failure during parse for date stamp
                                break; 
                        }
                        //This format of datestamp only had a date no time, used for edge 248 cases
                        calendar_str_to_int(working_string_pointer, -1, '\0', &focused_event_date_code); //transfering the datestamp year|month|day into event date stamp

                        //Note no time stamp, we assume time = 00:00.00 plus the offset in regards to utc, so same day by ahead if local is behind, behind if local is ahead
                        if (time_offset <= 0)//Check if negative
                        {
                            focused_event_time_code = (-1 * time_offset);//Local time is behind so utc equivalent is offset ahead
                        }
                        else
                        {
                            focused_event_date_code -=1;    //subtract a day since local time is ahead of utc
                            focused_event_time_code = 235959 - time_offset;
                        }
                        vPortFree(working_string_pointer); //freeing the start date stamp

                        //debugging stuff
                        /*
                        Serial.print("An event start offset is: ");
                        Serial.println(focused_event_start_byte_offset, HEX);

                        Serial.print("The event end offset is: ");
                        Serial.println(focused_event_end_byte_offset, HEX);
                        Serial.print("\tEvent date: ");
                        Serial.print(focused_event_date_code);
                        Serial.print(", Current date: ");
                        Serial.println(current_date_code);

                        Serial.print("\tEvent time: ");
                        Serial.print(focused_event_time_code);
                        Serial.print(", Current time: ");
                        Serial.println(current_time_code);
                        */
                        
                        if(focused_event_date_code <= current_date_code)//Is the event start date less than or the same as the current date
                        {
                            if(focused_event_time_code <= current_time_code)//Is the event start time less that or the same as the current date
                            {
                                working_byte_offset = find_next_keyword(file, "DTEND;VALUE=DATE:", working_byte_offset, focused_event_end_byte_offset, NEXTCHAR);//Check end date
                                if(working_byte_offset == EOF)
                                {
                                    return_value_code = -1;//Failure to find END time
                                    break;
                                }
                                else if(working_byte_offset == -2)
                                {
                                    //WE could not find an end date for the event, edge case of 2 events within ical, we same the event has zero-width
                                    continue;//Since the start time is the end time for non-specified endtime event this event is not relevant
                                }
                                else
                                {
                                    //Found an end date for the event
                                    working_string_pointer = parse_data_string(file, working_byte_offset, focused_event_end_byte_offset, ONELINE); //Parse line for end date
                                    if (working_string_pointer == NULL)
                                    {
                                        return_value_code = -1;
                                        break;  //Failure could not parse event end date
                                    }
                                    //transfering the end date into an int for comparison between now and the event date
                                    calendar_str_to_int(working_string_pointer, -1, '\0', &focused_event_date_code); //transfering the end datestamp year|month|day into event date stamp

                                    //No changes needed to the time stamp since again not specified so still zero
                                    vPortFree(working_string_pointer);
                                    if(focused_event_date_code == current_date_code)//Is the event the same as the current date
                                    {
                                        if(focused_event_time_code <= current_time_code)//Is the event end time less that or the same as the current date
                                        {
                                            continue;//Since the end date/time has already passed this event is not relevant
                                        }
                                    }
                                    else if(focused_event_date_code < current_date_code)//Is the end time less than the current time, ie event has finished already?
                                    {
                                        continue;//End time has already passed too
                                    }
                                }
                            }
                        }
                        //Since the else-if structure above was left without reaching a continue event is relevant and needs to be added to sector table
                        if (sector_count < (sector_count_max-2))//Is there more than 1 pair (2 elements) left in the sector table?
                        {
                            //checking if last sector offset is the begining of this sectors offset i.e. a continous region of byte offsets
                            if ((sector_count != 0) && (focused_event_start_byte_offset == sector_table[(-1 + sector_count)]))
                            {
                                //The last sector's end is the same as this sector's start so we can replace the last sector's end with this sector's end since it a continous byte-offset region
                                sector_table[(-1 + sector_count)] = focused_event_end_byte_offset;
                                //no need to increment the sector count since technically no new elements were added
                            }
                            else
                            {
                                sector_table[sector_count++] = focused_event_start_byte_offset;//Set current sector table element to start of focused, relevant event
                                sector_table[sector_count++] = focused_event_end_byte_offset;//Set related pair element to end of focused, relevant event
                            }
                            //Serial.print("\tSector Table Event Pair Added For Event # ");
                            //Serial.println(event_counter);
                        }
                        else
                        {
                            if(focused_event_start_byte_offset == sector_table[(-1 + sector_count)])//checking to see if the begining of this sector and end of the last sector is continous
                            {
                                sector_table[(-1 + sector_count)] = NOEND;//Since the start and end  is continous pl
                                sector_count+=2;// Adding 2 to sector count to prevent the function from writing sector byte offsets after NOEND element
                            }
                            sector_table[sector_count++] = focused_event_start_byte_offset;
                            sector_table[sector_count++] = NOEND;   //Since there 1 pair left in sector table we set the last pair of sector bytes to the start of the current relevant event and the end of the file,
                                                                    //Note that NOEND is that acutal end of file but, the find_keyword functions and the parsing functions treat NOEND as the literal end of 2GiB file 
                            //Serial.print("\tEnd sector table pair added For Event # ");
                            //Serial.print(event_counter);
                            //Serial.println("-------------------------------------------------------------------------------------------------------------------");
                            break;
                        }
                    }
                    else if (char_temp == 'T')//TZID, check if the same as calendar, add offset based on utc offset in calendar struc
                    {
                        working_byte_offset+=6;//Need to move byte offset from ; to Amer.. in DTSTART;TZID=AMERICA/EDMONTON:
                        extra_byte_offset = find_next_keyword(file, ":", working_byte_offset, focused_event_end_byte_offset, NEXTCHAR);
                        if(extra_byte_offset < 0)
                        {
                            return_value_code = -1;//Failure to find TZID colon
                            break;
                        }
                        else
                        {
                            //grabbing time zone id string, extra is decremented one since it is the byte offset after the colon not at the colon
                            working_string_pointer = parse_data_string(file, working_byte_offset, (-1 + extra_byte_offset), ONELINE);  //looking for the tzid string in DTSTART;TZID= "America/Edmonton" :20210215T000000
                            if (working_string_pointer == NULL)
                            {
                                return_value_code = -1;//Failure to parse TZID
                                break;
                            }

                            for(int i = 0; working_string_pointer[i] != '\0'; i++)
                            {
                                if (working_string_pointer[i] != user_calendar->timezone.time_zone_id[i])
                                {
                                    vPortFree(working_string_pointer);//freeing the TZID string found
                                    continue;//Since the event time zone and the calendar timezone arent the same we cant adjust/compared the event time with UTC at all
                                }
                            }
                            vPortFree(working_string_pointer);  //freeing the heap timezoneid
                            working_string_pointer = parse_data_string(file, extra_byte_offset, focused_event_end_byte_offset, ONELINE);//parseing the timestamp after the :
                            if (working_string_pointer == NULL)
                            {
                                return_value_code = -1;//Failure to parse event date stamp
                                break;
                            }
                            //pulling out the date stuff
                            calendar_str_to_int(working_string_pointer, -1, 'T', &focused_event_date_code);    //grabbing date stamp code into year|month|day into focused event date stamp
                            //pulling out the time stuff
                            calendar_str_to_int(&(working_string_pointer[9]), -1, '\0', &focused_event_time_code); //transfering the time stamp hour|minute|second into event time stamp, it isnt Z terminated like UTC
                            
                            //Still have to adjust time and date stamp since it's the calendar timezone id

                            vPortFree(working_string_pointer);  //freeing the heap timezoneid string
                            if (time_offset <= 0)//Check if negative
                            {
                                focused_event_time_code = (-1 * time_offset);//Local time is behind so utc equivalent is offset ahead
                            }
                            else
                            {
                                focused_event_date_code -=1;    //subtract a day since local time is ahead of utc
                                focused_event_time_code = 235959 - time_offset;
                            }

                            //now need to run through time check between event time and current time

                            if(focused_event_date_code <= current_date_code)//Is the event start date less than or the same as the current date
                            {
                                if(focused_event_time_code <= current_time_code)//Is the event start time less that or the same as the current date
                                {
                                    working_byte_offset = find_next_keyword(file, "DTEND;TZID=", working_byte_offset, focused_event_end_byte_offset, NEXTCHAR);//Check end date
                                    if(working_byte_offset == EOF)
                                    {
                                        return_value_code = -1;//Failure to find END time
                                        break;
                                    }
                                    else if(working_byte_offset == -2)
                                    {
                                        //WE could not find an end date for the event, edge case of 2 events within ical, we same the event has zero-width
                                        continue;//Since the start time is the end time for non-specified endtime event this event is not relevant
                                    }
                                    else
                                    {
                                        working_byte_offset = find_next_keyword(file, ":", working_byte_offset, focused_event_end_byte_offset, NEXTCHAR);//Jump to colon past know TZID
                                        if (working_byte_offset < 0)
                                        {
                                            return_value_code = -1; //Failure to find end time TZID stamp
                                            break;
                                        }
                                        //Found an end date for the event
                                        working_string_pointer = parse_data_string(file, working_byte_offset, focused_event_end_byte_offset, ONELINE); //Parse line for end date
                                        if (working_string_pointer == NULL)
                                        {
                                            return_value_code = -1;
                                            break;  //Failure could not parse event end date
                                        }
                                        //transfering the end date into an int for comparison between now and the event date
                                        calendar_str_to_int(working_string_pointer, -1, 'T', &focused_event_date_code);    //grabbing date stamp code into year|month|day into focused event date stamp
                                        //pulling out the time stuff
                                        calendar_str_to_int(&(working_string_pointer[9]), -1, '\0', &focused_event_time_code); //transfering the time stamp hour|minute|second into event time stamp, it isnt Z terminated like UTC
                                        vPortFree(working_string_pointer);

                                        //Adjusting end time based on calendar timezone offsets
                                        if (time_offset <= 0)//Check if negative
                                        {
                                            focused_event_time_code = (-1 * time_offset);//Local time is behind so utc equivalent is offset ahead
                                        }
                                        else
                                        {
                                            focused_event_date_code -=1;    //subtract a day since local time is ahead of utc
                                            focused_event_time_code = 235959 - time_offset;
                                        }

                                        //Checking
                                        if(focused_event_date_code == current_date_code)//Is the event the same as the current date
                                        {
                                            if(focused_event_time_code <= current_time_code)//Is the event end time less that or the same as the current date
                                            {
                                                continue;//Since the end date/time has already passed this event is not relevant
                                            }
                                        }
                                        else if(focused_event_date_code < current_date_code)//Is the end time less than the current time, ie event has finished already?
                                        {
                                            continue;//End time has already passed too
                                        }
                                    }
                                }
                            }
                            //Since control has passed else if structure and test add to sector table
                            if (sector_count < (sector_count_max-2))//Is there more than 1 pair (2 elements) left in the sector table?
                            {
                                //checking if last sector offset is the begining of this sectors offset i.e. a continous region of byte offsets
                                if ((sector_count != 0) && (focused_event_start_byte_offset == sector_table[(-1 + sector_count)]))
                                {
                                    //The last sector's end is the same as this sector's start so we can replace the last sector's end with this sector's end since it a continous byte-offset region
                                    sector_table[(-1 + sector_count)] = focused_event_end_byte_offset;
                                    //no need to increment the sector count since technically no new elements were added
                                }
                                else
                                {
                                    sector_table[sector_count++] = focused_event_start_byte_offset;//Set current sector table element to start of focused, relevant event
                                    sector_table[sector_count++] = focused_event_end_byte_offset;//Set related pair element to end of focused, relevant event
                                }
                                //Serial.print("\tSector Table Event Pair Added For Event # ");
                                //Serial.println(event_counter);
                            }
                            else
                            {
                                if(focused_event_start_byte_offset == sector_table[(-1 + sector_count)])//checking to see if the begining of this sector and end of the last sector is continous
                                {
                                    sector_table[(-1 + sector_count)] = NOEND;//Since the start and end  is continous pl
                                    sector_count+=2;// Adding 2 to sector count to prevent the function from writing sector byte offsets after NOEND element
                                }
                                sector_table[sector_count++] = focused_event_start_byte_offset;
                                sector_table[sector_count++] = NOEND;   //Since there 1 pair left in sector table we set the last pair of sector bytes to the start of the current relevant event and the end of the file,
                                                                        //Note that NOEND is that acutal end of file but, the find_keyword functions and the parsing functions treat NOEND as the literal end of 2GiB file 
                                //Serial.print("\tEnd sector table pair added For Event # ");
                                //Serial.print(event_counter);
                                //Serial.println("-------------------------------------------------------------------------------------------------------------------");
                                break;
                            }
                        } 
                    }
                    else
                    {
                        continue;//the start date format is unknown and not considered, event is considered to be irrelevant since it cant be parsed anyways
                    }
                }
                else
                {   
                    continue;   //The start date format must be in another format, this event is useless since we dont know how to parse it anyways, consider irrelevant re-loop for next event
                }
                
            }
        }
    }


    return return_value_code;
}

void update_sector_table(long *sector_table, const long event_start, const long event_end)
{
    //This function should update the sector table for when an event passes/finished
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
        Serial.print("\tIn a timezone of: ");
        calendar_str_print(user_event->event_time_zone_id);

        if (user_event->alarm_status == 0)
        {
            //Serial.println(user_event->event_start_date_code); //Not useful for humans to read
            Serial.print("\tThe event is on: ");
            Serial.print(user_event->event_start_year);
            Serial.print("/");
            Serial.print(user_event->event_start_month);
            Serial.print("/");
            Serial.println(user_event->event_start_day);
            //Serial.println(user_event->event_start_time_code);  //Not useful for humans to read
            Serial.print("\tAt time of: ");
            Serial.print(user_event->event_start_hour);
            Serial.print(":");
            Serial.print(user_event->event_start_minute, 2);
            Serial.print(".");
            Serial.println(user_event->event_start_minute, 2);


            //Serial.println(user_event->event_end_date_code); //Not useful for humans to read
            Serial.print("\tThe event ends on: ");
            Serial.print(user_event->event_end_year);
            Serial.print("/");
            Serial.print(user_event->event_end_month);
            Serial.print("/");
            Serial.println(user_event->event_end_day);
            //Serial.println(user_event->event_end_time_code);  //Not useful for humans to read
            Serial.print("\tAt time of: ");
            Serial.print(user_event->event_end_hour);
            Serial.print(":");
            Serial.print(user_event->event_end_minute, 2);
            Serial.print(".");
            Serial.println(user_event->event_end_minute, 2);
        }
        else
        {
            //Serial.println(user_event->event_start_date_code); //Not useful for humans to read
            Serial.print("\tThe alarm is on: ");
            Serial.print(user_event->event_start_year);
            Serial.print("/");
            Serial.print(user_event->event_start_month);
            Serial.print("/");
            Serial.println(user_event->event_start_day);
            //Serial.println(user_event->event_start_time_code);  //Not useful for humans to read
            Serial.print("\tAt time of: ");
            Serial.print(user_event->event_start_hour);
            Serial.print(":");
            Serial.print(user_event->event_start_minute, 2);
            Serial.print(".");
            Serial.println(user_event->event_start_minute, 2);
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