/*!**********************************************************************************************************************
@file main.c                                                                
@brief Main system file for the ENCM 369 firmware.  
***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SystemTime1ms = 0;     /*!< @brief Global system time incremented every ms, max 2^32 (~49 days) */
volatile u32 G_u32SystemTime1s  = 0;     /*!< @brief Global system time incremented every second, max 2^32 (~136 years) */
volatile u8  G_u8SystemFlags    = 0;     /*!< @brief Global system flags */

/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Main_" and be declared as static.
***********************************************************************************************************************/


/*!**********************************************************************************************************************
@fn void main(void)
@brief Main program where all tasks are initialized and executed.


***********************************************************************************************************************/

void main(void)
{
  G_u8SystemFlags |= _SYSTEM_INITIALIZING;

  /* Low level initialization */
  ClockSetup();
  GpioSetup();
  SPIInitialize();
  InterruptSetup();

  SysTickSetup();

  /* Driver initialization */
 
  /* Application initialization */
  UserAppInitialize();
  
  /* Exit initialization */
  G_u8SystemFlags &= ~_SYSTEM_INITIALIZING;
    
  /* Super loop */  
  while(1)
  {
    /* Drivers */
    
    /* Applications */
    UserAppRun();
    
    /* System sleep */
    HEARTBEAT_OFF();
    SystemSleep();     
    HEARTBEAT_ON();
    
  } /* end while(1) main super loop */
  
} /* end main() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
