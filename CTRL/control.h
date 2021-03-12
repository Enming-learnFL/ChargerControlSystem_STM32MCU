#ifndef _CONTROL_H
#define _CONTROL_H

#include "sys.h"

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

/**-----------------------------------------------------------------------------------
 **
 *	·break;语句位于switch中时只能跳出switch语句块，而不能跳出while语句块
 *	·利用以上特性可以编写UI的操作控制
 *	·赋值表达式的值是被赋值的对象的值，例如if(a=assign(whatever))这个条件，它既可以判断
 *	函数assign(whatever)的返回值，又可以把这个值赋给一个缓存变量，从而传递给下一波对象
 *	但是为了严谨，需要在给这个条件添加一个判断，来诱发表达式的计算！
 *	·函数的规律是，但凡函数要向外输出值，一者使用return，二者输入指针向外输出，后者常用
 *	·		错误反思：这是错误的代码，传入一个指针，竟然在同步的时候又对其进行了一次取址，这样得到的数据毫无意义
 *		怪不得每次同步数据都会出错，原因就在这里！！
 *		STMFLASH_Write(SYNC_SHUTTLE_BASE,(uint16_t*)&pchargeStruct,sizeof(_Charge_Info)/2);
 *		去掉取址就能得到正确的代码！万分注意字节的地址对齐！！
 */
/* The Programmer's Progress ----------------------------------------------------- */

#define bill(min) ((uint32_t)((0.75*min/(double)15)*100)) //测试用计价为每15分钟0.75元，即每小时3角 乘以100，转为定点数

#define TEST		0X01	//测试模式
#define NORM		0X00	//寻常模式
//
#define TOKEN		0XAAAA	 //帧头部，用以判断EEPROM里面的数据是否存在 二进制为1100 1100 1100 1100，共计16位bit
#define nbrOfCharger 16  //原型机架构16个充电桩，编号0-15，[15:0]

/*------------------------------------------------------------------------------------
	SYNC_SHUTTLE 数据同步梭位于STM32内置FLASH中，地址为0x0807F800
	选址在主存储器的页255起始0x0807F800，页地址范围为 0x0807F800-0x0807FFFF，共2K字节
-----------------------------------------------------------------------------------*/
#define SYNC_SHUTTLE_BASE 0x0807F800   //数据帧的同步梭位于EEPROM的基址

#define seqNbr_default 2		 //默认停在#2号充电位
#define time_default	 180	 //默认停在180分钟

//定义在内存flash中存储的数据结构体
typedef struct Charge_State{
	uint16_t	ucFlag;									//用以记录相应的标志位，高字节保留，低字节用以记录状态
	uint32_t 	ucArray_ID;							//当前正在充电的桩号
	uint32_t 	timeStamp;							//充电结束时的时间戳
} _State;
/* 
ucFlag标识字节 16位 共2字节
	\\*   高字节保留 为0X00   *     *低字节用以标记使用情况*
		|15|14|13|12|11|10| 9| 8|   |7 |6 |5 |4 | 3| 2| 1| 0|
[7] 	模式位		1:测试模式用 0:正式拓展模式使用

[6:5] (保留，初始为0)

[4]		故障位 	1:维修中 0:正常使用

[3:1] (保留，初始为0)

[0]		工作位		1:使用中 0:空闲状态

ucFlag初始状态应为 0X0080 = 0000 0000 1000 0000(B)
更新：高字节写入充电桩序号！
*/



//定义数据帧结构体
typedef volatile struct Charge_Info{
	uint16_t 		token;								//头部标记，用于判断结构体是否仍然存在
	uint16_t 		avail;								//当前可用的充电桩数目
	_State			stack[nbrOfCharger];	//结构体数组，用于存放各个充电桩的数据
} _Charge_Info;
/**
		充电桩数据帧结构体的组成部分
	1.	token = 0XAAAA (1100 1100 1100 1100);
	2.	avail = 0x0000 初始为nbrOfCharger，即0X0010，表示当前可用的充电桩数量
	3.	stack[nbrOfCharger] 栈，用以存放nbrOfCharger个充电桩当前的状态
			栈中其中的每个元素按照要求进行置位
		1) ucFlag = 0X0080，即第七位为测试位，这是默认测试状态；正常工作状态为0X0000
		2) ucArray_ID 为卡ID，初始为全零
		3) timeStamp 为到点的时间戳，工作时初始为全零，测试为全F 0xFFFF FFFF
	
	@Attention:seqNbr为充电桩序号，是重要的索引对象
*/

//定义断言，用以判断输入参数的合法性
#define IS_VALID_SEQNBR(seqNbr) ((seqNbr!=0xFF)&&(seqNbr<nbrOfCharger))
#define assert_param_seqNbr(seqNbr)	IS_VALID_SEQNBR(seqNbr)

extern _Charge_Info charge_Sync;

extern uint8_t 		seqNbr_Selected;				//已经确定的合法充电桩号
extern uint16_t 	chargeTime_Selected;		//已经确定的合法充电时长

extern uint8_t		seqNbr_thbwhlSwitch;		//当前指向的充电桩号
extern uint16_t		chargeTime_designated;	//当前选择的充电时长

//命名解释：_Charge_Info是以结构体为实际作用对象的函数操作，是intrude的，不是围绕的；其它的操作则是围绕的
void _Charge_Info_Init(_Charge_Info* pchargeStruct, uint8_t mode);
void _Charge_Info_Synchronize(_Charge_Info* pchargeStruct);
void _Charge_Info_unitReset(_Charge_Info* pchargeStruct,uint8_t seqNbr);
void _Charge_Info_unitSet(_Charge_Info* pchargeStruct,uint8_t seqNbr,uint16_t chargeTime_Selected,uint8_t* ucArray_ID);
void _Charge_Info_Proofread(_Charge_Info* pchargeStruct);
uint8_t _Charge_Info_Load(_Charge_Info* pchargeStruct);
uint8_t _Match_Info_UserID(_Charge_Info* pchargeStruct, uint8_t* ucArray_ID);

uint8_t Charge_Status_Check(_Charge_Info* pchargeStruct);
void Charger_MonitorHandler(void);//充电桩监视服务函数，装备于:RTC_IRQHandler();

void SettingsBeforeCharge(void);
void theMainProcess(void);


//硬件对接部分
void Charger_ShutDown(uint8_t seqNbr);
void Charger_PowerOn(uint8_t seqNbr);
void Charger_UnLock(uint8_t seqNbr);
void Charger_LockUp(uint8_t seqNbr);
uint8_t Charger_Terminate(uint8_t seqNbr);
uint8_t Charger_Commence(uint8_t seqNbr);
#endif

