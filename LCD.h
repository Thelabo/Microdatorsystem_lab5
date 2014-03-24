/* 
 * File:   LCD.h
 * Author: lfnil_000
 *
 * Created on den 12 september 2013, 11:11
 */

#ifndef LCD_H
#define	LCD_H
#include "DataTypes.h"
#include "xc.h"


// Adresses
#define LCDDATA (1)
#define LCDCMD (0)




void voInitLCD(void);
void voPutsLCD(s8 *s8StringPtr);
void voLCDHome(void);
void voLCDRowTwo(void);
void voLCDClear(void);
void voPutCharLCD(u8 u8Byte);
#endif	/* LCD_H */

