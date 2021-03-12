#include "sys.h"
#include "delay.h"
#include "common.h" //倒计时
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
 ALIENTEK战舰STM32开发板
 
* 充电桩主控系统 * 
 
 作者：高恩铭 @GaoEnMing

* @date    2021-xx-xx
* @brief   防拔充电桩工程设计
************************************************/

//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEK战舰STM32开发板
//用PCD与PICC交互的主要函数
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

extern vu8 cntdown;//引用则不能赋值


/**
  * @brief  寻卡与读示函数（调试用）
  * @param  运行属性 runAttribute 分为离散DISCRETE和连续CONTINUOUS
  * @retval 无
  */
uint8_t KeyValue[]={0xFF ,0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // 卡A密钥，A卡密钥
void CardDetect (uint8_t runAttribute)
{
//	uint32_t writeValue = 100;
	uint32_t readValue;
	char cStr [ 30 ];
  uint8_t ucArray_ID [ 4 ];    /*先后存放IC卡的类型和UID(IC卡序列号)*/ //uc是unsigned char                                                                                        
	uint8_t ucStatusReturn;      /*返回状态*/
	do{    
			/*寻卡*/
			if ( ( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK )  
			{   /*若失败再次寻卡*/
				ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );
				LED0_Blink(3);
				delay_ms(720);
			}			
			if ( ucStatusReturn == MI_OK  )
			{
				/*防冲撞（当有多张卡进入读写器操作范围时，防冲突机制会从其中选择一张进行操作）*/
				if ( PcdAnticoll ( ucArray_ID ) == MI_OK ) //卡号是这个时侯被赋予的！                                                                  
				{
					PcdSelect(ucArray_ID);			//与返还的卡号对应的这个卡建立通信
					LED0_Blink(1);				
					PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//校验密码 
	//        WriteAmount(0x11,writeValue); //写入金额
	//				delay_ms(20);
	//				static uint8_t	attachFlag = 0; //1表示附卡，0表示未附卡
					if((ReadAmount(0x11,&readValue) == MI_OK))	//读取金额
					{		
	//					writeValue +=100;
						successTone();//大端方式表示数据？低字节表示最高位？
						sprintf ( cStr, "The Card ID is: %02X%02X%02X%02X",ucArray_ID [0], ucArray_ID [1], ucArray_ID [2],ucArray_ID [3] );
						LCD_ShowString(30,40,320,24,24,cStr);

						sprintf ( cStr, "The residual amount: %d", readValue);				 										 	         
						LCD_ShowString(30,70,320,24,24,cStr);
	//					attachFlag = 1;
						PcdHalt();//卡片进入休眠状态，将无法被读取		
						delay_ms(20);
					}
				}			
			}
	//		else if(( PcdRequest( PICC_REQIDL, ucArray_ID ) == !MI_OK)&&( attachFlag == 1 ))
	//		{
	//			LED1_Blink(3);
	//			BEEP_Impulse(1);	//拿出卡
	//			attachFlag = 0;
	//		}	    
		}while( runAttribute );
}

//有校验地写入数据
//总要写在密码校验之后
//rval为0则表示没有问题，一切顺利
//rval若为其它值，则说明没有写入成功
//若余额没有写对，但是发生了其它的变动
//这个情况需要另外来考虑
uint8_t ChangeAmount_withCheck(uint32_t writeValue)
{
	uint8_t cnt;
	uint8_t rval=UNKNOWN;
	uint8_t res;
	uint32_t readValue;
	for(cnt=0;cnt<10;cnt++){//执行10次，并且返回最终的处理情况
		WriteAmount(0x11,writeValue); //写入金额
		delay_ms(20);//赋值表达式的返回值是左侧对象
		if(((res = ReadAmount(0x11,&readValue)) == MI_OK)&&(readValue==writeValue)){
			rval = 0;//一切顺利的返回值
			printf("\r\n余额写入成功！\r\n");
			break;
		}else{
			rval = res;
			if((res!=MI_NOTAGERR)&&(res!=MI_ERR))rval = UNKNOWN;
		}
	}
	if(rval)printf("\r\n余额写入失败！错误代码：%#2hx\r\n",(unsigned short)rval);
	return rval;
}

//有校验地读取数据
//总要写在密码校验之后
//rval为忠实地反映ReadAmount()的返回值
//这个函数仅作为多次读取的封装
uint8_t ReadAmount_withCheck(uint32_t *readValue)
{
	uint8_t cnt;
	uint8_t rval=UNKNOWN;
	for(cnt=0;cnt<10;cnt++){
		if((rval = ReadAmount(0x11,readValue))==MI_OK)return rval;
		delay_ms(20);			//如果成功读到就直接返回
	}
	printf("\r\n读取余额失败！错误代码：%#2hx\r\n",(unsigned short)rval);
	return rval;
}


//启动服务函数
uint8_t Charger_Commence_Handler(void)
{
	uint32_t writeValue;
	uint32_t readValue;
	uint32_t protoAmount;
  uint8_t ucArray_ID [ 4 ];    /*先后存放IC卡的类型和UID(IC卡序列号)*/                                                                                  
	uint8_t ucStatusReturn;      /*返回状态*/
	uint8_t rval = 0;
	uint8_t res = 0;						//接收返回值的暂存值，仅在返充环节使用，为了协调显示时间而设计
	uint8_t debugReg = 0x00;		//调试用状态监视寄存器：0、3、4、7位分别用以表示支付、启动、落锁、数据帧同步的状态
	uint8_t key_detect = 0;			//未有键按下
	
	
	Charge_CommencePage_StaticLayout();//显示界面
	
	do{
		key_detect=KEY_Scan(1);
		if(key_detect==KEY2_PRES){
			rval = BACK; //取消键按下，纯粹返回
			return rval;//这个返回渠道，0则是为主界面，非零则显示上个界面
		}
		/*寻卡*/
		if (( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID )) != MI_OK){  
			/*若失败再次寻卡*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );//获得响应
//			LED0_Blink(3);
		}
		if( ucStatusReturn == MI_OK){
			/*再次寻卡若是寻到的状态*/
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK ){//找到并锁定了一张卡
				AutoReturn_cntdownRefresh_sec(10);	//既有操作了，就续命程序运行时长10sec
				PcdSelect(ucArray_ID);
				PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//校验密码
				
				if((rval=ReadAmount_withCheck(&readValue))==MI_OK){//读也读卡成功了，索性开始下一阶段
					protoAmount = readValue;								//保存初始值
					User_Info_Disp(readValue,ucArray_ID); 	//显示余额信息
					
					if(assert_param_seqNbr(_Match_Info_UserID(&charge_Sync,ucArray_ID))){//一张卡只能启动一台充电桩
						//当前卡若已经占用一个在用的充电桩，则返回
						printf("\r\n提示：该卡被占用\r\n");
						Popover_Show_Str(0XF0,0X18B,"该卡已经启动过一台充电桩，不能再重复启动！",0X32,0X190,4000);
						continue;	//跳过此次寻卡，重新执行寻卡
						
					}else{//当前卡是空余的
						if(readValue<bill(chargeTime_Selected)){
							rval = IN_SUFF;
							Popover_Show_Str(0XF0,0X18B,"卡内余额不足！",0X32,0X190,3000);
							return rval;				//卡内余额不足，直接返回上一级
						}
						
						//若通过了余额检测，则提示成功显示
						successTone();//成功提示音	
						
						//写入金额模块
						writeValue = readValue-bill(chargeTime_Selected);//计算写入金额					
						if((rval=ChangeAmount_withCheck(writeValue))!=0){	//写入余额，非零则说明遇到错误
							Popover_Show_Str(0XF0,0X18B,"支付失败，请重试！",0X32,0X190,3000);
							printf("\r\n系统已检测到未成功的操作！错误代码：%#2hx\r\n",(unsigned short)rval);
							continue;
						}else{
							printf("\r\n支付操作成功！\r\n");
							debugReg|=(1<<0);//0位置位，标记支付成功
							User_Info_Disp(writeValue,ucArray_ID);//更新余额
						}
						
						//启动硬件模块
						if((rval = Charger_Commence(seqNbr_Selected))!=0){//充电桩启动失败
							charge_Sync.stack[seqNbr_Selected].ucFlag|=(1<<4); //置故障位
							_Charge_Info_Synchronize(&charge_Sync);						 //同步数据帧
							printf("\r\n硬件启动失败！\r\n");
							
							if((res=ChangeAmount_withCheck(protoAmount))!=0)printf("\r\n返充莫名其妙也失败了…\r\n");
							else printf("\r\n返充成功\r\n");
							Popover_Show_Str(0XF0,0X18B,"该充电桩未能成功启动！请尝试其它桩位",0X32,0X190,3000);
							
							if(res==0)User_Info_Disp(protoAmount,ucArray_ID);		//返充成功，则更新余额
							delay_ms_plus(1800);//让用户多看一会儿恢复的余额，好心安理得
							return rval;	//rval非0，返回上一级 					
						}else{//硬件启动成功
							debugReg|=((1<<3)|(1<<4));//3、4位置位，标记硬件启动成功
							
							_Charge_Info_unitSet(&charge_Sync,seqNbr_Selected,chargeTime_Selected,ucArray_ID);//写入数据帧
							_Charge_Info_Synchronize(&charge_Sync);//同步数据帧
							debugReg|= (1<<7);//标记数据帧同步成功
							
							printf("\r\n成功启动充电桩\r\n");
//							Advanced_Show_Str_Mid(0,386,"操作成功",72,480,0);//覆盖显示操作成功
							draw_enlarged_GuideStr(0x03);
							delay_ms_plus(1800);
						}
						printf("\r\ndebug:%#.2hx\r\n",(unsigned short)debugReg);//debugReg为 1001 1001(B) 即0x99时表明顺利进行
						
						if(debugReg&((1<<0)|(1<<3)|(1<<4)|(1<<7))){
							PcdHalt();//全部操作完成，令卡片休眠
							rval = 0;//写这个是确保每一步操作的每一个位若没有被置则都要有返还的处理方式，检查用
							return rval;//执行成功的出口，返回主界面！
						}
					}
				}else{//读卡失败，返回错误原因
					Popover_Show_Str(0XF0,0X18B,"读卡失败，请重试！",0X32,0X190,3000);
					continue;	//跳过此次寻卡，重新执行
				}
			}//确定卡
		}//再读卡
	}while((key_detect=KEY_Scan(0))==KEY2_PRES||cntdown);//取消键按下或者计时到点 执行先后顺序务必注意！键者在先，否则不会被探测
	if(key_detect==KEY2_PRES)rval = BACK; //取消键按下，纯粹返回
	return rval;//这个返回渠道，0则是为主界面，非零则显示上个界面
}


//结束服务函数
uint8_t Charger_Terminate_Handler(void)
{
	uint32_t writeValue;
	uint32_t readValue;
	uint32_t protoAmount;
	uint32_t timeRemain;
  uint8_t ucArray_ID [ 4 ];    /*先后存放IC卡的类型和UID(IC卡序列号)*/                                                                                  
	uint8_t ucStatusReturn;      /*返回状态*/
	uint8_t seqNbr = 0xff;
	uint8_t rval = 0;
	uint8_t errCnt = 0;
	uint8_t debugReg = 0x00;		//调试用状态监视寄存器：0、3、4、7位分别用以表示返充、关电、解锁、数据帧同步的状态
	uint8_t key_detect = 0;			//未有键按下
	
	Charge_TerminatePage_StaticLayout();//显示结束界面
	AutoReturn_cntdownRefresh_sec(10);
	
	do{/*寻卡*/
		key_detect=KEY_Scan(1);
		if(key_detect==KEY0_PRES){
			rval = BACK; //取消键按下，纯粹返回
			return rval;//这个返回渠道，0则是为主界面，非零则显示上个界面
		}
		if (( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK){  
			/*若失败再次寻卡*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );
//			LED0_Blink(3);
		}
		
		if( ucStatusReturn == MI_OK){
			
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK ){//找到并锁定了一张卡

				AutoReturn_cntdownRefresh_sec(10);	//既有操作了，就续命程序运行时长10sec
				PcdSelect(ucArray_ID);
				PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//校验密码 
				
				if((rval=ReadAmount_withCheck(&readValue))==MI_OK){
					protoAmount = readValue;								//保存初始值
					User_Info_Disp(readValue,ucArray_ID); 	//显示余额信息
					
					if(assert_param_seqNbr((seqNbr=_Match_Info_UserID(&charge_Sync,ucArray_ID)))){
						//当前卡有占用的充电桩，可以操作
						
						if((rval = Charger_Terminate(seqNbr))!=0){//非零，充电桩结束失败
							charge_Sync.stack[seqNbr].ucFlag|=(1<<4); //置故障位
							_Charge_Info_Synchronize(&charge_Sync);						 //同步数据帧
							printf("\r\n硬件结束失败！错误代码：%hx\r\n",(unsigned short)rval);
							Popover_Show_Str(0XF0,0X18B,"开锁失败，请稍后再试…",0X32,0X190,3000);
							errCnt++;
							if(errCnt>10){//十次操作未果
								Popover_Show_Str(0XF0,0X18B,"该充电桩出现未知故障，请联系工作人员",0X32,0X190,3000);
								delay_ms(100);
								return rval;	//rval非0，返回上一级，即主页面
							}else continue;
						}else{//硬件的结束成功
							debugReg|=((1<<3)|(1<<4));//3、4位置位，标记硬件启动成功
							
							timeRemain = charge_Sync.stack[seqNbr].timeStamp - RTC_GetCounter();//获得剩余秒时长
							writeValue = protoAmount + bill((timeRemain/60));//计算得返充值
//							Advanced_Show_Str_Mid(0,386,"操作成功",72,480,0);//覆盖显示操作成功
							draw_enlarged_GuideStr(0x03);
							successTone();//成功提示音		
							
							if((rval=ChangeAmount_withCheck(writeValue))!=0){	//写入余额，非零则说明遇到错误
							printf("\r\n系统已检测到未成功的操作！错误代码：%#2hx\r\n",(unsigned short)rval);
							}else{
								printf("\r\n返充操作成功！\r\n");
								debugReg|=(1<<0);//0位置位，标记支付成功
								User_Info_Disp(writeValue,ucArray_ID);//更新余额
								delay_ms_plus(1800);//让用户多看一会儿恢复的余额，好心安理得
							}
							
							_Charge_Info_unitReset(&charge_Sync,seqNbr);//复位数据帧，不要在读值之前复位它！！
							_Charge_Info_Synchronize(&charge_Sync);     //同步数据帧
							debugReg|= (1<<7);//7位置位，标记数据帧同步成功
							printf("\r\ndebug:%#.2hx\r\n",(unsigned short)debugReg);//debugReg为 1001 1001(B) 即0x99时表明顺利进行
						
							if(debugReg&((1<<0)|(1<<3)|(1<<4))){
								PcdHalt();//全部操作完成，令卡片休眠
								rval = 0;//写这个是确保每一步操作的每一个位若没有被置则都要有返还的处理方式，检查用
								return rval;//执行成功的出口，返回主界面！
							}
						}
					}else{
						//当前卡没有被占用的充电桩
						printf("\r\n提示：该卡没有注册任何充电桩\r\n");
						Popover_Show_Str(0XF0,0X18B,"该卡未启动任何充电桩！",0X32,0X190,2000);
						continue;	//跳过此次寻卡，重新寻卡
					}
				}else{//读卡失败，返回错误原因
					Popover_Show_Str(0XF0,0X18B,"读卡失败，请重试！",0X32,0X190,3000);
					continue;	//跳过此次寻卡，重新执行
				}
			}
		}
	}while((key_detect = KEY_Scan(0))==KEY0_PRES||cntdown);//取消键按下或者计时到点
	
	if(key_detect==KEY2_PRES)rval = BACK; //取消键按下，纯粹返回
	
	return rval;//这个返回渠道，0则是为主界面，非零则显示上个界面，一阶界面不判断其返回值
}


uint8_t  CardInfo_Reg = 0;		//定义函数使用的全局变量
//PICC卡监视
void Card_Surveillance(uint8_t runAttribute)
{
	uint32_t readValue;
  uint8_t ucArray_ID [ 4 ];    /*先后存放IC卡的类型和UID(IC卡序列号)*/                                                                                  
	uint8_t ucStatusReturn;      /*返回状态*/
	do{/*寻卡*/
		
		if (( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK){  
			/*若失败再次寻卡*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );
//			LED0_Blink(3);//占用时间太长。。
//			printf("\r\n%hx\r\n",(unsigned short)ucStatusReturn);
		}		
		if( ucStatusReturn == MI_OK){
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK ){//找到并锁定了一张卡，这个环节才会知晓卡号
				PcdSelect(ucArray_ID);
				AutoReturn_cntdownRefresh_sec(8);	//既有操作了，就续命程序运行时长20sec
				PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//校验密码
				if(ReadAmount_withCheck(&readValue)==MI_OK){
					User_Info_Disp(readValue,ucArray_ID); 	//显示余额信息
					CardInfo_Reg = 1;		//只要有东西正在显示
					successTone();//成功提示音	
					PcdHalt();//全部操作完成，令卡片休眠
				}
			}
		}else if(CardInfo_Reg==1&&cntdown==0){			//过久没有新的操作，则将显示区清空
			LCD_Fill(0,0,lcddev.width,300,BACK_COLOR);//填充背景色
			title_IdleDisp(DISCRETE);
			CardInfo_Reg = 0;
		}
	}while(runAttribute); //runAttribute单次或者死循环执行
}


//按照提示进行后台重置
//卡调试与充值
void Card_Balance_Feed(uint16_t balance_feed)
{
	uint8_t cntTemp;
	uint32_t readValue;
	uint32_t writeValue;
  uint8_t ucArray_ID [ 4 ];    /*先后存放IC卡的类型和UID(IC卡序列号)*/                                                                                  
	uint8_t ucStatusReturn;      /*返回状态*/
	
	cntTemp = cntdown;
	//置提示灯
	LED0 = 0;
	LED1 = 1;
	gui_back_Preserve(0,0,lcddev.width,300);//背景保护
	printf("\r\n启动调试模式，您有10sec操作时间\r\n");
	errorTone();//提示音
	AutoReturn_cntdownRefresh_sec(10);	//时间窗口为10秒
	do{/*寻卡*/
		if(cntTemp!=cntdown){//流水灯
			cntTemp = cntdown;
			if(cntTemp<4){
				LED0 = LED1;
				printf("\r\n%dsec后结束\r\n",cntTemp);
			}
			LED0 = !LED0;
			LED1 = !LED1;
		}
		if (( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK){  
			/*若失败再次寻卡*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );
//			printf("\r\n%hx\r\n",(unsigned short)ucStatusReturn);
		}		
		if( ucStatusReturn == MI_OK){
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK ){//找到并锁定了一张卡，这个环节才会知晓卡号
				PcdSelect(ucArray_ID);
				
				PcdAuthState( PICC_AUTHENT1A, 0x11, KeyValue, ucArray_ID );//校验密码
				if(ReadAmount_withCheck(&readValue)==MI_OK){
					User_Info_Disp(readValue,ucArray_ID); 	//显示余额信息
					
					writeValue = balance_feed*100;//复位余额为200元				
						if(ChangeAmount_withCheck(writeValue)!=0){	//写入余额，非零则说明遇到错误
							Popover_Show_Str(0XF0,0X18B,"支付失败，请重试！",0X32,0X190,3000);
						}else{
							printf("\r\n支付操作成功！\r\n");
							User_Info_Disp(writeValue,ucArray_ID);//更新余额
						}
					
					CardInfo_Reg = 1;		//只要有东西正在显示
					successTone();//成功提示音	
					PcdHalt();//全部操作完成，令卡片休眠
				}
			}
		}
		if(CardInfo_Reg==1&&cntdown==0){			//过久没有新的操作，则将显示区清空
//			LCD_Fill(0,0,lcddev.width,300,BACK_COLOR);//填充背景色
			CardInfo_Reg = 0;
		}
	}while(cntdown); //在时间允许的范围内执行
	gui_back_Recover();//恢复背景
	errorTone();
	LED0 = 1;//灭灯
	LED1 = 1;
	return;
}

