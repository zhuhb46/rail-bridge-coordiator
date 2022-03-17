#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"
#include <stdio.h>

#define USART2_REC_LEN 10 //定义最大接收字节数 200
#define EN_USART2_RX 1    //使能（1）/禁止（0）串口1接收

extern u8 USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u16 USART2_RX_STA;                //接收状态标记

/*
    uint8_t flagh;      //高标志位：0xAC
    uint8_t flagl;      //低标志位：0xAD
    uint8_t coor_cycle; //协调器工作周期 ## T_coor = coor_cycle*20 ## , 单位：s ,   例：coor_cycle = 90，即工作周期为1800秒，亦即30分钟醒一次
    uint8_t coor_work;  //协调器工作时间 ## W_coor = coor_work*20  ## , 单位：s ,   例：coor_work = 45，即工作时间为900秒，亦即15分钟用于接收数据
    uint8_t term_cycle; //子节点工作周期 ## T_term = term_cycle*20 - 100 ## , 单位：s ,   例：term_cycle = 90，即工作周期为1700秒，亦即28.5分钟醒一次
    uint8_t term_recon; //子节点失败重连 ## R_term = term_recon*10 + 10  ## , 单位：s ,   例：term_recon = 2，失败后30s重新尝试连接
*/
typedef struct _usart_recv
{
    uint8_t flagh;
    uint8_t flagl;
    uint8_t coor_cycle;
    uint8_t coor_work;
    uint8_t term_cycle;
    uint8_t term_recon;
    uint8_t term_win;
} usart_recv_4G;

extern usart_recv_4G my_usart_recv;

typedef struct _usart_send_wake
{
    uint8_t myName[16];
    uint8_t STARTWORK[10];
    uint16_t rtcnum;
    uint16_t Temp; //温度数据
    uint16_t Vref; //电压数据
} usart_send_wake;

extern usart_send_wake myusart_send_wake;

void uart1_init(void);
void uart2_init(void);
void uart3_init(void);
void uart1_send_byte(uint8_t data);
void uart1_send_longbyte(uint8_t *data, uint8_t lenth);
void USART_SendString(USART_TypeDef *pUSARTx, char *str);
void uart2_send_byte(uint8_t data);
void uart2_send_longbyte(uint8_t *data, uint8_t lenth);
void uart3_send_byte(uint8_t data);
void uart3_send_longbyte(uint8_t *data, uint8_t lenth);
#endif
