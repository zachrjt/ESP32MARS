/*!**********************************************************************************************************************
@file encm369_pic18.c                                                                
@brief This file provides core and GPIO functions for the ENCM 369 PIC activities.


------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- 

PROTECTED FUNCTIONS
- 

***********************************************************************************************************************/

#include "configuration.h"
#include "pic18f27q43.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxBsp"
***********************************************************************************************************************/
/* New variables */




/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;        /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;         /*!< @brief From main.c */
extern volatile u8  G_u8SystemFlags;           /*!< @brief From main.c */

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Bsp_" and be declared as static.
***********************************************************************************************************************/

/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/



/*!---------------------------------------------------------------------------------------------------------------------
@fn void ClockSetup(void)

@brief Loads all registers required to set up the processor clocks.

Requires:
- NONE

Promises:
- EFC is set up with proper flash access wait states based on 48MHz system clock
- PMC is set up with proper oscillators and clock sources

*/
void ClockSetup(void)
{
 
  
} /* end ClockSetup */

/*!---------------------------------------------------------------------------------------------------------------------
@fn void SPIInitialize(void)

@brief Setups SPI communication module

Requires:
- NONE

Promises:
- To set up the PIC as an SPI client

*/
void SPIInitialize(void)
{
   SPI1CON0bits.EN = 0; //Disable SPI for set up
    
    SPI1SCKPPS = 0b00010100; //Map Source clock pin to RC4
    SPI1SDIPPS = 0b00010101; //Map Data-in pin to RC5
    SPI1SSPPS  = 0b00010110; //Map Client Select pin to RC6
    
    RC7PPS = 0x32; //Map Data-out pin to RC7 (unused but just in case)
    
    SPI1CON0 = 0b00000000;  //Set PIC as client (default)
    SPI1CON1 = 0b00000100;  //default
    SPI1CON2 = 0b00000001;  //Set up SPI in receiver only mode
    
    SPI1CON0bits.EN = 1; // Enable SPI
}/* end SPIInitialize */

/*!---------------------------------------------------------------------------------------------------------------------
@fn void GpioSetup(void)

@brief Loads registers required to set up GPIO on the processor.

Requires:
- All configurations must match connected hardware.

Promises:
- PORTA setup for LED output

*/
void GpioSetup(void)
{
ANSELA = 0x00; //Setting up RA0-7 as digital IO
TRISA = 0x00; //Setting RA0-7 to be outputs
LATA  = 0x00; //Setting RA0-7 to be default off/low
    
ANSELB = 0x00; //Setting up RB0-7 as digital IO
TRISB = 0x00; //Setting RB0-7 to be outputs
LATB  = 0x00; //Setting RB0-7 to be default off/low
    
ANSELC = 0x00; //Setting up RC0-7 as digital IO
TRISC = 0xF0; //Setting RC0-3,7 to be outputs, RC4-6 as inputs
LATC  = 0x00; //Setting RC0-7 to be default off/low
   
  /* Configure DAC1 for Vdd and Vss references, on, and RA2 output. */
  DAC1CON  = 0xA0;
  DAC1DATL = 0;
 
} /* end GpioSetup() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn  void SysTickSetup(void)

@brief Initializes the 1ms and 1s System Ticks off the core timer.

Requires:
- NVIC is setup and SysTick handler is installed

Promises:
- Both global system timers are reset Timer0 is configured for 1ms intervals

*/
void SysTickSetup(void)
{
  G_u32SystemTime1ms = 0;      
  G_u32SystemTime1s  = 0;   
  
  /* Setup Timer2 for 1ms period 
   * Input clock: Fosc / 4 = 16MHz
   * Maximum prescaler 128 > 125kHz > 8us period
   * 125 x 8us = 1ms so use Timer2 match to 124
   * No postscaler requried */
  
  T2PR = 125;             // Match register for 1ms period
  T2CLKCON = 0x01;        // b'00000001' Fosc/4 input
  PIR3bits.TMR2IF = 0;    // Make sure interrupt flag is clear to start
  T2CON = 0xF0;           // b'11110000' Timer on, 1:128 prescale, 1:1 postscaler
  
  PIE3bits.TMR2IE = 1;    // Enable Timer2 interrupt: Systick is now running
  
} /* end SysTickSetup() */



/*!---------------------------------------------------------------------------------------------------------------------
@fn void SystemSleep(void)

@brief Puts the system into sleep mode.  


Requires:
- 
 * 
Promises:
- 

*/
void SystemSleep(void)
{    
  G_u8SystemFlags |= _SYSTEM_SLEEPING;
  while(G_u8SystemFlags & _SYSTEM_SLEEPING);
  
} /* end SystemSleep(void) */



/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/



