#ifndef _RC522_HANDLER
#define _RC522_HANDLER

#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEKս��STM32������
//��PCD��PICC��������Ҫ����
//�����ļ������뺯��Ԥ����
//PCD Powered By RC522    
//////////////////////////////////////////////////////////////////////////////////

#ifndef __RC522_CONFIG_H
/////////////////////////////////////////////////////////////////////
//��MF522ͨѶʱ���صĴ������
/////////////////////////////////////////////////////////////////////
#define 	MI_OK                 0x26
#define 	MI_NOTAGERR           0xcc
#define 	MI_ERR                0xbb
#endif

#ifndef __RC522_HANDLER_H
#define __RC522_HANDLER_H
/////////////////////////////////////////////////////////////////////
//�Զ��巵�ش���
/////////////////////////////////////////////////////////////////////
#define		IN_SUFF								0x01		//����
#define		TIMEOUT								0x02		//������ʱ
#define		FAILED								0x03		//Ӳ����������
#define		UNKNOWN								0xaa		//δ֪����
#define 	BACK									0xff		//���ⷵ��
#endif

#define u8Array_2_uint32(arg32,arg8) (arg32 = ((arg8[0]<<(8*0))|(arg8[1]<<(8*1))|(arg8[2]<<(8*2))|(arg8[3]<<(8*3))))

/**
  * @brief  Ѱ�����ʾ����
  * @param  �������� runAttribute ��Ϊ��ɢDISCRETE������CONTINUOUS
  * @retval ��
  */
void CardDetect(uint8_t runAttribute);
uint8_t ChangeAmount_withCheck(uint32_t writeValue);
void Card_Surveillance(uint8_t runAttribute);
uint8_t Charger_Commence_Handler(void);
uint8_t Charger_Terminate_Handler(void);

void Card_Balance_Feed(uint16_t balance_feed);//���ڵ���
#endif

