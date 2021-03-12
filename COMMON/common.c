#include "sys.h" 
#include "w25qxx.h"
#include "lcd.h"
#include "text.h"	
#include "string.h"												    
#include "usart.h"
#include "rtc.h"
#include <stm32f10x.h>
#include "ff.h"
#include "common.h" 
#include "malloc.h"
#include "exfuns.h"
#include "led.h"
#include "beep.h"
#include "control.h"

/************************************************
 ALIENTEKս��STM32������
 
* ���׮����ϵͳ * 
 
 ���ߣ��߶��� @GaoEnMing

* @date    2021-xx-xx
* @brief   ���γ��׮�������
************************************************/

/////////////////////////////////////////////////////////////////////
//�й�һ�����
//@Auther: Gao Enming
//���¿�ͷ�ѣ����ļ����ڴ�Ź��������һ���ļ������˷���ĺ����������빦�ܣ�����ʱ����ʾ����
//���죬��Ҫ������ͷ�ļ������ù淶һ��
//��һ�������汾������С�����ܣ���ֵ����ȷ
//���󷵻��Լ칦�ܵ�����
//ͼ���Ԫ�ص�װ��
//���뾫�򻯣�ʹ֮��������
/////////////////////////////////////////////////////////////////////


extern UINT br,bw;			//��д�����������洢������������Ӧ���������û�еط���ţ������ҵ�����exfun�У�����������
//PC2LCD2002����ȡģ����:����ʽ,˳��(��λ��ǰ),����.C51��ʽ.																		    
//��������:
//���������:ASCII��+��(' '+95)
//��ͨ����:ASCII��
//�Ѿ�����д����Flash����
u8*const APP_ASCII_S6030="1:/SYSTEM/APP/COMMON/fonts60.fon";	//���������60*30����������·�� 
//�̷�1��ʾΪ�ⲿflash W25Q128

u8* asc2_s6030=0;	//���������60*30��������󼯣������������ָ��
//����չ�������ڴ治���ں������ý���֮���ͷţ������Ȼ����

extern vu8 min_temp;		//��Ƿ���
extern vu8 hour_temp;	//���ʱ��
extern vu8 cntdown;			//�Զ����صĵ���ʱ

//realTime_Display
//��ʾʵʱʱ��
//��flash�ļ�ϵͳ�ж�ȡ�Ѿ������˵���ģ������ģ���ݷ����ⲿ�ڴ�XM8��
//��Ӧ��������Ϣ���ļ��ṹ��ָ���򱻱������ڲ��ڴ���
uint8_t realTime_DisplayInit(void)
{
	uint8_t rval=0;  //returnValue �нӺ�������ֵ
	uint8_t f_result;//fileResult  �н��ļ������ķ���ֵ
	FIL* f_calendar = NULL;
	//Ϊ�ļ��ṹ��ָ������ڲ��ڴ�ռ�
	f_calendar = (FIL*)mymalloc(SRAMIN,sizeof(FIL));//����FIL�ֽڵ��ڲ��ڴ�ռ�
	if(f_calendar==NULL)rval=1;//���ٿռ�ʧ�ܣ�����ֵ1
	else{//���ٿռ�ɹ�����ʼ��ȡ����ļ�
		f_result = f_open(f_calendar,(const TCHAR*)APP_ASCII_S6030,FA_READ);//�򿪴������������ļ�
		if(f_result==FR_OK){//����򿪳ɹ�����Ϊ��ȡ�ļ����ٻ�����
			asc2_s6030 = (uint8_t*)mymalloc(SRAMEX,f_calendar->fsize);//���ⲿ�ڴ�XM8�п���fsize��С���ڴ�
			if(asc2_s6030==0)rval=1;
			else{//���ɹ����ٻ���������W25Q128��FATfs�������ļ�ȫ�����뵽��������
				f_result = f_read(f_calendar,asc2_s6030,f_calendar->fsize,(UINT*)&br);//һ���Զ�ȡ�����ļ�
			}
			printf("\r\n�ɹ���ȡ���壬��С���ƣ�%dbytes\t\r\n",f_calendar->fsize);
			f_close(f_calendar);
		}
		if(f_result)rval=f_result;//�����ȡ�ļ�������ʲô���⣬��ԭ�ⲻ�����������ı���ֵ
	}
	return rval;
}


//����Ӧ��λ������ʾ�������ʽ��ʱ��
//ˢ��ģʽ
void realTime_reDisplay(uint8_t runAttribute)
{
	#ifndef _REAL_TIME_DISPLAY
	#define _REAL_TIME_DISPLAY
	#define RTC_CHARSIZE				60
	#define RTC_CHARWIDTH				RTC_CHARSIZE/2
	#define COLON_XOFF					480/2-RTC_CHARWIDTH/2								
	#define HOUR_XOFF						480/2-RTC_CHARWIDTH/2-RTC_CHARWIDTH*2
	#define MIN_XOFF						480/2+RTC_CHARWIDTH/2
	#define RTC_YOFF						480
	#define RTC_MODE						0X80	//ģʽΪ��λ���㣬�ǵ�����ʾ��ÿ��д���Ḳ�ǵ����е�����
	#endif
	do{//������ɢ�����ؼ�������
		RTC_Get();
		if((min_temp!=calendar.min)||(hour_temp!=calendar.hour)){
			min_temp=calendar.min;
			hour_temp=calendar.hour;
			LCD_ShowxNum_RTC(HOUR_XOFF,RTC_YOFF,calendar.hour,2,RTC_CHARSIZE,RTC_MODE);//��ʾСʱ
			LCD_ShowxNum_RTC(MIN_XOFF,RTC_YOFF,calendar.min,2,RTC_CHARSIZE,RTC_MODE);//��ʾ����
			LCD_ShowChar_RTC(COLON_XOFF,RTC_YOFF,':',RTC_CHARSIZE,RTC_MODE);//��ʾð�� �����ַ���Ҫʹ�õ����ţ�˫������ᱻ��Ϊ���ַ���
			
			_Charge_Info_Proofread(&charge_Sync);//�Է�������ʱ���жϷ���
		}
	}while(runAttribute);//0Ϊ���Σ�1Ϊ����
}

//����Ӧ��λ������ʾ�������ʽ��ʱ��
//��ʼ�ϵ��һ������ʾ
void realTime_Display(void)
{
	#ifndef _REAL_TIME_DISPLAY
	#define _REAL_TIME_DISPLAY
	#define RTC_CHARSIZE				60
	#define RTC_CHARWIDTH				RTC_CHARSIZE/2
	#define COLON_XOFF					480/2-RTC_CHARWIDTH/2								
	#define HOUR_XOFF						480/2-RTC_CHARWIDTH/2-RTC_CHARWIDTH*2
	#define MIN_XOFF						480/2+RTC_CHARWIDTH/2
	#define RTC_YOFF						480
	#define RTC_MODE						0X80	//ģʽΪ��λ���㣬�ǵ�����ʾ��ÿ��д���Ḳ�ǵ����е�����
	#endif
		RTC_Get();
		min_temp=calendar.min;
		hour_temp=calendar.hour;
		LCD_ShowxNum_RTC(HOUR_XOFF,RTC_YOFF,calendar.hour,2,RTC_CHARSIZE,RTC_MODE);//��ʾСʱ
		LCD_ShowxNum_RTC(MIN_XOFF,RTC_YOFF,calendar.min,2,RTC_CHARSIZE,RTC_MODE);//��ʾ����
		LCD_ShowChar_RTC(COLON_XOFF,RTC_YOFF,':',RTC_CHARSIZE,RTC_MODE);//��ʾð�� �����ַ���Ҫʹ�õ����ţ�˫������ᱻ��Ϊ���ַ���
}

//�����ɹ���������ʾ
void successTone(void)
{
	#define TONE_NUM 3
	uint8_t cnt;
	BEEP=0;
	LED1=1;
	delay_ms(10);
	for(cnt=0;cnt<TONE_NUM;cnt++){
		BEEP=1;
		LED1=0;
		delay_ms(80);
		LED1=1;
		BEEP=0;
		delay_ms(40);
	}
}

//�����������ʾ
void errorTone(void)
{
	#define eTONE_NUM 2
	uint8_t cnt;
	BEEP=0;
	LED1=1;
	delay_ms(10);
	for(cnt=0;cnt<eTONE_NUM;cnt++){
		BEEP=1;
		LED1=0;
		delay_ms(180);
		LED1=1;
		BEEP=0;
		delay_ms(40);
	}
}

//�Զ����صĵ���ʱˢ��
void AutoReturn_cntdownRefresh_sec(uint8_t sec)
{
	cntdown = sec;
	return;
}

//����ʱ�Լ�
//�����޲���ʱ������
void AutoReturn_cntdownTikTok(void)
{
	if(cntdown)cntdown--;								//��cnt��δ���㣬������ݼ�
	return;
}

//������delay_ms
//�˶�ͨ��ѭ��������ʱ����õ�ʱ��
void delay_ms_plus(uint16_t ms)
{
	u16 ms0 = ms%1860;//Ϊdelay��˳�����ö�ʹ��
	u8  ms1 = ms/1860;
	delay_ms(ms0);//���ʱΪ1864ms����˶����뷴�������С
	for(;ms1!=0;ms1--)delay_ms(1860);
}

