#include "Logger.h"
#include "FSIO.h"
#include "RTCC.h"

#define TMP_S (4)
#define ONE_SEC_DELAY (1000)
#define LOG_FILENAME ("tmp.log")
#define APPEND ("a")
#define CARD_AVAILIBLE (!PORTFbits.RF0)
extern unTimeBufferType munTime;
u16 mu16LoggerDelay = 0;
typedef enum
{
    INIT = 0,
    DELAYINIT,
    WAIT1S,
    STARTAD,
    BUSYAD,
    STORE,
    ERROR,
    WAIT_FOR_CARD,

} enLoggerStateType;

u16 u16RecursiveFilther();
/*
*************************************************************************
* FUNCTION NAME : voLoggerTaskInit
*
* DESCRIPTION : Init the ADC.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : setup info:
*
*************************************************************************
*/
void voLoggerTaskInit()
{
    AD1PCFG = 0xFFEF; // select with pins are analog in PORTB
    AD1CON1 = 0x00E0; // autosampling
    AD1CSSL = 0; // no scanning
    AD1CON3 = 0x1F02; // max sample time.
    AD1CON2 = 0;
    AD1CON1bits.ADON = 1; // turn on the adc.

}
/*
*************************************************************************
* FUNCTION NAME : voLoggerTask
*
* DESCRIPTION : Handles the temperatur logging to sd card.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : 
*
*************************************************************************
*/
void voLoggerTask(void)
{
    static enLoggerStateType senLoggerState= INIT;
    static enLoggerStateType senLoggerPrevState= INIT;
    static FSFILE *sLogFilePtr = NULL;
    static s8 ss8CurrentTMP=0;
    CETYPE errorCode;




    switch (senLoggerState)
    {
        /*
         * Init the sd card, goes to wait for card if no card is in socket.
         * if error ocured goes to error state, this is not implemented yet!
         */
        case INIT:
            

            
            if (CARD_AVAILIBLE)
            {
                if (FSInit())
                    senLoggerState = DELAYINIT;
                else
                    senLoggerState = ERROR;
            }
            else
            {
                senLoggerPrevState = senLoggerState;
                senLoggerState = WAIT_FOR_CARD;
            }


        break;
        /*
         * Load delay and goes to wait1sec state.
         */
        case DELAYINIT:

            DISABLE_INTERRUPTS(mu16LoggerDelay = ONE_SEC_DELAY );
            senLoggerState = WAIT1S;

        break;
        /*
         * delay for 1 sec.
         */
        case WAIT1S:

            if (!mu16LoggerDelay)
                senLoggerState = STARTAD;

        break;

        /*
         * Start ADC sampling
         */
        case STARTAD:
            
            AD1CHS = TMP_S;
            AD1CON1bits.SAMP = 1;

            senLoggerState = BUSYAD;
        break;

        /*
         * Wait until sampling is completed.
         */
        case BUSYAD:

            if (AD1CON1bits.DONE)
                senLoggerState = STORE;
        break;

        /*
         * Rechecks that the sd card is in the socket, if so the adc value is
         * first run thrue a recursive filther then converted to degrees,
         * and added to sd card.
         */
        case STORE:
            
            if (CARD_AVAILIBLE)
            {
                ss8CurrentTMP = ((u32)330*u16RecursiveFilther() - 51200)/1024;

                sLogFilePtr = FSfopen(LOG_FILENAME,APPEND);

                if (sLogFilePtr != NULL)
                {
                    FSfprintf(sLogFilePtr,"%x-%x-%x %x:%x:%x    %d \r\n",
                            munTime.u8Year,
                            munTime.u8Month,
                            munTime.u8Day,
                            munTime.u8Hr,
                            munTime.u8Min,
                            munTime.u8Sec,
                            ss8CurrentTMP);

                    if (FSfclose(sLogFilePtr) < 0)
                        senLoggerState = ERROR;
                    else
                        senLoggerState = DELAYINIT;
                }
                else
                    senLoggerState = ERROR;
            }
            else
            {
                senLoggerPrevState = senLoggerState;
                senLoggerState = WAIT_FOR_CARD;
            }


        break;
        /*
         * error handling should be inplemented here #todo
        */
        case ERROR:

            errorCode = FSerror();

            

            senLoggerState = INIT;

        break;

        case WAIT_FOR_CARD:

            if (CARD_AVAILIBLE)
                senLoggerState = INIT;


        break;

        default:

            senLoggerState = INIT;

        break;

    }
}
/*
*************************************************************************
* FUNCTION NAME : u16RecursiveFilther
*
* DESCRIPTION : Recursive filther to smothen the ADC readings.
*
* INPUT : None
*
* OUTPUT : Smothed value u16.
*
* NOTE :
*
*************************************************************************
*/
u16 u16RecursiveFilther()
{
    static s16 ss16CurrentADCValue = 0;
    if (ss16CurrentADCValue == 0)
        ss16CurrentADCValue = ADC1BUF0;
    else
        ss16CurrentADCValue = (875*(u32)ss16CurrentADCValue + (u32)ADC1BUF0*125)/1000;
    return ss16CurrentADCValue;
}