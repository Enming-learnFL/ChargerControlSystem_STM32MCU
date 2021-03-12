#include "GUI.h"
#include "text.h"
#include "lcd.h"
#include "sys.h" 
#include "fontupd.h"
#include "w25qxx.h"
#include "string.h"												    
#include "usart.h"	//����text����
#include "delay.h"
#include "malloc.h"
#include "control.h"
#include "common.h"
#include "stdarg.h"
#include "repository.h"

/************************************************
 ALIENTEKս��STM32������
 
* ���׮����ϵͳ * 
 
 ���ߣ��߶��� @GaoEnMing

* @date    2021-xx-xx
* @brief   ���γ��׮�������
************************************************/

/////////////////////////////////////////////////////////////////////
//ͼ�β�������GUI
//@Auther: Gao Enming
//��Ը�����û�����Ա�д�ն˽���
//���죺����
/////////////////////////////////////////////////////////////////////

//��λ�������溯��
//��Ҫ������ȻΪ��ɫ
//������ʾ��Ϊ��ɫ
//�������ʾ�����Ƕ��Ƶ�
//��Ҫ�����ں궨���н���
//mode���ģʽ
//0:ȫ����� 
//1:����������� 
//2:�������ʾ��
void Reset_Background(uint8_t mode)
{
	#ifndef _RESET_BACKGROUND 
	#define _RESET_BACKGROUND	//���ñ����������ר���궨��
	
	#define DIVISION_HEIG 			70
	#define MAIN_ZONE_COLOR 		BLACK
	#define	HINT_ZONE_COLOR 		RED
	#endif 
	u32 index=0;      
	u32 totalpoint=lcddev.width;
	u32 divisionpoint = lcddev.width;
	u32 screenpoint, hintbarpoint;
	
	totalpoint*=lcddev.height; 								//�õ��ܵ���
	divisionpoint*=(lcddev.height-DIVISION_HEIG); //�õ���ֹ���ֽ紦�ĵ���
	screenpoint = divisionpoint;							//����ʾ����ĵ���
	hintbarpoint = totalpoint-divisionpoint;	//��ʾ������ĵ���
	
		if((lcddev.id==0X6804)&&(lcddev.dir==1))//6804������ʱ�����⴦��  
		{						    
			lcddev.dir=0;	 
			lcddev.setxcmd=0X2A;
			lcddev.setycmd=0X2B;  	 			
			LCD_SetCursor(0x00,0x0000);		//���ù��λ��  
			lcddev.dir=1;	 
				lcddev.setxcmd=0X2B;
			lcddev.setycmd=0X2A;  	 
		}
			else 
			LCD_SetCursor(0x00,0x0000);		//���ù��λ�� 
			index = 0;//��ʼ��
			LCD_WriteRAM_Prepare();  	//��ʼд��GRAM	 	
			if(mode==2){ //mode=2���Ӷ�ֱ����ת��HINTִ��
				LCD_SetCursor(0x00,lcddev.height-DIVISION_HEIG);
				index = screenpoint-hintbarpoint;//����index
				LCD_WriteRAM_Prepare(); //����cursor�������Ҫ����д�������У�����ִ�д���
				goto HINT;
			}	   		  	
			do{//����Ļ����
				LCD->LCD_RAM=MAIN_ZONE_COLOR;
				index++;
			}while(index<divisionpoint);
			//��ɫ��������д�����		
			if(mode==1)goto END;
			
HINT:	do{
				LCD->LCD_RAM=HINT_ZONE_COLOR;
				index++;
			}while(index<totalpoint);
			//�ֱ�д���ɫ�ͺ�ɫ
			
END:	return;	
}

//�߿�ѡ����չʾ
//BORDER_INIT 0 ��ʼ����ʾ�Ͽ�
//SWITCH_UP   1 ����
//SWITCH_DOWN 2 ����
//BACK_REGAIN	3 ����ʱ��ʾ�¿�
//DEBUG_ALL   0xff ����ͬ��
void LCD_BorderSwitch(uint8_t Toggle)
{
	#ifndef _LCD_BorderSwitch	//���屾�غ�������ĺ궨��
	#define _LCD_BorderSwitch
	
	#define BORDER_INIT 0x00			//00
	#define SWITCH_UP		0x01			//01
	#define SWITCH_DOWN 0x02			//10	
	#define BACK_REGAIN	0x03			//11
	#define DEBUG_ALL		0xFF
	
	#define DIVISION_LINE 		430 //�ƽ�ָ����������Ľ���ֽ�λ��
	#define MIDLINE_LINE 			365
	#define DIVISION_GAP 			5
	#define DIVISION_SEL			MIDLINE_LINE
	#define DIVISION_ABOV			DIVISION_SEL-DIVISION_GAP
	#define DIVISION_BENE			DIVISION_SEL+DIVISION_GAP
	#define BORDER_COLOR 			GRAY
	#define UNSELECTED_COLOR	DIM
	
	#endif
	switch(Toggle){//��ѡ�ĳ�ʼ����ֻ��ʾ�Ͽ����ƶ��������ƶ���,ֻ��ʾ�¿򣻵�����ͬʱ��ʾ������
		case BORDER_INIT:LCD_DrawBorder(0,0,480,DIVISION_ABOV,20,UNSELECTED_COLOR);LCD_DrawBorder(0,DIVISION_BENE,480,728,20,UNSELECTED_COLOR);
										 LCD_DrawBorder(0,0,480,DIVISION_ABOV,20,BORDER_COLOR);break;
		case SWITCH_UP:LCD_DrawBorder(0,DIVISION_BENE,480,728,20,UNSELECTED_COLOR);LCD_DrawBorder(0,0,480,DIVISION_ABOV,20,BORDER_COLOR);break;
		case SWITCH_DOWN:LCD_DrawBorder(0,0,480,DIVISION_ABOV,20,UNSELECTED_COLOR);LCD_DrawBorder(0,DIVISION_BENE,480,728,20,BORDER_COLOR);break;
		case BACK_REGAIN:LCD_DrawBorder(0,DIVISION_BENE,480,728,20,UNSELECTED_COLOR);LCD_DrawBorder(0,0,480,DIVISION_ABOV,20,UNSELECTED_COLOR);
										 LCD_DrawBorder(0,DIVISION_BENE,480,728,20,BORDER_COLOR);break;
		case DEBUG_ALL:LCD_DrawBorder(0,0,480,DIVISION_ABOV,20,BORDER_COLOR);LCD_DrawBorder(0,DIVISION_BENE,480,728,20,BORDER_COLOR);break;
		default:return;
	}
}

void HintBar_Display(unsigned char* leftHint,unsigned char* rightHint)
{
	Reset_Background(2);//�����ʾ��
	if(*leftHint!=0)Advanced_Show_Str(16,737,480,50,leftHint,50,1);//���벻�ǿ��ַ�ʱ��˰�Ҳ
	if(*rightHint!=0)Advanced_Show_Str(lcddev.width-14-(50/2)*(strlen((const char*)rightHint)),737,480,50,rightHint,50,1);
																								//����д��Ϊ�˱��ֶ��룬���ұ����Ҷ˶���
	return;
}


//������ʾһ�����֣���ʾ����ʱ���رգ��ָ������������ص�����
//���ӱ߿�ľ��ж�����ʾ�ַ���
//(x,y)Ϊ��������
//len ���۵��ܿ�ȣ���չ
//��Ĭ�ϵ��ھ�����
//�ڴ��Ƿ��õĿֻţ�
//uint16_t PreserveBuff[480*800] __attribute__((at(0X68000000)));//�����ⲿSRAM�У���������ǲ����еģ���Ϊ���ڻ�Ҫ�õ��ڴ��
//ʹ��malloc�����ڴ�
//Holdms��ѡ��1860ms����Ϊ24Ϊ�Ĵ�������֧�ָ��õ�ʱ������������������
void Popover_Show_Str(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len,uint16_t Holdms)
{
	#ifndef _POPOVER
	#define _POPOVER
	
	#define STROKE 						14				//�ѿ��ȵ������� 
	#define INNER_DIAM_LEN		202				//Ĭ���ھ���������
	#define INNER_DIAM_HEIG		52				//Ĭ���ھ���������
	
	#endif
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     			//�ַ��������� 
	u8 mode=1;					//0 �ǵ��ӣ�1 ����	
	u8 halfsize = size/2;
	
	u16 sx_in,ex_in=0;		//Ϊʲô�������ֵ��
	u16 sy_in,ey_in=0;		//������ڿ�Ķ������޶Խǵ����궨��
	u16 sx_ex,ex_ex=0;
	u16 sy_ex,ey_ex=0;
	
	u16 popover_len;
	u16 popover_heig;
	u32 totalpoints;
	//��С��ջ����֪�Ƿ�ɱ�����,�����У�����������ķ�������������ԣ����о�ջ��ʹ�÷���
	/*uint16_t PreserveBuff[480*800];*///��Ҫӵ�и�����ڴ棬���ʹ�ö�̬������ڴ�

	uint16_t* PreserveBuff = 0;
	uint16_t* pts = 0;
	//uint16_t* pts=(uint16_t*)&PreserveBuff;//ջ��ָ��
	
	uint16_t line_comb,colnm_comb; 
	
	u16 strlenth=strlen((const char*)str);//�õ��ַ�����һ������ռ�����ַ�
	u8  charcnt = len/size;								//�õ�һ������ʾ�ĺ�����
	u8  ASCcharcnt = charcnt*2;						//һ������ʾ��ASC�ַ���
	u8  rownum = strlenth/(charcnt*2)+((strlenth%(charcnt*2))?1:0);//�õ��Ժ���Ϊ����������
	u16 rows_heig = rownum*size;					//��ʾ��Щ�ַ��ܹ�����������ܸ�
	
	sx_in=x;
	ex_in=x;
	sy_in=y;
	ey_in=y;
	
	if(len<size)return; 									//�����ʾ����̫խ���򷵻�
	if(((rows_heig/2+STROKE)>((y0<lcddev.height-y0)?y0:lcddev.height-y0))
		||((len/2+STROKE)>((x0<lcddev.width-x0)?x0:lcddev.width-x0)))return;
		//Ŀǰ�������������˼����еķ�Χ���򷵻�
	//��������磬�����ִ������Ĵ��룬����ʼ��ʾ
	sx_in-=((INNER_DIAM_LEN>(len/2))?INNER_DIAM_LEN:(len/2));
	ex_in+=((INNER_DIAM_LEN>(len/2))?INNER_DIAM_LEN:(len/2));
	sy_in-=((INNER_DIAM_HEIG>(rows_heig/2))?INNER_DIAM_HEIG:(rows_heig/2));
	ey_in+=((INNER_DIAM_HEIG>(rows_heig/2))?INNER_DIAM_HEIG:(rows_heig/2));
	sx_ex=sx_in-STROKE;
	ex_ex=ex_in+STROKE;
	sy_ex=sy_in-STROKE;
	ey_ex=ey_in+STROKE; //ȷ�����ĸ����꣬�����Ƕ��õĳ���
	
	popover_len = ex_ex-sx_ex+1;
	popover_heig = ey_ex-sy_ex+1;
	totalpoints=popover_len*popover_heig;
	
	PreserveBuff = (uint16_t*)mymalloc(SRAMEX,totalpoints*2);//�����ⲿ�ڴ���е�totalpoints*2���ֽ�
//	if(PreserveBuff!=NULL)printf("\r\n�ɹ������Դ�\r\n");
//	else printf("\r\n�Դ�����ʧ��\r\n");
	pts = PreserveBuff; 			//��ʼ��
	for(colnm_comb=0;colnm_comb<popover_len;colnm_comb++){
		for(line_comb=0;line_comb<popover_heig;line_comb++){
			*pts=LCD_ReadPoint(sx_ex+colnm_comb,sy_ex+line_comb);//�������Ͷ��ԣ����ж���
//			*pts=LCD_Fast_ReadPoint(sx_ex+colnm_comb,sy_ex+line_comb);//�ͺ�δ֪֮ǰ��Ҫ�����޸Ķ�ȡ����	
			pts++;			//��ptsΪָ�뽫����ֵ����buff��
		}
	}
	
	LCD_Fill(sx_in,sy_in,ex_in,ey_in,POOR); 							//��ʾ�ĵ�
	LCD_DrawBorder(sx_ex,sy_ex,ex_ex,ey_ex,STROKE,RED);		//��ʾ������ȷ��Ϊ��ɫ
	
	y0-=(rows_heig)/2;//������ʼ����
	x0-=len/2;
	y = y0;	/*��ʼ�����λ��*/																									
	if(strlen((const char*)str)>ASCcharcnt)x=x0;
	else x=x0+(len-strlen((const char*)str)*halfsize)/2;			
	while(*str!=0)//����δ����
	{ 
			if(!bHz){//�ж��Ƿ�Ϊ����ģ�飬�����Ǻ��֣�bHz���仯������0�����Ǻ��֣������Ϊ1��������һ��else�л�ԭ
				if(*str>0x80)bHz=1;//���� 0x80Ϊ128 ���ڴ���֮���Ǻ���
				else{ //else���ȷΪ�ַ� ���ַ���ʾ�ķ���������ʾ    
					if(x>(x0+len-size/2)){	//����дʣ��һ���ַ��ˣ����Ի���			   
						y+=size;
						if(strlen((const char*)str)>ASCcharcnt)x=x0;
						else x=x0+(len-strlen((const char*)str)*halfsize)/2;
					}							    
					if(y>(y0+rows_heig-size))break;//Խ�緵��      
					if(*str==13){ //���з���        
						y+=size;
						if(strlen((const char*)str)>ASCcharcnt)x=x0;
						else x=x0+(len-strlen((const char*)str)*halfsize)/2;
						str++; 
					}  
					else LCD_ShowChar_Ex(x,y,*str,size,mode);//��Ч����д�� 
					str++; 
					x+=size/2; //�ַ�,Ϊȫ�ֵ�һ�� 
				}
			}else{//������ʱ���ֵ�ȷ������      
					bHz=0;//�к��ֿ⣨�����˴�������Ϊ����һ���ַ����жϣ������flag����    
					if(x>(x0+len-size)){	//����    
							y+=size;
							if(strlen((const char*)str)>ASCcharcnt)x=x0;
							else x=x0+(len-strlen((const char*)str)*halfsize)/2;	  
					}
					if(y>(y0+rows_heig-size))break;//Խ�緵��  						     
					Show_Font_Ex(x,y,str,size,mode); //��ʾ�������,������ʾ 
					str+=2; 
					x+=size;//��һ������ƫ��	    
			}						 
	}//��ʾ���
	errorTone();	//��ʾ��
	delay_ms_plus(Holdms);
	
	//��һ��֮�󣬿�ʼ���лָ�����
	pts = PreserveBuff;//ָ����ջ������λ
	for(colnm_comb=0;colnm_comb<popover_len;colnm_comb++){
		for(line_comb=0;line_comb<popover_heig;line_comb++){
			LCD_Fast_DrawPoint(sx_ex+colnm_comb,sy_ex+line_comb,*pts);//�������Ͷ��ԣ����ж���
			pts++;			//��ptsΪָ�뽫����ֵ����buff��
		}
	}
	myfree(SRAMEX,PreserveBuff);//�ͷ��ڴ棬������ɽ
}


//���׮��Ϣ������ʾ
//modeΪ��ʾģʽ�����ֶ��߶���ʾͬһ����ɫ���߷ֱ�
//mode=0����ʾ�����״̬��ʾͬһ����ɫ
//mode=1����ʾ�������ʾĬ�ϵı�ɫ
//rvalΪ����ֵ
//rval=0����ʾ��ǰ��ŵĳ��׮����ռ�û����
//rval=0����ʾ��ǰλ�ò���ѡ��
//rval=1����ʾ��ǰ��ŵĳ��׮����ʹ��
uint8_t ChargeNbr_RQ_PieceDisp(uint16_t x,uint16_t y,uint8_t seqNbr,uint8_t size,uint8_t mode)
{
	#ifndef _ChargeNbr_PieceDisp
	#define _ChargeNbr_PieceDisp
	
	#define GAP 						10  			//�ּ�������Ϊ10
	#define SUB_RATIO 			0.48			//�����븽�ֵı���
	#define DROP_RATIO			0.37			//�����븽��֮���������
	#define OCP_COLR				0xF800		//ռ�õ���ɫ ��ɫ
	#define ERR_COLR				0xF81F		//���ϵ���ɫ ��ɫ
	#define AVL_COLR				0x07E0		//���е���ɫ ��ɫ
	#define DFT_COLR				0XFE80		//Ĭ�ϱ�ɫ   ��ɫ ���ɫ֮���ٻ�
	#endif
	unsigned char str[10];
	uint8_t rval=0;
	if(assert_param_seqNbr(seqNbr)){
		if(charge_Sync.stack[seqNbr].ucFlag&(1<<0)){
			POINT_COLOR = OCP_COLR;//������ʶλΪռ��
			Advanced_Show_Str(x+size+GAP,y+mult(size,DROP_RATIO),lcddev.width,size,"(ռ��)",mult(size,SUB_RATIO),0);
		}else if(charge_Sync.stack[seqNbr].ucFlag&(1<<4)){
			POINT_COLOR = ERR_COLR;//����λ����
			Advanced_Show_Str(x+size+GAP,y+mult(size,DROP_RATIO),lcddev.width,size,"(����)",mult(size,SUB_RATIO),0);
		}else{//��ʶλΪ����
			POINT_COLOR = AVL_COLR;
			Advanced_Show_Str(x+size+GAP,y+mult(size,DROP_RATIO),lcddev.width,size,"(����)",mult(size,SUB_RATIO),0);
			rval=1;//��ǰ��ſ���ʹ��
		}
		if(mode)POINT_COLOR = DFT_COLR;
		
		sprintf(str,"%.2d",seqNbr);//��ʾ��λ������λ������������ಹ�� //���Ա��� /*����������ˣ�������*/
		Advanced_Show_Str(x,y,lcddev.width,size,str,size,0);
	}
	return rval;
}

//���׮�Ź�������ʾ
//һ��״̬����0
//���׮����ɫָʾ�乤��״̬
//rvalΪ����ֵ
//rval=0����ʾ��ǰ��ŵĳ��׮����ռ�û����
//rval=0����ʾ��ǰλ�ò���ѡ��
//rval=1����ʾ��ǰ��ŵĳ��׮����ʹ��
uint8_t ChargeNbr_RQ_ScrollDisp(uint16_t x,uint16_t y,uint8_t seqNbr,uint8_t size,uint8_t mode)
{
	uint8_t rval=0;
	u16 colorTemp = POINT_COLOR;
	LCD_Fill(x,y-size-GAP,lcddev.width-21,y+size*2+GAP,BACK_COLOR);
	Advanced_Show_Str(lcddev.width-mult(size,0.80)-20,y+mult(size,0.18),lcddev.width,size,"��",mult(size,0.80),0);//��ʾ��ͷ
	rval = ChargeNbr_RQ_PieceDisp(x,y,seqNbr,size,mode);
	ChargeNbr_RQ_PieceDisp(x,y-size-GAP,seqNbr-1,size,mode);
	ChargeNbr_RQ_PieceDisp(x,y+size+GAP,seqNbr+1,size,mode);
	POINT_COLOR = colorTemp;//��ԭ��ɫ
	return rval;
}

//��ʾ�û���Ϣ
//uint8_t* ucArray_IDΪ��һ����ѡ����
//�Զ�ȷ�������ĸ�������;���ڸ���
//��������ʵʵ��д�������
void User_Info_Disp(uint32_t readValue,...)
{
	char cStr[30];
	va_list arg_ptr;
	uint8_t* ucArray_ID = NULL;
	
	uint16_t backColorTemp = BACK_COLOR;
//	BACK_COLOR = LCD_Fast_ReadPoint(29,29);
	BACK_COLOR = POOR;
	va_start(arg_ptr,readValue);
	ucArray_ID = va_arg(arg_ptr,uint8_t*);
	if(ucArray_ID!=NULL){//����������
		LCD_Fill(0,0,lcddev.width,300,POOR);			//���POORɫ
		sprintf( cStr, "���ţ�%02X%02X%02X%02X",ucArray_ID [0], ucArray_ID [1], ucArray_ID [2],ucArray_ID [3] );//���ֽڴ����λ���ݣ�
		Advanced_Show_Str(30,70,lcddev.width,48,cStr,48,0);
	}
	sprintf ( cStr, "��%.2f Ԫ", (float)((double)readValue/100));			//��ֻ�Ǹ�������˵������Ҫ����������ɫ				 										 	         
	Advanced_Show_Str(30,140,lcddev.width,48,cStr,48,0);
	va_end(arg_ptr);
	BACK_COLOR = backColorTemp;//�ָ�������д����ɫ
}


//������λ����������һ�������ڵ�������Դ�ĵ���ͼ��
//����ĵ���ͼ�����Ϊ���Σ��Ὣ�䰴�շ��εĸ�ʽ������Ӧ�Ĵ���
//���������
//x,yΪ�õ���ͼ������ʼ���ص�λ��
//pServoheadΪ�ŷ���ͷ�����ڶ�ȡ����
//sizeΪ����ͼ�������ش�С
//colorΪ�õ�ɫͼ������ɫ
//modeΪ����ͼ���Ƿ�Ϊ������ʾ
//modeΪ0���ǵ�����ʾ�������ش����Ա���ɫ
//modeΪ1��������ʾ�������ش���͸��ֱ�Ӵ��������Ŵ˴����Դ�����
void Draw_Random_MAP_withinSquare(uint16_t x, uint16_t y, uint8_t* pServohead, uint8_t size, uint16_t color ,uint8_t mode)
{
	u16 temp,tbit,tbyte;//�����
	u16 y0=y;	//������������ͷ����ʼλ�ã������ڴﵽsize�߽��ʱ���
	
	u16 csize=(size/8+((size%8)?1:0))*size;		//�õ�������ռ���ֽ���	���շ��ε�ȡģ����
	
	for(tbyte=0;tbyte<csize;tbyte++){//һ���ַ���ռ�е��ֽ��� ֻ��ȡ�㹻���ֽ������ɣ��������������ֽ�����������Ԥ����ó�����ֽ�����ֻҪ������ʱ�������ɣ���
																		//���Ŷ��ֽڴ�ͷ���н������ֽ�����һ��һ���ض�����	
		temp = pServohead[tbyte];	//�������ļ򵥵��������˴��ݣ�ֻҪ�����ֽ�Ϊ��λ�����������Ϳ��Ժ���ԭʼ����Ľṹ����
		
		for(tbit=0;tbit<8;tbit++){		
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,color);//��ǰ����ֵ
			else if((mode&0x01)==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);//�ű���ɫ����modeΪ0�����ǵ���ģʽ���ڵ�ǰ��Ϊ0ʱ�ű���ɫ
																												//��modeΪ1����ֱ������͸���ĵ�
			temp<<=1;//���λ��һλ
			y++;//ȡģ��ʽΪ���ϵ��£���������
			if(y>=lcddev.height)return;		//��������
			
			if((y-y0)==size)//�˴��Զ�������������Ӧ�ķ�Χ�ڣ���size�ķ�Χ�ͽ���ʾ�׶�
			{//����12������ı����������ֽڵ����������������ʾ�ı�����ȴΪ16λ��Ϊ�˲��������������������ʾ����
				y=y0;
				x++;
				if(x>=lcddev.width)return;	//��������
				break;
			}
		}  	 
	}//������ĵ�������ȫ�������꣬����MAP��Ӧ��ͼ���ͻ�ȫ�����ֳ���  	    	
	return;
}

//����׺�ĵȴ������ͼ����
//pdotSqrΪ���ͼ�����׵�ַ
//strlenΪ��׺��ͼ����
//������׶Σ�mode�����λ��ʾ�Ƿ���о��л�����
//mode[7]Ϊ1��ʾ����,�����x��y��Ϊ�ǶԽ�����
//mode[7]Ϊ0��ʾ����
//�������ΪѰ����
//����;��Ѱ����ʽ����
void Draw_Composed_dotMapSqr_String(uint16_t x, uint16_t y,u16 width,u16 height,u8 strlen,u8* pdotSqr,u8 size,u16 color,u8 mode)
{
	u16 x0;//(x0,y0)ʼ��Ϊ��ͼ������ʼ��
	u16 y0;	
	uint8_t cnt;
	uint16_t csize = (size/8+((size%8)?1:0))*size;//��õ���ͼ�����ֽ���
	if(mode&0x80){//mode[7]Ϊ1��ʾ����,�����x��y��Ϊ�ǶԽ�����
		x -= size*strlen/2;
		y -= size/2;
		if((x>lcddev.width)||(y>lcddev.height))return;//����λ�ò��ԣ����򣬾���ֹ����
	}
	x0 = x;
	y0 = y;
	for(cnt = 0;cnt<strlen; cnt++){
		if(x>(x0+width-size)){	//����    
							y+=size;
							x=x0;		  
					}
					if(y>(y0+height-size))break;//Խ�緵�أ�������ʾ  						     
					Draw_Random_MAP_withinSquare(x,y,pdotSqr,size,color,mode&0x01); //��ʾ���ͼ��,������ʾ 
					pdotSqr+=csize; //��ָ��ֱ�ӿ�Խһ��ͼ���ľ���
					x+=size;//��һ������ƫ��	    
	}//����strlen��ͼ�������ɽ�������
	return;
}

//���Ƶ�ɫ���׮ͼ��
//�Ƽ�ʹ����ɫ GBLUE 0X07FF
//modeΪ����ͼ���Ƿ�Ϊ������ʾ
//modeΪ0���ǵ�����ʾ�������ش����Ա���ɫ
//modeΪ1��������ʾ�������ش���͸��ֱ�Ӵ��������Ŵ˴����Դ�����
void draw_mono_charger_icos(uint16_t x, uint16_t y,uint16_t color, uint8_t mode)
{
	Draw_Random_MAP_withinSquare(x,y,(uint8_t*)_CHARGER_ICO114_114,114,color,mode);//ʹ��ʱ������ɫ
	return;
}



//������ʾ������ķŴ�������ʾ
void draw_enlarged_GuideStr(uint8_t strTag)
{
	uint8_t* pChar = NULL;
	switch(strTag){//ѡ���ʶ���
		case GUIDE__WELCOME: 				pChar = (uint8_t*)_GUIDE__WELCOME;				break;
		case GUIDE__OP_SUCCESS:			pChar = (uint8_t*)_GUIDE__OP_SUCCESS;			break;
		case GUIDE__SWIPE_CARD_PLS: pChar = (uint8_t*)_GUIDE__SWIPE_CARD_PLS;	break;
	}
	if(pChar==NULL)return;	//δ�ҵ��ֿ���ֱ���˳�
	//��䶼ֻ��4��2�ַ���ʵ�֣���˾��й�ͬ��
//	Draw_Composed_dotMapSqr_String(240,386,lcddev.width,lcddev.height,4,pChar,96,POINT_COLOR,0x80);//�ǵ��ӵľ�����ʾ
	Draw_Composed_dotMapSqr_String(240,410,lcddev.width,lcddev.height,4,pChar,96,POINT_COLOR,0x80);//�ǵ��ӵľ�����ʾ
}

uint16_t availTemp;
//����ģʽ�½���ȫ����ʾ
//����ģʽ��ֻ����ֵ�ĸ���
void title_IdleDisp(uint8_t runAttribute)
{
	char str[20];
	uint16_t colorTemp = POINT_COLOR;
	POINT_COLOR = LGRAYBLUE;//LIGHTGREEN;//GBLUE;//ORANGE;
	if(runAttribute==0){
		draw_mono_charger_icos(lcddev.width-119,80,GBLUE,0x01);
		sprintf(str,"���׮����:%2d",charge_Sync.avail);
		Advanced_Show_Str(lcddev.width-114-50*6.5-8,112,480,50,(uint8_t*)str,50,0x00);
	}else{
		if(availTemp!=charge_Sync.avail){//����֮
			availTemp=charge_Sync.avail;
			sprintf(str,"%2d",availTemp);
			Advanced_Show_Str(lcddev.width-114-58,112,480,50,(uint8_t*)str,50,0x00);
		}
	}
	POINT_COLOR = colorTemp;
	return;
}

//����һ���ṹ�����洢��Ӧ������
struct gui_back_PreserveStruct{
	uint16_t* ptr_PreserveBuff;
	uint16_t  sx;
	uint16_t  sy;
	uint16_t	ex;
	uint16_t 	ey;
}gui_back_PreserveBuff;
//gui��������
//(sx,sy)Ϊ��ʼ�Խ��ߵ㣬(ex,ey)Ϊ�����Խ��ߵ�
void gui_back_Preserve(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
	u16 popover_len;
	u16 popover_heig;
	u32 totalpoints;
	uint16_t* pts = NULL;
	uint16_t debug;
	
	uint16_t line_comb,colnm_comb; 
	
	if((ex<sx)||(ey<sy))return;
	
	popover_len = ex-sx+1;
	popover_heig = ey-sy+1;
	totalpoints=popover_len*popover_heig;
	
	gui_back_PreserveBuff.ex = ex;
	gui_back_PreserveBuff.ey = ey;
	gui_back_PreserveBuff.sx = sx;
	gui_back_PreserveBuff.sy = sy;
	
	gui_back_PreserveBuff.ptr_PreserveBuff = (uint16_t*)mymalloc(SRAMEX,totalpoints*2);//�����ⲿ�ڴ���е�totalpoints*2���ֽ�
	
//	if((pts = gui_back_PreserveBuff.ptr_PreserveBuff)==NULL){//��ʼ��
//		printf("\r\n�Դ�����ʧ��\r\n");			
//		return;
//	}
	
	if(gui_back_PreserveBuff.ptr_PreserveBuff==NULL){//��ʼ��
		printf("\r\n�Դ�����ʧ��\r\n");			
		return;
	}else {
		printf("\r\n�Դ�����ɹ�\r\n");
		pts = gui_back_PreserveBuff.ptr_PreserveBuff;//�Ҿ�Ȼ��Ұָ��д�˰��죡��������û��ʲô�����Ѿ��������ˣ�
	}		
	
	for(colnm_comb=0;colnm_comb<popover_len;colnm_comb++){
		for(line_comb=0;line_comb<popover_heig;line_comb++){
			*pts =LCD_ReadPoint(sx+colnm_comb,sy+line_comb);//�������Ͷ��ԣ����ж���
//			debug = *pts ;//�۲��ֵ,д�����������
			pts++;			//��ptsΪָ�뽫����ֵ����buff��
		}
	}
	printf("\r\n�Դ�д��ɹ�\r\n");	
	debug ++;//�����壬����ֻ�Ƿ�ֹ����
	return;
}

//����ͼ���ָ�
//���������ٴ�д��
void gui_back_Recover(void)
{
	u16 popover_len;
	u16 popover_heig;
	uint16_t* pts = NULL;
	uint16_t watch;
	
	uint16_t line_comb,colnm_comb; 
	
	pts = gui_back_PreserveBuff.ptr_PreserveBuff;//ָ����ջ������λ
  if(pts == NULL){
		printf("\r\n�Ҳ����Դ����ݣ�\r\n");			
		return;
	}
	popover_len = gui_back_PreserveBuff.ex-gui_back_PreserveBuff.sx+1;
	popover_heig = gui_back_PreserveBuff.ey-gui_back_PreserveBuff.sy+1;	

	for(colnm_comb=0;colnm_comb<popover_len;colnm_comb++){
		for(line_comb=0;line_comb<popover_heig;line_comb++){
			LCD_Fast_DrawPoint(gui_back_PreserveBuff.sx+colnm_comb,gui_back_PreserveBuff.sy+line_comb,*pts);//�������Ͷ��ԣ����ж���
//			watch = *pts;
			pts++;			//��ptsΪָ�뽫����ֵ����buff��
		}
	}
	printf("\r\n�����ָ��ɹ���\r\n");	
	watch++;//�����壬����Ϊ��ֹ����
	myfree(SRAMEX,gui_back_PreserveBuff.ptr_PreserveBuff);//�ͷ��ڴ棬������ɽ
	return;
}


//��ҳ�����Ĳ���
//��δ����Ļ�Ż�
//��ҳ��
void HomePage_StaticLayout(void)
{
	Reset_Background(0x00);
//	Advanced_Show_Str_Mid(0,386,"��ӭʹ��",72,480,0);
	draw_enlarged_GuideStr(GUIDE__WELCOME);
	realTime_Display();
	HintBar_Display("����","���");
	return;
}

//����ҳ��
//���ó��׮����ʱ��
void SettingPage_StaticLayout(void)
{
	Reset_Background(0x00);
	Advanced_Show_Str(40,150,480,50,"���׮�ţ�",50,1);
	Advanced_Show_Str(40,520,480,50,"���ʱ����",50,1);
	Advanced_Show_Str(375,520,480,50,"min",50,1);
	HintBar_Display("����","����");
	return;
}

//��翪ʼҳ��
void Charge_CommencePage_StaticLayout(void)
{
	Reset_Background(0x00);
//	Advanced_Show_Str_Mid(0,386,"��ˢ����",72,480,0);
	draw_enlarged_GuideStr(GUIDE__SWIPE_CARD_PLS);
	HintBar_Display("ȡ��","\0");
	return;
}

//������ҳ��
void Charge_TerminatePage_StaticLayout(void)
{
	Reset_Background(0x00);
//	Advanced_Show_Str_Mid(0,386,"��ˢ����",72,480,0);
	draw_enlarged_GuideStr(GUIDE__SWIPE_CARD_PLS);
	HintBar_Display("\0","ȡ��");
	return;
}

