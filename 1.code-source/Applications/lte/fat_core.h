#ifndef __FAT_CORE_H
#define __FAT_CORE_H

typedef unsigned char uint8_t; //将uint8_t别名为无符号字符型

#define FAT_UART_RX_SIZE 128    // fat 串口接收缓存大小
#define FAT_UART_RX_IDLETIME 10 // fat 串口接口空闲
#define FAT_TIMER_VAL 10        // fat 定时器间隔

#define FAT_DBG_INFO_FLG 1 // FAT调试信息输出标志

#define _FAT_DBGPRT_INFO(fmt, ...) printf(fmt, ##__VA_ARGS__)

#if FAT_DBG_INFO_FLG
#define FAT_DBGPRT_INFO(f, a...) _FAT_DBGPRT_INFO(f, ##a)
#else
#define FAT_DBGPRT_INFO(f, a...)
#endif

void fat_uart_send_byte(unsigned char data);
extern char cFatUartRecvBuf[FAT_UART_RX_SIZE];
extern unsigned char ucFatUartRecvFinishFlg; //串口接收一包数据标识

void fat_tim_proc(void);                                                 // fat 定时器处理，需要用户开启的定时器中断处理中
void fat_uart_recv_proc(unsigned char ch);                               // fat 串口接收处理，需要用户所用的接收中断处理中
void reg_fat_uart_send_byte(void (*uart_send_byte)(unsigned char data)); //注册 串口发送一个字节函数
void fat_delay(unsigned short int val);                                  //延时函数
void fat_uart_send_str(unsigned char *buf);
void fat_uart_clean(void);
int rev_send(void);
void fat_uart_send_buf(unsigned char *data, int length);

int wait_timeout(unsigned short int time); // 开启定时计数，达到设置定时值时返回1

int fat_send_wait_cmdres_blocking(char *cmd, unsigned short int timeoutval);    //发送命令后，开启定时计数，在设置的定时时间到达后返回1，用户此时可以进行对命令的响应结果进行处理
int fat_send_wait_cmdres_nonblocking(char *cmd, unsigned short int timeoutval); //发送命令后，开启定时计数，在设置的定时时间到达后或命令有响应数据时返回1，用户此时可以进行对命令的响应结果进行处理
int fat_send_wait_ctrl_blocking(char *cmd, unsigned short int timeoutval);
int fat_send_wake_blocking(uint8_t *cmd, unsigned short int timeoutval);

int fat_cmdres_keyword_matching(char *res); //命令响应数据中查找关键字

#endif
