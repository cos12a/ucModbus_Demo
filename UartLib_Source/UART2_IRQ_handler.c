/* Includes ------------------------------------------------------------------*/
#include "user_uart2_cfg.h"

extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;


/**
  * @brief This function handles DMA1 channel6 global interrupt.
  */
void DMA1_Channel6_IRQHandler(void)
{
   User_USART2_DMA_RX_IRQHandler(&hdma_usart2_rx);
}

/**
  * @brief This function handles DMA1 channel7 global interrupt.
  */
void DMA1_Channel7_IRQHandler(void)
{
    User_USART2_DMA_TX_IRQHandler(&hdma_usart2_tx);
}



void USART2_IRQHandler(void)
{
   User_UART2_IRQHandler(&huart2);
}


