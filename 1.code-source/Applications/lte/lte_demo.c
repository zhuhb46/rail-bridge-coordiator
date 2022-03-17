#include "string.h"
#include "stdlib.h"
#include "lte_demo.h"
#include "fat_core.h"
#include "gpio.h"
#include "stdio.h"
#include "uart.h"

#define LTE_POWER_ON_WAIT_TIME 2000         // LTE开机等待时间
#define SIGNALMIN 15                        //信号质量最低阀值
#define SIGNALMAX 31                        //信号质量最低阀值
#define SOCKET_BUF_SIZE 128                 // Socket数据缓存大小
char cSocketRecvBuf[SOCKET_BUF_SIZE] = {0}; // socket数据接收缓存
char cSocketSendBuf[SOCKET_BUF_SIZE] = {0}; // socket数据发送缓存
uint8_t ucStateNum = 0;                     //命令执行顺序标识值
uint8_t retrytimes = 0;                     //命令重试次数
uint8_t ucErrorTimes = 0;                   //错误次数累计值
uint8_t ucFlightModeTimes = 0;              //进入飞行模式次数
char this_cpa_addr[13];
char this_cpa_chel[11];
uint8_t coor_ctrl_flag = 0;

typedef enum
{
    MD_RESET,        //复位模块
    MD_AT_REQ,       // AT握手
    MD_WORK_STA_CHK, //工作状态检测
    MD_CONNETINIT,   //连接配置信息初始化
    MD_CONNETED,     //数据通信
    MD_SYSRESTART,
    MD_ADDR,
    MD_CHEL,
    MD_NULL,
    MD_FLIGHTMODE, //飞行模式
    MD_OK = 0xFE,  //正常
    MD_ERR = 0xFF, //异常
} MD_RUN_STATE;

int state = MD_RESET;

/**
 * @description:复位LTE模块
 * @param None
 * @return None
 */
int module_reset(void)
{
    switch (ucStateNum)
    {
    //拉低PEN引脚
    case 0:
        PEN_GPIO_SET_HIGH;
        ucStateNum++;
        break;
    //拉高PEN引脚
    case 1:
        if (wait_timeout(1000))
        {
            PEN_GPIO_SET_LOW;
            ucStateNum = 0;
            return 1;
        }
        break;
    default:
        break;
    }
    return 0;
}

/**
 * @description:
 * @param str：要检索的字符串
 * @param minval：要匹配信号质量区间最小值
 * @param minval：要匹配信号质量区间最大值
 * @return 0:信号质量不满足正常工作状态, 1:信号质量满足正常工作状态
 */
int match_csq(char *str, int minval, int maxval)
{
    int lpCsqVal = 0;
    char tmp[5] = {0};
    char *p = NULL, *q = NULL;

    p = strstr(str, "+CSQ:");
    if (p == NULL)
    {
        return 0;
    }

    p = p + 5;

    while (*p < 0x30 || *p > 0x39)
    {
        p++;
    }
    q = p;

    while (*p != ',')
    {
        p++;
    }
    memcpy(tmp, q, p - q);
    lpCsqVal = atoi(tmp);
    /* 判断信号质量是否在设置的区间内 */
    if (lpCsqVal >= minval && lpCsqVal <= maxval)
    {
        return 1;
    }
    return 0;
}

/**
 * @description: 检测模块工作状态是否就绪
 * @param None
 * @return 0：检测未完成；MD_OK：模块已就绪；MD_ERR：错误，不满足工作状态
 */
int module_is_ready(void)
{
    switch (ucStateNum)
    {
    //关闭AT命令回显
    case 0x00:
        if (fat_send_wait_cmdres_blocking("ATE0\r\n", 1000))
        {
            //收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
                //发送10次得不到正确应答
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    //读卡
    case 0x01:
        if (fat_send_wait_cmdres_blocking("AT+CPIN?\r\n", 1000))
        {
            //收到+CPIN：READY
            if (fat_cmdres_keyword_matching("+CPIN: READY"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
                //发送10次得不到正确应答
                if (ucErrorTimes++ > 10)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    //查询信号质量
    case 0x02:
        if (fat_send_wait_cmdres_blocking("AT+CSQ\r\n", 1000))
        {
            //收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
                //收到的是99（射频信号未初始化）
                if (fat_cmdres_keyword_matching("+CSQ: 99,99"))
                {
                    //发送30次得不到正确应答
                    if (ucErrorTimes++ > 30)
                    {
                        ucStateNum = MD_ERR;
                    }
                }
                else
                {
                    //信号值在SIGNALMIN~SIGNALMAX这个区间
                    if (match_csq(cFatUartRecvBuf, SIGNALMIN, SIGNALMAX))
                    {
                        ucErrorTimes = 0;
                        ucStateNum++;
                    }
                    else
                    {
                        ucStateNum = MD_ERR;
                    }
                }
            }
            //没收到应答
            else
            {
                //发送30次不应答
                if (ucErrorTimes++ > 30)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
        //查看当前GPRS附着状态
    case 0x03:
        if (fat_send_wait_cmdres_blocking("AT+CGATT?\r\n", 1000))
        {
            //收到+CGATT: 1
            if (fat_cmdres_keyword_matching("+CGATT: 1"))
            {
                ucErrorTimes = 0;
                ucStateNum = MD_OK;
            }
            else
            {
                //发送30次得不到正确应答
                if (ucErrorTimes++ > 30)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    //错误跳至飞行模式
    case MD_ERR:
        ucStateNum = 0;
        return MD_ERR;
    //完成
    case MD_OK:
        ucStateNum = 0;
        return MD_OK;
    default:
        break;
    }
    return 0;
}

/**
 * @description: Socket连接相关配置初始化
 * @param None
 * @return 0：检测未完成；MD_OK：模块已就绪；MD_ERR：错误，不满足工作状态
 */
int module_connet_parm_init(char *NETLINK)
{
    switch (ucStateNum)
    {
    //关闭移动场景
    case 0x00:
        if (fat_send_wait_cmdres_blocking("AT+CIPSHUT\r\n", 1000))
        {
            //收到SHUT OK
            if (fat_cmdres_keyword_matching("SHUT OK"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
                //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    //关闭移动场景后等待2秒
    case 0x01:
        if (wait_timeout(2000))
        {
            ucStateNum++;
        }
        break;
    //配置TCPIP应用为模式为透明传输模式
    case 0x02:
        if (fat_send_wait_cmdres_blocking("AT+CIPMODE=1\r\n", 1000))
        {
            //收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
                //发送30次得不到正确应答
                if (ucErrorTimes++ > 30)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    //设置移动卡的APN
    case 0x03:
        if (fat_send_wait_cmdres_blocking("AT+CSTT=\"CMNET\"\r\n", 1000))
        {
            //收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
                //发送30次得不到正确应答
                if (ucErrorTimes++ > 30)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
        //激活移动场景，获取IP地址
    case 0x04:
        if (fat_send_wait_cmdres_blocking("AT+CIICR\r\n", 1000))
        {
            //收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
                //发送30次得不到正确应答
                if (ucErrorTimes++ > 30)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
        //查询分配的IP地址
    case 0x05:
        if (fat_send_wait_cmdres_blocking("AT+CIFSR\r\n", 1000))
        {
            //收到用于分割ip地址的.
            if (fat_cmdres_keyword_matching("."))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
                //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    //建立连接
    case 0x06:
        if (fat_send_wait_cmdres_nonblocking(NETLINK, 15000))
        {
            //收到OK
            if (fat_cmdres_keyword_matching("OK\r\n\r\nCONNECT"))
            {
                fat_uart_clean();
                ucErrorTimes = 0;
                ucStateNum = MD_OK;
            }
            else
            {
                //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    //完成
    case MD_OK:
        ucStateNum = 0;
        return MD_OK;
    //错误跳至飞行模式
    case MD_ERR:
        ucStateNum = 0;
        return MD_ERR;
    default:
        break;
    }
    return 0;
}

/**
 * @description: 数据收发部分
 * @return 0：检测未完成；MD_ERR：错误，不满足工作状态
 */
int module_data(void)
{
    int ret = 0;
    switch (ucStateNum)
    {
    case 0x00:
        fat_delay(1000);
        ret = rev_send();
        //收到2为已收到数据并已发回
        if (ret == 2)
        {
            ucErrorTimes = 0;
        }
        //收到0为等待60s仍无接收到数据
        else if (ret == 0)
        {
            if (ucErrorTimes++ > 2)
            {
                ucStateNum = 0x01;
            }
            // printf("空闲等待时长：%d s , 超过4次退出\n", ucErrorTimes * 60);
        }
        break;
        //退出透传模式
    case 0x01:
        if (fat_send_wait_cmdres_nonblocking("+++", 2000))
        {
            //收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
                FAT_DBGPRT_INFO("uart recv: %s\r\n", cFatUartRecvBuf);
                ucErrorTimes = 0;
                ucStateNum = 0x02;
            }
            else
            {
                //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
        //进入透传模式
    case 0x02:
        if (fat_send_wait_cmdres_nonblocking("ATO\r\n", 1000))
        {
            //收到CONNECT
            if (fat_cmdres_keyword_matching("CONNECT"))
            {
                FAT_DBGPRT_INFO("uart recv: %s\r\n", cFatUartRecvBuf);
                ucErrorTimes = 0;
                ucStateNum = 0x00;
            }
            else
            {
                //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
        //错误跳至飞行模式
    case MD_ERR:
        ucStateNum = 0;
        return MD_ERR;
    default:
        break;
    }
    return 0;
}

/**
 * @description: 飞行模式处理函数
 * @param None
 * @return 0：检测未完成；MD_WORK_STA_CHK：重新开启网络跳至模块状态检测；MD_ERR：错误
 */
int module_flightmode()
{
    switch (ucStateNum)
    {
    case 0x00:
        ucFlightModeTimes++;
        ucStateNum++;
        break;
    case 0x01:
        if (ucFlightModeTimes == 2)
        {
            ucStateNum = MD_ERR;
        }
        else
            ucStateNum++;
        break;
    case 0x02:
        if (fat_send_wait_cmdres_blocking("AT+CFUN=0\r\n", 2000))
        {
            //收到OK
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                ucStateNum++;
            }
            else
            {
                //发送5次得不到正确应答，跳至MD_ERR
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case 0x03:
        if (wait_timeout(5000))
            ucStateNum++;
        break;
    case 0x04:
        if (fat_send_wait_cmdres_blocking("AT+CFUN=1\r\n", 2000))
        {
            //收到OK,状态更为MD_WORK_STA_CHK，跳至模块状态检测
            if (fat_cmdres_keyword_matching("OK"))
            {
                FAT_DBGPRT_INFO("再开启功能\r\n");
                ucStateNum = 0;
                return MD_WORK_STA_CHK;
            }
            else
            {
                //发送5次得不到正确应答，跳至MD_ERR
                if (ucErrorTimes++ > 5)
                {
                    ucStateNum = MD_ERR;
                }
            }
        }
        break;
    case MD_ERR:
        ucStateNum = 0;
        return MD_ERR;
    default:
        break;
    }
    return 0;
}

/**
 * @description: LTE模块UDP协议
 * @param None
 * @return None
 */
int module_UDP(char *NETLINK, char *MYMES)
{
    int ret = 0;
    switch (state)
    {
    //复位模块
    case MD_RESET:
        if (module_reset())
        {
            wait_timeout(LTE_POWER_ON_WAIT_TIME);
            state = MD_AT_REQ;
        }
        break;
    // AT握手
    case MD_AT_REQ:
        if (fat_send_wait_cmdres_blocking("AT\r\n", 5000)) // 5s
        {
            if (fat_cmdres_keyword_matching("OK"))
            {
                ucErrorTimes = 0;
                state = MD_CONNETINIT;
            }
            else
            {
                if (ucErrorTimes++ > 10)
                {
                    state = MD_ERR;
                }
            }
        }
        break;
    //模块状态检测
    case MD_WORK_STA_CHK:
        ret = module_is_ready();
        if (ret == MD_OK)
        {
            state = MD_CONNETINIT;
        }
        else if (ret == MD_ERR)
        {
            state = MD_FLIGHTMODE;
        }
        break;
    //连接参数初始化
    case MD_CONNETINIT:
        ret = module_connet_parm_init(NETLINK);
        if (ret == MD_OK)
        {
            ucFlightModeTimes = 0;
            state = MD_CONNETED;
        }
        else if (ret == MD_ERR)
        {
            state = MD_FLIGHTMODE;
        }
        break;
    //数据通信处理
    case MD_CONNETED:
        if (fat_send_wake_blocking((uint8_t *)MYMES, 2000))
        {
            //收到OK
            char recv[2] = {0xAC, 0xAD};
            if (fat_cmdres_keyword_matching(recv))
            {
                memcpy(&my_usart_recv, (usart_recv_4G *)cFatUartRecvBuf, sizeof(my_usart_recv));
                coor_ctrl_flag = 1;
                ucErrorTimes = 0;
                state = MD_SYSRESTART;
            }
            else
            {
                //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucErrorTimes = 0;
                    state = MD_SYSRESTART;
                }
            }
        }
        break;
    case MD_SYSRESTART:
        if (fat_send_wait_ctrl_blocking("should I restart ?", 1000))
        {
            if (fat_cmdres_keyword_matching("restart now"))
            {
                ucErrorTimes = 0;
                __set_FAULTMASK(1); //关闭总中断
                NVIC_SystemReset(); //请求单片机重启
                state = MD_ADDR;
            }
            else if ((ucFatUartRecvFinishFlg = 1) && fat_cmdres_keyword_matching("do not restart"))
            {
                //故意写赋值 ucFatUartRecvFinishFlg = 1,否则不能进入此if判断
                ucErrorTimes = 0;
                state = MD_ADDR;
            }
            else
            {
                //发送2次得不到正确应答
                if (ucErrorTimes++ > 2)
                {
                    ucErrorTimes = 0;
                    state = MD_ADDR;
                }
            }
        }
        break;
    case MD_ADDR:
        if (fat_send_wait_ctrl_blocking("getaddr", 2000))
        {
            if (fat_cmdres_keyword_matching("AT+ADDR="))
            {
                memcpy(this_cpa_addr, cFatUartRecvBuf, 12); // 获取LoRa地址配置
                printf("recved addr: ");
                USART_SendString(USART3, this_cpa_addr); //向服务器反馈
                ucErrorTimes = 0;
                state = MD_CHEL;
            }
            else
            {
                //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucErrorTimes = 0;
                    return 255;
                }
            }
        }
        break;
    case MD_CHEL:
        if (fat_send_wait_ctrl_blocking("getchel", 2000))
        {
            if (fat_cmdres_keyword_matching("AT+CH="))
            {
                memcpy(this_cpa_chel, cFatUartRecvBuf, 10); // 获取LoRa信道配置
                printf("recved chel: ");
                USART_SendString(USART3, this_cpa_chel); //向服务器反馈
                ucErrorTimes = 0;
                return 254;
            }
            else
            {
                //发送5次得不到正确应答
                if (ucErrorTimes++ > 5)
                {
                    ucErrorTimes = 0;
                    return 255;
                }
            }
        }
        break;
    case MD_NULL:
        state = MD_RESET;
        break;
    //飞行模式处理
    case MD_FLIGHTMODE:
        ret = module_flightmode();
        if (ret == MD_WORK_STA_CHK)
        {
            state = MD_WORK_STA_CHK;
        }
        else if (ret == MD_ERR)
        {
            ucFlightModeTimes = 0;
            state = MD_ERR;
        }
        break;
    //错误
    case MD_ERR:
        ucErrorTimes = 0;
        state = MD_RESET;
        break;
    default:
        state = MD_RESET;
        break;
    }
    return state;
}
