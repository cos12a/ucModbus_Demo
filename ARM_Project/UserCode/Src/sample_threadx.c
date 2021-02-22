/* This is a small demo of the high-performance ThreadX kernel.  It includes examples of eight
   threads of different priorities, using a message queue, semaphore, mutex, event flags group, 
   byte pool, and block pool.  */

#include   "tx_api.h"

#include <stdio.h>
#include "SEGGER_RTT.h"

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "dma.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

#include "mb.h"
//#include  <shell.h>



#define     DEMO_STACK_SIZE         1024
#define     DEMO_BYTE_POOL_SIZE     9120
#define     DEMO_BLOCK_POOL_SIZE    100
#define     DEMO_QUEUE_SIZE         100


/* Define the ThreadX object control blocks...  */

TX_THREAD               thread_0;
//TX_THREAD               thread_1;
//TX_THREAD               thread_2;
//TX_THREAD               thread_3;
//TX_THREAD               thread_4;
//TX_THREAD               thread_5;
//TX_THREAD               thread_6;
//TX_THREAD               thread_7;
//TX_QUEUE                queue_0;
//TX_SEMAPHORE            semaphore_0;
//TX_MUTEX                mutex_0;
//TX_EVENT_FLAGS_GROUP    event_flags_0;
TX_BYTE_POOL            byte_pool_0;
TX_BLOCK_POOL           block_pool_0;



/* Define byte pool memory.  */

UCHAR                   byte_pool_memory[DEMO_BYTE_POOL_SIZE];


/* Define event buffer.  */

#ifdef TX_ENABLE_EVENT_TRACE
UCHAR   trace_buffer[0x1000];
#endif


/* Define the counters used in the demo application...  */

ULONG           thread_0_counter = 0u;
ULONG           thread_1_counter = 0u;
ULONG           thread_1_messages_sent = 0u;
ULONG           thread_2_counter = 0u;
ULONG           thread_2_messages_received = 0u;
ULONG           thread_3_counter = 0u;
ULONG           thread_4_counter = 0u;
ULONG           thread_5_counter = 0u;
ULONG           thread_6_counter = 0u;
ULONG           thread_7_counter = 0u;


/* Define thread prototypes.  */

void    thread_0_entry(ULONG thread_input);
//void    thread_1_entry(ULONG thread_input);
//void    thread_2_entry(ULONG thread_input);
//void    thread_3_and_4_entry(ULONG thread_input);
//void    thread_5_entry(ULONG thread_input);
//void    thread_6_and_7_entry(ULONG thread_input);

void    modbus_thread_entry(ULONG thread_input);



/* Define what the initial system looks like.  */
/* 定义初始系统的外观�?*/


/* (void *first_unused_memory):定义未使用的内存指针�?第一个可用内存地址的值放置在低级初始化函数中�?
此变量的内容传递给应用程序的系统定义函数�?*/
// 定义应用程序使用到的字节内存池结构�?

void    tx_application_define(void *first_unused_memory)
{

CHAR    *pointer = TX_NULL;
UINT    result;

#ifdef TX_ENABLE_EVENT_TRACE
    tx_trace_enable(trace_buffer, sizeof(trace_buffer), 32);
#endif
    
    /* Create a byte memory pool from which to allocate the thread stacks.  */
    /* 创建一个字节内存池，用于从中分配线程堆栈�?*/

    result = tx_byte_pool_create(&byte_pool_0, "byte pool 0", byte_pool_memory, DEMO_BYTE_POOL_SIZE);
    if ( result != TX_SUCCESS){
      Error_Handler();
    }
    /* Put system definition stuff in here, e.g. thread creates and other assorted
       create information.  */
    /* 将系统定义内容放在此处，例如线程创建和其他各种创建信息�?*/

    /* Allocate the stack for thread 0.  */
    /* 为线�?0 分配堆栈�?*/
    result = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if ( result != TX_SUCCESS){
          Error_Handler();
      }

    /* Create the main thread.  */
    result = tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
    if ( result != TX_SUCCESS){
           Error_Handler();
       }


//    /* Allocate the stack for thread 1.  */
//     result = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
//    if ( result != TX_SUCCESS){
//           Error_Handler();
//       }
//
//    /* Create threads 1 and 2. These threads pass information through a ThreadX 
//       message queue.  It is also interesting to note that these threads have a time
//       slice.  */
//  /* 创建线程 1 �?2。这些线程通过 ThreadX 消息队列传递信息�?
//        值得注意的是，这些线程具有时间片�?*/
//    tx_thread_create(&modbus_thread, "modbus thread", modbus_thread_entry, 1,  
//            pointer, DEMO_STACK_SIZE, 
//            16, 16, 4, TX_AUTO_START);
    
}




/* Define the test threads.  */

void    thread_0_entry(ULONG thread_input)
{

//    UINT    status;
//
//    SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal Sample\r\n");
//    SEGGER_RTT_WriteString(0, "Press <1> to continue in blocking mode (Application waits if necessary, no data lost)\r\n");
//    SEGGER_RTT_WriteString(0, "Press <2> to continue in non-blocking mode (Application does not wait, data lost if fifo full)\r\n");
//    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
//    SEGGER_RTT_printf(0, "printf Test: %%c,         'S' : %c.\r\n", 'S');
//    SEGGER_RTT_printf(0, "printf Test: %%5c,        'E' : %5c.\r\n", 'E');
//    SEGGER_RTT_printf(0, "printf Test: %%-5c,       'G' : %-5c.\r\n", 'G');
//    SEGGER_RTT_printf(0, "printf Test: %%5.3c,      'G' : %-5c.\r\n", 'G');
//    SEGGER_RTT_printf(0, "printf Test: %%.3c,       'E' : %-5c.\r\n", 'E');
//    SEGGER_RTT_printf(0, "printf Test: %%c,         'R' : %c.\r\n", 'R');
//    
//    SEGGER_RTT_printf(0, "printf Test: %%s,      \"RTT\" : %s.\r\n", "RTT");
//    SEGGER_RTT_printf(0, "printf Test: %%s, \"RTT\\r\\nRocks.\" : %s.\r\n", "RTT\r\nRocks.");
//

//    MB_Init(1000);
//    MB_ChCtr = 0;
//    MB_CfgCh(1u,MODBUS_SLAVE,200u, MODBUS_MODE_RTU,&huart1,5u,MODBUS_WR_EN);
//    
    MB_Init(1000);
    MB_ChCtr = 0;
    MB_CfgCh(1u,MODBUS_SLAVE,200u, MODBUS_MODE_RTU,&huart1,5u,MODBUS_WR_EN);   
    Shell_Init();
    ShShell_Init();
    Terminal_Init();    

    MbCmd_Init();    
    

    /* This thread simply sits in while-forever-sleep loop.  */
    while(1)
    {

        /* Increment the thread counter.  */
        thread_0_counter++;
//        SEGGER_RTT_printf(0, "printf thread_0_entry: , thread_0_counter : %u.\r\n", thread_0_counter);

        /* Sleep for 10 ticks.  */
        tx_thread_sleep(5000ul);

        /* Set event flag 0 to wakeup thread 5.  */
//        status =  tx_event_flags_set(&event_flags_0, 0x1, TX_OR);

        /* Check status.  */
//        if (status != TX_SUCCESS)
//            break;
    }
}


void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
    
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    if (Clk_TS_UTC_sec < CLK_TS_SEC_MAX) {
        Clk_TS_UTC_sec++;
    }
    CPU_CRITICAL_EXIT();



}

