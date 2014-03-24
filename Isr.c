#include "DataTypes.h"
#include "xc.h"

#define RTS _RF13
#define REJECT (1)
extern u8 mu8KnightRiderDelay;
extern u8 mu8LCDDelay;
extern u16 u16RTCCUpdateDelay;
extern u16 mu16LoggerDelay;

/*
*************************************************************************
* FUNCTION NAME : _T1Interrupt
*
* DESCRIPTION : Interrupt handler for timer1, counts down u8KnightRiderDelay
* when timer1 overflows.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : -
*
*************************************************************************
*/
void _ISR _T1Interrupt(void)
{
    if (mu8KnightRiderDelay > 0)
        mu8KnightRiderDelay--;

    if (mu8LCDDelay > 0)
        mu8LCDDelay--;

    if (u16RTCCUpdateDelay > 0)
        u16RTCCUpdateDelay--;

    if (mu16LoggerDelay > 0)
        mu16LoggerDelay--;

    IFS0bits.T1IF = 0;
}
/*
*************************************************************************
* FUNCTION NAME : _U2RXInterrupt
*
* DESCRIPTION : Interrupt handler for uart2, the interrupt will trigger
 * when the internal fifo is 3/4 full, the isr will then set RTS to 1.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : -
*
*************************************************************************
*/
void _ISR _U2RXInterrupt(void)
{
    IFS1bits.U2RXIF = 0;
    
    RTS = REJECT;
    
    
}
