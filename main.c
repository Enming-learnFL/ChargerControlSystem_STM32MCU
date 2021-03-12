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
 ALIENTEKս��STM32������
 
* ���׮����ϵͳ * 
 
 ���ߣ��߶��� @GaoEnMing
 
* @version V2.5
* @date    2021-xx-xx
* @brief   ���γ��׮�������
************************************************/

/************************************************
�޸�˵����
�޸�Ϊ��ɫ��������ɫ��䴰�ڣ���ɫ�ͻ�ɫ����
************************************************/

/************************************************
����˵����
V1.0	ʵ�ֻ����۷ѹ��ܣ�ʹ������ʱ���������

V2.0	������UI�������棬�ð�����ʵ�֣�������ʵʱʱ�ӣ���д�˼Ʒѵ������߼��������˺���ϵͳ�������ַ��Ŵ��ܣ�����Ӧ4.3'LCD��
V2.1	�Ż��˰����������ȣ��������ܲ����������ӳ٣��޸�������֡��д��BUG
V2.2	ϸ����ѡ�����Ŀ�ѡ����������ʽ���ģ����Դ洢С��
V2.3	��˵���ϵͳ���������Աģʽ�����Ը�������������Ż���ͨ�����ڱ���Ĺ���
V2.4	����ģ��������ƶ��˸���������������ģ���¼���114*114�ĳ��׮ͼ�꣬�ḻ�����������
V2.5	�����˴��ڵ��Թ��ܣ������������ҳ�棬������趨�õ������뿨��

��ؽ������������ȶ��ԣ����ݶ�ȡȨ�ޣ��궨���λ�����ϵͳ��

*����Ҫ��覴ã���׼ϵͳʱ���ʱ
************************************************/

extern _Charge_Info charge_Sync;	//�ù�������Ҫ�Ľṹ�������������control.c�У�
extern uint16_t availTemp;

 int main(void)
 {
	uint8_t cnt;
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 	usmart_dev.init(72);	//��ʼ��USMART		
	 
	for(cnt=3;cnt!=0;cnt--){
		delay_ms_plus(1000);
		printf("\r\ncntdown:%d\r\n",cnt); //�����������ڹ۲촰����
	} 
	
	RTC_Init();//��ʼ��ʵʱʱ�� 
	 
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();						//��ʼ������
	BEEP_Init();					//��ʼ��������
	LCD_Init();			   		//��ʼ��LCD   
	W25QXX_Init();				//��ʼ��W25Q128
//	SD_Init(); 						//��ʼ��SD��
	FSMC_SRAM_Init();			//��ʼ���ⲿSRAM
 	my_mem_init(SRAMIN);	//��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);	//��ʼ���ⲿ�ڴ��
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
 	f_mount(fs[0],"0:",1); 		//����SD�� 
 	f_mount(fs[1],"1:",1); 		//����FLASH.
	font_init();
	

	
	/*RC522ģ����������ĳ�ʼ������*/
  RC522_Init ();
	PcdReset ();
	/*���ù�����ʽ*/   
	M500PcdConfigISOType ( 'A' );
	
	realTime_DisplayInit();	
	_Charge_Info_Init(&charge_Sync,TEST);//��ʼ��ͬ������֡
	_Charge_Info_Load(&charge_Sync);//��FLASHģ���EEPROM��������֡���ɹ��ˣ�
	availTemp = charge_Sync.avail;
	Reset_Background(0x00);
	
	while(CONTINUOUS)
	{
		theMainProcess();
	}
}
