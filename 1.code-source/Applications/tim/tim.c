#include "tim.h"
#include "LoRa.h"
#include "gpio.h"
#include "fat_core.h"
/**
 * @description: TIM2定时器初始化
 * @param ms: 单位毫秒，定时器时间间隔
 * @return None
 */

u8 startCount = 0;

uint8_t addrget[2] = {0x01, 0x02};
uint8_t channelget = 0;
uint8_t ackTempIdx = 0;
uint8_t ackDataIdx = 0;
uint16_t Timer3Rtc_1s = 0;
uint16_t Timer3Rtc_20s = 0;
uint16_t Timer3Rtc_Restart = 0;
uint8_t Timer3Rtc_FLAG = 0;

uint8_t time_cycle = TIME_CYCLE;
uint8_t time_work_coor = TIME_WORK_COOR;
uint8_t time_cycle_term = TIME_CYCLE_TERM;
uint8_t time_recnt_term = TIME_RECNT_TERM;
uint8_t work_windows = WORKWINDS;

void tim2_init(int ms)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = ms * 1000 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

void TIM3_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_ITConfig(
		TIM3,
		TIM_IT_Update,
		ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM3, ENABLE);
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
/**
 * @brief  This function handles TIM2 interrupt request.
 * @param  None
 * @retval None
 */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		fat_tim_proc();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

/**
 * @brief  This function handles TIM3 interrupt request.
 * @param  None
 * @retval None
 */
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		for (uint8_t termalidx = 0; termalidx < work_windows; termalidx++) //监视子节点是否长期未更新
		{
			if (bufWatch[termalidx] == 1)
			{
				termalMonitor[termalidx]++;
			}
		}
		DeubgeTick = ~DeubgeTick; //指示灯翻转
		startCount++;
		if (Timer3Rtc_1s >= 19)
		{
			Timer3Rtc_Restart++;
			Timer3Rtc_20s++;
			Timer3Rtc_1s = 0;
			if (Timer3Rtc_20s >= time_cycle)
			{
				Timer3Rtc_20s = 0;
				Timer3Rtc_FLAG = 1;
			}
		}
		else
			Timer3Rtc_1s++;
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}
