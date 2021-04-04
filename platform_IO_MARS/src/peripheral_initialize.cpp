//INCLUDE SECTION START------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "task_macros.h"
#include "function_macros.h"
//INCLUDE SECTION END--------------------------------------------------------------------------------------------------------------------------------------------------------------




//FUNCTION SECTION START-----------------------------------------------------------------------------------------------------------------------------------------------------------
int setup_functions(void)
{
    //run setup/diagnostics on functions
    //return values indicates failure/sucess
    return PERIPHERAL_FAILURE;
}

int WiFiSetup(void)
{
  return PERIPHERAL_SUCCESS;
}


int SDCardSetup(void)
{
    return PERIPHERAL_SUCCESS;
}

 
int WebServerSetup(void)
{
    return PERIPHERAL_SUCCESS;
}


int LocalTimeSetup()
{
    return PERIPHERAL_SUCCESS;
}


int AlarmSetup()
{
    return PERIPHERAL_SUCCESS;
}

int SDCardInterruptSetup()
{
    return PERIPHERAL_SUCCESS;
}
//FUNCTION SECTION END-------------------------------------------------------------------------------------------------------------------------------------------------------------
