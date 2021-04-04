#ifndef ICAL_LIBARY_H
    #define ICAL_LIBARY_H
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
#endif//(ICAL_LIBARY_H)