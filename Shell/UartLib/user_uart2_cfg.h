/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_UART2_H
#define __USER_UART2_H

 /* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "main.h"
#include "string.h"
#include "usart.h"

#ifdef __cplusplus
 extern "C" {
#endif





void User_UART2_DMATransmitCplt(DMA_HandleTypeDef *hdma);
void User_UART2_DMAError(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef User_DMA_Abort_IT(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef User_UART2_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef User_UART2_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef Usart2_DMA_Tx_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);
HAL_StatusTypeDef Usart2_DMA_Rx_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);
void User_USART2_DMA_RX_IRQHandler(DMA_HandleTypeDef *hdma);
void  User_USART2_DMA_TX_IRQHandler(DMA_HandleTypeDef *hdma);
void User_UART2_IRQHandler(UART_HandleTypeDef *huart);
HAL_StatusTypeDef User_UART2_Transmit_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef User_UART2_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef User_UART2_DMA_Tx_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);
HAL_StatusTypeDef User_UART2_DMA_Rx_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);



void User_UART2_ReceiveDMA_Callback(DMA_HandleTypeDef *hdma);
void User_UART2_TxDMACplt_Callback(DMA_HandleTypeDef *hdma);
void User_UART2_TxCpltCallback(UART_HandleTypeDef *huart);



/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef hUART2;




#ifdef __cplusplus
}
#endif

#endif /* __STM32_ASSERT_H */


