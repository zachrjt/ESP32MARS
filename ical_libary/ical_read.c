#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main(int argc, char *argv[])
{
    if(argc == 1)   //arguments were not specified
    {
        exit(1);
    }
    else
    {
        FILE *i_cal_file;
        i_cal_file = fopen(argv[1], "r");
        if (i_cal_file)   //pointer isn't NULL
        {

        }
    }
}



