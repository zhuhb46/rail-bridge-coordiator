#ifndef __ADC_H
#define __ADC_H
#include "stm32f10x.h"
#define ADC_CH_TEMP ADC_Channel_16 //温度传感器通道

void Adc_Init(void);
u16 T_Get_Temp(void);                   //取得温度值
u16 T_Get_Adc(u8 ch);                   //获得某个通道值
u16 T_Get_Adc_Average(u8 ch, u8 times); //得到某个通道10次采样的平均值

u16 Get_Adc2(u8 ch);
u16 Get_Adc2_Average(u8 ch, u8 times);
u8 Watch_BAT(float threshold);
extern float voltage_warring;

typedef struct _adc_data
{
    uint16_t Temp; //温度数据
    uint16_t Vref; //电压数据
} adc_data;

extern adc_data my_adc_data;

#endif
