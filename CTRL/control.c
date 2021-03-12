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
 ALIENTEK战舰STM32开发板
 
* 充电桩主控系统 * 
 
 作者：高恩铭 @GaoEnMing

* @date    2021-xx-xx
* @brief   防拔充电桩工程设计
************************************************/
/////////////////////////////////////////////////////////////////////
//CRUCIAL CONTROL 主控系统
//@Auther: Gao Enming
/////////////////////////////////////////////////////////////////////

//在全局中开辟一隅空间，为穿梭于掉电SRAM与静态EEPROM间的系统功能打造专属的结构体变量
_Charge_Info charge_Sync;	


uint8_t 	seqNbr_Selected;				//已经确定的合法充电桩号
uint16_t 	chargeTime_Selected;		//已经确定的合法充电时长

uint8_t		seqNbr_thbwhlSwitch;		//当前指向的充电桩号
uint16_t	chargeTime_designated;	//当前选择的充电时长

//_Charge_Info结构体快速赋值函数
//输入参数：pchargeStruct 指向_Charge_Info的指针
//mode为赋值的模式：
//模式mode：1表示按照测试状态进行赋值
//模式mode：0表示按照标准状态赋初值
//定义两个宏 ：
//TEST 1 表示模式1——测试状态；
//NORM 0 表示模式0——常规状态
void _Charge_Info_Init(_Charge_Info* pchargeStruct, uint8_t mode)
{
	uint8_t cnt;
	uint8_t prototype = 1;   //原型机阶段
	uint8_t nbrOfOccupied = 0;//占用的充电桩数目
	uint16_t ucflagLB = 0X0000;//初始化低字节
	memset(pchargeStruct,0,sizeof(_Charge_Info));//对结构体进行初始化，先完全赋以0
	pchargeStruct->token = TOKEN;
	pchargeStruct->avail = nbrOfCharger;
	
	//调试阶段的置位，原型机阶段
	if(prototype)ucflagLB|=(1<<7);//将所有的测试位全部置位
	
	/*在此处扩展充电桩唯一识别号的初始化功能*/
	for(cnt=0;cnt<nbrOfCharger;cnt++){//高字节总是写入充电桩的序号
			pchargeStruct->stack[cnt].ucFlag = ucflagLB|(cnt<<8);//序号与测试置位；左移不会溢出
	}
	
	
	if(mode){//当mode=1，为测试演示功能时执行；mode=0即正常实用状态则无须
		pchargeStruct->stack[1].ucFlag|=(1<<0);		//第x位置位的标准写法
		pchargeStruct->stack[1].ucArray_ID = 0X99466FE5;//#1号，充电桩的唯一识别卡号 占位用，实际上不对应任何一种卡号
		pchargeStruct->stack[1].timeStamp = 0XFFFFFFFF;//设置其时间戳为无限
		//演示用
		pchargeStruct->stack[3].ucFlag|=(1<<0);
		pchargeStruct->stack[3].ucArray_ID = 0X577C63D7;//#3号，充电桩的唯一识别卡号
		pchargeStruct->stack[3].timeStamp = 0XFFFFFFFF;//设置其时间戳为无限
	}//可以以此为基础编写数据是否为非法的衍生函数，校验函数，待…
	
	for(cnt=0;cnt<nbrOfCharger;cnt++){
		if(pchargeStruct->stack[cnt].ucFlag&(1<<0))nbrOfOccupied++;//如果处于工作状态，则工作中的充电桩数++
		else continue;
	}
	pchargeStruct->avail-=nbrOfOccupied;//更新结构体中avail的值
}

//_Charge_Info_Load()
//上电开机后向内存中的结构体加载数据
//输入参数为结构体指针
//返回值rval为0，表示找到了EEPROM里的数据，并且成功加载
//返回值rval为1，表示数据帧曾发生丢失，但已经修复，并且顺利加载
//返回值rval为2，表示数据重置后仍然找不到，需要手动排除故障
uint8_t _Charge_Info_Load(_Charge_Info* pchargeStruct)
{
	uint8_t cnt=0;
	uint8_t rval=0;
	uint16_t tokenBuff=0;
	_Charge_Info chargeFrameReset;//第一次上电后的数据帧重建
	while(cnt<10){
		cnt++;
		STMFLASH_Read(SYNC_SHUTTLE_BASE,(uint16_t*)&tokenBuff,1);//读取数据帧的头部标记，共10次
		if(tokenBuff==TOKEN)break;
		delay_ms(20);
	}
	if(tokenBuff==TOKEN){
		rval=0;
		printf("\r\n充电数据帧保存完好！\t\r\n");
	}else{//数据丢失，或者初次上电的同步关系，尝试恢复数据
		rval=1;
		printf("\r\n未找到数据帧，将重新写入…\t\r\n");
		_Charge_Info_Init(&chargeFrameReset,TEST);//写入测试数据于缓存中
		STMFLASH_Write(SYNC_SHUTTLE_BASE,(uint16_t*)&chargeFrameReset,sizeof(_Charge_Info)/2);//写入测试数据
		delay_ms(10);//略等间隙
		STMFLASH_Read(SYNC_SHUTTLE_BASE,(uint16_t*)&tokenBuff,1);//再次读取token
		if(tokenBuff!=TOKEN){
		printf("\r\n测试数据写入失败！\t\r\n");
		return 0X02;
		}else printf("\r\n测试数据写入成功！\t\r\n");
	}
	STMFLASH_Read(SYNC_SHUTTLE_BASE,(uint16_t*)pchargeStruct,sizeof(_Charge_Info)/2);//读出当处的值
	printf("\r\n充电数据帧加载成功！\t\r\n");
	return rval;
}//此函数可以进一步完善

//Charge_Status_Check()
//检测位于合法时间戳区间的超时成员
//初级阶段直接用数组索引
//2021-01-01 00:00:00 起始时间戳为0x5FEDF580
//url = https://tool.lu/timestamp/
uint8_t Charge_Status_Check(_Charge_Info* pchargeStruct)
{
	uint32_t timeStampNow = RTC_GetCounter();  	//获得当前时刻秒时间戳
	uint32_t timeStampTemp = 0;
	uint8_t 	cnt;
	uint8_t 	seqNbr=0xFF;		//充电桩序号,0xFF表示并未找到
	for(cnt=0;cnt<nbrOfCharger;cnt++){
		if(pchargeStruct->stack[cnt].ucFlag&(1<<0)){
			timeStampTemp = pchargeStruct->stack[cnt].timeStamp;
			if(timeStampTemp<timeStampNow && timeStampTemp>0x5FEDF580)//以2021-01-01 00:00:00为产品起始时间戳，此前的时间不合法
				seqNbr = cnt;
				break;
		}
	}
	return seqNbr;
}

//输入数组序号，将结构体相应成员复位
void _Charge_Info_unitReset(_Charge_Info* pchargeStruct,uint8_t seqNbr)
{
	uint8_t cnt;
	uint8_t nbrOfOccupied = 0;
	if(assert_param_seqNbr(seqNbr)){ //验证之是否合法，没有的时侯就无需之
		pchargeStruct->stack[seqNbr].ucFlag &= 0XFFFE;//工作状态位清零
		pchargeStruct->stack[seqNbr].ucArray_ID &= 0X00000000;//32位ucArray_ID清零
		pchargeStruct->stack[seqNbr].timeStamp &= 0X00000000;//时间戳清零
		//更新avail的语句模块
		for(cnt=0;cnt<nbrOfCharger;cnt++){
			if(pchargeStruct->stack[cnt].ucFlag&(1<<0))nbrOfOccupied++;//如果处于工作状态，则工作中的充电桩数++
			else continue;
		}
		pchargeStruct->avail = nbrOfCharger - nbrOfOccupied;//更新结构体中avail可用充电桩数量的值
	}
	return ;
}

//_Charge_Info_unitSet()
//写入正确的数据帧
//根据时长计算截至的时间戳
//按照卡上数据协议写入卡号
void _Charge_Info_unitSet(_Charge_Info* pchargeStruct,uint8_t seqNbr,uint16_t chargeTime_Selected,uint8_t* ucArray_ID)
{
	uint8_t cnt;
	uint8_t nbrOfOccupied = 0;
	uint32_t dueTimeStamp = RTC_GetCounter();//获取当前时间戳
	if(assert_param_seqNbr(seqNbr)){ //验证之是否合法，没有的时侯就无需之
		dueTimeStamp+=chargeTime_Selected*60; //获得截至时间戳
		pchargeStruct->stack[seqNbr].ucFlag |= (1<<0);//工作状态位置位
		//写入32位ucArray_ID，低字节写入低字节，高字节写入高字节，与原先的存储方式一样，区别在于读取时的顺序
		pchargeStruct->stack[seqNbr].ucArray_ID = ((ucArray_ID[0]<<8*0)|(ucArray_ID[1]<<8*1)|(ucArray_ID[2]<<8*2)|(ucArray_ID[3]<<8*3));
		pchargeStruct->stack[seqNbr].timeStamp = dueTimeStamp;//写入时间戳
		//更新avail的语句模块
		for(cnt=0;cnt<nbrOfCharger;cnt++){
			if(pchargeStruct->stack[cnt].ucFlag&(1<<0))nbrOfOccupied++;//如果处于工作状态，则工作中的充电桩数++
			else continue;
		}//在for外
		pchargeStruct->avail = nbrOfCharger - nbrOfOccupied;//更新结构体中avail可用充电桩数量的值
	}
	return ;
}

//Charger_MonitorHandler()
//充电桩状态监视服务函数
//轮询各充电桩的状态
//将已到时间的充电桩结束运行
//无输入参数，配合秒中断使用
//见RTC_IRQHandler()
void Charger_MonitorHandler(void)				
{
	uint8_t seqNbr = Charge_Status_Check(&charge_Sync); //检查刚结束的充电桩
	if(assert_param_seqNbr(seqNbr)){
		Charger_Terminate(seqNbr);	//结束充电进程（此处逻辑可以优化，辅以返回值，当务之急先实现基础功能）
		_Charge_Info_unitReset(&charge_Sync,seqNbr);			//修改数据帧
		_Charge_Info_Synchronize(&charge_Sync);						//同步数据帧
		printf("\r\n#%2d号充电完毕\t\r\n",seqNbr);
	}
	return;
}

//向FLASH模拟的EEPROM空间中同步数据帧
//为什么一旦发生同步，数据帧就会失效？？
//这里面究竟发生了什么？
void _Charge_Info_Synchronize(_Charge_Info* pchargeStruct)
{
	STMFLASH_Write(SYNC_SHUTTLE_BASE,(uint16_t*)pchargeStruct,sizeof(_Charge_Info)/2);
	return;
}
/**
	错误反思：这是错误的代码，传入一个指针，竟然在同步的时候又对其进行了一次取址，这样得到的数据毫无意义
	怪不得每次同步数据都会出错，原因就在这里！！
	STMFLASH_Write(SYNC_SHUTTLE_BASE,(uint16_t*)&pchargeStruct,sizeof(_Charge_Info)/2);
	去掉取址就能得到正确的代码！
**/


//实物层对接
void Charger_ShutDown(uint8_t seqNbr)		//关电
{
	delay_ms(20);
	return;
}

void Charger_PowerOn(uint8_t seqNbr)		//上电
{
	delay_ms(20);
	return;
}

void Charger_UnLock(uint8_t seqNbr)			//开锁
{
	delay_ms(20);
	return;
}

void Charger_LockUp(uint8_t seqNbr)			//落锁
{
	delay_ms(20);
	return;
}

uint8_t Charger_Terminate(uint8_t seqNbr)	//充电结束
{
	if(assert_param_seqNbr(seqNbr)){
	Charger_ShutDown(seqNbr);
	Charger_UnLock(seqNbr);
	}
	return 0x00;
}

uint8_t Charger_Commence(uint8_t seqNbr)	 //充电开始
{
	if(assert_param_seqNbr(seqNbr)){
	Charger_LockUp(seqNbr);
	Charger_PowerOn(seqNbr);
	}
	return 0x00;
}


//_Charge_Info_Proofread()
//数据帧标准格式校对，待后期扩展
//并且更正错误
void _Charge_Info_Proofread(_Charge_Info* pchargeStruct)
{
	uint8_t cnt;
	uint8_t nbrOfOccupied = 0;
	for(cnt=0;cnt<nbrOfCharger;cnt++){
		if((pchargeStruct->stack[cnt].ucFlag&(1<<0))||(pchargeStruct->stack[cnt].ucFlag&(1<<4)))nbrOfOccupied++;
		//如果处于工作状态或故障状态，则不能用的充电桩数++
		else continue;
	}
	pchargeStruct->avail = nbrOfCharger - nbrOfOccupied;//更新结构体中avail可用充电桩数量的值
	_Charge_Info_Synchronize(pchargeStruct);//更新数据帧
	return;
}


//从数据帧中找寻契合编号的成员
uint8_t _Match_Info_UserID(_Charge_Info* pchargeStruct,uint8_t* ucArray_ID)
{
	uint8_t cnt;
	uint8_t seqNbr_rtrn = 0xFF;//输出的桩位号，0xff表示没有
	uint32_t ucArray_ID_temp = ((ucArray_ID[0]<<8*0)|(ucArray_ID[1]<<8*1)|(ucArray_ID[2]<<8*2)|(ucArray_ID[3]<<8*3));//转为32位缓存
	for(cnt=0;cnt<nbrOfCharger;cnt++){
		if(pchargeStruct->stack[cnt].ucArray_ID == ucArray_ID_temp){
			seqNbr_rtrn=cnt;//遍历结构体，找到后将组号附给之，若没有，则seqNbr_rtrn仍为0xFF
			break;
		}
	}
	return seqNbr_rtrn;
}

//第二个页面
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
				uint8_t toggle = ABOV;//上下界面的标识，初始为上
				uint16_t temp;
//				uint8_t  res;
				seqNbr_thbwhlSwitch = seqNbr_default;		//标记的起始	
				chargeTime_designated = time_default;
				
				
RUTH:		AutoReturn_cntdownRefresh_sec(20);  //初始刷新启动
				SettingPage_StaticLayout();//显示静态界面
				LCD_BorderSwitch(toggle);//显示框选
				ChargeNbr_RQ_ScrollDisp(290,150,seqNbr_thbwhlSwitch,50,0);	//初始显示
				Advanced_Show_Nbr(290,520,chargeTime_designated,50);		//显示默认选项
				
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
																if(temp&(1<<0))Popover_Show_Str(0XF0,0X18B,"该充电桩已占用！",0X32,0X190,1000);
																else if(temp&(1<<4))Popover_Show_Str(0XF0,0X18B,"该充电桩已故障！",0X32,0X190,1000);
																else{
																	seqNbr_Selected = seqNbr_thbwhlSwitch;
																	LCD_BorderSwitch(0x02);
																	toggle=BENE;//只有在无故障的条件下才会通过，修改BUG
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
																LCD_BorderSwitch(0x01);//返回
															}else{//返回
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
				}while(cntdown);//计数允许之前不许断
				
DEAD:		LED1=1;
				return;
}


//主进程
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
				Card_Surveillance(DISCRETE);		//读取卡
				key_detect = KEY_Scan(0);				//读取按键
				switch (key_detect){
					case KEY0_PRES: SettingsBeforeCharge(); goto TAG;
					case KEY2_PRES: Charger_Terminate_Handler(); goto TAG;
					default : break;
				}
			}
}




