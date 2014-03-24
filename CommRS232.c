#define BAUD9600 (103)
#define RTS _RF13
#define CTS_READY (!_RF12)
#define ACCEPT (0)
#define UART2_HAS_DATA (U2STAbits.URXDA)
#define TRANSMIT_BUFFER_READY (!U2STAbits.UTXBF)
#define END_STRING (0x0)

#define K_ENTER (0x0d)
#define K_BACKSPACE (0x7f)

#define MAX_BUFFER_SIZE (40)





#include "CommRS232.h"
#include "RTCC.h"


// Return codes from executeCommand
typedef enum
{
    NO_COMMAND=0,
    OK,
    SYNTAX,
    VALUE_ERROR,
    RET_VALUE,

}enCmdResult;

typedef enum
{
    RECEIVE = 0,
    PROCESS,
    ECHO_BYTE,
    EXECUTE,
    ECHO_STRING,
} enRS232StateType;
/*
 * union to hold data when using sscanf to make it memory efficient when adding
 * more commands.
 */
typedef union
{
    struct
    {
        u16 u16Year;
        u16 u16Month;
        u16 u16Day;
        u16 u16Hour;
        u16 u16Min;
        u16 u16Sec;
    }stDate;

}unDataStorage;


enCmdResult u8ExecuteCommand(u8 * u8StringPtr);
enCmdResult u8SetDateCmd(u8 *u8StringPtr, unDataStorage *unStorageBuffer);
u8 u8Range(u16 u16Value,u8 u8From, u8 u8To);

/*
*************************************************************************
* FUNCTION NAME : voCommTaskInit
*
* DESCRIPTION : Init Uart2 with a baud rate of 9600 bps, RTS and CTS enabled
 * interrupt is generated when the internal fifo is at 3/4 full.
*
* INPUT : s8StringPtr
*
* OUTPUT : None
*
* NOTE : 
 * 8 bit
 * no parity
 * one stopbit
 * RTS/CTS flowcontrol on PORTF 13/12
 * 9600 BAUD
*
*************************************************************************
*/
void voCommTaskInit(void)
{
    U2BRG = BAUD9600;
    U2MODE = 0x8008; // enable uart2, 8 bit no parity 1 stopbit.
    U2STA = 0x0480; // transmit enable, 3/4 fifo enable interrupt.
    IEC1bits.U2RXIE = 1; // enable intrerrupt.
    TRISFbits.TRISF13 = 0; // set RTS pin to output.
    
    RTS = 0;
    

}
/*
*************************************************************************
* FUNCTION NAME : voCommTask
*
* DESCRIPTION : State matchine for the console terminal. 
 *
 * SEND: Send back whatever data was sent, and change back to the idle state.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/
void voCommTask(void)
{
    static u8 su8RecivedByte = 0;
    static enRS232StateType senRS232State = RECEIVE;
    static u8 su8Buffer[MAX_BUFFER_SIZE];
    static u8 su8BufferPtr = 0;
    static enCmdResult senCmdStatus = OK;


    switch (senRS232State)
    {
        /*
         * Receive state is the idle state when no date is entered it stays here
         * and wait for new data.
         */
        case RECEIVE:

            if (UART2_HAS_DATA)
            {
                su8RecivedByte = U2RXREG; // one bit buffer for the current char
                senRS232State = PROCESS;
                
            }
            else
                RTS = ACCEPT;

        break;
        /*
         * The process state handles processing of ascii codes for
         * different keys pressed, atm only backspace and enter is inplemented
         * it also handles boundier checks for the stringBuffer.
         */
        case PROCESS:

            if (su8RecivedByte == K_ENTER)
            {
                senRS232State = EXECUTE;
                su8Buffer[su8BufferPtr] = END_STRING;
                su8BufferPtr = 0;
            }
            else if (su8RecivedByte == K_BACKSPACE)
            {
                if (su8BufferPtr > 0)
                {
                    su8BufferPtr--;
                    senRS232State = ECHO_BYTE;
                }
                else
                    senRS232State = RECEIVE;
            }
            else
            {

                if (su8BufferPtr < MAX_BUFFER_SIZE)
                {
                    su8Buffer[su8BufferPtr++] = su8RecivedByte;
                    senRS232State = ECHO_BYTE;
                }
                else
                    senRS232State = RECEIVE;
            }

        break;
        /*
         * Echo byte state is the state that echos whatever the user puts on
         * the serial consol back. It stays in this state until it can send
         * one byte back.
         */
        case ECHO_BYTE:

            if (CTS_READY && TRANSMIT_BUFFER_READY)
            {
                U2TXREG = su8RecivedByte; // send data from the one bit buffer.
                senRS232State = RECEIVE;
            }



        break;
        /*
         * Execute state is where commands get executed, the state also
         * handles status codes from the u8ExecuteCommand function.
         */
        case EXECUTE:

            senCmdStatus = u8ExecuteCommand(su8Buffer);
            if (senCmdStatus == OK)
                strcpy((char*)su8Buffer, "\n\rOK\r\n");

            else if (senCmdStatus == SYNTAX)
                strcpy((char*)su8Buffer, "\n\rSYNTAX ERROR\r\n");

            else if (senCmdStatus == NO_COMMAND)
                strcpy((char*)su8Buffer, "\n\rNO CMD\r\n");

            else if (senCmdStatus == VALUE_ERROR)
                strcpy((char*)su8Buffer, "\n\rVALUE ERROR\r\n");

            else if (senCmdStatus == RET_VALUE)
                strcat((char*)su8Buffer,"RET OK\r\n");
            
            else
                strcpy((char*)su8Buffer,"\n\rUNHANDLED ERROR\r\n");

            su8BufferPtr = 0;
            senRS232State = ECHO_STRING;


        break;
        /*
         * Echo string state send the current string in the stringbuffer back
         * to the user.
         */
        case ECHO_STRING:

            if (CTS_READY && TRANSMIT_BUFFER_READY)
            {
                U2TXREG = su8Buffer[su8BufferPtr++]; // send data from the one bit buffer.
                if (su8Buffer[su8BufferPtr] == END_STRING)
                {
                    senRS232State = RECEIVE;
                    su8BufferPtr = 0;
                }
            }

        break;

        default:
            senRS232State = RECEIVE;
        break;

    }
}
/*
*************************************************************************
* FUNCTION NAME : u8Range
*
* DESCRIPTION : A simple funktion that check if a value is within a certain
 * range.
*
* INPUT : u16Value, u8From,u8To
*
* OUTPUT : 1 for true 0 for false.
*
* NOTE : None
*
*************************************************************************
*/
u8 u8Range(u16 u16Value,u8 u8From, u8 u8To)
{
    return ((u16Value >= u8From) && (u16Value <= u8To));
}
/*
*************************************************************************
* FUNCTION NAME : voCommTask
*
* DESCRIPTION : State matchine for the echoing service, the two states is:
 * IDLE: Waiting for data to arrive change state when there is a byte
 * in uart2s internal fifo.
 *
 * SEND: Send back whatever data was sent, and change back to the idle state.
*
* INPUT : None
*
* OUTPUT : None
*
* NOTE : None
*
*************************************************************************
*/
enCmdResult u8HelpCmd(u8 *u8StringPtr, unDataStorage *unStorageBuffer)
{
    if (strncmp((const char*)u8StringPtr,"help",4) == 0)
    {
        strcpy((char*)u8StringPtr, "\n\rCMD: help,date\r\n");
        return RET_VALUE;
    }
    return NO_COMMAND;
}



/*
*************************************************************************
* FUNCTION NAME : u8SetDateCmd
*
* DESCRIPTION : command handler for the set date command, uses sscanf to fill
 * values to the unStorageBuffer, that used a union to save memory, to make it
 * scalable. The command handler has responsibility for handling all the error
 * and bounderis checks for the command.
*
* INPUT : u8StringPtr, unStorageBuffer
*
* OUTPUT : enCmdResult
*
* NOTE : Return codes:
 * VALUE_ERROR : describe bounderie errors and malformed data.
 * NO_COMMAND : Not this command.
 * SYNTAX : Syntax error, command entered the wrong way.
*
*************************************************************************
*/
enCmdResult u8SetDateCmd(u8 *u8StringPtr, unDataStorage *unStorageBuffer)
{
    /*
     * Check if its this command.
     */
    if (strncmp((const char*)u8StringPtr,"date ",5) == 0)
    {
        /*
         * Check if data is entered in the right format, and save it if its ok.
         */
        
        if (sscanf(((char*)u8StringPtr+4),"%d-%d-%d %d:%d:%d"  ,(int*)&unStorageBuffer->stDate.u16Year,
                                                                (int*)&unStorageBuffer->stDate.u16Month,
                                                                (int*)&unStorageBuffer->stDate.u16Day,
                                                                (int*)&unStorageBuffer->stDate.u16Hour,
                                                                (int*)&unStorageBuffer->stDate.u16Min,
                                                                (int*)&unStorageBuffer->stDate.u16Sec) == 6)
        {
         
        
            /*
             * Range checks start here, to make sure values entered is sane.
             */
            if (!u8Range(unStorageBuffer->stDate.u16Year,0,99))
                return VALUE_ERROR;

            if (!u8Range(unStorageBuffer->stDate.u16Month,1,12))
                return VALUE_ERROR;

            if (!u8Range(unStorageBuffer->stDate.u16Day,1,32))
                return VALUE_ERROR;

            if (!u8Range(unStorageBuffer->stDate.u16Hour,0,23))
                return VALUE_ERROR;

            if (!u8Range(unStorageBuffer->stDate.u16Min,0,59))
                return VALUE_ERROR;

            if (!u8Range(unStorageBuffer->stDate.u16Sec,0,59))
                return VALUE_ERROR;

            /*
             * If all is ok we execute the SetRTCC command and update
             * the RTCC clock and date. And return code OK. 
             */
            voSetRTCC(  unStorageBuffer->stDate.u16Year,
                        unStorageBuffer->stDate.u16Month,
                        unStorageBuffer->stDate.u16Day,
                        unStorageBuffer->stDate.u16Hour,
                        unStorageBuffer->stDate.u16Min,
                        unStorageBuffer->stDate.u16Sec);

            return OK;
        }
        return SYNTAX;


    }
    return NO_COMMAND;
}



/*
*************************************************************************
* FUNCTION NAME : u8ExecuteCommand
*
* DESCRIPTION : Handles execution of commands, and relaying of status messages.
 * Makes use of a function pointer array with different command handlers, this to
 * make it more scalable.
*
* INPUT : u8StringPtr
*
* OUTPUT : enCmdResult
*
* NOTE : Return codes:
 * VALUE_ERROR : describe bounderie errors and malformed data.
 * NO_COMMAND : Not this command.
 * SYNTAX : Syntax error, command entered the wrong way.
 * RET_VALUE : a value is passed from the handle to the buffer.
*
*************************************************************************
*/
enCmdResult u8ExecuteCommand(u8 * u8StringPtr)
{
    #define NUM_CMD (2)

    unDataStorage unStorageBuffer;
    enCmdResult enCmdStatus = NO_COMMAND;
    u8 index = 0;
    /*
     * Array with function pointers to the different type of command handlers.
     */
    enCmdResult (*u8Commands[NUM_CMD]) (u8 *u8StringPtr,unDataStorage *unStorageBuffer) =
    {
        u8SetDateCmd,
        u8HelpCmd
    };
    
    
    for (index = 0; index < NUM_CMD;index++)
    {
        enCmdStatus= (*u8Commands[index])(u8StringPtr,&unStorageBuffer);
        if (enCmdStatus)
            return enCmdStatus;
    }

    



    return NO_COMMAND;

}
