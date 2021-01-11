/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                    Copyright 2003-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                   MODBUS uC/OS-III LAYER INTERFACE
*
* Filename : mb_os.c
* Version  : V2.14.00
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define   MB_OS_MODULE
#include <mb.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/
extern TX_BYTE_POOL            byte_pool_0;

/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
//static  OS_SEM     MB_OS_RxSemTbl[MODBUS_CFG_MAX_CH];

static TX_SEMAPHORE            MB_OS_RxSemTbl[MODBUS_CFG_MAX_CH];


#endif

#if (MODBUS_CFG_SLAVE_EN  == DEF_ENABLED)


#define MB_OS_CFG_RX_TASK_STK_SIZE         512

static TX_THREAD     MB_OS_RxTaskTCB;

#define DEMO_QUEUE_SIZE         1u

static TX_QUEUE    MB_OS_SlaveRxQueue;



#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static  void  MB_OS_InitMaster(void);
static  void  MB_OS_ExitMaster(void);
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static  void  MB_OS_InitSlave (void);
static  void  MB_OS_ExitSlave (void);
//static  void  MB_OS_RxTask    (void  *p_arg);
static void    MB_OS_RxTask(ULONG thread_input);
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              MB_OS_Init()
*
* Description : This function initializes the RTOS interface.  This function creates the following:
*
*               (1) A message queue to signal the reception of a packet.
*
*               (2) A task that waits for packets to be received.
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : MB_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_OS_Init (void)
{
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    MB_OS_InitMaster();
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
    MB_OS_InitSlave();
#endif
}


/*
*********************************************************************************************************
*                                          MB_OS_InitMaster()
*
* Description : This function initializes and creates the kernel objectes needed for Modbus Master
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_OS_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static  void  MB_OS_InitMaster (void)
{
    CPU_INT08U   i;
    OS_ERR       err;


    for (i = 0; i < MODBUS_CFG_MAX_CH; i++) {                 /* Create a semaphore for each channel   */
          /* Create the semaphore used by threads 3 and 4.  */
        tx_semaphore_create(&MB_OS_RxSemTbl[i], "uC/Modbus Rx Sem", 0);
    }
}
#endif


/*
*********************************************************************************************************
*                                          MB_OS_InitSlave()
*
* Description : This function initializes and creates the kernel objectes needed for Modbus Salve
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_OS_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static  void  MB_OS_InitSlave (void)
{
//    OS_ERR  err;
    CHAR    *pointer = TX_NULL;
    UINT    reslts;

    
/* Allocate the stack for thread 0.  */
/* 为线程 0 分配堆栈。 */
    reslts = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, MB_OS_CFG_RX_TASK_STK_SIZE, TX_NO_WAIT);
    if ( reslts != TX_SUCCESS){
//          Error_Handler();
      }

    /* Create the main thread.  */
    reslts = tx_thread_create(&MB_OS_RxTaskTCB, "Modbus Rx Task", MB_OS_RxTask, 0,  
            pointer, MB_OS_CFG_RX_TASK_STK_SIZE, 
            3, 3, TX_NO_TIME_SLICE, TX_DONT_START);

    // 创建消息队列
    /* Allocate the message queue.  */
    reslts = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);

    if (reslts != TX_SUCCESS){

#if     (RTT_PRINTF_EN == DEF_ENABLED)           
        SEGGER_RTT_printf(0, "Modbus Allocate the message queue fail.\r\n");  
#endif
      Error_Handler();
    }
      
    /* Create the message queue .  */
  reslts = tx_queue_create (&MB_OS_SlaveRxQueue, "uC/Modbus Slave rx queue", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));
    if (reslts != TX_SUCCESS){
#if     (RTT_PRINTF_EN == DEF_ENABLED)           
            SEGGER_RTT_printf(0, "uC/Modbus Slave rx queue Create the message queue fail.\r\n"); 
#endif
      Error_Handler();

    }
   
    tx_thread_resume(&MB_OS_RxTaskTCB);

                
}
#endif


/*
*********************************************************************************************************
*                                             MB_OS_Exit()
*
* Description : This function is called to terminate the RTOS interface for Modbus channels.  We delete
*               the following uC/OS-II objects:
*
*               (1) An event flag group to signal the reception of a packet.
*               (2) A task that waits for packets to be received.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_OS_Exit (void)
{
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    MB_OS_ExitMaster();
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
    MB_OS_ExitSlave();
#endif
}


/*
*********************************************************************************************************
*                                          MB_OS_ExitMaster()
*
* Description : This function is called to terminate the RTOS interface for Modbus Master channels.  The
*               following objects are deleted.
*
*               (1) An event flag group to signal the reception of a packet.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_OS_Exit().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static  void  MB_OS_ExitMaster (void)
{
    CPU_INT08U  i;
    OS_ERR      err;
    UINT    reslt;


    for (i = 0; i < MODBUS_CFG_MAX_CH; i++) {                 /* Delete semaphore for each channel     */
        reslt = tx_semaphore_delete(&MB_OS_RxSemTbl[i]);
    }
}
#endif


/*
*********************************************************************************************************
*                                          MB_OS_ExitSlave()
*
* Description : This function is called to terminate the RTOS interface for Modbus Salve channels.
*               The following objects are deleted.
*
*               (1) A task that waits for packets to be received.
*               (2) A message queue to signal the reception of a packet.
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : MB_OS_Exit().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
void  MB_OS_ExitSlave (void)
{
//    OS_ERR  err;

    UINT    reslt = tx_thread_delete(&MB_OS_RxTaskTCB);


}
#endif


/*
*********************************************************************************************************
*                                              MB_OS_RxSignal()
*
* Description : This function signals the reception of a packet either from the Rx ISR(s) or the RTU timeout
*               timer(s) to indicate that a received packet needs to be processed.
*
* Argument(s) : pch     specifies the Modbus channel data structure in which a packet was received.
*
* Return(s)   : none.
*
* Caller(s)   : MB_ASCII_RxByte(),
*               MB_RTU_TmrUpdate().
*
* Note(s)     : none.
*********************************************************************************************************
*/
//MODBUS_CH *temp_pch;

void  MB_OS_RxSignal (MODBUS_CH *pch)
{
//    OS_ERR  err;
    UINT            status;


    if (pch != (MODBUS_CH *)0) {
        switch (pch->MasterSlave) {
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
            case MODBUS_MASTER:
             status = tx_semaphore_put(&MB_OS_RxSemTbl[pch->Ch]);       // 将收到的数据传给应用任务
                 break;
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
            case MODBUS_SLAVE:
            default:
              //将收到的数据传送给任务
              /* Send message to queue 0.  */
              status =  tx_queue_send(&MB_OS_SlaveRxQueue, &pch, TX_NO_WAIT);
#if     (RTT_PRINTF_EN == DEF_ENABLED)
                      SEGGER_RTT_printf(0, "Modbus read data rx ctr: %u.\r\n", pch->RxCtr);      
#endif

              /* Check completion status.  */
              if (status != TX_SUCCESS)
              break;
//                 (void)OSTaskQPost(&MB_OS_RxTaskTCB,
//                                    pch,
//                                    sizeof(void *),
//                                    OS_OPT_POST_FIFO,
//                                   &err);
                 break;
#endif
        }
    }
}


/*
*********************************************************************************************************
*                                              MB_OS_RxWait()
*
* Description : This function waits for a response from a slave.
*
* Argument(s) : pch     specifies the Modbus channel data structure to wait on.
*
*               perr    is a pointer to a variable that will receive an error code.  Possible errors are:
*
*                       MODBUS_ERR_NONE        the call was successful and a packet was received
*                       MODBUS_ERR_TIMED_OUT   a packet was not received within the specified timeout
*                       MODBUS_ERR_NOT_MASTER  the channel is not a Master
*                       MODBUS_ERR_INVALID     an invalid error was detected
*
* Return(s)   : none.
*
* Caller(s)   : MBM_FCxx()  Modbus Master Functions
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  MB_OS_RxWait (MODBUS_CH   *pch,
                    CPU_INT16U  *perr)
{
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    OS_ERR  err;
    CPU_TS  ts;
    UINT status;


    if (pch != (MODBUS_CH *)0) {
        if (pch->MasterSlave == MODBUS_MASTER) {
          // 等待信号量,直到收到为止
           status = tx_semaphore_get(&MB_OS_RxSemTbl[pch->Ch], pch->RxTimeout);  
        switch (status) {
                case TX_DELETED:
                case TX_NO_INSTANCE:
                case TX_SEMAPHORE_ERROR:
                case TX_NO_INSTANCE:
                     *perr = MODBUS_ERR_INVALID;
                     break;

                case TX_WAIT_ERROR:
                     *perr = MODBUS_ERR_TIMED_OUT;
                     break;

                case  TX_SUCCESS:
                     *perr = MODBUS_ERR_NONE;
                     break;
            }
      
        } else {
            *perr = MODBUS_ERR_NOT_MASTER;
        }
    } else {
        *perr = MODBUS_ERR_NULLPTR;
    }
#else
    *perr = MODBUS_ERR_INVALID;
#endif
}

/*
*********************************************************************************************************
*                                            MB_OS_RxTask()
*
* Description : This task is created by MB_OS_Init() and waits for signals from either the Rx ISR(s) or
*               the RTU timeout timer(s) to indicate that a packet needs to be processed.
*
* Argument(s) : p_arg       is a pointer to an optional argument that is passed by uC/OS-II to the task.
*                           This argument is not used.
*
* Return(s)   : none.
*
* Caller(s)   : This is a Task.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
//static  void  MB_OS_RxTask (void *p_arg)
static void    MB_OS_RxTask(ULONG thread_input)

{
//    OS_ERR       err;
//    OS_MSG_SIZE  msg_size;
//    CPU_TS       ts;
    MODBUS_CH   *pch;         // 收到数据指针
    UINT    status;
    ULONG   received_message;


//    (void)p_arg;

    while (DEF_TRUE) {
//        pch = (MODBUS_CH *)OSTaskQPend(0,        /* Wait for a packet to be received                   */
//                                       OS_OPT_PEND_BLOCKING,
//                                       &msg_size,
//                                       &ts,
//                                       &err);

          /* Retrieve a message from the queue.  */
        status = tx_queue_receive(&MB_OS_SlaveRxQueue, &received_message, TX_WAIT_FOREVER);

        /* Check completion status and make sure the message is what we 
           expected.  */
        if (status != TX_SUCCESS){
          Error_Handler();
            continue;      
        }
        pch = (MODBUS_CH   *)received_message;
        MB_RxTask(pch);                          /* Process the packet received                        */
    }
}


#endif

