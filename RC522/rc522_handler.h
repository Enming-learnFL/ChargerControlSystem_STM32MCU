#ifndef _RC522_HANDLER
#define _RC522_HANDLER

#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEK战舰STM32开发板
//用PCD与PICC交互的主要函数
//工程文件包含与函数预定义
//PCD Powered By RC522    
//////////////////////////////////////////////////////////////////////////////////

#ifndef __RC522_CONFIG_H
/////////////////////////////////////////////////////////////////////
//和MF522通讯时返回的错误代码
/////////////////////////////////////////////////////////////////////
#define 	MI_OK                 0x26
#define 	MI_NOTAGERR           0xcc
#define 	MI_ERR                0xbb
#endif

#ifndef __RC522_HANDLER_H
#define __RC522_HANDLER_H
/////////////////////////////////////////////////////////////////////
//自定义返回代码
/////////////////////////////////////////////////////////////////////
#define		IN_SUFF								0x01		//余额不足
#define		TIMEOUT								0x02		//操作超时
#define		FAILED								0x03		//硬件启动故障
#define		UNKNOWN								0xaa		//未知故障
#define 	BACK									0xff		//纯粹返回
#endif

#define u8Array_2_uint32(arg32,arg8) (arg32 = ((arg8[0]<<(8*0))|(arg8[1]<<(8*1))|(arg8[2]<<(8*2))|(arg8[3]<<(8*3))))

/**
  * @brief  寻卡与读示函数
  * @param  运行属性 runAttribute 分为离散DISCRETE和连续CONTINUOUS
  * @retval 无
  */
void CardDetect(uint8_t runAttribute);
uint8_t ChangeAmount_withCheck(uint32_t writeValue);
void Card_Surveillance(uint8_t runAttribute);
uint8_t Charger_Commence_Handler(void);
uint8_t Charger_Terminate_Handler(void);

void Card_Balance_Feed(uint16_t balance_feed);//串口调试
#endif

