/*
 * PollingRoutine.c
 *
 *  Created on: Oct 24, 2023
 *      Author: karl.yamashita
 *
 *
 *      Template for projects.
 *
 *      The object of this PollingRoutine.c/h files is to not have to write code in main.c which already has a lot of generated code.
 *      It is cumbersome having to scroll through all the generated code for your own code and having to find a USER CODE section so your code is not erased when CubeMX re-generates code.
 *      
 *      Direction: Call PollingInit before the main while loop. Call PollingRoutine from within the main while loop
 * 
 *      Example;
        // USER CODE BEGIN WHILE
        PollingInit();
        while (1)
        {
            PollingRoutine();
            // USER CODE END WHILE

            // USER CODE BEGIN 3
        }
        // USER CODE END 3

 */


#include "main.h"


const char fw_version[] = "GNSS_12_v1.0.0";

const char hellofromstm32[] = "Hello from STM32 with GNSS 12";


#define UART2_DMA_RX_QUEUE_SIZE 10 // queue size
#define UART2_DMA_TX_QUEUE_SIZE 4
UART_DMA_Data uart2_dmaDataRxQueue[UART2_DMA_RX_QUEUE_SIZE] = {0};
UART_DMA_Data uart2_dmaDataTxQueue[UART2_DMA_TX_QUEUE_SIZE] = {0};
UART_DMA_Struct_t uart2_msg =
{
	.huart = &huart2,
	.rx.queueSize = UART2_DMA_RX_QUEUE_SIZE,
	.rx.msgQueue = uart2_dmaDataRxQueue,
	.tx.queueSize = UART2_DMA_TX_QUEUE_SIZE,
	.tx.msgQueue = uart2_dmaDataTxQueue,
	.dma.dmaPtr.SkipOverFlow = true
};


#define UART1_DMA_RX_QUEUE_SIZE 10 // queue size
#define UART1_DMA_TX_QUEUE_SIZE 4
UART_DMA_Data uart1_dmaDataRxQueue[UART1_DMA_RX_QUEUE_SIZE] = {0};
UART_DMA_Data uart1_dmaDataTxQueue[UART1_DMA_TX_QUEUE_SIZE] = {0};
UART_DMA_Struct_t uart1_msg =
{
	.huart = &huart1,
	.rx.queueSize = UART1_DMA_RX_QUEUE_SIZE,
	.rx.msgQueue = uart1_dmaDataRxQueue,
	.tx.queueSize = UART1_DMA_TX_QUEUE_SIZE,
	.tx.msgQueue = uart1_dmaDataTxQueue,
	.dma.dmaPtr.SkipOverFlow = true
};


char msg_copy_command[UART_DMA_QUEUE_DATA_SIZE] = {0};
int passthrough_flag = 1; // allows GNSS data to be passed through to command port

void PollingInit(void)
{
	// Enable UART DMA Idle  interrupt. We are using circular mode, so we only need to call these two once.
	UART_DMA_EnableRxInterruptIdle(&uart1_msg);
	UART_DMA_EnableRxInterruptIdle(&uart2_msg);

	// toggle led for SOH
	TimerCallbackRegisterOnly(&timerCallback, LED_Toggle);
	TimerCallbackTimerStart(&timerCallback, LED_Toggle, 500, TIMER_REPEAT);

	// Check to see if we need to send all the commands registered
	TimerCallbackRegisterOnly(&timerCallback, Command_List_Poll);
	TimerCallbackTimerStart(&timerCallback, Command_List_Poll, 10, TIMER_REPEAT);

	// Indicate the STM32 is now running
	UART_DMA_NotifyUser(&uart2_msg, (char*)hellofromstm32, strlen((char*)hellofromstm32), true);
}

void PollingRoutine(void)
{
	// Poll the timer callback
	TimerCallbackPoll(&timerCallback);

	// parse the large circular buffer
	UART_DMA_ParseCircularBuffer(&uart1_msg);
	UART_DMA_ParseCircularBuffer(&uart2_msg);

	// check hal status
	UART_DMA_CheckHAL_Status(&uart1_msg);
	UART_DMA_CheckHAL_Status(&uart2_msg);

	// Check for new UART messages
	UART1_CheckForNewMessage(&uart1_msg);
	UART2_CheckForNewMessage(&uart2_msg);
}

// command
void UART2_CheckForNewMessage(UART_DMA_Struct_t *msg)
{
	int status = 0;
	char *ptr;
	char retStr[UART_DMA_QUEUE_DATA_SIZE] = "OK";

	if(UART_DMA_RxMsgRdy(msg))
	{
		memset(&msg_copy_command, 0, sizeof(msg_copy_command)); // clear
		memcpy(&msg_copy_command, msg->rx.msgToParse->data, strlen((char*)msg->rx.msgToParse->data) - 2); // remove CR/LF

		ptr = (char*)msg->rx.msgToParse->data;
		ToLower(ptr);

		if(strncmp(ptr, "version", strlen("version"))== 0)
		{
			status = version.func(NULL, retStr);
		}
		else if(strncmp(ptr, "help", strlen("help"))== 0)
		{
			ptr += strlen("help");
			status = help.func(ptr, NULL);
		}
		else if(strncmp(ptr, "gnrmc", strlen("gnrmc"))== 0)
		{
			ptr += strlen("gnrmc");
			status = gnrmc.func(ptr, retStr);
		}
		else if(strncmp(ptr, "pass", strlen("pass"))== 0)
		{
			ptr += strlen("pass");
			status = pass.func(ptr, NULL);
		}
		else
		{
			status = COMMAND_UNKNOWN;
		}

		// check return status
		if(status == NO_ACK)
		{
			return;
		}
		else if(status != 0) // other return status other than NO_ACK or NO_ERROR
		{
			PrintError(msg, msg_copy_command, status);
		}
		else // NO_ERROR
		{
			PrintReply(msg, msg_copy_command, retStr);
		}

		memset(&msg->rx.msgToParse->data, 0, UART_DMA_QUEUE_DATA_SIZE); // clear current buffer index
	}
}

// GNSS
void UART1_CheckForNewMessage(UART_DMA_Struct_t *msg)
{
	char *ptr;

	if(UART_DMA_RxMsgRdy(msg))
	{
		ptr = (char*)msg->rx.msgToParse->data;

		if(passthrough_flag)
		{
			UART_DMA_NotifyUser(&uart2_msg, ptr, strlen(ptr), false);
		}

		if(strncmp(ptr, "$GNRMC", strlen("$GNRMC")) == 0) // looking for $GNRMC message only
		{
			NMEA_GNRMC_Data(ptr); // parse message
		}
		else
		{
			// ignore rest of messages
		}
	}
}

void PrintError(UART_DMA_Struct_t *msg, char *msg_copy, uint32_t error)
{
	char str[64] = {0};

	GetErrorString(error, str);

	strcat(msg_copy, "=");
	strcat(msg_copy, str);

	UART_DMA_NotifyUser(msg, msg_copy, strlen(msg_copy), true);
}

/*
 * Description: Returns the original message + string arguments
 * Input: DMA message data structure, the original string message, the string to add to the original message
 * Output: none
 * Return: none
 */
void PrintReply(UART_DMA_Struct_t *msg, char *msg_copy, char *msg2)
{
	strcat(msg_copy, "=");
	strcat(msg_copy, msg2);

	UART_DMA_NotifyUser(msg, msg_copy, strlen(msg_copy), true);
}

/*
 * Description: Return the version
 */
int Version(char *msg, char* retStr)
{
	sprintf(retStr, fw_version);

	return NO_ERROR;
}

int Passthrough(char *msg, char* retStr)
{
	passthrough_flag = atoi(msg);

	return NO_ERROR;
}

/*
 * Description: This is a generic call from CommandList_Drv.c
 * 				This will allow the CommandList_Drv.c driver to be portable for any microcontroller project.
 * 				But in this function we'll call the uC actual UART Tx driver
 */
int Command_ListNotify(char *msg)
{
	int status = NO_ERROR;

	UART_DMA_NotifyUser(&uart2_msg, msg, strlen(msg), true);

	return status;
}

/*
 * Description: Toggle LED
 */
void LED_Toggle(void)
{
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}
