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
#include "rc522_function.h"
#include "delay.h"
#include "string.h"		
#include "control.h"
#include "stmflash.h"
#include "stm32f10x_rtc.h"	 
#include "rc522_handler.h"
#include "GUI.h"
#include "key.h"
#include "led.h"

/************************************************
 ALIENTEKս��STM32������
 
* ���׮����ϵͳ * 
 
 ���ߣ��߶��� @GaoEnMing

* @date    2021-xx-xx
* @brief   ���γ��׮�������
************************************************/
/////////////////////////////////////////////////////////////////////
//CRUCIAL CONTROL ����ϵͳ
//@Auther: Gao Enming
/////////////////////////////////////////////////////////////////////

//��ȫ���п���һ��ռ䣬Ϊ�����ڵ���SRAM�뾲̬EEPROM���ϵͳ���ܴ���ר���Ľṹ�����
_Charge_Info charge_Sync;	


uint8_t 	seqNbr_Selected;				//�Ѿ�ȷ���ĺϷ����׮��
uint16_t 	chargeTime_Selected;		//�Ѿ�ȷ���ĺϷ����ʱ��

uint8_t		seqNbr_thbwhlSwitch;		//��ǰָ��ĳ��׮��
uint16_t	chargeTime_designated;	//��ǰѡ��ĳ��ʱ��

//_Charge_Info�ṹ����ٸ�ֵ����
//���������pchargeStruct ָ��_Charge_Info��ָ��
//modeΪ��ֵ��ģʽ��
//ģʽmode��1��ʾ���ղ���״̬���и�ֵ
//ģʽmode��0��ʾ���ձ�׼״̬����ֵ
//���������� ��
//TEST 1 ��ʾģʽ1��������״̬��
//NORM 0 ��ʾģʽ0��������״̬
void _Charge_Info_Init(_Charge_Info* pchargeStruct, uint8_t mode)
{
	uint8_t cnt;
	uint8_t prototype = 1;   //ԭ�ͻ��׶�
	uint8_t nbrOfOccupied = 0;//ռ�õĳ��׮��Ŀ
	uint16_t ucflagLB = 0X0000;//��ʼ�����ֽ�
	memset(pchargeStruct,0,sizeof(_Charge_Info));//�Խṹ����г�ʼ��������ȫ����0
	pchargeStruct->token = TOKEN;
	pchargeStruct->avail = nbrOfCharger;
	
	//���Խ׶ε���λ��ԭ�ͻ��׶�
	if(prototype)ucflagLB|=(1<<7);//�����еĲ���λȫ����λ
	
	/*�ڴ˴���չ���׮Ψһʶ��ŵĳ�ʼ������*/
	for(cnt=0;cnt<nbrOfCharger;cnt++){//���ֽ�����д����׮�����
			pchargeStruct->stack[cnt].ucFlag = ucflagLB|(cnt<<8);//����������λ�����Ʋ������
	}
	
	
	if(mode){//��mode=1��Ϊ������ʾ����ʱִ�У�mode=0������ʵ��״̬������
		pchargeStruct->stack[1].ucFlag|=(1<<0);		//��xλ��λ�ı�׼д��
		pchargeStruct->stack[1].ucArray_ID = 0X99466FE5;//#1�ţ����׮��Ψһʶ�𿨺� ռλ�ã�ʵ���ϲ���Ӧ�κ�һ�ֿ���
		pchargeStruct->stack[1].timeStamp = 0XFFFFFFFF;//������ʱ���Ϊ����
		//��ʾ��
		pchargeStruct->stack[3].ucFlag|=(1<<0);
		pchargeStruct->stack[3].ucArray_ID = 0X577C63D7;//#3�ţ����׮��Ψһʶ�𿨺�
		pchargeStruct->stack[3].timeStamp = 0XFFFFFFFF;//������ʱ���Ϊ����
	}//�����Դ�Ϊ������д�����Ƿ�Ϊ�Ƿ�������������У�麯��������
	
	for(cnt=0;cnt<nbrOfCharger;cnt++){
		if(pchargeStruct->stack[cnt].ucFlag&(1<<0))nbrOfOccupied++;//������ڹ���״̬�������еĳ��׮��++
		else continue;
	}
	pchargeStruct->avail-=nbrOfOccupied;//���½ṹ����avail��ֵ
}

//_Charge_Info_Load()
//�ϵ翪�������ڴ��еĽṹ���������
//�������Ϊ�ṹ��ָ��
//����ֵrvalΪ0����ʾ�ҵ���EEPROM������ݣ����ҳɹ�����
//����ֵrvalΪ1����ʾ����֡��������ʧ�����Ѿ��޸�������˳������
//����ֵrvalΪ2����ʾ�������ú���Ȼ�Ҳ�������Ҫ�ֶ��ų�����
uint8_t _Charge_Info_Load(_Charge_Info* pchargeStruct)
{
	uint8_t cnt=0;
	uint8_t rval=0;
	uint16_t tokenBuff=0;
	_Charge_Info chargeFrameReset;//��һ���ϵ�������֡�ؽ�
	while(cnt<10){
		cnt++;
		STMFLASH_Read(SYNC_SHUTTLE_BASE,(uint16_t*)&tokenBuff,1);//��ȡ����֡��ͷ����ǣ���10��
		if(tokenBuff==TOKEN)break;
		delay_ms(20);
	}
	if(tokenBuff==TOKEN){
		rval=0;
		printf("\r\n�������֡������ã�\t\r\n");
	}else{//���ݶ�ʧ�����߳����ϵ��ͬ����ϵ�����Իָ�����
		rval=1;
		printf("\r\nδ�ҵ�����֡��������д�롭\t\r\n");
		_Charge_Info_Init(&chargeFrameReset,TEST);//д����������ڻ�����
		STMFLASH_Write(SYNC_SHUTTLE_BASE,(uint16_t*)&chargeFrameReset,sizeof(_Charge_Info)/2);//д���������
		delay_ms(10);//�Եȼ�϶
		STMFLASH_Read(SYNC_SHUTTLE_BASE,(uint16_t*)&tokenBuff,1);//�ٴζ�ȡtoken
		if(tokenBuff!=TOKEN){
		printf("\r\n��������д��ʧ�ܣ�\t\r\n");
		return 0X02;
		}else printf("\r\n��������д��ɹ���\t\r\n");
	}
	STMFLASH_Read(SYNC_SHUTTLE_BASE,(uint16_t*)pchargeStruct,sizeof(_Charge_Info)/2);//����������ֵ
	printf("\r\n�������֡���سɹ���\t\r\n");
	return rval;
}//�˺������Խ�һ������

//Charge_Status_Check()
//���λ�ںϷ�ʱ�������ĳ�ʱ��Ա
//�����׶�ֱ������������
//2021-01-01 00:00:00 ��ʼʱ���Ϊ0x5FEDF580
//url = https://tool.lu/timestamp/
uint8_t Charge_Status_Check(_Charge_Info* pchargeStruct)
{
	uint32_t timeStampNow = RTC_GetCounter();  	//��õ�ǰʱ����ʱ���
	uint32_t timeStampTemp = 0;
	uint8_t 	cnt;
	uint8_t 	seqNbr=0xFF;		//���׮���,0xFF��ʾ��δ�ҵ�
	for(cnt=0;cnt<nbrOfCharger;cnt++){
		if(pchargeStruct->stack[cnt].ucFlag&(1<<0)){
			timeStampTemp = pchargeStruct->stack[cnt].timeStamp;
			if(timeStampTemp<timeStampNow && timeStampTemp>0x5FEDF580)//��2021-01-01 00:00:00Ϊ��Ʒ��ʼʱ�������ǰ��ʱ�䲻�Ϸ�
				seqNbr = cnt;
				break;
		}
	}
	return seqNbr;
}

//����������ţ����ṹ����Ӧ��Ա��λ
void _Charge_Info_unitReset(_Charge_Info* pchargeStruct,uint8_t seqNbr)
{
	uint8_t cnt;
	uint8_t nbrOfOccupied = 0;
	if(assert_param_seqNbr(seqNbr)){ //��֤֮�Ƿ�Ϸ���û�е�ʱ�������֮
		pchargeStruct->stack[seqNbr].ucFlag &= 0XFFFE;//����״̬λ����
		pchargeStruct->stack[seqNbr].ucArray_ID &= 0X00000000;//32λucArray_ID����
		pchargeStruct->stack[seqNbr].timeStamp &= 0X00000000;//ʱ�������
		//����avail�����ģ��
		for(cnt=0;cnt<nbrOfCharger;cnt++){
			if(pchargeStruct->stack[cnt].ucFlag&(1<<0))nbrOfOccupied++;//������ڹ���״̬�������еĳ��׮��++
			else continue;
		}
		pchargeStruct->avail = nbrOfCharger - nbrOfOccupied;//���½ṹ����avail���ó��׮������ֵ
	}
	return ;
}

//_Charge_Info_unitSet()
//д����ȷ������֡
//����ʱ�����������ʱ���
//���տ�������Э��д�뿨��
void _Charge_Info_unitSet(_Charge_Info* pchargeStruct,uint8_t seqNbr,uint16_t chargeTime_Selected,uint8_t* ucArray_ID)
{
	uint8_t cnt;
	uint8_t nbrOfOccupied = 0;
	uint32_t dueTimeStamp = RTC_GetCounter();//��ȡ��ǰʱ���
	if(assert_param_seqNbr(seqNbr)){ //��֤֮�Ƿ�Ϸ���û�е�ʱ�������֮
		dueTimeStamp+=chargeTime_Selected*60; //��ý���ʱ���
		pchargeStruct->stack[seqNbr].ucFlag |= (1<<0);//����״̬λ��λ
		//д��32λucArray_ID�����ֽ�д����ֽڣ����ֽ�д����ֽڣ���ԭ�ȵĴ洢��ʽһ�����������ڶ�ȡʱ��˳��
		pchargeStruct->stack[seqNbr].ucArray_ID = ((ucArray_ID[0]<<8*0)|(ucArray_ID[1]<<8*1)|(ucArray_ID[2]<<8*2)|(ucArray_ID[3]<<8*3));
		pchargeStruct->stack[seqNbr].timeStamp = dueTimeStamp;//д��ʱ���
		//����avail�����ģ��
		for(cnt=0;cnt<nbrOfCharger;cnt++){
			if(pchargeStruct->stack[cnt].ucFlag&(1<<0))nbrOfOccupied++;//������ڹ���״̬�������еĳ��׮��++
			else continue;
		}//��for��
		pchargeStruct->avail = nbrOfCharger - nbrOfOccupied;//���½ṹ����avail���ó��׮������ֵ
	}
	return ;
}

//Charger_MonitorHandler()
//���׮״̬���ӷ�����
//��ѯ�����׮��״̬
//���ѵ�ʱ��ĳ��׮��������
//�����������������ж�ʹ��
//��RTC_IRQHandler()
void Charger_MonitorHandler(void)				
{
	uint8_t seqNbr = Charge_Status_Check(&charge_Sync); //���ս����ĳ��׮
	if(assert_param_seqNbr(seqNbr)){
		Charger_Terminate(seqNbr);	//���������̣��˴��߼������Ż������Է���ֵ������֮����ʵ�ֻ������ܣ�
		_Charge_Info_unitReset(&charge_Sync,seqNbr);			//�޸�����֡
		_Charge_Info_Synchronize(&charge_Sync);						//ͬ������֡
		printf("\r\n#%2d�ų�����\t\r\n",seqNbr);
	}
	return;
}

//��FLASHģ���EEPROM�ռ���ͬ������֡
//Ϊʲôһ������ͬ��������֡�ͻ�ʧЧ����
//�����澿��������ʲô��
void _Charge_Info_Synchronize(_Charge_Info* pchargeStruct)
{
	STMFLASH_Write(SYNC_SHUTTLE_BASE,(uint16_t*)pchargeStruct,sizeof(_Charge_Info)/2);
	return;
}
/**
	����˼�����Ǵ���Ĵ��룬����һ��ָ�룬��Ȼ��ͬ����ʱ���ֶ��������һ��ȡַ�������õ������ݺ�������
	�ֲ���ÿ��ͬ�����ݶ������ԭ����������
	STMFLASH_Write(SYNC_SHUTTLE_BASE,(uint16_t*)&pchargeStruct,sizeof(_Charge_Info)/2);
	ȥ��ȡַ���ܵõ���ȷ�Ĵ��룡
**/


//ʵ���Խ�
void Charger_ShutDown(uint8_t seqNbr)		//�ص�
{
	delay_ms(20);
	return;
}

void Charger_PowerOn(uint8_t seqNbr)		//�ϵ�
{
	delay_ms(20);
	return;
}

void Charger_UnLock(uint8_t seqNbr)			//����
{
	delay_ms(20);
	return;
}

void Charger_LockUp(uint8_t seqNbr)			//����
{
	delay_ms(20);
	return;
}

uint8_t Charger_Terminate(uint8_t seqNbr)	//������
{
	if(assert_param_seqNbr(seqNbr)){
	Charger_ShutDown(seqNbr);
	Charger_UnLock(seqNbr);
	}
	return 0x00;
}

uint8_t Charger_Commence(uint8_t seqNbr)	 //��翪ʼ
{
	if(assert_param_seqNbr(seqNbr)){
	Charger_LockUp(seqNbr);
	Charger_PowerOn(seqNbr);
	}
	return 0x00;
}


//_Charge_Info_Proofread()
//����֡��׼��ʽУ�ԣ���������չ
//���Ҹ�������
void _Charge_Info_Proofread(_Charge_Info* pchargeStruct)
{
	uint8_t cnt;
	uint8_t nbrOfOccupied = 0;
	for(cnt=0;cnt<nbrOfCharger;cnt++){
		if((pchargeStruct->stack[cnt].ucFlag&(1<<0))||(pchargeStruct->stack[cnt].ucFlag&(1<<4)))nbrOfOccupied++;
		//������ڹ���״̬�����״̬�������õĳ��׮��++
		else continue;
	}
	pchargeStruct->avail = nbrOfCharger - nbrOfOccupied;//���½ṹ����avail���ó��׮������ֵ
	_Charge_Info_Synchronize(pchargeStruct);//��������֡
	return;
}


//������֡����Ѱ���ϱ�ŵĳ�Ա
uint8_t _Match_Info_UserID(_Charge_Info* pchargeStruct,uint8_t* ucArray_ID)
{
	uint8_t cnt;
	uint8_t seqNbr_rtrn = 0xFF;//�����׮λ�ţ�0xff��ʾû��
	uint32_t ucArray_ID_temp = ((ucArray_ID[0]<<8*0)|(ucArray_ID[1]<<8*1)|(ucArray_ID[2]<<8*2)|(ucArray_ID[3]<<8*3));//תΪ32λ����
	for(cnt=0;cnt<nbrOfCharger;cnt++){
		if(pchargeStruct->stack[cnt].ucArray_ID == ucArray_ID_temp){
			seqNbr_rtrn=cnt;//�����ṹ�壬�ҵ�����Ÿ���֮����û�У���seqNbr_rtrn��Ϊ0xFF
			break;
		}
	}
	return seqNbr_rtrn;
}

//�ڶ���ҳ��
void SettingsBeforeCharge(void)
{
	#ifndef _SettingsBeforeCharge_
	#define _SettingsBeforeCharge_	
	#define ABOV 0 //00
	#define BENE 3 //11
	#define MINMAX 971
	#define MINMIN 29
	#endif
				uint8_t key_detect;
				uint8_t toggle = ABOV;//���½���ı�ʶ����ʼΪ��
				uint16_t temp;
//				uint8_t  res;
				seqNbr_thbwhlSwitch = seqNbr_default;		//��ǵ���ʼ	
				chargeTime_designated = time_default;
				
				
RUTH:		AutoReturn_cntdownRefresh_sec(20);  //��ʼˢ������
				SettingPage_StaticLayout();//��ʾ��̬����
				LCD_BorderSwitch(toggle);//��ʾ��ѡ
				ChargeNbr_RQ_ScrollDisp(290,150,seqNbr_thbwhlSwitch,50,0);	//��ʼ��ʾ
				Advanced_Show_Nbr(290,520,chargeTime_designated,50);		//��ʾĬ��ѡ��
				
				do{
					if(LED1!=0)LED1=0;
					key_detect = KEY_Scan(0);
					switch(key_detect){
						case KEY0_PRES:	if(toggle){
																AutoReturn_cntdownRefresh_sec(20);
																chargeTime_Selected = chargeTime_designated;
																if(Charger_Commence_Handler()!=0)goto RUTH;else goto DEAD;
															}else{
																AutoReturn_cntdownRefresh_sec(20);
																temp = charge_Sync.stack[seqNbr_thbwhlSwitch].ucFlag;
																if(temp&(1<<0))Popover_Show_Str(0XF0,0X18B,"�ó��׮��ռ�ã�",0X32,0X190,1000);
																else if(temp&(1<<4))Popover_Show_Str(0XF0,0X18B,"�ó��׮�ѹ��ϣ�",0X32,0X190,1000);
																else{
																	seqNbr_Selected = seqNbr_thbwhlSwitch;
																	LCD_BorderSwitch(0x02);
																	toggle=BENE;//ֻ�����޹��ϵ������²Ż�ͨ�����޸�BUG
																}
															}break;
						case KEY1_PRES:	if(toggle){
																AutoReturn_cntdownRefresh_sec(20);
																if(chargeTime_designated>MINMIN)chargeTime_designated-=15;
																Advanced_Show_Nbr(290,520,chargeTime_designated,50);
															}else{
																AutoReturn_cntdownRefresh_sec(20);
																if(seqNbr_thbwhlSwitch < nbrOfCharger-1)seqNbr_thbwhlSwitch++;
																ChargeNbr_RQ_ScrollDisp(290,150,seqNbr_thbwhlSwitch,50,0);
															}break;
						case KEY2_PRES:	if(toggle){
																AutoReturn_cntdownRefresh_sec(20);
																toggle=ABOV;
																LCD_BorderSwitch(0x01);//����
															}else{//����
																LED1=1;
																return;
															}break;
						case WKUP_PRES:	if(toggle){
																AutoReturn_cntdownRefresh_sec(20);
																if(chargeTime_designated<MINMAX)chargeTime_designated+=15;
																Advanced_Show_Nbr(290,520,chargeTime_designated,50);
															}else{
																AutoReturn_cntdownRefresh_sec(20);
																if(seqNbr_thbwhlSwitch > 0)seqNbr_thbwhlSwitch--;
																ChargeNbr_RQ_ScrollDisp(290,150,seqNbr_thbwhlSwitch,50,0);
															}break;
					}
				}while(cntdown);//��������֮ǰ�����
				
DEAD:		LED1=1;
				return;
}


//������
void theMainProcess(void)
{
			uint8_t key_detect;
TAG:	HomePage_StaticLayout();
			realTime_Display();
			title_IdleDisp(DISCRETE);
			while(1)
			{	
				realTime_reDisplay(DISCRETE);
				title_IdleDisp(CONTINUOUS);
				Card_Surveillance(DISCRETE);		//��ȡ��
				key_detect = KEY_Scan(0);				//��ȡ����
				switch (key_detect){
					case KEY0_PRES: SettingsBeforeCharge(); goto TAG;
					case KEY2_PRES: Charger_Terminate_Handler(); goto TAG;
					default : break;
				}
			}
}




