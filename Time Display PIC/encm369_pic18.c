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

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxBsp"
***********************************************************************************************************************/
/* New variables */




/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;        /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;         /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;          /*!< @brief From main.c */

extern u8 G_u8TimeFlag;
extern u8 G_au8Time0;
extern u8 G_au8Time1;
extern u8 G_au8Time2;
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
    SPI1CON2 = 0b00000011;  //Set up SPI in receiver and transmission mode
    
    SPI1CON0bits.EN = 1; // Enable SPI
}/* end SPIInitialize */

/*!---------------------------------------------------------------------------------------------------------------------
@fn void INTERRUPTInitialize(void)

@brief Setups interrupts and interrupt priority

Requires:
- NONE

Promises:
- TMR0 interrupt is enabled as a low priority interrupt

*/
void INTERRUPTInitialize(void)
{
    INTCON0bits.IPEN = 1;   //Enabling interrupts
    IPR3bits.TMR0IP = 0;    //Setting TMR0 interrupt priority to low default high
    
    PIR3bits.TMR0IF = 0;    //Clearing the TMR0IF flag
    PIR3bits.SPI1RXIF = 0;  //Clearing the SPI1RXI flag
    
    PIE3bits.TMR0IE = 1;    //Enabling TMR0 interrupt ability
    PIE3bits.SPI1RXIE = 1;  //Enable SPI receive interrupt ability
    
    INTCON0bits.GIEH = 1;   //Enabling high-priority unmasked interrupts
    INTCON0bits.GIEL = 1;   //Enabling low-priority unmasked interrupts
    
}/* end INTERRUPTInitialize */

/*!---------------------------------------------------------------------------------------------------------------------
 * 
 * 
@fn void __interrupt(irq(IRQ_TMR0), low_priority) TMR0_ISR(void)

@brief Setups interrupts and interrupt priority

Requires:
- To be called via Vector Table from an interrupt event

Promises:
- To respond to the interrupt

*/
void __interrupt(irq(IRQ_TMR0), low_priority) TMR0_ISR(void)
{
    PIR3bits.TMR0IF = 0;    //Clearing the TMR0IF flag
    T0CON0 &= 0x7F; //Disabling timer0
    
    TMR0H = (0xFFFF - 0x0001) >> 8; 
    
    //Pre-loading TMR0L/H 
    TMR0L = (0xFFFF - 0x0001) & 0x00FF;  
    
    T0CON0 |= 0x80; //Enabling timer
    G_u8TimeFlag = 0xFF;
}/* end __interrupt */

/*!---------------------------------------------------------------------------------------------------------------------
 * 
 * 
@fn void __interrupt(irq(IRQ_TMR0), low_priority) TMR0_ISR(void)

@brief Setups interrupts and interrupt priority

Requires:
- To be called via Vector Table from an interrupt event

Promises:
- To respond to the interrupt

*/
void __interrupt(irq(IRQ_SPI1RX), high_priority) SPI1RX_ISR(void)  //Could have used transfer counter interrupt instead, probably will implement it in the future
{
    static u8 u8Counter = 0;
    static u8 u8received = 0;
    
    u8received = SPI1RXB;               //Read from receive buffer
    
    if(u8Counter == 0)                  //Send Time0 first, then Time1, then Time2
    {
        if((u8received & 0b00000111) == 0x00)
        {
            G_au8Time2 = u8received;
            u8Counter++;
        }
        else if(u8received == 1)        //First Time request from ESP32
        {
            SPI1TXB = G_au8Time0;       //send Time0 into transfer buffer for ESP32 request
        }
    }
    else if(u8Counter == 1)
    {
        G_au8Time1 = u8received;
        u8Counter++;
    }
    else
    {
        G_au8Time0 = u8received;
        u8Counter = 0;
    }
    
    PIR3bits.SPI1RXIF = 0;  //Clearing the SPI1RXI flag
}/* end __interrupt */


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
@fn void GpioSetup(void)

@brief Loads registers required to set up GPIO on the processor.

Requires:
- All configurations must match connected hardware.

Promises:
- Output pin for PA31_HEARTBEAT is configured
- RA0-7 configured for digital output, initial value 0x00
- RB0-7 configured for digital output initial value 0x00
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
    TRISC = 0x70; //Setting RC0-3,7 to be outputs, RC4-6 as inputs
    LATC  = 0x00; //Setting RC0-7 to be default off/low
} /* end GpioSetup() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn  void SysTickSetup(void)

@brief Initializes the 1ms and 1s System Ticks off the core timer.

Requires:
- NVIC is setup and SysTick handler is installed

Promises:
- Both global system timers are reset and the SysTick core timer is configured for 1ms intervals

*/
void SysTickSetup(void)
{
  G_u32SystemTime1ms = 0;      
  G_u32SystemTime1s  = 0;   
  
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



