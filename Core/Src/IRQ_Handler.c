/*
 * IRQ_Handler.c
 *
 *  Created on: May 7, 2025
 *      Author: karl.yamashita
 */

#include "main.h"


//
// Description: Copy the DMA buffer data to circular buffer. The circular buffer will be parsed in polling routine.
//
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	int i = 0;
	uint32_t size = 0;

	if(huart == uart1_msg.huart)
	{
		size = UART_DMA_GetSize(&uart1_msg, Size);

		for(i = 0; i < size; i++)
		{
			uart1_msg.dma.circularBuffer[uart1_msg.dma.circularPtr.index_IN] = uart1_msg.dma.dma_data[uart1_msg.dma.dmaPtr.index_IN];
			RingBuff_Ptr_Input(&uart1_msg.dma.circularPtr, UART_DMA_CIRCULAR_SIZE);
			RingBuff_Ptr_Input(&uart1_msg.dma.dmaPtr, UART_DMA_BUFFER_SIZE);
		}
	}
	else if(huart == uart2_msg.huart)
	{
		size = UART_DMA_GetSize(&uart2_msg, Size);

		for(i = 0; i < size; i++)
		{
			uart2_msg.dma.circularBuffer[uart2_msg.dma.circularPtr.index_IN] = uart2_msg.dma.dma_data[uart2_msg.dma.dmaPtr.index_IN];
			RingBuff_Ptr_Input(&uart2_msg.dma.circularPtr, UART_DMA_CIRCULAR_SIZE);
			RingBuff_Ptr_Input(&uart2_msg.dma.dmaPtr, UART_DMA_BUFFER_SIZE);
		}
	}
	// repeat for other UART ports using (else if)
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == uart1_msg.huart)
	{
		uart1_msg.tx.txPending = false;
		UART_DMA_SendMessage(&uart1_msg);
	}
	else if(huart == uart2_msg.huart)
	{
		uart2_msg.tx.txPending = false;
		UART_DMA_SendMessage(&uart2_msg);
	}
	// repeat for other UART ports using (else if)
}

