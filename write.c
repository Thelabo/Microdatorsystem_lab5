#include <xc.h>
#include <stdio.h>
#include "DataTypes.h"
#include "LCD.h"

u8 write(u8 u8Handle, void *u8Buffer, u8 u8Len)
{

    u8 u8Index= 0;
    switch(u8Handle)
    {
        case 1:
            
            for (u8Index=0;u8Index < u8Len; u8Index++)
                voPutCharLCD(((s8*)u8Buffer)[u8Index]);
                
        break;

        default:
            break;
    }
    return u8Len;
}
