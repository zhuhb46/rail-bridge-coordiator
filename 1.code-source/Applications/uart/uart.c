#include <string.h>
#include "uart.h"
#include "fat_core.h"

//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#pragma import(__use_no_semihosting)
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
//标准库需要的支持函数
struct __FILE
{
	int handle;
};
FILE __stdout;

/**
 * @description:重定向c库函数printf到串口，重定向后可使用printf函数
 * @param 无
 * @return 无
 */
int fputc(int ch, FILE *f)
{
	while ((USART3->SR & 0X40) == 0)
		;
	USART3->DR = (u8)ch;
	return ch;
}

/**
 * @description: uart1初始化，用于调试信息打印
 * @param None
 * @return None
 */
void uart1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);
}

/**
 * @description: UART2初始化，用于跟模组通信
 * @param  None
 * @return None
 */
void uart2_init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
}

void uart3_init(void)
{
	// GPIO端口设置
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure; //声明一个结构体变量，用来初始化GPIO
	//使能串口的RCC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能UART3所在GPIOB的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	//串口使用的GPIO口配置
	// Configure USART3 Rx (PB.11) as input floating
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Configure USART3 Tx (PB.10) as alternate function push-pull
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//配置串口
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	// Configure USART3
	USART_Init(USART3, &USART_InitStructure); //配置串口3
	// Enable USART3 Receive interrupts 使能串口接收中断
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
	// Enable the USART3
	USART_Cmd(USART3, ENABLE); //使能串口3
}

/**
 * @description: UART1发送一个字节函数
 * @param None
 * @return None
 */
void uart1_send_byte(uint8_t data)
{
	USART_SendData(USART1, data);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{
	}
}
void uart1_send_longbyte(uint8_t *data, uint8_t lenth)
{
	for (int i = 0; i < lenth; i++)
	{
		uart1_send_byte(data[i]);
	}
}
void USART_SendString(USART_TypeDef *pUSARTx, char *str)
{
	unsigned int k = 0;
	do
	{
		USART_SendData(pUSARTx, *(str + k));
		while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) == RESET)
		{
		}
		k++;
	} while (*(str + k) != '\0');
}
/**
 * @description: UART2发送一个字节函数
 * @param None
 * @return None
 */
void uart2_send_byte(uint8_t data)
{
	USART_SendData(USART2, data);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
	{
	}
}
void uart2_send_longbyte(uint8_t *data, uint8_t lenth)
{
	for (int i = 0; i < lenth; i++)
	{
		uart2_send_byte(data[i]);
	}
}
/**
 * @description: UART3发送一个字节函数
 * @param None
 * @return None
 */
void uart3_send_byte(uint8_t data)
{
	USART_SendData(USART3, data);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
	{
	}
}
void uart3_send_longbyte(uint8_t *data, uint8_t lenth)
{
	for (int i = 0; i < lenth; i++)
	{
		uart3_send_byte(data[i]);
	}
}

/**
 * @brief  This function handles USART2 interrupt request.
 * @param  None
 * @retval None
 */

#define Timer3RtcCycle 90
#define TIME_CYCLE 89	   //协调器工作大周期
#define TIME_WORK_COOR 60  //协调器工作时长
#define TIME_CYCLE_TERM 95 //子节点工作大周期
#define TIME_RECNT_TERM 2  //子节点重复工作时长

u8 USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.
u16 USART2_RX_STA = 0; //接收状态标记
usart_recv_4G my_usart_recv;
usart_send_wake myusart_send_wake;

void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) // 接收中断
	{
		uint8_t ch = USART2->DR;
		fat_uart_recv_proc(ch);

		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		USART_ClearFlag(USART2, USART_FLAG_RXNE); // 清除标志位
	}
}
