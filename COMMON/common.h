#ifndef _COMMON_H
#define _COMMON_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEK战舰STM32开发板
//通用宏定义			  
////////////////////////////////////////////////////////////////////////////////// 

//runAttribute参数 离散运行 连续运行
#define DISCRETE 					0x00
#define CONTINUOUS 				0x01
//自动返回的时长设置
#define AUTO_RETURN_SEC_DEFAULT 	20

extern u8* asc2_s6030;

uint8_t realTime_DisplayInit(void);
void realTime_reDisplay(uint8_t runAttribute);
void realTime_Display(void);

//通用控件
void successTone(void);
void errorTone(void);
void AutoReturn_cntdownRefresh_sec(uint8_t sec);
void AutoReturn_cntdownTikTok(void);//计时流逝
void delay_ms_plus(uint16_t ms);

#endif

