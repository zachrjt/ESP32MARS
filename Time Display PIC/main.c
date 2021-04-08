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
volatile u32 G_u32SystemFlags   = 0;     /*!< @brief Global system flags */

u8 G_u8TimeFlag = 0x00;
u8 G_u8receivedData = 0x05;
u8 G_u8writeData = 0x10;
u8 G_u8SPIFlag = 0;

/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */

extern u8 G_au8Time0;
extern u8 G_au8Time1;
extern u8 G_au8Time2;
extern u8 G_au8AlarmTime0;
extern u8 G_au8AlarmTime1;
extern u8 G_au8AlarmTime2;
extern u8 G_u8AlarmFlag;

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Main_" and be declared as static.
***********************************************************************************************************************/


//BCD decoding data, note bit 7 is g, 5-0 bits are f,e,d,c,b, and a.
//This is because my tired brain wired it backwards and for some reason LATA's 6th bit, 
//2nd MSB,doesn't work properly
//__EEPROM_DATA(0b00111111, 0b00000110, 0b10011011, 0b10001111, 0b10100110, 0b10101101, 0b10111101, 0b00000111);
//__EEPROM_DATA(0b10111111, 0b10101111, 0b10110111, 0b10111100, 0b10111100, 0b00111001, 0b10111001, 0b10110001);


/*!**********************************************************************************************************************
@fn void main(void)
@brief Main program where all tasks are initialized and executed.


***********************************************************************************************************************/

void main(void)
{ 
    G_u32SystemFlags |= _SYSTEM_INITIALIZING;

  /* Low level initialization */
  ClockSetup();
  SysTickSetup();
  GpioSetup();
  INTERRUPTInitialize();
  SPIInitialize();
  
      
  static u8 u8DigitCounter = 0;  //Setting digit display, digits 1-4
  static u8 u8TimeCounter = 0;  //Setting time array focus
  
  //BCD decoding values like seen in EEPROM statement commented out above
  u8 au8DisplayCode [16] = {0b00111111, 0b00000110, 0b10011011, 0b10001111, 0b10100110, 0b10101101, 0b10111101, 0b00000111,
                            0b10111111, 0b10101111, 0b10110111, 0b10111100, 0b10111100, 0b00111001, 0b10111001, 0b10110001};
  u8 u8PORTADisplayValue = 0x00;    //Display value used to set port 
  
  /* Driver initialization */
  //SegmentDecoderIntialize();
  
  /* Application initialization */
  UserAppInitialize();
  TimeXusInitialize();
  /* Exit initialization */
  /* Super loop */
  while(1)
  {
    /* Drivers */
    
    /* Applications */
    if(G_u8TimeFlag == 0xFF)
    {
        G_u8TimeFlag = 0x00;
        UserAppRun();
    }
    
    /* System sleep */
    //HEARTBEAT_OFF();
    //SystemSleep();
    //PORTA &= 0; //you get to pay with a much wider, though dimmer, range of brightnesses
    if(u8TimeCounter == 0)//increase value to increase brightness (to an extent) and decrease/remove to decrease (since it updates really fast think PWM)
    {
        PORTA &= 0x40;
        u8TimeCounter = 0;
        if(u8DigitCounter == 0) // Time storing conventions are hard and obtuse to reach with a single loop, could have reworked but brain too small
        {
            LATB = 0x01;
            u8PORTADisplayValue = au8DisplayCode[(G_au8Time1 >> 4) & 0x0F];    //Print mins
            u8DigitCounter++;
        } 
        else if(u8DigitCounter == 1)
        {
            LATB = 0x02;
            u8PORTADisplayValue = au8DisplayCode[(G_au8Time0 >> 0) & 0x07];    //Print 10s of mins
            u8DigitCounter++;
        }
        else if(u8DigitCounter == 2)
        {
            LATB = 0x04;
            u8PORTADisplayValue = au8DisplayCode[(G_au8Time0 >> 3) & 0x0F];    //Print Hours
            u8DigitCounter++;
        }
        else
        {
            LATB = 0x08;
            u8PORTADisplayValue = au8DisplayCode[(G_au8Time0 >> 7) & 0x01];    //Print 10s of Hours
            u8DigitCounter = 0;
        }
        PORTA = (PORTA & 0x40) + u8PORTADisplayValue;   //Setting PORTA based 
    }
    else
    {
        u8TimeCounter++;
    }
    //HEARTBEAT_ON();
  }
} /* end main() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

