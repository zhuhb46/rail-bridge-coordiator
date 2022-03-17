#ifndef _LoRa_H //???,??????
#define _LoRa_H

/*---------------------------?????--------------------------------------*/
#include "uart.h"
#include "stdio.h"

#define usart1_REC_LEN 2 //定义最大接收字节数 200
#define EN_usart1_RX 1   //使能（1）/禁止（0）串口1接收
#define RowMax 110       //接收数据最大的包数
#define ColumnMax 128    //接收数据最大的字节
#define pacMAX 5         //挂载子节点最大数目
#define pacNumMax 26     //子节点数据包数目

extern uint8_t usart1_RX_BUF[usart1_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern uint8_t dataBuffer[RowMax][ColumnMax]; // 10行68列的多维数组，可存放10个节点的数据
extern uint16_t shortAddr[pacMAX];
extern uint16_t usart1_RX_STA; //接收状态标记
extern uint8_t packNum;
extern uint8_t newMacNum; //定位新获取的MAC所在行
extern uint8_t Row;
extern uint8_t Column;
extern int getAddr;
extern uint8_t dotNum;  //每个包内各点的计数
extern uint8_t idxAddr; //用于循环遍历短地址数组
extern uint8_t addrNum; //记录已经获取到的短地址总数
extern uint8_t tempIdx; //临时牌照
extern uint8_t tempIdxFS;
extern uint8_t chipId_accu; // MAC地址辨别精度
extern uint8_t getMAC;      //获取到MAC数据

extern uint8_t ackAddrBuf[pacMAX][2]; //短地址存储区
extern uint8_t ackChannelBuf[pacMAX]; //信道存储区
extern uint8_t TempIdxBuf[pacMAX][2]; //临时牌照存储区
extern uint8_t ChipIdBuf[pacMAX][12]; //单片机唯一ID存储区
extern uint8_t VoltageBuf[pacMAX][2]; //电压值存储区
extern uint8_t bufWatch[pacMAX];
extern uint8_t curTmlNum; //记录当前服务的节点数
extern uint8_t workState;
extern uint8_t termalMonitor[pacMAX]; //子节点连接超时判断
extern char FeedBack[30];
extern uint8_t RecvCnt;

void LoRaSendWork(uint8_t *ackAddr, uint8_t channel, uint8_t *ChipId, uint8_t TempIdx, uint8_t REWORKTIME, uint8_t RECONTTIME);
void LoRaSendAck(uint8_t *ackAddr, uint8_t channel, uint8_t *ChipId, uint8_t idx);
uint8_t LoRaInit(char *compare_ADDR, char *compare_CHEL, char *FeedBack, uint8_t *recvnct);
void print_LoRa_env(char *FeedBack, uint8_t *recvnct);

#endif
