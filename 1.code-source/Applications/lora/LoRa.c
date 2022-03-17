#include "LoRa.h"
#include "uart.h"
#include "string.h"
#include "tim.h"
#include "fat_core.h"
#include "gpio.h"

static uint8_t wakeBuf[] = {0x77, 0x61, 0x6B, 0x65}; //"wake"
uint8_t usart1_RX_BUF[usart1_REC_LEN];

#define WF0XAB 0
#define WF0XAD 1
#define WFADDR 2
#define WFCNT 3
#define WFIDX 4
#define WFDATA 5
#define WF0XAE 6
#define WFWAKE 7
#define WFWORK 8
#define WFCHEL 9
#define WFCHID 10
#define WFTPID 11

uint16_t usart1_RX_STA = 0; //接收数据状态
uint8_t usart1_RX_NUM = 0;
uint8_t packNum = 0;   //定位数据所在行
uint8_t newMacNum = 0; //定位新获取的MAC所在行
uint8_t dotNum = 0;    //标记数据的编号
uint8_t Row = 0;
uint8_t Column = 0;
uint8_t idxAddr = 0; //标记子节点短地址的位置
uint8_t addrNum = 0; //记录子节点数目
uint8_t t = 0;
int getAddr = 0;
uint8_t workCnt = 0;
uint8_t dataBuffer[RowMax][ColumnMax] = {0}; //存放串口接收到的数据
uint16_t shortAddr[pacMAX] = {0};            //短地址位置存储器
uint16_t preShortAddr = 0;

uint8_t acktempid = 4;
uint8_t ackAddr[2] = {0}; //短地址应答
uint8_t ackChannel = 0;   //信道应答
uint8_t tempIdx = 0;      //当前挂载的子节点数目
uint8_t ChipId[12] = {0}; //子节点ID

uint8_t ackAddrBuf[pacMAX][2] = {0}; //短地址存储器
uint8_t ackChannelBuf[pacMAX] = {0}; //信道存储器
uint8_t TempIdxBuf[pacMAX][2] = {0}; //编号存储器
uint8_t ChipIdBuf[pacMAX][12] = {0}; // MAC存储器
uint8_t VoltageBuf[pacMAX][2] = {0}; //电压存储器
uint8_t tempIdxFS = 0;               //
uint8_t chipId_accu = 12;            // MAC地址辨别精度
uint8_t getMAC = 0;                  //获取到MAC数据
uint8_t bufWatch[pacMAX] = {0};
uint8_t curTmlNum = 0; //记录当前服务的节点数
uint8_t workState = 0;
uint8_t termalMonitor[pacMAX] = {0}; //子节点连接超时判断
uint8_t data_recv_default = 0;
char FeedBack[30] = {0};
uint8_t RecvCnt = 0;

void LoRaSendWork(uint8_t *ackAddr, uint8_t channel, uint8_t *ChipId, uint8_t TempIdx, uint8_t REWORKTIME, uint8_t RECONTTIME)
{
    uart1_send_byte(ackAddr[0]);
    uart1_send_byte(ackAddr[1]);
    uart1_send_byte(channel);
    uart1_send_byte(0x77);
    uart1_send_byte(0x6F);
    uart1_send_byte(0x72);
    uart1_send_byte(0x6B);
    for (int i = 0; i < 12; i++)
    {
        uart1_send_byte(ChipId[i]);
    }
    uart1_send_byte(TempIdx);
    uart1_send_byte(REWORKTIME);
    uart1_send_byte(RECONTTIME);
    uart1_send_byte(0x00);
    uart1_send_byte(0x00);
}

void LoRaSendAck(uint8_t *ackAddr, uint8_t channel, uint8_t *ChipId, uint8_t idx)
{
    uart1_send_byte(ackAddr[0]);
    uart1_send_byte(ackAddr[1]);
    uart1_send_byte(channel);
    uart1_send_byte(0x61);
    uart1_send_byte(0x63);
    uart1_send_byte(0x6B);
    uart1_send_byte(0x62);
    for (int i = 0; i < 12; i++)
    {
        uart1_send_byte(ChipId[i]);
    }
    uart1_send_byte(idx);
}

void print_LoRa_env(char *FeedBack, uint8_t *recvnct)
{
    memset(FeedBack, 0, 30);
    USART_SendString(USART1, "AT+ADDR\r\n");
    *recvnct = 0;
    fat_delay(300);
    USART_SendString(USART2, FeedBack);
    USART_SendString(USART3, FeedBack);

    memset(FeedBack, 0, 30);
    USART_SendString(USART1, "AT+CH\r\n");
    *recvnct = 0;
    fat_delay(300);
    USART_SendString(USART2, FeedBack);
    USART_SendString(USART3, FeedBack);

    memset(FeedBack, 0, 30);
    USART_SendString(USART1, "AT+WMODE\r\n");
    *recvnct = 0;
    fat_delay(300);
    USART_SendString(USART2, FeedBack);
    USART_SendString(USART3, FeedBack);

    memset(FeedBack, 0, 30);
    USART_SendString(USART1, "AT+CFGTF\r\n");
    *recvnct = 0;
    fat_delay(300);
    USART_SendString(USART2, FeedBack);
    USART_SendString(USART3, FeedBack);
}

uint8_t LoRaInit(char *compare_ADDR, char *compare_CHEL, char *FeedBack, uint8_t *recvnct)
{
    LoRaPower = 0; // LoRa模块断电
    fat_delay(1000);
    LoRaPower = 1; // LoRa模块通电
    uint8_t i = 0;
    FeedBack[0] = 0;
    char compare_a[2] = "a";
    fat_delay(100);
    while (strncmp(FeedBack, compare_a, 1) != 0)
    {
        *recvnct = 0;
        USART_SendString(USART1, "+++");
        fat_delay(100);
        if (i > 15)
        {
            USART_SendString(USART2, FeedBack);
            printf("FeedBack: %s", FeedBack);
            return 0;
        }
        else
            i++;
    }
    memset(FeedBack, 0, 30);
    i = 0;
    char compare_OK[5] = "+OK";
    while (strncmp(FeedBack, compare_OK, 3) != 0)
    {
        USART_SendString(USART1, "a");
        *recvnct = 0;
        fat_delay(100);
        if (i > 15)
        {
            USART_SendString(USART2, FeedBack);
            printf("FeedBack: %s", FeedBack);
            return 1;
        }
        else
            i++;
    }
    memset(FeedBack, 0, 30);
    i = 0;
    while (strstr((const char *)(FeedBack), "OK") == NULL)
    {
        USART_SendString(USART1, compare_ADDR);
        *recvnct = 0;
        fat_delay(500);
        if (i > 5)
        {
            USART_SendString(USART2, FeedBack);
            printf("FeedBack: %s", FeedBack);
            return 2;
        }
        else
            i++;
    }
    memset(FeedBack, 0, 30);
    i = 0;
    while (strstr((const char *)(FeedBack), "OK") == NULL)
    {
        USART_SendString(USART1, compare_CHEL);
        *recvnct = 0;
        fat_delay(500);
        if (i > 5)
        {
            USART_SendString(USART2, FeedBack);
            printf("FeedBack: %s", FeedBack);
            return 3;
        }
        else
            i++;
    }
    USART_SendString(USART1, "AT+WMODE=FP\r\n");
    memset(FeedBack, 0, 30);
    fat_delay(100);

    return 88;
}

/**
 * @brief  This function handles USART1 interrupt request.
 * @param  None
 * @retval None
 */

void USART1_IRQHandler(void) //中断服务函数
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        if (workState == 0)
        {
            if (RecvCnt >= 30)
                RecvCnt = 0;
            FeedBack[RecvCnt] = USART_ReceiveData(USART1);
            RecvCnt++;
        }

        if (workState == 1)
        {
            dataBuffer[Row][Column] = USART_ReceiveData(USART1);
            switch (usart1_RX_STA) //串口1服务函数的状态机
            {
            case WFWORK:          //准备向子节点发送work的指令以及分配服务ID
                if (Column == 20) //收满20个字节的数据，并对数据进行分析
                {
                    tempIdxFS = 100;
                    if (curTmlNum < work_windows) //如果没查到，就分配ID并向此节点发送WORK的指令
                    {
                        for (uint8_t j = 0; j < work_windows; j++)
                        {
                            if (bufWatch[j] == 0)
                            {
                                bufWatch[j] = 1; //将遍历到的空闲区设置为占用状态
                                termalMonitor[j] = 0;
                                tempIdx = j; //将此空闲区分配给该子节点
                                curTmlNum++; //当前服务窗口数自加1
                                break;
                            }
                        }
                        memcpy(ChipIdBuf[tempIdx], &dataBuffer[Row][7], 12);
                        tempIdxFS = 101;
                        getMAC = 1;
                    }
                    newMacNum = Row; //记录此数据包所在位置
                    data_recv_default = 1;
                }
                else
                {
                    Column++;
                }
                break;

            case WFWAKE:                                                           //接收WAKE字段的服务函数
                if ((Column == 3) && (wakeBuf[Column] == dataBuffer[Row][Column])) //收满4字节，并判断其是否为WAKE
                {
                    usart1_RX_STA = WFWORK; //收到WAKE，状态值切换至发送WORK状态
                    workCnt = 0;
                    Column++;
                }
                else if (wakeBuf[Column] == dataBuffer[Row][Column]) //依字节接收并判断
                {
                    Column++;
                }
                else
                {
                    data_recv_default = 1;
                }
                break;

            case WF0XAE:                             //接受并判断0XAE包尾
                if (dataBuffer[Row][Column] == 0xAE) //接收到0XAE的服务函数
                {
                    packNum = Row; //记录此数据包所在位置
                    getAddr = 1;
                    data_recv_default = 1;
                }
                else //并非包尾，继续收数据
                {
                    Column++;
                    usart1_RX_STA = WFDATA;
                }
                break;

            case WFDATA:                             //接收数据状态
                if (dataBuffer[Row][Column] == 0xAE) //数据包中有0XAE
                {
                    usart1_RX_STA = WF0XAE; //转为判断包尾
                    Column++;
                }
                else if (Column == ColumnMax - 1) //数据过长，提前结束接收数据
                {
                    packNum = Row; //记录数据所在位置
                    getAddr = 1;
                    data_recv_default = 1;
                }
                else //无异常，继续收数据
                {
                    Column++;
                }
                break;

            case WFIDX:          //数据包编号
                if (Column == 3) //判断数据位置是否正确
                {
                    dotNum = dataBuffer[Row][Column]; //记录数据包编号
                    memcpy(dataBuffer[idxAddr * pacNumMax + dotNum + 1], dataBuffer[Row], 4);
                    Row = idxAddr * pacNumMax + dotNum + 1; //更改数组地址
                    usart1_RX_STA = WFDATA;                 //进入读取数据状态
                    Column++;
                }
                else //数据异常，初始化状态机
                {
                    data_recv_default = 1;
                }
                break;

            case WFTPID:         //节点编号
                if (Column == 2) //判断编号在数据包中的位置是否对应
                {
                    idxAddr = dataBuffer[Row][Column]; //读取编号
                    termalMonitor[idxAddr] = 0;
                    usart1_RX_STA = WFIDX;
                    Column++;
                }
                else //数据异常，初始化状态机
                {
                    data_recv_default = 1;
                }
                break;

            case WF0XAD: //判断是否为包头0XAD
                if (dataBuffer[Row][1] == 0xAD)
                {
                    usart1_RX_STA = WFTPID; //确认包头，下一步读取编号
                    Column++;
                }
                else //数据异常，初始化状态机
                {
                    data_recv_default = 1;
                }
                break;

            case WF0XAB:
                if (dataBuffer[Row][0] == 0xAB) //收到数据包头0XAB
                {
                    usart1_RX_STA = WF0XAD;
                    Column++;
                }
                else if ((dataBuffer[Row][0] == 0x77) && (curTmlNum < work_windows)) //收到握手包头0X77，如果当前服务窗口占满即不进行握手应答
                {
                    usart1_RX_STA = WFWAKE;
                    Column++;
                }
                else //数据异常，初始化状态机
                {
                    data_recv_default = 1;
                }
                break;

            default:
                data_recv_default = 1;
                break;
            }
        }

        if (data_recv_default == 1) //初始化接收状态机
        {
            usart1_RX_STA = WF0XAB;
            Column = 0;
            Row = RowMax - 1; //将数据先存进缓存区
            dataBuffer[Row][0] = 0;
            dataBuffer[Row][1] = 0;
            data_recv_default = 0;
        }

        if (getAddr == 1) //数据应答
        {
            usart1_RX_BUF[0] = 0;
            ackTempIdx = dataBuffer[packNum][2];
            ackDataIdx = dataBuffer[packNum][3];
            addrget[0] = ackAddrBuf[ackTempIdx][0];
            addrget[1] = ackAddrBuf[ackTempIdx][1];
            channelget = ackChannelBuf[ackTempIdx];
            LoRaSendAck(addrget, channelget, ChipIdBuf[ackTempIdx], ackDataIdx);
            getAddr = 0;
            if ((dataBuffer[packNum][6] == 0xAF) && (ackDataIdx == 13))
                bufWatch[ackTempIdx] = 2;
            else if ((dataBuffer[packNum][7] == 0xBF) && (ackDataIdx == 24))
                bufWatch[ackTempIdx] = 2;
            else if (ackDataIdx == 24)
                bufWatch[ackTempIdx] = 2;
        }
        if (getMAC == 1) //握手应答
        {
            if (tempIdxFS == 101) //如果没查到，就分配ID并向此节点发送WORK的指令
            {
                ackAddrBuf[tempIdx][0] = dataBuffer[newMacNum][4];
                ackAddrBuf[tempIdx][1] = dataBuffer[newMacNum][5];
                ackChannelBuf[tempIdx] = dataBuffer[newMacNum][6];
                LoRaSendWork(ackAddrBuf[tempIdx], ackChannelBuf[tempIdx], ChipIdBuf[tempIdx], tempIdx, time_cycle_term, time_recnt_term);
                memcpy(dataBuffer[tempIdx * pacNumMax], dataBuffer[newMacNum], 21);
            }
            else //查到了历史记录并发送WORK指令，令其工作
                LoRaSendWork(ackAddrBuf[acktempid], ackChannelBuf[acktempid], ChipIdBuf[acktempid], acktempid, 0x11, time_recnt_term);
            getMAC = 0;
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE); //清接收中断
        USART_ClearFlag(USART1, USART_FLAG_RXNE);
    }
}
