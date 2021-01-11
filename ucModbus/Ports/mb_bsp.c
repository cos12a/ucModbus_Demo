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
*                                            uC/Modbus
*
*                                      Board Support Package
*                                        Atmel AVR32UC3A
*
* Filename : mb_bsp.c
* Version  : V2.14.00
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

//#include    <includes.h>
#include  <cpu.h>
#include  <mb_cfg.h>
#include  <mb_def.h>

#include  <mb_os.h>
#include "mb.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "usart.h"




/*
*********************************************************************************************************
*                                            AVR32UC3A REGISTER BIT DEFINES
*********************************************************************************************************
*/








/*
*********************************************************************************************************
*                                        REGISTER DEFINITIONS
*
* Note(s) : (1) The device register definition structure MUST take into account appropriate
*               register offsets and apply reserved space as required.  The registers listed
*               within the register definition structure MUST reflect the exact ordering and
*               data sizes illustrated in the device user guide.
*********************************************************************************************************
*/





/*
*********************************************************************************************************
*                                             LOCAL VARIABLES
*********************************************************************************************************
*/

//static  CPU_INT32U  MB_Tmr_ReloadCnts;

 
/*
*********************************************************************************************************
*                                             MB_CommExit()
*
* Description : Terminates Modbus communications.  All Modbus channels are closed.
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommExit (void)
{
    CPU_INT08U   ch;
    MODBUS_CH   *pch;
    for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {
      pch = MB_ChTblPrt[ch];
        MB_CommTxIntDis(pch);
        MB_CommRxIntDis(pch);
    }
}


 
/*
*********************************************************************************************************
*                                           MB_CommPortCfg()
*
* Description : Initializes the serial port to the desired baud rate and the UART will be
*               configured for N, 8, 1 (No parity, 8 bits, 1 stop).
*
* Argument(s) : pch        is a pointer to the Modbus channel
*               port_nbr   is the desired serial port number.  This argument allows you to assign a
*                          specific serial port to a specific Modbus channel.
*               baud       is the desired baud rate for the serial port.
*               parity     is the desired parity and can be either:
*
*                          MODBUS_PARITY_NONE
*                          MODBUS_PARITY_ODD
*                          MODBUS_PARITY_EVEN
*
*               bits       specifies the number of bit and can be either 7 or 8.
*               stops      specifies the number of stop bits and can either be 1 or 2
*
* Return(s)   : none.
*
* Caller(s)   : MB_CfgCh()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommPortCfg (UART_HandleTypeDef         *port_nbr)
{
  if (HAL_UART_Init(port_nbr) != HAL_OK)
  {
    Error_Handler();
  }
  /* Process Unlocked */
  __HAL_UNLOCK(port_nbr);
  
  /* Enable the UART Parity Error Interrupt */
  __HAL_UART_ENABLE_IT(port_nbr, UART_IT_PE);
  
  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_ENABLE_IT(port_nbr, UART_IT_ERR);
  
  /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(port_nbr, UART_IT_RXNE);
}


 
/*
*********************************************************************************************************
*                                         MB_CommRxIntDis()
*
* Description : Disables Rx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommExit()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommRxIntDis (MODBUS_CH  *pch)
{

  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(pch->PortNbr->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  CLEAR_BIT(pch->PortNbr->Instance->CR3, USART_CR3_EIE);

}

 
/*
*********************************************************************************************************
*                                          MB_CommRxIntEn()
*
* Description : Enables Rx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_TxByte()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommRxIntEn (MODBUS_CH  *pch)
{   


//  __HAL_UART_ENABLE_IT(huart2, UART_IT_TXE);


}

 
/*
*********************************************************************************************************
*                                       MB_CommRxTxISR_Handler()
*
* Description : ISR for either a received or transmitted character.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is a ISR
*
* Note(s)     : (1) The pseudo-code for this function should be:  
*
*               if (Rx Byte has been received) {
*                  c = get byte from serial port;
*                  Clear receive interrupt;
*                  pch->RxCtr++;                      Increment the number of bytes received
*                  MB_RxByte(pch, c);                 Pass character to Modbus to process
*              }
*
*              if (Byte has been transmitted) {
*                  pch->TxCtr++;                      Increment the number of bytes transmitted
*                  MB_TxByte(pch);                    Send next byte in response
*                  Clear transmit interrupt           Clear Transmit Interrupt flag
*              }
*********************************************************************************************************
*/
HAL_StatusTypeDef UART_Receive_IT(UART_HandleTypeDef *huart)
{
  MODBUS_CH          *pch;
  CPU_INT08U          ch;
  CPU_INT08U          rx_data = 0;

      for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {       /* Find the channel assigned to this port       */
          pch = MB_ChTblPrt[ch];
          if (pch->PortNbr == huart) {
              ch = MODBUS_CFG_MAX_CH;
          } 
      }
          pch->RxCtr++;                                           /* Increment the Rx counter                                  */
          rx_data = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
          MB_RxByte(pch, rx_data);
    return  HAL_OK;
}

 
/*
*********************************************************************************************************
*                                           MB_RTU_TmrInit()
*
* Description : Initializes RTU timeout timer.
*
* Argument(s) : freq          Is the frequency of the modbus RTU timer interrupt.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrInit (void)
{
    MB_RTU_TmrResetAll();                                     /* Reset all the RTU timers.                                   */
}
#endif


 
/*
*********************************************************************************************************
*                                           MB_RTU_TmrExit()
*
* Description : Disables RTU timeout timer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrExit (void)
{


  
}
#endif

 
/*
*********************************************************************************************************
*                                       MB_RTU_TmrISR_Handler()
*
* Description : Handles the case when the RTU timeout timer expires.
*
* Arguments   : none.
*
* Returns     : none.
*
* Caller(s)   : This is a ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrISR_Handler (void)
{

    MB_RTU_TmrCtr++;                                          /* Indicate that we had activities on this interrupt.          */
    MB_RTU_TmrUpdate();                                       /* Check for RTU timers that have expired                      */
}

#endif


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  for (char ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {
    if (MB_ChTblPrt[ch]->PortNbr == huart){
//      MB_ChTblPrt[ch]->TxBufPtr = &MB_ChTblPrt[ch]->TxBuf[0];
    }
  }
    /* Initialize the UART state*/
  huart->ErrorCode = HAL_UART_ERROR_NONE;
  huart->gState = HAL_UART_STATE_READY;
  huart->RxState = HAL_UART_STATE_READY;
}




 void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
      
#if     (RTT_PRINTF_EN == DEF_ENABLED)
        SEGGER_RTT_printf(0, "HAL_UART_ErrorCallback Error code: %u.\r\n", huart->ErrorCode);      
#endif
  
  /* Prevent unused argument(s) compilation warning */
  /* Process Unlocked */
  __HAL_UNLOCK(huart);
  
  /* Enable the UART Parity Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_PE);
  
  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);
  
  /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);

  /* Reset ErrorCode */
  huart->ErrorCode = HAL_UART_ERROR_NONE;

  /* Restore huart->gState and huart->RxState to Ready */
  huart->gState  = HAL_UART_STATE_READY;
  huart->RxState = HAL_UART_STATE_READY;
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_ErrorCallback could be implemented in the user file
   */
}

