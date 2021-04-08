/*!*********************************************************************************************************************
@file user_app.c                                                                
@brief User's tasks / applications are written here.  This description
should be replaced by something specific to the task.

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void UserApp1Initialize(void)
- void UserApp1Run(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u8 G_u8UserAppFlags;                             /*!< @brief Global state flags */
u8 G_au8Time0;
u8 G_au8Time1;
u8 G_au8Time2;
u8 G_au8AlarmTime0;
u8 G_au8AlarmTime1;
u8 G_au8AlarmTime2;
u8 G_u8AlarmFlag;

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                     /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_<type>" and be declared as static.
***********************************************************************************************************************/


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void UserAppInitialize(void)

@brief
Initializes the application's variables.

Should only be called once in main init section.

Requires:
- RA0-7 setup as digital output

Promises:
- For RA6 to be high
*/
void UserAppInitialize(void)
{

    //LED initialization
    LATA = 0x40; //Setting RA6 latch to digital high, and RA0-5,7 latches low
    
    //Set up initial time here 1:59:53
    G_au8Time0 = 0b10010011; //Time[0]: (1-bit) 10's of hours | (4-bits) hours | (3-bits) 10's of minutes
    G_au8Time1 = 0b10010101; //Time[1]: (4-bits) minutes | (4-bits) 10's of seconds
    G_au8Time2 = 0b01010000; //Time[2]: (4-bits) seconds | (1-bit) pm | (1-bit) NULL | (2-bits) SPI code (0)
    
    G_au8AlarmTime0 = 0b00101011; //set up alarm time here (unused)
    G_au8AlarmTime1 = 0b00000000;
    G_au8AlarmTime2 = 0b00000000;
    
    G_u8AlarmFlag = 0;
} /* end UserAppInitialize() */

  
/*!----------------------------------------------------------------------------------------------------------------------
@fn void UserAppRun(void)

@brief Application code that runs once per system loop

Requires:+
- Time to pass normally, so can't be near black hole

Promises:
- depression
*/

void UserAppRun(void)
{
    if((G_au8Time2 & 0xF0) == 0x90) //Is seconds = 9?
    {
        if((G_au8Time1 & 0x0F) == 0x05) //Is seconds = 59?
        {
            if(G_au8Time1 == 0x95) //Is minutes 9.59?
            {
                if((G_au8Time0 & 0x07) == 0b00000101 ) //Is minutes 59.59?
                {
                    if(G_au8Time0 > 0x80)  //is it (10-12):59.59?, (different cases of time switching)
                    {
                        if(G_au8Time0 == 0b10010101)    //is it 12:59.59 ?
                        {
                            G_au8Time0 = 0b00001000; //Setting time to 01:00.00
                        }
                        else
                        {
                            if(G_au8Time0 == 0b10001101)    //is it 11:59.59 ?
                            {
                                G_au8Time2 ^= 0b00001000;  // switch from am to pm or viceversa
                            }
                            G_au8Time0 += 0x08; //incrementing hours
                        }
                    }
                    else
                    {
                        if(G_au8Time0 == 0x4D) //is it 9:59.59?
                        {
                            G_au8Time0 = 0x80; //Setting time to 10:00.00
                        }
                        else
                        {
                            G_au8Time0 += 0x08; //incrementing hours
                        }
                    }
                    G_au8Time0 &= 0xF8;    //setting 10's of minutes to 00
                }
                else
                {
                    G_au8Time0 += 0x01;    //increment 10's of minutes
                }
                G_au8Time1 = 0x00;    //Set minutes and 10's seconds to zero
            }
            else
            {
                G_au8Time1 = (G_au8Time1 & 0xF0) + 0x10;  //increment minutes and set 10's seconds to 0
            }
        }
        else
        {
            G_au8Time1 += 0x01;  //increment tens of seconds
        }
        G_au8Time2 &= 0x0F;   //set seconds to zero
    }
    else
    {
        G_au8Time2 += 0x10; //increment seconds
    }
    
    if((G_au8Time2 & 0b00001000) == 0b00001000)     //LED to check if pm vs am flag works
    {
        LATCbits.LATC3 ^= 1;
    }
    
    SPI1STATUSbits.CLRBF = 1;   //update Transfer buffer for ESP32 Time request (Buffer only has a capacity of 2)
    SPI1TXB = G_au8Time2;
    SPI1TXB = G_au8Time1;
    
    /*if(((G_au8Time0 == G_au8AlarmTime0) && (G_au8Time1 == G_au8AlarmTime1)) && (G_au8Time2 == G_au8AlarmTime2))
    {
        G_u8AlarmFlag = 1;  //used for possible alarm system
    }*/
    
    
    LATA ^=0x40; //blinking light that doesn't work since LATA06 is dumb
} /* end UserAppRun */


/*!--------------------------------------------------------------------------------------------------------------------
@fn void TimeXusInitialize(void)

@brief
Initializes the application's variables.

Should only be called once in main init section.

Requires:

Promises:
- For timer0 to be enabled in async. 16-bit mode with a prescaler of 1:1 for the (Fosc/4) source.
- For timer0 to run for 1 second 
*/
void TimeXusInitialize(void)
{
    OSCCON3bits.SOSCPWR = 0;     //Setting power mode to Low Power for 32.768Hz Quartz Crystals
    OSCENbits.SOSCEN = 1;       //Enabling secondary oscillator
    
    //Waiting until secondary oscillator is setup since sometimes failure can happen if crystal is bad.
    while(OSCSTATbits.SOR != 1)
    {
    }
    T0CON1 = 0b11011110;    //Setting timer0 to asynchronous mode with (SOSC) as the source with a pre-scaler of 1:32768
    T0CON0 = 0b10010000;    //Enabling timer0 in 16-bit mode with a post-scaler value of 1:1

    
    //setting up timer to count for 1 second so that TimeXus doesn't have to be called else where except the __interrupt handler in encm369_pic18.c
    T0CON0 &= 0x7F; //Disabling timer0
    
    TMR0H = (0xFFFF - 0x0001) >> 8; 
    
    //Pre-loading TMR0L/H 
    TMR0L = (0xFFFF - 0x0001) & 0x00FF;  
    
    T0CON0 |= 0x80; //Enabling timer0
    
} /* end TimerXusInitialize() */


/*!--------------------------------------------------------------------------------------------------------------------
@fn void TimeXus(void)

@brief doesnt do anything
 * 

Requires:
- Timer0 configured

Promises:
- Pre-loads TMR0H:L to clock out desired period
- Timer0 enabled

*/

void TimeXus(void)
{
    //moved this shit to the interrupt service for TMR0 in encm369_pic.c
    
}  /* end TimeXus () */

/*!--------------------------------------------------------------------------------------------------------------------
@fn void SegmentDecoderIntialize(void)

@brief
Initializes the EEPROM data for use as a lookup table for 7 segment displays

Should only be called once in main init section.

Requires:
- Zach to be a good embedded programmer; I can't btw.

Promises:
- For addresses 380000 - 380010 to have EEPROM values
- Data programmed is hex words for a segment a-f in that order
- Data is programmed at start if not present, configure PICKit settings not to wipe EEPROM data and use user defined memory ranges

*/
void SegmentDecoderIntialize(void)
{
    NVMADR = 380000;        //Setting NVM address to data state word address
    NVMCON1bits.CMD = 0x00; //Setting NVM configuration bits to read word
    NVMCON0bits.GO = 1;     //Read word
    while (NVMCON0bits.GO); //Waiting until byte is read
    
    if(1)   //data state word = 0x0000 if programmed already
    {
        /*Since data is not programmed programming data to EEPROM
        MUST erase words before modifying however, 
        can't erase 1 word must erase whole page*/
        uint16_t bufferRAM = 0x2500;  
        uint16_t *bufferRamPtr = (uint16_t*) bufferRAM; // Defining a pointer to the first location of the Buffer RAM
        
        //Reading existing EEPROM page
        NVMADR = 0x380000;            //Setting NVM address to start of EEPROM
        NVMCON1bits.CMD = 0x02;     //Configuration bits: read entire page to buffer RAM
        INTCON0bits.GIE = 0;        //Disabling interrupts while reading since busy
        NVMCON0bits.GO = 1;         //Perform read operation
        while (NVMCON0bits.GO);     //Waiting until read is done
        
        //Erasing EEPROM page
        NVMCON1bits.CMD = 0x06;     //Configuration bits: erase entire page
        NVMLOCK = 0x55;             //Unlock sequence needed to write data
        NVMLOCK = 0xAA;
        NVMCON0bits.GO = 1;         //Erasing page
        while (NVMCON0bits.GO);     //Waiting until erase is done
        
        if(NVMCON1bits.WRERR)        //Checking if erase was successful
        {
            //Erase was bad staying in loop
            //might have failsafe function at one point?
        }
        //Editing PAGE in buffer ram
        u8 u8ArrayIndex = 0;
        //Array 2-bytes words contain the decoded segment bytes for each address offset and 
        u16 au16DecodingValues[] = {0x3F06, 0x5B4F, 0x666D, 0x7D07,  
                                  0x7F6F, 0x777C, 0x395E, 0x7971, 0x0000};
        
        //u8 u8DataOffset = u8 (0x380000 & ((0x1024 * 2) - 1) /2); //offset
        u8 u8Offset = (u8) ((0x380000 & ((128 * 2) - 1)) / 2); 
        while (u8ArrayIndex < 0x09)
        {
            *bufferRamPtr = au16DecodingValues[u8ArrayIndex];
            bufferRamPtr += 1; //might work??? I think it will??
            //this does not work I cant figure out how to edit buffer ram data
            //it jumps all over the place and I cant find debugging location of 
            //buffer ram????
            //The write after this fails probbaly because the data is fucked up
            //Might come back and use TABLE functions like it suggest, not much info online nor in the datasheet on exact usage
            u8ArrayIndex++;
        }
        *bufferRamPtr = 0x0000;
        
        //Write EEPROM page
        NVMADR = 0x380000;
        NVMCON1bits.CMD = 0x05;     //Configuration bits write page
        NVMLOCK = 0x55;             //Unlock sequence to write data
        NVMLOCK = 0xAA;
        NVMCON0bits.GO = 1;         //Writing page
        while (NVMCON0bits.GO);     //Waiting until write is done
        
        if(NVMCON1bits.WRERR)        //Checking if write was successful
        {
            //Write was bad staying in loop
            //might have failsafe function at one point?
        }
        
        INTCON0bits.GIE = 1;        //Disabling interrupts while reading since busy
        NVMCON1bits.CMD = 0x00;     //Enabling writes to memory
    }
} /* end SegmentDecoderIntialize() */

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------*/
