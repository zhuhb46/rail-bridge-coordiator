#include "chipid.h"

u32 mcuID[3] = {0};
uint8_t stm32ChipId[12] = {0};

uint32_t idAddr[] = {0x1FFFF7AC,  /*STM32F0唯一ID起始地址*/
                     0x1FFFF7E8,  /*STM32F1唯一ID起始地址*/
                     0x1FFF7A10,  /*STM32F2唯一ID起始地址*/
                     0x1FFFF7AC,  /*STM32F3唯一ID起始地址*/
                     0x1FFF7A10,  /*STM32F4唯一ID起始地址*/
                     0x1FF0F420,  /*STM32F7唯一ID起始地址*/
                     0x1FF80050,  /*STM32L0唯一ID起始地址*/
                     0x1FF80050,  /*STM32L1唯一ID起始地址*/
                     0x1FFF7590,  /*STM32L4唯一ID起始地址*/
                     0x1FF0F420}; /*STM32H7唯一ID起始地址*/

/*获取MCU的唯一ID*/

void GetSTM32MCUID(uint32_t *id, MCUTypedef type, uint8_t *stm32ChipId)
{
  if (id != 0)
  {
    id[0] = *(uint32_t *)(idAddr[type]);
    id[1] = *(uint32_t *)(idAddr[type] + 4);
    id[2] = *(uint32_t *)(idAddr[type] + 8);
  }
  stm32ChipId[0] = 0xFF & (id[0] >> 24);
  stm32ChipId[1] = 0xFF & (id[0] >> 16);
  stm32ChipId[2] = 0xFF & (id[0] >> 8);
  stm32ChipId[3] = 0xFF & (id[0]);
  stm32ChipId[4] = 0xFF & (id[1] >> 24);
  stm32ChipId[5] = 0xFF & (id[1] >> 16);
  stm32ChipId[6] = 0xFF & (id[1] >> 8);
  stm32ChipId[7] = 0xFF & (id[1]);
  stm32ChipId[8] = 0xFF & (id[2] >> 24);
  stm32ChipId[9] = 0xFF & (id[2] >> 16);
  stm32ChipId[10] = 0xFF & (id[2] >> 8);
  stm32ChipId[11] = 0xFF & (id[2]);
}
