#ifndef __LTE_H
#define __LTE_H

#include "stm32f10x.h"

extern char this_cpa_addr[13];
extern char this_cpa_chel[11];
extern int state;
extern uint8_t coor_ctrl_flag;

int module_UDP(char *NETLINK, char *MYMES);

#endif
