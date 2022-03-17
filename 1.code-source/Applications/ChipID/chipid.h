#ifndef __chipid_H
#define __chipid_H	 
#include "stm32f10x.h"

/*定义STM32 MCU的类型*/
typedef enum {
  STM32F0,
  STM32F1,
  STM32F2,
  STM32F3,
  STM32F4,
  STM32F7,
  STM32L0,
  STM32L1,
  STM32L4,
  STM32H7,
}MCUTypedef;
 
extern u32 mcuID[3];
extern uint8_t stm32ChipId[12];  
void GetSTM32MCUID(uint32_t *id, MCUTypedef type, uint8_t *stm32ChipId);


#endif

