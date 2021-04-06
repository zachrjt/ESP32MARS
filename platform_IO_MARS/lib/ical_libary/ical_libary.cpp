#include <Arduino.h>      //Arduino Stuff
#include <SPI.h>          //SPI Libary
#include <SD.h>           //SD Card libary

#include "ical_libary.h"  //Associated header file

char *parse_data_string(File *file, const long file_byte_offset, ICALMODERTN const byte return_string_mode)
{
    long current_file_byte_offset = file_byte_offset;
    int max_text_buffer_index = 77;
    int text_buffer_index = 0;
    char *text_buffer = (char *)(pvPortMalloc(sizeof(char) * (max_text_buffer_index))); //the maximum length of a line in a ical file is 77 with the CR-LF sequence, 
    if (!file->seek(file_byte_offset))                         //going to the specified position
    {
        return NULL; //specifed position could not be seeked out
    }
    char buffer_byte = '\0';

    while ((text_buffer_index < max_text_buffer_index))
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


long find_next_keyword(File *file, const char *keyword, const long file_byte_offset, ICALMODERTN const byte return_offset_mode)
{
    long current_byte_offset = file_byte_offset; //The byte offset to returned, of the first character of the keyword we're looking for

    int keyword_index = 0;                       //Pointer index of our keyword string
    int keyword_length = 0;                      //The length of the keyword
    for (int i = 0; keyword[i] != '\0'; i++, keyword_length++); //Finding the length of keyword passed, I know C++ has strings but, I dont feel like learning them

    char buffer_byte = '\0';                    //Initializing a buffer byte to read bytes in from the file

    if (!file->seek(file_byte_offset)) //Going to the specified position within the file
    {
        current_byte_offset = EOF; //Specifed position could not be seeked out since invalid, returning EOF
        return current_byte_offset;
    }

    while (1) //if file-> is not true then we have reached the end of the file or an error, return EOF
    {
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


long find_previous_keyword(File *file, const char *keyword, const long file_byte_offset, ICALMODERTN const byte return_offset_mode)
{
    long current_byte_offset = file_byte_offset; //The byte offset to returned, of the first character of the keyword we're looking for

    int keyword_length = 0;                      //The length of the keyword
    for (int i = 0; keyword[i] != '\0'; i++, keyword_length++); //Finding the length of keyword passed
    int keyword_index = keyword_length-1;         //Pointer index of our keyword string, in mode 0x00 (reverse) starts at max and decreases

    char buffer_byte = '\0';                    //Initializing a buffer byte to read bytes in from the file
    if (!file->seek(file_byte_offset)) //Going to the specified position within the file
    {
        current_byte_offset = EOF; //Specifed position could not be seeked out since invalid, returning EOF
        return current_byte_offset;
    }

    while (1)
    {
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



long *find_event(File *file, const long start_offset_byte, const long *read_sector_table, long date, long time, long tolerance)
{
    //This needs some basic operations and calls to find_event_limits
    //First needs the creation/function to make a read_sector_table 
        //Read sector table basically is a table of byte offset ranges that are valid to read from, it makes read the file after intialization muchhhhh easier
            //We wont need to re-read 150000 lines and any lines/events that have already happened can be skipped over and dont need to be parsed
    return 0;
}


byte intialize_calendar(File *file, Calendar *user_calendar)
{
    //This function just needs some basic text parsing for the name and time zone id calling the parse string and find keyword funcs
    long working_byte_offset = 0;
    working_byte_offset = find_next_keyword(file, "X-WR-CALNAME:", working_byte_offset, ICALMODERTN 0x11); //Find byte-offset of start of calendar name
    if(working_byte_offset == EOF)
    {
        return -1;  //Failure could not find calendar name keyword
    }
    char *working_string_pointer = NULL;
    working_string_pointer = parse_data_string(file, working_byte_offset, 0x00);    //Parse name line for name
    if (working_string_pointer == NULL)
    {
        return -1;  //Failure could not parse calendar name
    }
    for(int i = 0; (working_string_pointer[i] != '\0') && (i < MAX_NAME_SIZE); i++)
    {
        user_calendar->agenda_name[i] = working_string_pointer[i];  //Moving calendar name from heap data to the calendar object 
    }
    vPortFree(working_string_pointer);  //freeing name from heap
    return 0;
}

void calendar_str_print(const char * char_array)
{
    //This is used as a debugging function to print string data to the serial console to test features and debug processes
    if(!Serial)    //check if serial port is available, ie forgot to edit out debug code elsewhere and this code is called on un-plugged TTgo
    {
        return;
    }

    #ifdef PRINTOUTDEBUG
        for (int i = 0; char_array[i] != '\0' ; i++)
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

byte calendar_str_to_int(const char * str_num, int num_length, int *int_pointer)
{
    int temp_num = 0;
    int temp_exponent = 0;
    int i = 0;
    while((i < num_length) && (str_num[i] != '\0'))
    {
        temp_exponent = 1;
        for(int j = (i+1); j < num_length; j++)
        {
            temp_exponent*=10;   //This is for the base 2 conversion
        }

        temp_num += (temp_exponent * ((int)str_num[i] - 48));    //ascii offset of 48 needed
        i++;    //increment i
    }
    if((str_num[i] == '\0'))    //Error occured, reached end of string before length specified could be achieved
    {
        return -1;              //Failure/Error return -1
    }
    else
    {
        *int_pointer = temp_num;  //Change the pointer's content 
        return 0;               //Success return 0
    }

}
