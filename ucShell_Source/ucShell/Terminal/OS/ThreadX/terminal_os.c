
 
 /*
 *********************************************************************************************************
 *                                            INCLUDE FILES
 *********************************************************************************************************
 */
 
 #include  <terminal.h>
 
#include   "tx_api.h"

#include "sample_cfg.h"
 
 
 /*
 *********************************************************************************************************
 *                                            LOCAL DEFINES
 *********************************************************************************************************
 */

 
 /*
 *********************************************************************************************************
 *
 *                                              TERMINAL
 *
 *                                           VT100 MODE PORT
 *
 * Filename : terminal_mode.c
 * Version  : V1.04.00
 *********************************************************************************************************
 * Note(s)  : (1) ECMA-48 'Control Functions for Coded Character Sets' (5th edition), standardizes
 *                a set of terminal emulation commands.  The ISO/IEC and ANSI issued corresponding
 *                standards, ISO/IEC 6429 and ANSI X3.64, respectively.  The DEC VT100 video terminal
 *                implemented a command set which conformed to ANSI X3.64, so this type of terminal
 *                emulation is often known as VT100.
 *
 *            (2) To use this terminal mode, the terminal program employed (Hyperterminal, puTTY,
 *                etc.) should be setup in VT100 mode and character echo should be disabled.
 *                Depending on the command set used, it may be necessary to generate a CR ('\r') on
 *                each LF ('\n').
 *
 *            (3) Only the several ECMA-48 commands relevant to terminal operation are supported.
 *                These are :
 *
 *                (a) Cursor Up.
 *                (b) Cursor Down.
 *                (c) Cursor Left.
 *                (d) Cursor Right.
 *
 *                In addition, several editing keys are supported :
 *
 *                (a) Insert.
 *                (b) Delete.
 *                (c) End.
 *                (d) Home.
 **
 *            (4) Only 7-bit mode is supported.
 *********************************************************************************************************
 */
 
 
 /*
 *********************************************************************************************************
 *                                            INCLUDE FILES
 *********************************************************************************************************
 */
 
#include  <terminal.h>
 
 /*
 *********************************************************************************************************
 *                                            LOCAL DEFINES
 *********************************************************************************************************
 */
 

 
 /*
 *********************************************************************************************************
 *                                       LOCAL GLOBAL VARIABLES
 *********************************************************************************************************
 */

extern TX_BYTE_POOL            byte_pool_0;
static  TX_THREAD   Terminal_OS_Thread;
 


 /*
 *********************************************************************************************************
 *                                      LOCAL FUNCTION PROTOTYPES
 *********************************************************************************************************
 */
 
  
 
 
 /*
 *********************************************************************************************************
 *                                      LOCAL FUNCTION PROTOTYPES
 *********************************************************************************************************
 */
 
 static  void  Terminal_OS_Task(ULONG thread_entry);
 
 /*
 *********************************************************************************************************
 *                                         Terminal_OS_Init()
 *
 * Description : Initialize the terminal task.
 *
 * Argument(s) : p_arg       Argument to pass to the task.
 *
 * Return(s)   : DEF_FAIL    Initialize task failed.
 *               DEF_OK      Initialize task successful.
 *
 * Caller(s)   : Terminal_Init()
 *
 * Note(s)     : The RTOS needs to create Terminal_OS_Task().
 *********************************************************************************************************
 */
 
 CPU_BOOLEAN  Terminal_OS_Init (void *p_arg)
 {
    UINT  status;
    CHAR    *pointer = TX_NULL;
    /* Allocate the stack for thread 1.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, TERMINAL_OS_CFG_TASK_STK_SIZE, TX_NO_WAIT);
    if ( status != TX_SUCCESS){        
        Error_Handler();
    }

    status = tx_thread_create(&Terminal_OS_Thread, "Terminal", Terminal_OS_Task, 1,  
                        pointer, TERMINAL_OS_CFG_TASK_STK_SIZE, 
                        TERMINAL_OS_CFG_TASK_PRIO, TERMINAL_OS_CFG_TASK_THRESHOLD_PRIO, 4, TX_AUTO_START);

     if ( status != TX_SUCCESS){        
         Error_Handler();
     }

     if ( status != TX_SUCCESS){
//        Error_Handler();
        return (DEF_FAIL);
     }
 
     return (DEF_OK);
 }



 /*
 *********************************************************************************************************
 *                                         Terminal_OS_Task()
 *
 * Description : RTOS interface for terminal main loop.
 *
 * Argument(s) : p_arg       Argument to pass to the task.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : RTOS.
 *
 * Note(s)     : none.
 *********************************************************************************************************
 */

 
 static  void  Terminal_OS_Task (ULONG thread_entry)
 {
   
     Terminal_Task_Direct ( );
      
 }




/*
*********************************************************************************************************
*                                         TerminalMode_Clr()
*
* Description : Clear the terminal line.
*
* Argument(s) : nbr_char    Number of characters on line.
*
*               cursor_pos  Current cursor position.
*
* Return(s)   : none.
*
* Caller(s)   : Terminal_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  TerminalMode_Clr (CPU_SIZE_T  nbr_char,
                        CPU_SIZE_T  cursor_pos)
{
    CPU_SIZE_T  ix;


    for (ix = cursor_pos; ix < nbr_char; ix++) {
        Terminal_WrStr((CPU_CHAR *)" ", 1u);
    }
    for (ix = 0; ix < nbr_char; ix++) {
        Terminal_WrStr((CPU_CHAR *)"\b \b", 3u);
    }
}


/*
*********************************************************************************************************
*                                       TerminalMode_NewLine()
*
* Description : Move terminal to new line.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Terminal_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  TerminalMode_NewLine (void)
{
    Terminal_WrStr((CPU_CHAR *)"\r\n", 2u);
}


/*
*********************************************************************************************************
*                                        TerminalMode_Prompt()
*
* Description : Show prompt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Terminal_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  TerminalMode_Prompt (void)
{
    Terminal_WrStr((CPU_CHAR *)"\r\n> ", 4u);
}

