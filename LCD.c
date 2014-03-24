#include "LCD.h"

// Commands
#define NEXT_ROW (0b11000000)
#define CLEAR (0b00000001)
#define HOME (0b00000010)

u8 mu8LCDDelay = 0;

u8 u8LCDRead(void);

/*
*************************************************************************
* FUNCTION NAME : voInitLCD
*
* DESCRIPTION : Initalization of the LCD 44780 display.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : This function will use blocking delays. And it will modify the 
 * TMR1 registry to get delays below 1ms.
*
*************************************************************************
*/
void voInitLCD(void)
{
    TMR1 = 0;
    DISABLE_INTERRUPTS(mu8LCDDelay = 32); while (mu8LCDDelay); // 32ms init delay

    PMADDR = LCDCMD;
    PMDIN1 = 0b00111000; // funcion set: 8-bit interface, 2 lines, 5x7
    TMR1 = 0; while (TMR1 < 192); //250 ns * 192 = 48us delay

    PMDIN1 = 0b00001100; // Display ON, cursor off, blink off
    TMR1 = 0; while (TMR1 < 192); //250 ns * 192 = 48us delay

    PMDIN1 = 0b00000001; // Clear display.
    TMR1 = 1600;
    DISABLE_INTERRUPTS(mu8LCDDelay = 2); while (mu8LCDDelay); // 1.6ms delay

    PMDIN1 = 0b00000110; // Increment cursor, no shift.
    TMR1 = 1600;
    DISABLE_INTERRUPTS(mu8LCDDelay = 2); while (mu8LCDDelay); // 1.6ms delay
}
/*
*************************************************************************
* FUNCTION NAME : u8LCDRead
*
* DESCRIPTION : Read the HD44780 registry.
*
* INPUT : None
*
* OUTPUT : HD44780 registry
*
* NOTE : Uses blocking delays.
*
*************************************************************************
*/
u8 u8LCDRead(void)
{
    u16 dummy = 0;
    
    while(PMMODEbits.BUSY);
    PMADDR = LCDCMD;
    dummy = PMDIN1;
    while(PMMODEbits.BUSY);
    return PMDIN1;
}
/*
*************************************************************************
* FUNCTION NAME : voLCDWrite
*
* DESCRIPTION : Write data to the HD44780 display.
*
* INPUT : u8Data, u16LCDAddr
*
* OUTPUT : None
*
* NOTE : Uses blocking delays. if LCDCMD is used as adress commands will
 * be passed to the display. If LCDDATA is used a character will be
 * writen on the display.
*
*************************************************************************
*/
void voLCDWrite(u8 u8Data, u16 u16LCDAddr)
{
    while(u8LCDRead() & 0x80); // read lcd busy flag
    while(PMMODEbits.BUSY);
    PMADDR = u16LCDAddr;
    PMDIN1 = u8Data;
}
/*
*************************************************************************
* FUNCTION NAME : voPutsLCD
*
* DESCRIPTION : Write a text string to the display.
*
* INPUT : s8StringPtr
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/
void voPutsLCD(s8 *s8StringPtr)
{
    while (*s8StringPtr)
        voLCDWrite(*s8StringPtr++,LCDDATA);
}
/*
*************************************************************************
* FUNCTION NAME : voLCDHome
*
* DESCRIPTION : Returns the cursor to adress 0.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/
void voLCDHome(void)
{
    voLCDWrite(HOME,LCDCMD);
   
}

/*
*************************************************************************
* FUNCTION NAME : voPutCharLCD
*
* DESCRIPTION : Simple interface to writa a char to the LCD.
*
* INPUT : u8byte
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/

void voPutCharLCD(u8 u8Byte)
{
    voLCDWrite(u8Byte, LCDDATA);
}

/*
*************************************************************************
* FUNCTION NAME : voLCDRowTwo
*
* DESCRIPTION : Set the cursor to next line addr 64.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE :
*
*************************************************************************
*/
void voLCDRowTwo(void)
{
   voLCDWrite(NEXT_ROW,LCDCMD);
}
/*
*************************************************************************
* FUNCTION NAME : voLCDClear
*
* DESCRIPTION : Clear the screen of the display.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE :
*
*************************************************************************
*/
void voLCDClear(void)
{
    voLCDWrite(CLEAR,LCDCMD);
}