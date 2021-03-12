#include "sys.h"
#include "delay.h"
#include "common.h" //����ʱ
#include "rc522_function.h"
#include "rc522_config.h"
#include "led.h"
#include "lcd.h"
#include "rc522_handler.h"
#include "control.h"
#include "rtc.h"
#include "key.h"
#include "GUI.h"
#include "text.h"

/************************************************
 ALIENTEKս��STM32������
 
* ���׮����ϵͳ * 
 
 ���ߣ��߶��� @GaoEnMing

* @date    2021-xx-xx
* @brief   ���γ��׮�������
************************************************/

//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEKս��STM32������
//��PCD��PICC��������Ҫ����
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

extern vu8 cntdown;//�������ܸ�ֵ


/**
  * @brief  Ѱ�����ʾ�����������ã�
  * @param  �������� runAttribute ��Ϊ��ɢDISCRETE������CONTINUOUS
  * @retval ��
  */
uint8_t KeyValue[]={0xFF ,0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // ��A��Կ��A����Կ
void CardDetect (uint8_t runAttribute)
{
//	uint32_t writeValue = 100;
	uint32_t readValue;
	char cStr [ 30 ];
  uint8_t ucArray_ID [ 4 ];    /*�Ⱥ���IC�������ͺ�UID(IC�����к�)*/ //uc��unsigned char                                                                                        
	uint8_t ucStatusReturn;      /*����״̬*/
	do{    
			/*Ѱ��*/
			if ( ( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK )  
			{   /*��ʧ���ٴ�Ѱ��*/
				ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );
				LED0_Blink(3);
				delay_ms(720);
			}			
			if ( ucStatusReturn == MI_OK  )
			{
				/*����ײ�����ж��ſ������д��������Χʱ������ͻ���ƻ������ѡ��һ�Ž��в�����*/
				if ( PcdAnticoll ( ucArray_ID ) == MI_OK ) //���������ʱ�����ģ�                                                                  
				{
					PcdSelect(ucArray_ID);			//�뷵���Ŀ��Ŷ�Ӧ�����������ͨ��
					LED0_Blink(1);				
					PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//У������ 
	//        WriteAmount(0x11,writeValue); //д����
	//				delay_ms(20);
	//				static uint8_t	attachFlag = 0; //1��ʾ������0��ʾδ����
					if((ReadAmount(0x11,&readValue) == MI_OK))	//��ȡ���
					{		
	//					writeValue +=100;
						successTone();//��˷�ʽ��ʾ���ݣ����ֽڱ�ʾ���λ��
						sprintf ( cStr, "The Card ID is: %02X%02X%02X%02X",ucArray_ID [0], ucArray_ID [1], ucArray_ID [2],ucArray_ID [3] );
						LCD_ShowString(30,40,320,24,24,cStr);

						sprintf ( cStr, "The residual amount: %d", readValue);				 										 	         
						LCD_ShowString(30,70,320,24,24,cStr);
	//					attachFlag = 1;
						PcdHalt();//��Ƭ��������״̬�����޷�����ȡ		
						delay_ms(20);
					}
				}			
			}
	//		else if(( PcdRequest( PICC_REQIDL, ucArray_ID ) == !MI_OK)&&( attachFlag == 1 ))
	//		{
	//			LED1_Blink(3);
	//			BEEP_Impulse(1);	//�ó���
	//			attachFlag = 0;
	//		}	    
		}while( runAttribute );
}

//��У���д������
//��Ҫд������У��֮��
//rvalΪ0���ʾû�����⣬һ��˳��
//rval��Ϊ����ֵ����˵��û��д��ɹ�
//�����û��д�ԣ����Ƿ����������ı䶯
//��������Ҫ����������
uint8_t ChangeAmount_withCheck(uint32_t writeValue)
{
	uint8_t cnt;
	uint8_t rval=UNKNOWN;
	uint8_t res;
	uint32_t readValue;
	for(cnt=0;cnt<10;cnt++){//ִ��10�Σ����ҷ������յĴ������
		WriteAmount(0x11,writeValue); //д����
		delay_ms(20);//��ֵ���ʽ�ķ���ֵ��������
		if(((res = ReadAmount(0x11,&readValue)) == MI_OK)&&(readValue==writeValue)){
			rval = 0;//һ��˳���ķ���ֵ
			printf("\r\n���д��ɹ���\r\n");
			break;
		}else{
			rval = res;
			if((res!=MI_NOTAGERR)&&(res!=MI_ERR))rval = UNKNOWN;
		}
	}
	if(rval)printf("\r\n���д��ʧ�ܣ�������룺%#2hx\r\n",(unsigned short)rval);
	return rval;
}

//��У��ض�ȡ����
//��Ҫд������У��֮��
//rvalΪ��ʵ�ط�ӳReadAmount()�ķ���ֵ
//�����������Ϊ��ζ�ȡ�ķ�װ
uint8_t ReadAmount_withCheck(uint32_t *readValue)
{
	uint8_t cnt;
	uint8_t rval=UNKNOWN;
	for(cnt=0;cnt<10;cnt++){
		if((rval = ReadAmount(0x11,readValue))==MI_OK)return rval;
		delay_ms(20);			//����ɹ�������ֱ�ӷ���
	}
	printf("\r\n��ȡ���ʧ�ܣ�������룺%#2hx\r\n",(unsigned short)rval);
	return rval;
}


//����������
uint8_t Charger_Commence_Handler(void)
{
	uint32_t writeValue;
	uint32_t readValue;
	uint32_t protoAmount;
  uint8_t ucArray_ID [ 4 ];    /*�Ⱥ���IC�������ͺ�UID(IC�����к�)*/                                                                                  
	uint8_t ucStatusReturn;      /*����״̬*/
	uint8_t rval = 0;
	uint8_t res = 0;						//���շ���ֵ���ݴ�ֵ�����ڷ��价��ʹ�ã�Ϊ��Э����ʾʱ������
	uint8_t debugReg = 0x00;		//������״̬���ӼĴ�����0��3��4��7λ�ֱ����Ա�ʾ֧��������������������֡ͬ����״̬
	uint8_t key_detect = 0;			//δ�м�����
	
	
	Charge_CommencePage_StaticLayout();//��ʾ����
	
	do{
		key_detect=KEY_Scan(1);
		if(key_detect==KEY2_PRES){
			rval = BACK; //ȡ�������£����ⷵ��
			return rval;//�������������0����Ϊ�����棬��������ʾ�ϸ�����
		}
		/*Ѱ��*/
		if (( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID )) != MI_OK){  
			/*��ʧ���ٴ�Ѱ��*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );//�����Ӧ
//			LED0_Blink(3);
		}
		if( ucStatusReturn == MI_OK){
			/*�ٴ�Ѱ������Ѱ����״̬*/
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK ){//�ҵ���������һ�ſ�
				AutoReturn_cntdownRefresh_sec(10);	//���в����ˣ���������������ʱ��10sec
				PcdSelect(ucArray_ID);
				PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//У������
				
				if((rval=ReadAmount_withCheck(&readValue))==MI_OK){//��Ҳ�����ɹ��ˣ����Կ�ʼ��һ�׶�
					protoAmount = readValue;								//�����ʼֵ
					User_Info_Disp(readValue,ucArray_ID); 	//��ʾ�����Ϣ
					
					if(assert_param_seqNbr(_Match_Info_UserID(&charge_Sync,ucArray_ID))){//һ�ſ�ֻ������һ̨���׮
						//��ǰ�����Ѿ�ռ��һ�����õĳ��׮���򷵻�
						printf("\r\n��ʾ���ÿ���ռ��\r\n");
						Popover_Show_Str(0XF0,0X18B,"�ÿ��Ѿ�������һ̨���׮���������ظ�������",0X32,0X190,4000);
						continue;	//�����˴�Ѱ��������ִ��Ѱ��
						
					}else{//��ǰ���ǿ����
						if(readValue<bill(chargeTime_Selected)){
							rval = IN_SUFF;
							Popover_Show_Str(0XF0,0X18B,"�������㣡",0X32,0X190,3000);
							return rval;				//�������㣬ֱ�ӷ�����һ��
						}
						
						//��ͨ��������⣬����ʾ�ɹ���ʾ
						successTone();//�ɹ���ʾ��	
						
						//д����ģ��
						writeValue = readValue-bill(chargeTime_Selected);//����д����					
						if((rval=ChangeAmount_withCheck(writeValue))!=0){	//д����������˵����������
							Popover_Show_Str(0XF0,0X18B,"֧��ʧ�ܣ������ԣ�",0X32,0X190,3000);
							printf("\r\nϵͳ�Ѽ�⵽δ�ɹ��Ĳ�����������룺%#2hx\r\n",(unsigned short)rval);
							continue;
						}else{
							printf("\r\n֧�������ɹ���\r\n");
							debugReg|=(1<<0);//0λ��λ�����֧���ɹ�
							User_Info_Disp(writeValue,ucArray_ID);//�������
						}
						
						//����Ӳ��ģ��
						if((rval = Charger_Commence(seqNbr_Selected))!=0){//���׮����ʧ��
							charge_Sync.stack[seqNbr_Selected].ucFlag|=(1<<4); //�ù���λ
							_Charge_Info_Synchronize(&charge_Sync);						 //ͬ������֡
							printf("\r\nӲ������ʧ�ܣ�\r\n");
							
							if((res=ChangeAmount_withCheck(protoAmount))!=0)printf("\r\n����Ī������Ҳʧ���ˡ�\r\n");
							else printf("\r\n����ɹ�\r\n");
							Popover_Show_Str(0XF0,0X18B,"�ó��׮δ�ܳɹ��������볢������׮λ",0X32,0X190,3000);
							
							if(res==0)User_Info_Disp(protoAmount,ucArray_ID);		//����ɹ�����������
							delay_ms_plus(1800);//���û��࿴һ����ָ��������İ����
							return rval;	//rval��0��������һ�� 					
						}else{//Ӳ�������ɹ�
							debugReg|=((1<<3)|(1<<4));//3��4λ��λ�����Ӳ�������ɹ�
							
							_Charge_Info_unitSet(&charge_Sync,seqNbr_Selected,chargeTime_Selected,ucArray_ID);//д������֡
							_Charge_Info_Synchronize(&charge_Sync);//ͬ������֡
							debugReg|= (1<<7);//�������֡ͬ���ɹ�
							
							printf("\r\n�ɹ��������׮\r\n");
//							Advanced_Show_Str_Mid(0,386,"�����ɹ�",72,480,0);//������ʾ�����ɹ�
							draw_enlarged_GuideStr(0x03);
							delay_ms_plus(1800);
						}
						printf("\r\ndebug:%#.2hx\r\n",(unsigned short)debugReg);//debugRegΪ 1001 1001(B) ��0x99ʱ����˳������
						
						if(debugReg&((1<<0)|(1<<3)|(1<<4)|(1<<7))){
							PcdHalt();//ȫ��������ɣ��Ƭ����
							rval = 0;//д�����ȷ��ÿһ��������ÿһ��λ��û�б�����Ҫ�з����Ĵ���ʽ�������
							return rval;//ִ�гɹ��ĳ��ڣ����������棡
						}
					}
				}else{//����ʧ�ܣ����ش���ԭ��
					Popover_Show_Str(0XF0,0X18B,"����ʧ�ܣ������ԣ�",0X32,0X190,3000);
					continue;	//�����˴�Ѱ��������ִ��
				}
			}//ȷ����
		}//�ٶ���
	}while((key_detect=KEY_Scan(0))==KEY2_PRES||cntdown);//ȡ�������»��߼�ʱ���� ִ���Ⱥ�˳�����ע�⣡�������ȣ����򲻻ᱻ̽��
	if(key_detect==KEY2_PRES)rval = BACK; //ȡ�������£����ⷵ��
	return rval;//�������������0����Ϊ�����棬��������ʾ�ϸ�����
}


//����������
uint8_t Charger_Terminate_Handler(void)
{
	uint32_t writeValue;
	uint32_t readValue;
	uint32_t protoAmount;
	uint32_t timeRemain;
  uint8_t ucArray_ID [ 4 ];    /*�Ⱥ���IC�������ͺ�UID(IC�����к�)*/                                                                                  
	uint8_t ucStatusReturn;      /*����״̬*/
	uint8_t seqNbr = 0xff;
	uint8_t rval = 0;
	uint8_t errCnt = 0;
	uint8_t debugReg = 0x00;		//������״̬���ӼĴ�����0��3��4��7λ�ֱ����Ա�ʾ���䡢�ص硢����������֡ͬ����״̬
	uint8_t key_detect = 0;			//δ�м�����
	
	Charge_TerminatePage_StaticLayout();//��ʾ��������
	AutoReturn_cntdownRefresh_sec(10);
	
	do{/*Ѱ��*/
		key_detect=KEY_Scan(1);
		if(key_detect==KEY0_PRES){
			rval = BACK; //ȡ�������£����ⷵ��
			return rval;//�������������0����Ϊ�����棬��������ʾ�ϸ�����
		}
		if (( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK){  
			/*��ʧ���ٴ�Ѱ��*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );
//			LED0_Blink(3);
		}
		
		if( ucStatusReturn == MI_OK){
			
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK ){//�ҵ���������һ�ſ�

				AutoReturn_cntdownRefresh_sec(10);	//���в����ˣ���������������ʱ��10sec
				PcdSelect(ucArray_ID);
				PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//У������ 
				
				if((rval=ReadAmount_withCheck(&readValue))==MI_OK){
					protoAmount = readValue;								//�����ʼֵ
					User_Info_Disp(readValue,ucArray_ID); 	//��ʾ�����Ϣ
					
					if(assert_param_seqNbr((seqNbr=_Match_Info_UserID(&charge_Sync,ucArray_ID)))){
						//��ǰ����ռ�õĳ��׮�����Բ���
						
						if((rval = Charger_Terminate(seqNbr))!=0){//���㣬���׮����ʧ��
							charge_Sync.stack[seqNbr].ucFlag|=(1<<4); //�ù���λ
							_Charge_Info_Synchronize(&charge_Sync);						 //ͬ������֡
							printf("\r\nӲ������ʧ�ܣ�������룺%hx\r\n",(unsigned short)rval);
							Popover_Show_Str(0XF0,0X18B,"����ʧ�ܣ����Ժ����ԡ�",0X32,0X190,3000);
							errCnt++;
							if(errCnt>10){//ʮ�β���δ��
								Popover_Show_Str(0XF0,0X18B,"�ó��׮����δ֪���ϣ�����ϵ������Ա",0X32,0X190,3000);
								delay_ms(100);
								return rval;	//rval��0��������һ��������ҳ��
							}else continue;
						}else{//Ӳ���Ľ����ɹ�
							debugReg|=((1<<3)|(1<<4));//3��4λ��λ�����Ӳ�������ɹ�
							
							timeRemain = charge_Sync.stack[seqNbr].timeStamp - RTC_GetCounter();//���ʣ����ʱ��
							writeValue = protoAmount + bill((timeRemain/60));//����÷���ֵ
//							Advanced_Show_Str_Mid(0,386,"�����ɹ�",72,480,0);//������ʾ�����ɹ�
							draw_enlarged_GuideStr(0x03);
							successTone();//�ɹ���ʾ��		
							
							if((rval=ChangeAmount_withCheck(writeValue))!=0){	//д����������˵����������
							printf("\r\nϵͳ�Ѽ�⵽δ�ɹ��Ĳ�����������룺%#2hx\r\n",(unsigned short)rval);
							}else{
								printf("\r\n��������ɹ���\r\n");
								debugReg|=(1<<0);//0λ��λ�����֧���ɹ�
								User_Info_Disp(writeValue,ucArray_ID);//�������
								delay_ms_plus(1800);//���û��࿴һ����ָ��������İ����
							}
							
							_Charge_Info_unitReset(&charge_Sync,seqNbr);//��λ����֡����Ҫ�ڶ�ֵ֮ǰ��λ������
							_Charge_Info_Synchronize(&charge_Sync);     //ͬ������֡
							debugReg|= (1<<7);//7λ��λ���������֡ͬ���ɹ�
							printf("\r\ndebug:%#.2hx\r\n",(unsigned short)debugReg);//debugRegΪ 1001 1001(B) ��0x99ʱ����˳������
						
							if(debugReg&((1<<0)|(1<<3)|(1<<4))){
								PcdHalt();//ȫ��������ɣ��Ƭ����
								rval = 0;//д�����ȷ��ÿһ��������ÿһ��λ��û�б�����Ҫ�з����Ĵ���ʽ�������
								return rval;//ִ�гɹ��ĳ��ڣ����������棡
							}
						}
					}else{
						//��ǰ��û�б�ռ�õĳ��׮
						printf("\r\n��ʾ���ÿ�û��ע���κγ��׮\r\n");
						Popover_Show_Str(0XF0,0X18B,"�ÿ�δ�����κγ��׮��",0X32,0X190,2000);
						continue;	//�����˴�Ѱ��������Ѱ��
					}
				}else{//����ʧ�ܣ����ش���ԭ��
					Popover_Show_Str(0XF0,0X18B,"����ʧ�ܣ������ԣ�",0X32,0X190,3000);
					continue;	//�����˴�Ѱ��������ִ��
				}
			}
		}
	}while((key_detect = KEY_Scan(0))==KEY0_PRES||cntdown);//ȡ�������»��߼�ʱ����
	
	if(key_detect==KEY2_PRES)rval = BACK; //ȡ�������£����ⷵ��
	
	return rval;//�������������0����Ϊ�����棬��������ʾ�ϸ����棬һ�׽��治�ж��䷵��ֵ
}


uint8_t  CardInfo_Reg = 0;		//���庯��ʹ�õ�ȫ�ֱ���
//PICC������
void Card_Surveillance(uint8_t runAttribute)
{
	uint32_t readValue;
  uint8_t ucArray_ID [ 4 ];    /*�Ⱥ���IC�������ͺ�UID(IC�����к�)*/                                                                                  
	uint8_t ucStatusReturn;      /*����״̬*/
	do{/*Ѱ��*/
		
		if (( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK){  
			/*��ʧ���ٴ�Ѱ��*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );
//			LED0_Blink(3);//ռ��ʱ��̫������
//			printf("\r\n%hx\r\n",(unsigned short)ucStatusReturn);
		}		
		if( ucStatusReturn == MI_OK){
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK ){//�ҵ���������һ�ſ���������ڲŻ�֪������
				PcdSelect(ucArray_ID);
				AutoReturn_cntdownRefresh_sec(8);	//���в����ˣ���������������ʱ��20sec
				PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//У������
				if(ReadAmount_withCheck(&readValue)==MI_OK){
					User_Info_Disp(readValue,ucArray_ID); 	//��ʾ�����Ϣ
					CardInfo_Reg = 1;		//ֻҪ�ж���������ʾ
					successTone();//�ɹ���ʾ��	
					PcdHalt();//ȫ��������ɣ��Ƭ����
				}
			}
		}else if(CardInfo_Reg==1&&cntdown==0){			//����û���µĲ���������ʾ�����
			LCD_Fill(0,0,lcddev.width,300,BACK_COLOR);//��䱳��ɫ
			title_IdleDisp(DISCRETE);
			CardInfo_Reg = 0;
		}
	}while(runAttribute); //runAttribute���λ�����ѭ��ִ��
}


//������ʾ���к�̨����
//���������ֵ
void Card_Balance_Feed(uint16_t balance_feed)
{
	uint8_t cntTemp;
	uint32_t readValue;
	uint32_t writeValue;
  uint8_t ucArray_ID [ 4 ];    /*�Ⱥ���IC�������ͺ�UID(IC�����к�)*/                                                                                  
	uint8_t ucStatusReturn;      /*����״̬*/
	
	cntTemp = cntdown;
	//����ʾ��
	LED0 = 0;
	LED1 = 1;
	gui_back_Preserve(0,0,lcddev.width,300);//��������
	printf("\r\n��������ģʽ������10sec����ʱ��\r\n");
	errorTone();//��ʾ��
	AutoReturn_cntdownRefresh_sec(10);	//ʱ�䴰��Ϊ10��
	do{/*Ѱ��*/
		if(cntTemp!=cntdown){//��ˮ��
			cntTemp = cntdown;
			if(cntTemp<4){
				LED0 = LED1;
				printf("\r\n%dsec�����\r\n",cntTemp);
			}
			LED0 = !LED0;
			LED1 = !LED1;
		}
		if (( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK){  
			/*��ʧ���ٴ�Ѱ��*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );
//			printf("\r\n%hx\r\n",(unsigned short)ucStatusReturn);
		}		
		if( ucStatusReturn == MI_OK){
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK ){//�ҵ���������һ�ſ���������ڲŻ�֪������
				PcdSelect(ucArray_ID);
				
				PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//У������
				if(ReadAmount_withCheck(&readValue)==MI_OK){
					User_Info_Disp(readValue,ucArray_ID); 	//��ʾ�����Ϣ
					
					writeValue = balance_feed*100;//��λ���Ϊ200Ԫ				
						if(ChangeAmount_withCheck(writeValue)!=0){	//д����������˵����������
							Popover_Show_Str(0XF0,0X18B,"֧��ʧ�ܣ������ԣ�",0X32,0X190,3000);
						}else{
							printf("\r\n֧�������ɹ���\r\n");
							User_Info_Disp(writeValue,ucArray_ID);//�������
						}
					
					CardInfo_Reg = 1;		//ֻҪ�ж���������ʾ
					successTone();//�ɹ���ʾ��	
					PcdHalt();//ȫ��������ɣ��Ƭ����
				}
			}
		}
		if(CardInfo_Reg==1&&cntdown==0){			//����û���µĲ���������ʾ�����
//			LCD_Fill(0,0,lcddev.width,300,BACK_COLOR);//��䱳��ɫ
			CardInfo_Reg = 0;
		}
	}while(cntdown); //��ʱ������ķ�Χ��ִ��
	gui_back_Recover();//�ָ�����
	errorTone();
	LED0 = 1;//���
	LED1 = 1;
	return;
}

