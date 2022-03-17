#include "stm32f10x.h"
#include "gpio.h"
#include "uart.h"
#include "tim.h"
#include "fat_core.h"
#include "lte_demo.h"
#include "uart.h"
#include "string.h"
#include "LoRa.h"
#include "adc.h"
#include "chipid.h"
#include "iwdg.h"

/************************************************************/
// 默认配置
#define MYNAME "COORDINATOR_043"
#define CPA_ADDR "AT+ADDR=43\r\n"
#define CPA_CHEL "AT+CH=43\r\n"
#define CAT1NETLINE "AT+CIPSTART=\"UDP\",\"47.97.11.140\",6400\r\n"
#define VOLTAGE_LOW 3.45
#define VOLTAGE_HIGH 3.65

/************************************************************/
// 主函数
int main()
{
#if FAT_DBG_INFO_FLG
	uart3_init();
#endif
	pen_gpio_init();
	uart1_init();
	uart2_init();
	tim2_init(FAT_TIMER_VAL);
	TIM3_Int_Init(36000 - 1, 2000 - 1);				// 1s tick
	Adc_Init();										//初始化ADC
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组2
	reg_fat_uart_send_byte(uart2_send_byte);
	uint8_t STARTWORK[] = "STARTWORK";
	uint8_t STOPWORK[] = "STOPWORK";
	uint8_t myName[] = MYNAME;
	voltage_warring = VOLTAGE_LOW;
	/************************************************************/
	DeubgeTick = 1; //指示灯
	workState = 0;	//工作状态初始化
	LoRaPower = 0;	// LoRa模块断电
	/************************************************************/
	my_adc_data.Temp = T_Get_Adc_Average(ADC_CH_TEMP, 10); //获取温度数据
	my_adc_data.Vref = Get_Adc2(ADC_Channel_9);			   //获取参考电压值
	uint8_t myadcbuf[4] = {0};
	myadcbuf[0] = my_adc_data.Temp >> 8;
	myadcbuf[1] = my_adc_data.Temp;
	myadcbuf[2] = my_adc_data.Vref >> 8;
	myadcbuf[3] = my_adc_data.Vref;
	uart2_send_longbyte(myadcbuf, sizeof(myadcbuf));
	printf("the system startwork\r\n");
	/************************************************************/
	IWDG_Init(IWDG_Prescaler_256, 4000); //看门狗
	IWDG_Feed();						 //喂狗
	/************************************************************/
	Timer3Rtc_FLAG = 1;
	while (1)
	{
		if (Timer3Rtc_FLAG > 0) //系统初始化，此语句内均为数据初始化
		{
			/*************************************************************************/
			/*初始化数据及外设*/
			TIM_Cmd(TIM2, ENABLE); //使能TIMx外设
			printf("start work initial");
			Timer3Rtc_FLAG = 0;
			workState = 0;				  //初始化
			if (Timer3Rtc_Restart > 2160) //定时重启
			{
				__set_FAULTMASK(1); //关闭总中断
				NVIC_SystemReset(); //请求单片机重启
				while (1)			//死循环用于触发看门狗
				{
				}
			}
			getAddr = 0;	  //串口1接收到子节点数据，将此变量置为‘1’，协调器向子节点反馈后重设为‘0’
			getMAC = 0;		  //串口1接收到子节点握手，将此变量置为‘1’，协调器向子节点反馈后重设为‘0’
			Row = RowMax - 1; //串口1接收数据放置于RowMax - 1行的位置，作为数据暂存
			Column = 0;		  //串口1接收数据列初始化
			dotNum = 0;		  //用于判断接收到的数据为第几个数据包
			idxAddr = 0;	  //用于表示是那个子节点
			addrNum = 0;	  //挂载子节点数量的统计
			tempIdx = 0;	  //挂载子节点编号的配置
			curTmlNum = 0;	  //当前服务子节点的数量统计
			coor_ctrl_flag = 0;
			uint8_t whosetlora = 0;
			time_cycle = TIME_CYCLE;		   //协调器工作周期默认配置
			time_work_coor = TIME_WORK_COOR;   //协调器工作时长默认配置
			time_cycle_term = TIME_CYCLE_TERM; //子节点工作周期默认配置
			time_recnt_term = TIME_RECNT_TERM; //协调器工作时长默认配置
			work_windows = WORKWINDS;
			for (uint8_t i = 0; i < work_windows; i++) //初始化协调器服务窗口监视器
			{
				bufWatch[i] = 0;			 //用于监视是否在传输数据
				termalMonitor[i] = 0;		 //用于监视是否传输中断
				memset(ChipIdBuf[i], 0, 12); //将全部mac地址空间清零
			}
			for (uint8_t i = 0; i < RowMax; i++)
			{
				memset(dataBuffer[i], 0, ColumnMax); //初始化数据区
			}
			memset(&my_usart_recv, 0, sizeof(my_usart_recv)); //配置文件结构体清零
			ADC_Cmd(ADC1, ENABLE);							  //初始化ADC
			ADC_Cmd(ADC2, ENABLE);							  //初始化ADC
			IWDG_Feed();									  //喂狗
			/************************************************************/
			// 封装要发送的苏醒数据
			myusart_send_wake.Temp = T_Get_Adc_Average(ADC_CH_TEMP, 10);	   //获取温度
			myusart_send_wake.Vref = Get_Adc2_Average(ADC_Channel_9, 20);	   //获取参考电压值
			myusart_send_wake.rtcnum = Timer3Rtc_Restart;					   //获取时间
			memcpy(myusart_send_wake.myName, myName, sizeof(myName));		   //本地编号
			memcpy(myusart_send_wake.STARTWORK, STARTWORK, sizeof(STARTWORK)); //开始工作的标志
			char mysendwakebuf[sizeof(myusart_send_wake)] = {0};
			memcpy(mysendwakebuf, &myusart_send_wake, sizeof(myusart_send_wake));
			mysendwakebuf[26] = myusart_send_wake.rtcnum >> 8;
			mysendwakebuf[27] = myusart_send_wake.rtcnum;
			mysendwakebuf[28] = myusart_send_wake.Temp >> 8;
			mysendwakebuf[29] = myusart_send_wake.Temp;
			mysendwakebuf[30] = myusart_send_wake.Vref >> 8;
			mysendwakebuf[31] = myusart_send_wake.Vref;
			/************************************************************/
			/*初始化4G模块*/
			char netlinkport[] = CAT1NETLINE; //网络参数
			startCount = 0;
			state = 0;
			while (1)
			{
				uint8_t netstate = module_UDP(netlinkport, mysendwakebuf);
				if (netstate == 254)
				{
					printf("4G start work\r\n");
					whosetlora = 1;
					break;
				}
				else if (netstate == 255)
				{
					printf("4G recv error\r\n");
					whosetlora = 0;
					break;
				}
				if (startCount <= 40) //超时重启
					IWDG_Feed();	  //喂狗
			}
			/************************************************************/
			//向服务器发送苏醒数据，并获取控制反馈
			// IWDG_Feed();
			printf("cat1 start work !!!\r\n");
			IWDG_Feed(); //喂狗
			fat_delay(300);
			if (Watch_BAT(voltage_warring)) //设置电压阈值
			{
				voltage_warring = VOLTAGE_LOW;
				if (coor_ctrl_flag == 1) // 获取工作时长配置
				{
					uint8_t sendbuf[sizeof(my_usart_recv)] = {0};
					memcpy(sendbuf, &my_usart_recv, sizeof(my_usart_recv)); //向服务器反馈
					uart2_send_longbyte(sendbuf, sizeof(my_usart_recv));
					if ((sendbuf[0] == 0xAC) && (sendbuf[1] == 0xAD))
					{
						time_cycle = my_usart_recv.coor_cycle;																						//工作周期
						time_work_coor = (my_usart_recv.coor_work < my_usart_recv.coor_cycle) ? my_usart_recv.coor_work : my_usart_recv.coor_cycle; //工作时长
						time_cycle_term = my_usart_recv.term_cycle;																					//子节点周期
						time_recnt_term = my_usart_recv.term_recon;																					//子节点重连
						if (my_usart_recv.term_win > 0)
							work_windows = (my_usart_recv.term_win < 6) ? my_usart_recv.term_win : 3; //工作窗口
					}
					coor_ctrl_flag = 0;
					printf("time_cycle:%d\r\n", time_cycle);
					printf("time_work_coor:%d\r\n", time_work_coor);
					printf("time_cycle_term:%d\r\n", time_cycle_term);
					printf("time_recnt_term:%d\r\n", time_recnt_term);
					printf("work_windows:%d\r\n", work_windows);
					printf("this_cpa_addr:%s\r\n", this_cpa_addr);
					printf("this_cpa_chel:%s\r\n", this_cpa_chel);
				}
				/************************************************************/
				// LoRa初始化
				fat_delay(100);
				if (whosetlora == 0)
				{
					memcpy(this_cpa_addr, CPA_ADDR, sizeof(CPA_ADDR)); // LoRa地址默认配置
					memcpy(this_cpa_addr, CPA_CHEL, sizeof(CPA_CHEL)); // LoRa信道默认配置
				}
				uint8_t setLoRaTimeOut = 0;
				while (1)
				{
					IWDG_Feed();														  //喂狗
					if (88 == LoRaInit(this_cpa_addr, this_cpa_chel, FeedBack, &RecvCnt)) //初始化成功
					{
						fat_uart_send_str("set lora success");
						printf("set lora success\r\n");
						print_LoRa_env(FeedBack, &RecvCnt);
						memset(FeedBack, 0, 30);
						USART_SendString(USART1, "AT+ENTM\r\n"); //退出AT模式
						RecvCnt = 0;
						fat_delay(300);
						USART_SendString(USART2, FeedBack);
						USART_SendString(USART1, FeedBack);
						break;
					}
					else //初始化失败
					{
						fat_uart_send_str("set lora error\r\n");
						printf("set lora error\r\n");
						setLoRaTimeOut++;
						if (5 == setLoRaTimeOut)
							break;
					}
				}
				DeubgeTick = 0; //亮指示灯
				workState = 1;	//开始工作
			}
			else //电压过低无法工作
			{
				voltage_warring = VOLTAGE_HIGH;
				workState = 2;
				fat_uart_send_str("Voltage is too low to work,so i have to sleep and wait for charged");
			}
		}
		/************************************************************/
		// LoRa接收数据并向服务器转发
		while (workState == 1)
		{
			if (Timer3Rtc_20s > time_work_coor)
			{
				workState = 2;
			}
			IWDG_Feed();												 //喂狗
			for (uint8_t bufPack = 0; bufPack < work_windows; bufPack++) //此循环只负责向4G发送数据
			{
				IWDG_Feed();												   //喂狗
				if ((termalMonitor[bufPack] >= 6) || (bufWatch[bufPack] == 2)) //子节点数据接收完毕，或者子节点工作超时，发送该数据，注销该子节点
				{
					memset(ChipIdBuf[bufPack], 1, 12); //清空MAC地址
					my_adc_data.Temp = T_Get_Adc_Average(ADC_CH_TEMP, 10);
					my_adc_data.Vref = Get_Adc2_Average(ADC_Channel_9, 10); //获取参考电压值
					myadcbuf[0] = my_adc_data.Temp >> 8;
					myadcbuf[1] = my_adc_data.Temp;
					myadcbuf[2] = my_adc_data.Vref >> 8;
					myadcbuf[3] = my_adc_data.Vref;
					memcpy(&dataBuffer[bufPack * pacNumMax][21], &myadcbuf, sizeof(myadcbuf));
					memcpy(&dataBuffer[bufPack * pacNumMax][30], myName, sizeof(myName));
					for (uint8_t i = 0; i < pacNumMax; i++)
					{
						DeubgeTick = ~DeubgeTick;											 //指示灯翻转
						IWDG_Feed();														 //喂狗
						uart2_send_longbyte(dataBuffer[bufPack * pacNumMax + i], ColumnMax); //发送缓冲区中的数据
						memset(dataBuffer[bufPack * pacNumMax + i], 0, ColumnMax);			 //清空缓冲区
						if (i % 7 == 0)														 //每七个小包组成一个大包发出去
						{
							fat_delay(500);
							if ((dataBuffer[bufPack * pacNumMax + i + 1][0] == 0) && (dataBuffer[bufPack * pacNumMax + i + 1][1] == 0)) //判断数据包是否完整，只发送有数据的部分
								break;
						}
					}
					bufWatch[bufPack] = 0; //释放服务窗口
					if (curTmlNum > 0)	   //防止减溢出
						curTmlNum--;
					termalMonitor[bufPack] = 0; //释放断联监视
				}
			}
		}
		/************************************************************/
		//负责去使能外设以及清空缓冲区
		if (workState == 2)
		{
			for (uint8_t bufPack = 0; bufPack < work_windows; bufPack++)
			{
				if (bufWatch[bufPack] == 1)
				{
					memset(ChipIdBuf[bufPack], 1, 12);
					my_adc_data.Temp = T_Get_Adc_Average(ADC_CH_TEMP, 10);
					my_adc_data.Vref = Get_Adc2_Average(ADC_Channel_9, 10); //获取参考电压值
					memcpy(&dataBuffer[bufPack * pacNumMax][21], &my_adc_data, 4);
					memcpy(&dataBuffer[bufPack * pacNumMax][30], myName, sizeof(myName));
					for (uint8_t i = 0; i < pacNumMax; i++)
					{
						DeubgeTick = ~DeubgeTick;											 //指示灯翻转
						IWDG_Feed();														 //喂狗
						uart2_send_longbyte(dataBuffer[bufPack * pacNumMax + i], ColumnMax); //发送缓冲区中的数据
						memset(dataBuffer[bufPack * pacNumMax + i], 0, ColumnMax);			 //清空缓冲区
						if (i % 7 == 0)
						{
							fat_delay(500);
							if ((dataBuffer[bufPack * pacNumMax + i + 1][0] == 0) && (dataBuffer[bufPack * pacNumMax + i + 1][1] == 0)) //判断数据包是否完整，只发送有数据的部分
								break;
						}
					}
					bufWatch[bufPack] = 0;
				}
			}
			fat_delay(350);
			uart2_send_longbyte(STOPWORK, sizeof(STOPWORK));
			fat_delay(350);
			DeubgeTick = 1;
			LoRaPower = 0;	  // LoRa模块断电
			NetLinkPower = 1; //网络模块断电
			getAddr = 0;
			getMAC = 0;
			Row = RowMax - 1;
			Column = 0;
			dotNum = 0;
			idxAddr = 0;
			addrNum = 0;
			tempIdx = 0;
			curTmlNum = 0;
			for (uint8_t i = 0; i < work_windows; i++)
			{
				bufWatch[i] = 0;
				memset(ChipIdBuf[i], 0, 12);
			}
			IWDG_Feed();			//喂狗
			ADC_Cmd(ADC1, DISABLE); //去使能ADC1
			ADC_Cmd(ADC2, DISABLE); //去使能ADC2
			// USART_Cmd(USART1, DISABLE); //去使能USART1
			// USART_Cmd(USART2, DISABLE); //去使能USART2
			// USART_Cmd(USART3, DISABLE); //去使能USART3
			TIM_Cmd(TIM2, DISABLE); //去使能TIMx外设
			workState = 3;
		}
		/************************************************************/
		// 休眠
		DeubgeTick = ~DeubgeTick; //关闭指示灯
		IWDG_Feed();			  //喂狗
		__WFI();
	}
}
