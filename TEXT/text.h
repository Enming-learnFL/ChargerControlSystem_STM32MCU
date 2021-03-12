#ifndef __TEXT_H__
#define __TEXT_H__	 
#include <stm32f10x.h>
#include "fontupd.h"
/************************************************
 ALIENTEK战舰STM32开发板
 
* 充电桩主控系统 * 
 
 作者：高恩铭 @GaoEnMing

* @date    2021-xx-xx
* @brief   防拔充电桩工程设计
************************************************/	 
 					     
void Get_HzMat(unsigned char *code,unsigned char *mat,u8 size);			//得到汉字的点阵码
void Get_SrcHzMat(unsigned char *code,unsigned char *mat);			//得到标准24pixels汉字字源
void Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode);					//在指定位置显示一个汉字
void Show_Font_Ex(u16 x,u16 y,u8 *font,u8 size,u8 mode);			//在指定位置显示任意大小的汉字
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);	//在指定位置显示一个字符串 
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u16 len);
void Advanced_Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);//不拘于显示字体大小
void Advanced_Show_Nbr(u16 x,u16 y,u16 Nbr,u8 size);//不拘于显示字体大小，显示3位数字(订制)
void Advanced_Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u16 len,uint8_t mode);	//居中显示，字体大小任意
uint8_t* zoomChar(uint16_t in_width,uint16_t in_heig,uint16_t out_width,uint16_t out_heig,	
									uint8_t *in_ptr,/*uint8_t *out_ptr,*/ uint8_t en_cn);	//根据字源缩放任意大小
void Stellar_Show_Str_Mid(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len);	//居中显示段落
void Remounted_Show_Str(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len);		//带框的居中显示段落
#endif
