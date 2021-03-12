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
 ALIENTEK战舰STM32开发板
 
* 充电桩主控系统 * 
 
 作者：高恩铭 @GaoEnMing

* @date    2021-xx-xx
* @brief   防拔充电桩工程设计
************************************************/

/////////////////////////////////////////////////////////////////////
//托管一般代码
//@Auther: Gao Enming
//万事开头难，该文件用于存放工程所需的一般文件，不宜分类的函数，定义与功能，比如时间显示函数
//待办，需要将所有头文件的引用规范一下
//下一个升级版本，引入小数功能，充值更精确
//错误返回自检功能的完善
//图标等元素的装饰
//代码精简化，使之更加美观
/////////////////////////////////////////////////////////////////////


extern UINT br,bw;			//读写变量，用来存储函数流出的相应计数，免得没有地方存放，总算找到了在exfun中，声明过来！
//PC2LCD2002字体取模方法:逐列式,顺向(高位在前),阴码.C51格式.																		    
//特殊字体:
//数码管字体:ASCII集+℃(' '+95)
//普通字体:ASCII集
//已经事先写好在Flash中了
u8*const APP_ASCII_S6030="1:/SYSTEM/APP/COMMON/fonts60.fon";	//数码管字体60*30大数字字体路径 
//盘符1表示为外部flash W25Q128

u8* asc2_s6030=0;	//数码管字体60*30大字体点阵集，索引该字体的指针
//所扩展出来的内存不会在函数调用结束之后被释放，因此仍然存在

extern vu8 min_temp;		//标记分钟
extern vu8 hour_temp;	//标记时钟
extern vu8 cntdown;			//自动返回的倒计时

//realTime_Display
//显示实时时间
//从flash文件系统中读取已经保存了的字模，将字模数据放在外部内存XM8中
//相应的日期信息的文件结构体指针则被保存在内部内存中
uint8_t realTime_DisplayInit(void)
{
	uint8_t rval=0;  //returnValue 承接函数返回值
	uint8_t f_result;//fileResult  承接文件操作的返回值
	FIL* f_calendar = NULL;
	//为文件结构体指针分配内部内存空间
	f_calendar = (FIL*)mymalloc(SRAMIN,sizeof(FIL));//开辟FIL字节的内部内存空间
	if(f_calendar==NULL)rval=1;//开辟空间失败，返回值1
	else{//开辟空间成功，则开始读取相关文件
		f_result = f_open(f_calendar,(const TCHAR*)APP_ASCII_S6030,FA_READ);//打开存放特殊字体的文件
		if(f_result==FR_OK){//如果打开成功，则为读取文件开辟缓冲区
			asc2_s6030 = (uint8_t*)mymalloc(SRAMEX,f_calendar->fsize);//在外部内存XM8中开辟fsize大小的内存
			if(asc2_s6030==0)rval=1;
			else{//若成功开辟缓冲区，则将W25Q128中FATfs的字体文件全部读入到缓冲区中
				f_result = f_read(f_calendar,asc2_s6030,f_calendar->fsize,(UINT*)&br);//一次性读取整个文件
			}
			printf("\r\n成功读取字体，大小共计：%dbytes\t\r\n",f_calendar->fsize);
			f_close(f_calendar);
		}
		if(f_result)rval=f_result;//如果读取文件出现了什么意外，就原封不动返回这样的报错值
	}
	return rval;
}


//在相应的位置上显示数码管形式的时间
//刷新模式
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
	#define RTC_MODE						0X80	//模式为高位补零，非叠加显示，每次写都会覆盖掉已有的内容
	#endif
	do{//连续离散次数控件大括号
		RTC_Get();
		if((min_temp!=calendar.min)||(hour_temp!=calendar.hour)){
			min_temp=calendar.min;
			hour_temp=calendar.hour;
			LCD_ShowxNum_RTC(HOUR_XOFF,RTC_YOFF,calendar.hour,2,RTC_CHARSIZE,RTC_MODE);//显示小时
			LCD_ShowxNum_RTC(MIN_XOFF,RTC_YOFF,calendar.min,2,RTC_CHARSIZE,RTC_MODE);//显示分钟
			LCD_ShowChar_RTC(COLON_XOFF,RTC_YOFF,':',RTC_CHARSIZE,RTC_MODE);//显示冒号 单个字符需要使用单引号，双引号则会被认为是字符串
			
			_Charge_Info_Proofread(&charge_Sync);//以分钟来计时的中断服务
		}
	}while(runAttribute);//0为单次，1为连续
}

//在相应的位置上显示数码管形式的时间
//初始上电的一次性显示
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
	#define RTC_MODE						0X80	//模式为高位补零，非叠加显示，每次写都会覆盖掉已有的内容
	#endif
		RTC_Get();
		min_temp=calendar.min;
		hour_temp=calendar.hour;
		LCD_ShowxNum_RTC(HOUR_XOFF,RTC_YOFF,calendar.hour,2,RTC_CHARSIZE,RTC_MODE);//显示小时
		LCD_ShowxNum_RTC(MIN_XOFF,RTC_YOFF,calendar.min,2,RTC_CHARSIZE,RTC_MODE);//显示分钟
		LCD_ShowChar_RTC(COLON_XOFF,RTC_YOFF,':',RTC_CHARSIZE,RTC_MODE);//显示冒号 单个字符需要使用单引号，双引号则会被认为是字符串
}

//操作成功的响铃提示
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

//报错的响铃提示
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

//自动返回的倒计时刷新
void AutoReturn_cntdownRefresh_sec(uint8_t sec)
{
	cntdown = sec;
	return;
}

//倒计时自减
//进程无操作时间流逝
void AutoReturn_cntdownTikTok(void)
{
	if(cntdown)cntdown--;								//若cnt尚未归零，则继续递减
	return;
}

//升级版delay_ms
//此段通过循环可以延时任意久的时长
void delay_ms_plus(uint16_t ms)
{
	u16 ms0 = ms%1860;//为delay的顺利调用而使用
	u8  ms1 = ms/1860;
	delay_ms(ms0);//最长延时为1864ms；因此多输入反倒溢出变小
	for(;ms1!=0;ms1--)delay_ms(1860);
}

