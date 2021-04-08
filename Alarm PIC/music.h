/**********************************************************************************************************************
File: music.h      

Description:
Definitions for musical notes

***********************************************************************************************************************/


/* Note lengths */
#define MEASURE_TIME              (u16)2048  /* Time in ms for 1 measure (1 full note) - should be divisible by 16 */
#define FULL_NOTE                 (u16)(MEASURE_TIME)
#define HALF_NOTE                 (u16)(MEASURE_TIME / 2)
#define QUARTER_NOTE              (u16)(MEASURE_TIME / 4)
#define EIGHTH_NOTE               (u16)(MEASURE_TIME / 8)
#define SIXTEENTH_NOTE            (u16)(MEASURE_TIME / 16)

#define N1                        FULL_NOTE                 
#define N2                        HALF_NOTE                 
#define N4                        QUARTER_NOTE              
#define N8                        EIGHTH_NOTE               
#define N6                        SIXTEENTH_NOTE            

/* Note length adjustments */
#define REGULAR_NOTE_ADJUSTMENT   (u16)50
#define STACCATO_NOTE_TIME        (u16)75
#define HOLD_NOTE_ADJUSTMENT      (u16)0

#define RT                        REGULAR_NOTE_ADJUSTMENT
#define ST                        STACCATO_NOTE_TIME        
#define HT                        HOLD_NOTE_ADJUSTMENT            

/* Musical note definitions: 1/64th the total period in us */

#define NOTE_C4                   (u16)60  // f = 262 /* Middle C */
#define NOTE_D4                   (u16)53  // f = 294
#define NOTE_E4                   (u16)47  // f = 330
#define NOTE_F4                   (u16)45  // f = 349
#define NOTE_G4                   (u16)40  // f = 392
#define NOTE_G4S                  (u16)38  // f = 415
#define NOTE_A4                   (u16)36  // f = 440
#define NOTE_D5                   (u16)27  // f = 576
#define NOTE_NONE                 (u16)0

/* Musical note definitions - short hand */
#define C4                   (u16)NOTE_C4  /* Middle C */
#define D4                   (u16)NOTE_D4
#define E4                   (u16)NOTE_E4
#define F4                   (u16)NOTE_F4
#define G4                   (u16)NOTE_G4
#define G4S                  (u16)NOTE_G4S
#define A4                   (u16)NOTE_A4
#define D5                   (u16)NOTE_D5

#define NN                   (u16)NOTE_NONE