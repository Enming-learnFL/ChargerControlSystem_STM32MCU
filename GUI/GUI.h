#ifndef _GUI_H
#define _GUI_H
#include "sys.h"	 
//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEK精英STM32F103开发板V1
//界面开发
//////////////////////////////////////////////////////////////////////////////////	 

#define mult(x,y) ((unsigned short	int)(((int)x)*((float)y)))	//我也不知道为什么自己要写得这么复杂？防止精度奇怪？
	
extern struct gui_back_PreserveStruct gui_back_PreserveBuff;

void Reset_Background(uint8_t mode);//mode-> 0:全部清除 1:清除主界面 2:清除提示条
void LCD_BorderSwitch(uint8_t Toggle);//选中框的上下切
void HintBar_Display(unsigned char* leftHint,unsigned char* rightHint);//提示条的语句显示
void Popover_Show_Str(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len, uint16_t Holdms);//短暂弹窗显示函数
uint8_t ChargeNbr_RQ_PieceDisp(uint16_t x,uint16_t y,uint8_t seqNbr,uint8_t size,uint8_t mode);
uint8_t ChargeNbr_RQ_ScrollDisp(uint16_t x,uint16_t y,uint8_t seqNbr,uint8_t size,uint8_t mode);//显示切换的效果
void User_Info_Disp(uint32_t readValue,...);
void Draw_Random_MAP_withinSquare(uint16_t x, uint16_t y, uint8_t* pServohead, uint8_t size, uint16_t color, uint8_t mode);
void Draw_Composed_dotMapSqr_String(uint16_t x, uint16_t y,u16 width,u16 height,u8 strlen,u8* pdotSqr,u8 size,u16 color,u8 mode);
void draw_mono_charger_icos(uint16_t x, uint16_t y,uint16_t color, uint8_t mode);
void draw_enlarged_GuideStr(uint8_t strTag);
void title_IdleDisp(uint8_t runAttribute);
void gui_back_Preserve(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey);
void gui_back_Recover(void);



//各页面静态语句的布局（初版尚未做不同尺寸的屏幕适应）

void HomePage_StaticLayout(void);
void SettingPage_StaticLayout(void);
void Charge_CommencePage_StaticLayout(void);
void Charge_TerminatePage_StaticLayout(void);
#endif

