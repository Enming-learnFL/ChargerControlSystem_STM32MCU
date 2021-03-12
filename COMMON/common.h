#ifndef _COMMON_H
#define _COMMON_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEKս��STM32������
//ͨ�ú궨��			  
////////////////////////////////////////////////////////////////////////////////// 

//runAttribute���� ��ɢ���� ��������
#define DISCRETE 					0x00
#define CONTINUOUS 				0x01
//�Զ����ص�ʱ������
#define AUTO_RETURN_SEC_DEFAULT 	20

extern u8* asc2_s6030;

uint8_t realTime_DisplayInit(void);
void realTime_reDisplay(uint8_t runAttribute);
void realTime_Display(void);

//ͨ�ÿؼ�
void successTone(void);
void errorTone(void);
void AutoReturn_cntdownRefresh_sec(uint8_t sec);
void AutoReturn_cntdownTikTok(void);//��ʱ����
void delay_ms_plus(uint16_t ms);

#endif

