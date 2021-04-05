#include <Arduino.h>      //Arduino Stuff
#include <SPI.h>          //SPI Libary
#include <SD.h>           //SD Card libary

#include "ical_libary.h"  //Associated header file

long find_keyword(File *file, const char *keyword, const long file_byte_offset, ICALMODE const byte return_offset_mode)
{
    long keyword_byte_offset = file_byte_offset; //The byte offset to returned, of the first character of the keyword we're looking for
    int text_buffer_index = 0;                   //Array index of our line text buffer
    int keyword_index = 0;                       //Pointer index of our keyword string
    int keyword_length = 0;                      //The length of the keyword
    int keyword_state = 0;                       //State of if the keyword has been found

    for (int i = 0; keyword[i] != '\0'; i++, keyword_length++); //Finding the length of keyword passed, I know C++ has strings but, I dont feel like learning them
    int maxbuffer_length = (77 - (keyword_length));             //The maximum buffer length is 75 with the end CR-LF, but if we havent found the 

    char text_buffer[maxbuffer_length] = {};    //A line text buffer, used to recieve data from the file consisting of up to one line...

    char buffer_byte = '\0';                    //Initializing a buffer byte to read bytes in from the file

    if (!file->seek(file_byte_offset)) //Going to the specified position within the file
    {
        keyword_byte_offset = EOF; //Specifed position could not be seeked out since invalid, returning EOF
    }

    while (text_buffer_index != EOF && file->available()) //if available is not true then EOF
    {
        text_buffer_index = 0; //reseting text buffer index value
        keyword_index = 0;     //Reseting keyword pattern index value
        buffer_byte = '\0';    //Clearing buffer byte

        for (text_buffer_index = 0; text_buffer_index < maxbuffer_length; text_buffer_index++)
        {
            buffer_byte = file->read();
            if (buffer_byte == -1)             //read() method of File will return -1/EOF if no more characters could be read, ie EOF
            {
                keyword_byte_offset = EOF;     //Return EOF indicating error during keyword parsing
                break;
            }
            else if (buffer_byte == '\r')      //If the current buffer byte is CR...
            {

                if (file->peek() != '\n')      //Check if the next byte is LF...
                {
                    keyword_byte_offset = EOF; //Since the next character was not LF, it indicates a bad .ical file since...
                                               //..any CR in an .ical file cannot be without LF, return error
                }
                else
                {
                    keyword_byte_offset++;
                }

                break;                         //Otherwise no error, but end of line need to parse next line
            }
            else if('\0' != buffer_byte)       //Not an error nor a CR so a regular char byte
            {
                keyword_byte_offset ++;
                text_buffer[text_buffer_index] = buffer_byte;   //Setting current byte in text buffer to the read byte

                if (buffer_byte == keyword[keyword_index++])    //If the current byte matches the first/next character in the keyword
                {   
                    if (keyword_index == keyword_length)        //If a keyword match was found, ie reached end of keyword string;
                    {
                        
                        keyword_byte_offset -= keyword_index;
                        keyword_state = 1;
                        text_buffer_index++;
                        break;                                  //Since keyword was found returning the keyword_byte_offset to...
                                                                //...first character of the keyword match
                    }
                }
                else
                {
                    keyword_index = 0;                          //Keyword pattern broken or not even found yet
                }
            }
        }
        if (keyword_byte_offset == EOF)
        {
            break;
        }
        else
        {   
            //For testing
            /*for (int i = 0; i < text_buffer_index; i++)
            {
                Serial.print(text_buffer[i]);
            }
            Serial.print('\n');*/
            if(keyword_state == 1)
            {
                break;
            }
        }
    }
    return keyword_byte_offset; //Since no key value was found in the entire file return EOF
}

char *parse_data_line(File *file, const long file_byte_offset)
{
    long current_file_byte_offset = file_byte_offset;
    int max_text_buffer_index = 77;
    int text_buffer_index = 0;
    int buffer_state = 0;                                      //state indicates if a CRLF sequence was found, happens on every ical line
    char *text_buffer = (char *)(pvPortMalloc(sizeof(char) * (max_text_buffer_index))); //the maximum length of a line in a ical file is 77 with the CR-LF sequence, 
                                                                                        //using 78 so string is always null terminated if one line
    if (!file->seek(file_byte_offset))                         //going to the specified position
    {
        return NULL; //specifed position could not be seeked out
    }
    buffer_state = 0;
    char buffer_byte = '\0';

    for (text_buffer_index = 0; (text_buffer_index < max_text_buffer_index) && (file->available()); text_buffer_index++)
    {

        if (!buffer_state) //if we have not found a CRLF sequence
        {
            if (text_buffer_index == (max_text_buffer_index - 1))     //text buffer may need to be larger for multi-line strings, in that case we allocated more heap
            {
                char *temp_buffer = (char *)(malloc((sizeof(char) * (75 + max_text_buffer_index))));
                for (int i = 0; i <= text_buffer_index; i++)
                {
                    temp_buffer[i] = text_buffer[i];
                }
                free(text_buffer);
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
                        if((file->peek() == '\t') || (file->peek() == ' '))   //if the next byte is a whitespace character it means the string is multi-line
                        {
                            file->read();   //discard white space
                            text_buffer_index -=2;    //rewrite over the CR-LF sequence with the rest of the string
                        }
                        else    //the next character was not a whitespace so end of string
                        {
                            text_buffer_index -=2;    //rewrite over the CR-LF sequence with null characters ending string
                            buffer_state = 1;
                        }
                    }
                }
            }
        }
        else
        {
            //fill next text_buffer element with a null character
            text_buffer[text_buffer_index] = '\0';
            break;
        }
    }
    Serial.println(max_text_buffer_index);
    return text_buffer;
}

long *find_event(File *file, const long start_offset_byte, long date, long time, long tolerance)
{
    return 0;
}

int intialize_calendar(File *file, Calendar *user_calendar)
{
    return 0;
}