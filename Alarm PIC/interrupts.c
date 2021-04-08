/*!**********************************************************************************************************************
@file interrupts.c                                                                
@brief This file provides interrupt configuration and service routines
for the ENCM 369 PIC activities.
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
- void InterruptSetup(void)

ISRs
- SW_ISR

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
extern volatile u8  G_u8SystemFlags;           /*!< @brief From main.c */

extern volatile u8 G_u8UserAppFlags;           /*!< @brief From user_app.c */
extern volatile u8 G_u8UserAppTimePeriodHi;    /*!< @brief From user_app.c */
extern volatile u8 G_u8UserAppTimePeriodLo;    /*!< @brief From user_app.c */
extern volatile u8 G_u8UserAppAlarmFlag;    /*!< @brief From user_app.c */

extern u8 G_au8UserAppsinTable[];              /*!< @brief From user_app.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Int_" and be declared as static.
***********************************************************************************************************************/


/***********************************************************************************************************************
Interrupt Routines                                                         
***********************************************************************************************************************/


/*!---------------------------------------------------------------------------------------------------------------------
@fn void InterruptSetup(void)

@brief Loads all registers required to run interrupts.  Individual interrupts
 * are NOT enabled here, but globals are.

Requires:
- CONFIG bits are correctly set:
 > 

Promises:
- Vector-table based interrupt system is enabled
- Low and high priority global interrupts are enabled

*/
void InterruptSetup(void)
{
  /* Interrupt configuration (MVECEN must be SET/ON in CONFIG3) */
  INTCON0bits.IPEN = 1; // 

  /* Enable interrupts */  
  INTCON0bits.GIEH = 1; // Enable high priority interrupts
  INTCON0bits.GIEL = 1; // Enable low priority interrupts
  
  PIR3bits.SPI1RXIF = 0;  //Clearing the SPI1RXI flag
  PIE3bits.SPI1RXIE = 1;  //Enable SPI receive interrupt ability
    
} /* end InterruptSetup() */


void __interrupt(irq(IRQ_SWINT), high_priority) SW_ISR(void)
{
  PIR0bits.SWIF = 0; // Clear the interrupt flag
  
} /* end DEFAULT_ISR */


void __interrupt(irq(default), low_priority) DEFAULT_ISR(void)
{
  /* Unhandled interrupts go here. Since no flags are cleared,
   * the code will likely be stuck here but debugging and halting
   * will indicate that and the counter will roughly tell how
   * long it has been stuck (though will roll over fairly quickly */
  static u32 u32UnhandledCounter = 0;
  
  u32UnhandledCounter++;
  
  /* Could disable interrupts here so the code carries on... */
  // INTCON0bits.GIEL = 0; // Enable low priority interrupts

} /* end DEFAULT_ISR */


/* User-timer for regular event updates
   Requires G_u8UserAppTimePeriodHi/Lo and _U8_CONTINUOUS in G_u8UserAppFlags 
*/
void __interrupt(irq(IRQ_TMR1), high_priority) TMR1_ISR(void)
{
  static u8 u8Index = 0;

  /* Reload the timer - do this first to minimize latency */
  TMR1H = G_u8UserAppTimePeriodHi;
  TMR1L = G_u8UserAppTimePeriodLo;
  
  /*********************************************************************
   Handle the timing event here (usually a call-back function)
   In this case, we load the next value of the sinusoid.
   KEEP THIS SHORT!
  **********************************************************************/
    
  DAC1DATL = G_au8UserAppsinTable[u8Index];  
  u8Index += 4; // Let u8Index roll over back to 0 as sin table wraps around seamlessly
  
  /*********************************************************************
   End of event handling
  **********************************************************************/
  
  /* Clear the interrupt flag */
  PIR3bits.TMR1IF = 0;
    
  /* Turn off the timer and interrupt if this is one-shot */
  if( !(G_u8UserAppFlags & _U8_CONTINUOUS) )
  {
    PIE3bits.TMR1IE = 0;
    T1CONbits.ON = 0;
  }
 
} /* end TMR1_ISR */


/* Manage the system tick functionality using Timer 2 */
void __interrupt(irq(IRQ_TMR2), high_priority) TMR2_ISR(void)
{
  /* Clear the interrupt and sleep flags */
  PIR3bits.TMR2IF = 0;
  G_u8SystemFlags &= ~_SYSTEM_SLEEPING;
  
  /* Increment the system tick timer variable */
  G_u32SystemTime1ms++;
  
#if 0 /* For the sake of speed, don't bother with the 1s timer */
  if( (G_u32SystemTime1ms % 1000) == 0)
  {
    G_u32SystemTime1s++;
  }
#endif
 
} /* end TMR2_ISR */

void __interrupt(irq(IRQ_SPI1RX), high_priority) SPI1RX_ISR(void)
{
    PIR3bits.SPI1RXIF = 0;  //Clearing the SPI1RXI flag
    
    G_u8UserAppAlarmFlag = SPI1RXB;
}/* end __interrupt */