/*
*********************************************************************************************************
*                                              uC/Shell
*                                            Shell utility
*
*                    Copyright 2007-2020 Silicon Laboratories Inc. www.silabs.com
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
*                                              TERMINAL
*
*                                    TEMPLATE COMMUNICATIONS PORT
*
* Filename : terminal_serial.c
* Version  : V1.04.00
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  TERMINAL_VT100_ESC_CHAR                        0x1Bu

/*
*********************************************************************************************************
*                                           C0 COMMAND SET
*********************************************************************************************************
*/

#define  TERMINAL_VT100_C0_BS                           0x08u   /* Backspace.                                           */
#define  TERMINAL_VT100_C0_LF                           0x0Au   /* Line feed.                                           */
#define  TERMINAL_VT100_C0_CR                           0x0Du   /* Carriage return.                                     */

#define  TERMINAL_VT100_C0_ESC                          0x1Bu   /* Escape.                                              */

/*
*********************************************************************************************************
*                                           C1 COMMAND SET
*********************************************************************************************************
*/

#define  TERMINAL_VT100_C1_CSI                          0x5Bu   /* Control sequence  introducer.                        */

/*
*********************************************************************************************************
*                                 CONTROL SEQUENCE BYTE VALUE LIMITS
*********************************************************************************************************
*/

#define  TERMINAL_VT100_PB_MIN                          0x30
#define  TERMINAL_VT100_PB_MAX                          0x3F
#define  TERMINAL_VT100_IB_MIN                          0x20
#define  TERMINAL_VT100_IB_MAX                          0x2F
#define  TERMINAL_VT100_FB_MIN                          0x40
#define  TERMINAL_VT100_FB_MAX                          0x7E

/*
*********************************************************************************************************
*                              NO PARAMETER CONTROL SEQUENCE FINAL BYTE
*********************************************************************************************************
*/

#define  TERMINAL_VT100_NP_CUU                          0x41u   /* Cursor up.                                           */
#define  TERMINAL_VT100_NP_CUD                          0x42u   /* Cursor down.                                         */
#define  TERMINAL_VT100_NP_CUF                          0x43u   /* Cursor right.                                        */
#define  TERMINAL_VT100_NP_CUB                          0x44u   /* Cursor left.                                         */

/*
*********************************************************************************************************
*                                    CONTROL & FUNCTION KEY VALUES
*********************************************************************************************************
*/

#define  TERMINAL_VT100_KEY_HOME                        0x31u
#define  TERMINAL_VT100_KEY_INSERT                      0x32u
#define  TERMINAL_VT100_KEY_DELETE                      0x33u
#define  TERMINAL_VT100_KEY_END                         0x34u
#define  TERMINAL_VT100_KEY_PAGEDOWN                    0x35u
#define  TERMINAL_VT100_KEY_PAGEUP                      0x36u

#define  TERMINAL_VT100_P1_KEY                          0x7Eu

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <terminal.h>


#include "sample_cfg.h"


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

#include  <lib_str.h>
#include "user_uart2_cfg.h"


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


//#define SEGGER_RTT_MODE

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static uint8_t read_buffer[TERMINAL_CFG_MAX_CMD_LEN];
static TX_SEMAPHORE rdByte_sem;

#if defined( SEGGER_RTT_MODE )
static TX_TIMER rdData_timer;

#else

static uint8_t rx_DMA_buffer[TERMINAL_CFG_MAX_CMD_LEN];
static TX_SEMAPHORE Tx_sem;

#endif

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if defined( SEGGER_RTT_MODE )
static void timer_receive_data(ULONG data);

#endif

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        TerminalSerial_Init()
*
* Description : Initialize serial communications.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if interface was opened.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Terminal_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  TerminalSerial_Init (void)
{ 
    UINT status;
    status = tx_semaphore_create(&rdByte_sem, "Serial_byte_input_semaphore", 0u);  

    /* If status equals TX_SUCCESS, my_semaphore is ready for use. */ 
    if (status != TX_SUCCESS){
        Error_Handler();

    }

#if defined( SEGGER_RTT_MODE )
    status = tx_timer_create(&rdData_timer,"read data timer", 
    timer_receive_data, NULL, 50, 300, TX_AUTO_ACTIVATE); 
    if ( status != TX_SUCCESS){        
        Error_Handler();
    }
             
#else

    status = tx_semaphore_create(&Tx_sem, "Serial_byte_input_semaphore", 1u);  

    /* If status equals TX_SUCCESS, my_semaphore is ready for use. */ 
    if (status != TX_SUCCESS){
        Error_Handler();
    }    
    User_UART2_Receive_DMA(&huart2, rx_DMA_buffer, TERMINAL_CFG_MAX_CMD_LEN);
    
#endif

    
    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                        TerminalSerial_Exit()
*
* Description : Uninitialize serial communications.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Terminal_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  TerminalSerial_Exit (void)
{
    UINT status; 
    
    /* Delete counting semaphore. Assume that the counting 
    semaphore has already been created. */ 
    status = tx_semaphore_delete(&rdByte_sem);     
    /* If status equals TX_SUCCESS, the counting semaphore is  deleted. */ 
    if (status != TX_SUCCESS){
        Error_Handler();
    }
#if defined( SEGGER_RTT_MODE )
    status = tx_timer_delete(&rdData_timer);    
       /* If status equals TX_SUCCESS, the counting semaphore is  deleted. */ 
       if (status != TX_SUCCESS){
           Error_Handler();
       }
#else

    status = tx_semaphore_delete(&Tx_sem);    
    /* If status equals TX_SUCCESS, the counting semaphore is  deleted. */ 
    if (status != TX_SUCCESS){
        Error_Handler();
    }
#endif
}


/*
*********************************************************************************************************
*                                         TerminalSerial_Wr()
*
* Description : Serial output.
*
* Argument(s) : pbuf        Pointer to the buffer to transmit.
*
*               buf_len     Number of bytes in the buffer.
*
* Return(s)   : none.
*
* Caller(s)   : Terminal_Out().
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT16S  TerminalSerial_Wr (void        *pbuf,
                               CPU_SIZE_T   buf_len)
{  
#if defined( SEGGER_RTT_MODE )
    SEGGER_RTT_Write(0,  pbuf, buf_len);
#else
    
    UINT status = tx_semaphore_get(&Tx_sem, TX_WAIT_FOREVER); 
    if (status != TX_SUCCESS){
        Error_Handler();
    }
    User_UART2_Transmit_DMA(&huart2, pbuf, buf_len);
#endif
    return (0);
}


/*
*********************************************************************************************************
*                                       TerminalSerial_RdByte()
*
* Description : Serial byte input.
*
* Argument(s) : none.
*
* Return(s)   : Byte read from port.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/
//
//CPU_INT08U  TerminalSerial_RdByte (void)
//{
//    UINT status;
//    static uint8_t c;
//    if (tx_len == 0u){        
//        status = tx_semaphore_get(&rdByte_sem, TX_WAIT_FOREVER); 
//        if (status != TX_SUCCESS){
//            Error_Handler();
//        }
//        c = 0u;
//    }
//    tx_len--;
//    return (read_buffer[c++]);
//}

CPU_BOOLEAN TerminalSerial_RdData(CPU_CHAR **cmdBuf)
{
    CPU_BOOLEAN  ok = DEF_OK;
    UINT status;
    status = tx_semaphore_get(&rdByte_sem, TX_WAIT_FOREVER); 
    if (status != TX_SUCCESS){
        Error_Handler();
    }
    *cmdBuf = (char *)read_buffer;
    return (ok);
}
/*
*********************************************************************************************************
*                                       TerminalSerial_WrByte()
*
* Description : Serial byte output.
*
* Argument(s) : c           Byte to write.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  TerminalSerial_WrByte (CPU_INT08U  c)
{
#if defined( SEGGER_RTT_MODE )
    SEGGER_RTT_Write(0,  &c, 1u);
#else
    UINT status = tx_semaphore_get(&Tx_sem, TX_WAIT_FOREVER); 
    if (status != TX_SUCCESS){
        Error_Handler();
    }
    User_UART2_Transmit_IT(&huart2, &c, 1u);
#endif

}






#if defined( SEGGER_RTT_MODE )
 
 static void timer_receive_data(ULONG data)
 {
  unsigned rCnt;
  UINT status;
 //  0.3秒读一下数据.
 //  将接收到的数据全部传给线程处理.
    rCnt = SEGGER_RTT_HasData(0u);  
    if (rCnt) {   
        SEGGER_RTT_Read(0u,read_buffer,rCnt);
        status = tx_semaphore_put(&rdByte_sem);
        if (status != TX_SUCCESS){
            Error_Handler();
        }    
    }
}
 
#else









__STATIC_INLINE uint32_t 

USER_DMA_GetDataLength(DMA_TypeDef *DMAx, uint32_t Channel)
{
  return (READ_BIT(((DMA_Channel_TypeDef *)((uint32_t)((uint32_t)DMAx + CHANNEL_OFFSET_TAB[Channel - 1U])))->CNDTR,
                   DMA_CNDTR_NDT));
}




void User_UART2_ReceiveDMA_Callback(DMA_HandleTypeDef *hdma)
{
    static size_t old_pos = 0u;
    size_t pos;
    /* Calculate current position in buffer */
    pos = TERMINAL_CFG_MAX_CMD_LEN - USER_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_6);
    if (pos != old_pos) {                       /* Check change in received data */
        if (pos > old_pos) {                    /* Current position is over previous one */
            memcpy((char *)&read_buffer[0], (char const *)&rx_DMA_buffer[old_pos], pos - old_pos);
//            tx_len = pos - old_pos;
        }
        else {
            memcpy((char *)&read_buffer[0], (char const *)&rx_DMA_buffer[old_pos], TERMINAL_CFG_MAX_CMD_LEN - old_pos);
//            tx_len = TERMINAL_CFG_MAX_CMD_LEN - old_pos;     
            /* Check and continue with beginning of buffer */
            if (pos > 0) {
                memcpy((char *)&read_buffer[TERMINAL_CFG_MAX_CMD_LEN - old_pos], (char const *)&rx_DMA_buffer[0], pos);
//                tx_len += pos;
            }
           
        }
        UINT status = tx_semaphore_put(&rdByte_sem);
        if (status != TX_SUCCESS){
            Error_Handler();
        }         
        old_pos = pos;                              /* Save current position as old */
        /* Check and manually update if we reached end of buffer */
//        if (old_pos == TERMINAL_CFG_MAX_CMD_LEN) {
//            old_pos = 0;
//        }        
    }
    
}




void User_UART2_TxDMACplt_Callback(DMA_HandleTypeDef *hdma)
{

    UINT status = tx_semaphore_put(&Tx_sem);
           if (status != TX_SUCCESS){
               Error_Handler();
           } 

}


void User_UART2_TxCpltCallback(UART_HandleTypeDef *huart)
{

    UINT status = tx_semaphore_put(&Tx_sem);
           if (status != TX_SUCCESS){
               Error_Handler();
           } 


}

#endif 
