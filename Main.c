/* 
 * File:   Main.c
 * Author: lfnil_000
 *
 * Created on den 30 augusti 2013, 14:11
 */
// CONFIG2
#pragma config POSCMOD = HS             // Primary Oscillator Select (HS Oscillator mode selected)
#pragma config OSCIOFNC = ON            // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as port I/O (RC15))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRI            // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))
#pragma config IESO = OFF               // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) disabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)          // JTAG Port Enable (JTAG port is disabled)

/*
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "LCD.h"
#include "DataTypes.h"
#include "KnightRider.h"
#include "RTCC.h"
#include "CommRS232.h"
#include "Logger.h"

void voBoardInit(void);
int main(void);

int main(void) {

    voLoggerTaskInit();
    voBoardInit();
    voRTCCTaskInit();
    voInitLCD();
    
    voCommTaskInit();

    while (1)
    {
        
        voKnightRiderTask();
        voRTCCTask();
        voCommTask();
        voLoggerTask();
        
    }
    
}

/*
*************************************************************************
* FUNCTION NAME : voBoardInit
*
* DESCRIPTION : Init the board, and setup timer1 and the PMP port so its able
 * to talk to the HD44780 LCD unit.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : setup info:
  * PORTA to be all output.
 * timer1 to be:
  * Prescaler: 1
  * PR1: 4000
  * with makes it tick at 1ms with a clock of 8Mhz
 * Parallell Master Port:
  * 
 *
 * .
*
*************************************************************************
*/
void voBoardInit(void)
{
    // PORT init
    TRISA = 0x0000;
    PORTA = 0x0000;
    TRISFbits.TRISF0 = 1;
    

    // timer1 init
    T1CONbits.TON = 1;
    T1CONbits.TCKPS = 0x0;
    TMR1 = 0x00;
    PR1 = 4000;

    IPC0bits.IC1IP = 0x01;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;

    // PMP init
    PMCON = 0x83BF;
    PMMODE = 0x3FF;
    PMAEN = 0x0001;
}
