#ifndef __TEXT_H__
#define __TEXT_H__	 
#include <stm32f10x.h>
#include "fontupd.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������V3
//������ʾ ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/1/20
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 
 					     
void Get_HzMat(unsigned char *code,unsigned char *mat,u8 size);			//�õ����ֵĵ�����
void Get_SrcHzMat(unsigned char *code,unsigned char *mat);			//�õ���׼24pixels������Դ
void Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode);					//��ָ��λ����ʾһ������
void Show_Font_Ex(u16 x,u16 y,u8 *font,u8 size,u8 mode);			//��ָ��λ����ʾ�����С�ĺ���
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);	//��ָ��λ����ʾһ���ַ��� 
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u16 len);
void Advanced_Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);//��������ʾ�����С
void Advanced_Show_Nbr(u16 x,u16 y,u16 Nbr,u8 size);//��������ʾ�����С����ʾ3λ����(����)
void Advanced_Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u16 len,uint8_t mode);	//������ʾ�������С����
uint8_t* zoomChar(uint16_t in_width,uint16_t in_heig,uint16_t out_width,uint16_t out_heig,	
									uint8_t *in_ptr,/*uint8_t *out_ptr,*/ uint8_t en_cn);	//������Դ���������С
void Stellar_Show_Str_Mid(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len);	//������ʾ����
void Remounted_Show_Str(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len);		//����ľ�����ʾ����
#endif
