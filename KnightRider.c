#include "KnightRider.h"


/*
 * Global declerations.
 */

#define LED_TABLE_SIZE (9) // Size of the ledTable array.
#define KNIGHT_RIDER_DELAY (100) // Delay in ms.

u8 mu8KnightRiderDelay = 0;

u8 mu8LedTable[] = {
    0b10000000,
    0b11000000,
    0b01100000,
    0b00110000,
    0b00011000,
    0b00001100,
    0b00000110,
    0b00000011,
    0b00000001,
};

typedef enum
{
    LIGHT_LEDS=0,
    IDLE,
}enKnightRiderStateType ;

/*
 * End
 */

/*
*************************************************************************
* FUNCTION NAME : voKnightRiderTask
*
* DESCRIPTION : Display the KnightRider scanner on 8 leds.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : State matchine consists of two stats LIGHT_LEDS and IDLE.
 * LIGHT_LEDS = is the state when the enough time has passed and the current
 * light pattern is turned on.
 * IDLE = is the idle state, when the timer is not 0. 
*
*************************************************************************
*/
void voKnightRiderTask(void)
{
    static enKnightRiderStateType senKnightRiderState = LIGHT_LEDS;
    static u8 su8LedTableCursor = 0;
    static s8 ss8Inc = 1;


    switch (senKnightRiderState)
    {
        case LIGHT_LEDS:
            PORTA = mu8LedTable[su8LedTableCursor];
            su8LedTableCursor += ss8Inc;

            if (su8LedTableCursor == LED_TABLE_SIZE -1)
                ss8Inc = -1;
            if (su8LedTableCursor == 0)
                ss8Inc = 1;

            DISABLE_INTERRUPTS(mu8KnightRiderDelay = KNIGHT_RIDER_DELAY);
            senKnightRiderState = IDLE;
            break;

        case IDLE:
            if (mu8KnightRiderDelay == 0)
                senKnightRiderState = LIGHT_LEDS;
            break;

        default:
            senKnightRiderState = LIGHT_LEDS;
            break;
    }
}
