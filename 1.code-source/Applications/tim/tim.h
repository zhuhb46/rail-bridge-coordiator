#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"

#define TIME_CYCLE 90      //协调器工作大周期
#define TIME_WORK_COOR 45  //协调器工作时长
#define TIME_CYCLE_TERM 90 //子节点工作大周期
#define TIME_RECNT_TERM 2  //子节点重复工作时长
#define WORKWINDS 3        //工作窗口

extern uint8_t time_cycle;
extern uint8_t time_work_coor;
extern uint8_t time_cycle_term;
extern uint8_t time_recnt_term;
extern uint8_t work_windows;

extern u8 startCount;
extern uint8_t addrget[2];
extern uint8_t channelget;
extern uint8_t ackTempIdx;
extern uint8_t ackDataIdx;
extern uint16_t Timer3Rtc_1s;
extern uint16_t Timer3Rtc_20s;
extern uint8_t Timer3Rtc_FLAG;
extern uint16_t Timer3Rtc_Restart;

void tim2_init(int ms); // TIM2定时器初始化
void TIM3_Int_Init(u16 arr, u16 psc);

#endif
