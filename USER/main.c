#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "lcd.h"  
#include "key.h"     
#include "usmart.h" 
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"   
#include "text.h"
#include "GUI.h"
#include "sram.h"	  
#include "rtc.h"
#include "common.h"
#include "control.h"
#include "rc522_handler.h"
#include "beep.h"
#include "rc522_config.h"
#include "rc522_function.h"
 	
 
/************************************************
 ALIENTEK战舰STM32开发板
 
* 充电桩主控系统 * 
 
 作者：高恩铭 @GaoEnMing
 
* @version V2.5
* @date    2021-xx-xx
* @brief   防拔充电桩工程设计
************************************************/

/************************************************
修改说明：
修改为黑色背景，黑色填充窗口，红色和黄色字体
************************************************/

/************************************************
更新说明：
V1.0	实现基础扣费功能，使用正计时来处理余额

V2.0	更新了UI操作界面，用按键来实现；加入了实时时钟，重写了计费的整套逻辑；引入了汉字系统，集成字符放大功能，更适应4.3'LCD屏
V2.1	优化了按键的灵敏度，几乎感受不到操作的延迟；修改了数据帧的写入BUG
V2.2	细调了选择界面的框选动画；余额格式更改，可以存储小数
V2.3	搭建了调试系统，引入管理员模式，可以更方便地重置余额；优化了通过串口报错的功能
V2.4	用字模软件重新制定了更精美的引导语字模，新加入114*114的充电桩图标，丰富主界面的内容
V2.5	增加了串口调试功能，启动余额重置页面，将会把设定好的余额充入卡中

（亟待解决读卡器稳定性，数据读取权限，宏定义的位置设计系统）

*不重要的瑕疵：标准系统时间对时
************************************************/

extern _Charge_Info charge_Sync;	//该工程最重要的结构体变量（定义在control.c中）
extern uint16_t availTemp;

 int main(void)
 {
	uint8_t cnt;
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
 	usmart_dev.init(72);	//初始化USMART		
	 
	for(cnt=3;cnt!=0;cnt--){
		delay_ms_plus(1000);
		printf("\r\ncntdown:%d\r\n",cnt); //掉电重启串口观察窗口期
	} 
	
	RTC_Init();//初始化实时时钟 
	 
 	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();						//初始化按键
	BEEP_Init();					//初始化蜂鸣器
	LCD_Init();			   		//初始化LCD   
	W25QXX_Init();				//初始化W25Q128
//	SD_Init(); 						//初始化SD卡
	FSMC_SRAM_Init();			//初始化外部SRAM
 	my_mem_init(SRAMIN);	//初始化内部内存池
	my_mem_init(SRAMEX);	//初始化外部内存池
	exfuns_init();				//为fatfs相关变量申请内存  
 	f_mount(fs[0],"0:",1); 		//挂载SD卡 
 	f_mount(fs[1],"1:",1); 		//挂载FLASH.
	font_init();
	

	
	/*RC522模块所需外设的初始化配置*/
  RC522_Init ();
	PcdReset ();
	/*设置工作方式*/   
	M500PcdConfigISOType ( 'A' );
	
	realTime_DisplayInit();	
	_Charge_Info_Init(&charge_Sync,TEST);//初始化同步数据帧
	_Charge_Info_Load(&charge_Sync);//从FLASH模拟的EEPROM加载数据帧，成功了！
	availTemp = charge_Sync.avail;
	Reset_Background(0x00);
	
	while(CONTINUOUS)
	{
		theMainProcess();
	}
}
