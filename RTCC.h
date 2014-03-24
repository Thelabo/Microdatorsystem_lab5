/* 
 * File:   RTCC.h
 * Author: lfnil_000
 *
 * Created on den 13 september 2013, 15:59
 */

#ifndef RTCC_H
#define	RTCC_H

#include "xc.h"
#include "DataTypes.h"
#include "LCD.h"
#include <stdio.h>

/*
 * Union to hold all the data taken from the RTTC module
 * the raw registers is stored under u16PtrXX, and
 * direct access to the data is under the form u8Type.
 * Tex: u8Sec gets the BCDbyte for seconds.
 */
typedef union
{
    struct
    {
        u8 u8Sec;
        u8 u8Min;
        u8 u8Hr;
        u8 u8WeekDay;
        u8 u8Day;
        u8 u8Month;
        u8 u8Year;

    };
    struct
    {
        u16 u16Ptr00;
        u16 u16Ptr01;
        u16 u16Ptr10;
        u16 u16Ptr11;
    };
}unTimeBufferType;


void voRTCCTaskInit(void);
void voRTCCTask(void);
void voSetRTCC(u16 u16Year,u16 u16Month,u16 u16Day,u16 u16Hour,u16 u16Min,u16 u16Sec);

#endif	/* RTCC_H */

