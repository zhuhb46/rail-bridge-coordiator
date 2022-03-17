#include "adc.h"
#include "fat_core.h"

// T（℃）={（V25-Vsense）/Avg_Slope}+25

/*

T（℃）={（V25-Vsense）/Avg_Slope}+25

V25=Vsense在25度时的数值（典型值为：1.43）。
Avg_Slope=温度与Vsense曲线的平均斜率（单位为mv/℃或uv/℃）（典型值为4.3Mv/℃）。
利用以上公式，我们就可以方便的计算出当前温度传感器的温度了
*/

adc_data my_adc_data;
float voltage_warring = 3.45;

void Adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE); //使能GPIOA,ADC1通道时钟

	RCC_ADCCLKConfig(RCC_PCLK2_Div6); //分频因子6时钟为72M/6=12MHz

	ADC_DeInit(ADC1); //将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;					// ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;						//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;					//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				// ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;								//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);									//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

	ADC_TempSensorVrefintCmd(ENABLE); //开启内部温度传感器

	ADC_Cmd(ADC1, ENABLE); //使能指定的ADC1

	ADC_ResetCalibration(ADC1); //重置指定的ADC1的复位寄存器

	while (ADC_GetResetCalibrationStatus(ADC1))
		; //获取ADC1重置校准寄存器的状态,设置状态则等待

	ADC_StartCalibration(ADC1); //

	while (ADC_GetCalibrationStatus(ADC1))
		; //获取指定ADC1的校准程序,设置状态则等待

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC2, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	ADC_DeInit(ADC2);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC2, &ADC_InitStructure);
	ADC_Cmd(ADC2, ENABLE);
	ADC_ResetCalibration(ADC2);
	while (ADC_GetResetCalibrationStatus(ADC2))
		;
	ADC_StartCalibration(ADC2);
	while (ADC_GetCalibrationStatus(ADC2))
		;
}
//获得ADC值
// ch:通道值 0~3
u16 T_Get_Adc(u8 ch)
{

	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5); // ADC1,ADC通道3,第一个转换,采样时间为239.5周期

	ADC_SoftwareStartConvCmd(ADC1, ENABLE); //使能指定的ADC1的软件转换启动功能
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
		;								 //等待转换结束
	return ADC_GetConversionValue(ADC1); //返回最近一次ADC1规则组的转换结果
}

//得到ADC采样内部温度传感器的值
//取10次,然后平均
u16 T_Get_Temp(void)
{
	u16 temp_val = 0;
	u8 t;
	for (t = 0; t < 10; t++)
	{
		temp_val += T_Get_Adc(ADC_Channel_16); // TampSensor
		fat_delay(5);
	}
	return temp_val / 10;
}

//获取通道ch的转换值
//取times次,然后平均
u16 T_Get_Adc_Average(u8 ch, u8 times)
{
	u32 temp_val = 0;
	u8 t;
	for (t = 0; t < times; t++)
	{
		temp_val += T_Get_Adc(ch);
		fat_delay(5);
	}
	return temp_val / times;
}

u16 Get_Adc2(u8 ch)
{
	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_239Cycles5); // ADC1,ADC通道,采样时间为239.5周期

	ADC_SoftwareStartConvCmd(ADC2, ENABLE); //使能指定的ADC1的软件转换启动功能

	while (!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC))
		; //等待转换结束

	return ADC_GetConversionValue(ADC2); //返回最近一次ADC1规则组的转换结果
}

u16 Get_Adc2_Average(u8 ch, u8 times)
{
	u32 temp_val = 0;
	u8 t;
	for (t = 0; t < times; t++)
	{
		temp_val += Get_Adc2(ch);
		fat_delay(5);
	}
	return temp_val / times;
}

u8 Watch_BAT(float threshold)
{
	if ((u16)(threshold / 6.6 * 4095) > T_Get_Adc_Average(ADC_Channel_9, 10))
		return 0;
	return 1;
}
