#include "RTCC.h"

#define RTTC_DELAY (500) // Update freq of the clock in ms.

typedef enum
{
    UPDATE = 0,
    SLEEP,
}enRTTCStateType;





u16 u16RTCCUpdateDelay = 0;
unTimeBufferType munTime; // Data from the RTTC module.
unTimeBufferType munTimeCheck; // Used for checking the data.

unTimeBufferType munSetTime;

void voPutBCD(u8 u8BCDByte);
void voReadRTCCValues(void);
void voUpdateRTCCValues(void);

/*
*************************************************************************
* FUNCTION NAME : voRTCCTaskInit
*
* DESCRIPTION : Init the RTCC module, enables the external RT clock
 * and sets the time.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/

u8 u8ToBCD(u16 u16Byte)
{
    return (((u16Byte / 10) * 16) + (u16Byte % 10));
}

void voSetRTCC(u16 u16Year,u16 u16Month,u16 u16Day,u16 u16Hour,u16 u16Min,u16 u16Sec)
{
    munSetTime.u8Year= u8ToBCD(u16Year);
    munSetTime.u8Month= u8ToBCD(u16Month);
    munSetTime.u8Day= u8ToBCD(u16Day);
    munSetTime.u8Hr= u8ToBCD(u16Hour);
    munSetTime.u8Min= u8ToBCD(u16Min);
    munSetTime.u8Sec= u8ToBCD(u16Sec);

    // unlock sequence for the set date and time registry.
    asm volatile("disi #5");
    asm volatile("mov #0x55, w7");
    asm volatile("mov w7,_NVMKEY");
    asm volatile("mov #0xAA, w8");
    asm volatile("mov w8,_NVMKEY");
    asm volatile("bset _RCFGCAL, #13"); // RTCWREN =1;
    asm volatile("nop");
    asm volatile("nop");

    RCFGCALbits.RTCEN = 0;
    RCFGCALbits.RTCPTR = 3;
    // Sets the time and date.
    RTCVAL = munSetTime.u16Ptr11; // Year
    RTCVAL = munSetTime.u16Ptr10; // Month/day
    RTCVAL = munSetTime.u16Ptr01; // Weekday/hour
    RTCVAL = munSetTime.u16Ptr00; // min/sec

    RCFGCALbits.RTCEN = 1;
    RCFGCALbits.RTCWREN = 0;


}

void voRTCCTaskInit(void)
{
    // Unlock sequence for the external low freq crystal.
    asm volatile("disi #0x3FFF");
    asm volatile ("mov #OSCCON,W1");
    asm volatile ("mov.b #0x46, W2");
    asm volatile ("mov.b #0x57, W3");
    asm volatile ("mov.b #0x02, W0"); // SOSCEN =1
    asm volatile ("mov.b W2, [W1]");
    asm volatile ("mov.b W3, [W1]");
    asm volatile ("mov.b W0, [W1]");
    
    // unlock sequence for the set date and time registry.
    asm volatile("disi #5");
    asm volatile("mov #0x55, w7");
    asm volatile("mov w7,_NVMKEY");
    asm volatile("mov #0xAA, w8");
    asm volatile("mov w8,_NVMKEY");
    asm volatile("bset _RCFGCAL, #13"); // RTCWREN =1;
    asm volatile("nop");
    asm volatile("nop");

    RCFGCALbits.RTCEN = 0;
    RCFGCALbits.RTCPTR = 3;
    // Sets the time and date.
    RTCVAL = 0x2013; // Year
    RTCVAL = 0x1005; // Month/day
    RTCVAL = 0x0216; // Weekday/hour
    RTCVAL = 0x3100; // min/sec

    RCFGCALbits.RTCEN = 1;
    RCFGCALbits.RTCWREN = 0;

}
/*
*************************************************************************
* FUNCTION NAME : voPutBCD
*
* DESCRIPTION : Put a BCDByte to stdout.
*
* INPUT : u8BCDByte, one byte of BCD code.
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/
void voPutBCD(u8 u8BCDByte)
{
    u8 u8HByte = (u8BCDByte >> 4) + 0x30;
    u8 u8LByte = (u8BCDByte & 0x0F) + 0x30;
    
    putchar(u8HByte);
    putchar(u8LByte);

}
/*
*************************************************************************
* FUNCTION NAME : voReadRTCCValues
*
* DESCRIPTION : Update the two unions munTime and munTimeCheck
 * with the actual data from the RTCC module.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/
void voReadRTCCValues(void)
{
    RCFGCALbits.RTCPTR = 3;
    munTime.u16Ptr11 = RTCVAL;
    munTime.u16Ptr10 = RTCVAL;
    munTime.u16Ptr01 = RTCVAL;
    munTime.u16Ptr00 = RTCVAL;

    RCFGCALbits.RTCPTR = 3;
    munTimeCheck.u16Ptr11 = RTCVAL;
    munTimeCheck.u16Ptr10 = RTCVAL;
    munTimeCheck.u16Ptr01 = RTCVAL;
    munTimeCheck.u16Ptr00 = RTCVAL;
}
/*
*************************************************************************
* FUNCTION NAME : voUpdateRTCCValues
*
* DESCRIPTION : Checks if rollover has happend by rereading the values
 * and comparing them until they are equal. The updated values is stored
 * in munTime.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/
void voUpdateRTCCValues(void)
{
    do
    {
        voReadRTCCValues();
    }
    while (!((munTime.u16Ptr11 == munTimeCheck.u16Ptr11) &&
           (munTime.u16Ptr10 == munTimeCheck.u16Ptr10) &&
           (munTime.u16Ptr01 == munTimeCheck.u16Ptr01) &&
           (munTime.u16Ptr00 == munTimeCheck.u16Ptr00)));
    
    

}
/*
*************************************************************************
* FUNCTION NAME : voRTCCTask
*
* DESCRIPTION : Displays the real time clock on stdout
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/
void voRTCCTask(void)
{
    static enRTTCStateType senRTTCState = UPDATE;
    
    switch (senRTTCState)
    {
        case UPDATE:
            voUpdateRTCCValues();
            voLCDHome();
            voPutsLCD((s8*)"20");
            voPutBCD(munTime.u8Year);
            putchar(':');
            voPutBCD(munTime.u8Month);
            putchar(':');
            voPutBCD(munTime.u8Day);

            voLCDRowTwo();

            voPutBCD(munTime.u8Hr);
            putchar(':');
            voPutBCD(munTime.u8Min);
            putchar(':');
            voPutBCD(munTime.u8Sec);
            
            
            DISABLE_INTERRUPTS(u16RTCCUpdateDelay = RTTC_DELAY);
            senRTTCState = SLEEP;
           
        break;
        
        case SLEEP:
            if (!u16RTCCUpdateDelay)
                senRTTCState = UPDATE;
        break;
        
        default:
        break;
    }
}