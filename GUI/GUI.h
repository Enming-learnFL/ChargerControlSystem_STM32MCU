#ifndef _GUI_H
#define _GUI_H
#include "sys.h"	 
//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEK��ӢSTM32F103������V1
//���濪��
//////////////////////////////////////////////////////////////////////////////////	 

#define mult(x,y) ((unsigned short	int)(((int)x)*((float)y)))	//��Ҳ��֪��Ϊʲô�Լ�Ҫд����ô���ӣ���ֹ������֣�
	
extern struct gui_back_PreserveStruct gui_back_PreserveBuff;

void Reset_Background(uint8_t mode);//mode-> 0:ȫ����� 1:��������� 2:�����ʾ��
void LCD_BorderSwitch(uint8_t Toggle);//ѡ�п��������
void HintBar_Display(unsigned char* leftHint,unsigned char* rightHint);//��ʾ���������ʾ
void Popover_Show_Str(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len, uint16_t Holdms);//���ݵ�����ʾ����
uint8_t ChargeNbr_RQ_PieceDisp(uint16_t x,uint16_t y,uint8_t seqNbr,uint8_t size,uint8_t mode);
uint8_t ChargeNbr_RQ_ScrollDisp(uint16_t x,uint16_t y,uint8_t seqNbr,uint8_t size,uint8_t mode);//��ʾ�л���Ч��
void User_Info_Disp(uint32_t readValue,...);
void Draw_Random_MAP_withinSquare(uint16_t x, uint16_t y, uint8_t* pServohead, uint8_t size, uint16_t color, uint8_t mode);
void Draw_Composed_dotMapSqr_String(uint16_t x, uint16_t y,u16 width,u16 height,u8 strlen,u8* pdotSqr,u8 size,u16 color,u8 mode);
void draw_mono_charger_icos(uint16_t x, uint16_t y,uint16_t color, uint8_t mode);
void draw_enlarged_GuideStr(uint8_t strTag);
void title_IdleDisp(uint8_t runAttribute);
void gui_back_Preserve(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey);
void gui_back_Recover(void);



//��ҳ�澲̬���Ĳ��֣�������δ����ͬ�ߴ����Ļ��Ӧ��

void HomePage_StaticLayout(void);
void SettingPage_StaticLayout(void);
void Charge_CommencePage_StaticLayout(void);
void Charge_TerminatePage_StaticLayout(void);
#endif

