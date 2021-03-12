#include "GUI.h"
#include "text.h"
#include "lcd.h"
#include "sys.h" 
#include "fontupd.h"
#include "w25qxx.h"
#include "string.h"												    
#include "usart.h"	//拷贝text内容
#include "delay.h"
#include "malloc.h"
#include "control.h"
#include "common.h"
#include "stdarg.h"
#include "repository.h"

/************************************************
 ALIENTEK战舰STM32开发板
 
* 充电桩主控系统 * 
 
 作者：高恩铭 @GaoEnMing

* @date    2021-xx-xx
* @brief   防拔充电桩工程设计
************************************************/

/////////////////////////////////////////////////////////////////////
//图形操作界面GUI
//@Auther: Gao Enming
//心愿：运用汇编语言编写终端进程
//待办：暂无
/////////////////////////////////////////////////////////////////////

//复位基本界面函数
//主要界面依然为黑色
//引导提示框为红色
//该面板显示界面是定制的
//主要参数在宏定义中进行
//mode清除模式
//0:全部清除 
//1:单清除主界面 
//2:单清除提示条
void Reset_Background(uint8_t mode)
{
	#ifndef _RESET_BACKGROUND 
	#define _RESET_BACKGROUND	//设置本函数所需的专属宏定义
	
	#define DIVISION_HEIG 			70
	#define MAIN_ZONE_COLOR 		BLACK
	#define	HINT_ZONE_COLOR 		RED
	#endif 
	u32 index=0;      
	u32 totalpoint=lcddev.width;
	u32 divisionpoint = lcddev.width;
	u32 screenpoint, hintbarpoint;
	
	totalpoint*=lcddev.height; 								//得到总点数
	divisionpoint*=(lcddev.height-DIVISION_HEIG); //得到截止至分界处的点数
	screenpoint = divisionpoint;							//主显示区域的点数
	hintbarpoint = totalpoint-divisionpoint;	//提示条区域的点数
	
		if((lcddev.id==0X6804)&&(lcddev.dir==1))//6804横屏的时候特殊处理  
		{						    
			lcddev.dir=0;	 
			lcddev.setxcmd=0X2A;
			lcddev.setycmd=0X2B;  	 			
			LCD_SetCursor(0x00,0x0000);		//设置光标位置  
			lcddev.dir=1;	 
				lcddev.setxcmd=0X2B;
			lcddev.setycmd=0X2A;  	 
		}
			else 
			LCD_SetCursor(0x00,0x0000);		//设置光标位置 
			index = 0;//初始化
			LCD_WriteRAM_Prepare();  	//开始写入GRAM	 	
			if(mode==2){ //mode=2，从而直接跳转至HINT执行
				LCD_SetCursor(0x00,lcddev.height-DIVISION_HEIG);
				index = screenpoint-hintbarpoint;//更新index
				LCD_WriteRAM_Prepare(); //更改cursor后务必需要重新写入命令行，否则将执行错误
				goto HINT;
			}	   		  	
			do{//主屏幕清理
				LCD->LCD_RAM=MAIN_ZONE_COLOR;
				index++;
			}while(index<divisionpoint);
			//黑色部分主体写入完毕		
			if(mode==1)goto END;
			
HINT:	do{
				LCD->LCD_RAM=HINT_ZONE_COLOR;
				index++;
			}while(index<totalpoint);
			//分别写入黑色和红色
			
END:	return;	
}

//边框选动画展示
//BORDER_INIT 0 初始化显示上框
//SWITCH_UP   1 下移
//SWITCH_DOWN 2 上移
//BACK_REGAIN	3 返回时显示下框
//DEBUG_ALL   0xff 两框同显
void LCD_BorderSwitch(uint8_t Toggle)
{
	#ifndef _LCD_BorderSwitch	//定义本地函数所需的宏定义
	#define _LCD_BorderSwitch
	
	#define BORDER_INIT 0x00			//00
	#define SWITCH_UP		0x01			//01
	#define SWITCH_DOWN 0x02			//10	
	#define BACK_REGAIN	0x03			//11
	#define DEBUG_ALL		0xFF
	
	#define DIVISION_LINE 		430 //黄金分割比所计算出的界面分界位置
	#define MIDLINE_LINE 			365
	#define DIVISION_GAP 			5
	#define DIVISION_SEL			MIDLINE_LINE
	#define DIVISION_ABOV			DIVISION_SEL-DIVISION_GAP
	#define DIVISION_BENE			DIVISION_SEL+DIVISION_GAP
	#define BORDER_COLOR 			GRAY
	#define UNSELECTED_COLOR	DIM
	
	#endif
	switch(Toggle){//框选的初始化，只显示上框；上移动画，下移动画,只显示下框；调试则同时显示两个框
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
	Reset_Background(2);//清空提示条
	if(*leftHint!=0)Advanced_Show_Str(16,737,480,50,leftHint,50,1);//输入不是空字符时如此罢也
	if(*rightHint!=0)Advanced_Show_Str(lcddev.width-14-(50/2)*(strlen((const char*)rightHint)),737,480,50,rightHint,50,1);
																								//这样写是为了保持对齐，并且保持右端对齐
	return;
}


//弹窗显示一段文字，显示若干时间后关闭，恢复弹窗下面像素的内容
//附加边框的居中对齐显示字符串
//(x,y)为中心坐标
//len 两臂的总宽度，臂展
//有默认的内径与框粗
//内存是否够用的恐慌？
//uint16_t PreserveBuff[480*800] __attribute__((at(0X68000000)));//放在外部SRAM中，这个方法是不可行的，因为后期还要用到内存池
//使用malloc分配内存
//Holdms宜选择1860ms，因为24为寄存器，不支持更久的时长，否则将溢出反倒变短
void Popover_Show_Str(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len,uint16_t Holdms)
{
	#ifndef _POPOVER
	#define _POPOVER
	
	#define STROKE 						14				//裱框宽度的像素数 
	#define INNER_DIAM_LEN		202				//默认内径长像素数
	#define INNER_DIAM_HEIG		52				//默认内径高像素数
	
	#endif
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     			//字符或者中文 
	u8 mode=1;					//0 非叠加，1 叠加	
	u8 halfsize = size/2;
	
	u16 sx_in,ex_in=0;		//为什么会有随机值？
	u16 sy_in,ey_in=0;		//外框与内框的二四象限对角点坐标定义
	u16 sx_ex,ex_ex=0;
	u16 sy_ex,ey_ex=0;
	
	u16 popover_len;
	u16 popover_heig;
	u32 totalpoints;
	//不小的栈，不知是否可被运行,若不行，则采用清屏的方法解决；若可以，则研究栈的使用方法
	/*uint16_t PreserveBuff[480*800];*///想要拥有更大的内存，务必使用动态分配的内存

	uint16_t* PreserveBuff = 0;
	uint16_t* pts = 0;
	//uint16_t* pts=(uint16_t*)&PreserveBuff;//栈顶指针
	
	uint16_t line_comb,colnm_comb; 
	
	u16 strlenth=strlen((const char*)str);//得到字符数，一个汉字占两个字符
	u8  charcnt = len/size;								//得到一行能显示的汉字数
	u8  ASCcharcnt = charcnt*2;						//一行能显示的ASC字符数
	u8  rownum = strlenth/(charcnt*2)+((strlenth%(charcnt*2))?1:0);//得到以汉字为度量的行数
	u16 rows_heig = rownum*size;					//显示这些字符总共所需的行数总高
	
	sx_in=x;
	ex_in=x;
	sy_in=y;
	ey_in=y;
	
	if(len<size)return; 									//如果显示的行太窄，则返回
	if(((rows_heig/2+STROKE)>((y0<lcddev.height-y0)?y0:lcddev.height-y0))
		||((len/2+STROKE)>((x0<lcddev.width-x0)?x0:lcddev.width-x0)))return;
		//目前的字数若超出了既已有的范围，则返回
	//若不会出界，则继续执行下面的代码，即开始显示
	sx_in-=((INNER_DIAM_LEN>(len/2))?INNER_DIAM_LEN:(len/2));
	ex_in+=((INNER_DIAM_LEN>(len/2))?INNER_DIAM_LEN:(len/2));
	sy_in-=((INNER_DIAM_HEIG>(rows_heig/2))?INNER_DIAM_HEIG:(rows_heig/2));
	ey_in+=((INNER_DIAM_HEIG>(rows_heig/2))?INNER_DIAM_HEIG:(rows_heig/2));
	sx_ex=sx_in-STROKE;
	ex_ex=ex_in+STROKE;
	sy_ex=sy_in-STROKE;
	ey_ex=ey_in+STROKE; //确定了四个坐标，它们是定好的常量
	
	popover_len = ex_ex-sx_ex+1;
	popover_heig = ey_ex-sy_ex+1;
	totalpoints=popover_len*popover_heig;
	
	PreserveBuff = (uint16_t*)mymalloc(SRAMEX,totalpoints*2);//申请外部内存池中的totalpoints*2个字节
//	if(PreserveBuff!=NULL)printf("\r\n成功申请显存\r\n");
//	else printf("\r\n显存申请失败\r\n");
	pts = PreserveBuff; 			//初始化
	for(colnm_comb=0;colnm_comb<popover_len;colnm_comb++){
		for(line_comb=0;line_comb<popover_heig;line_comb++){
			*pts=LCD_ReadPoint(sx_ex+colnm_comb,sy_ex+line_comb);//数据类型都对，进行读点
//			*pts=LCD_Fast_ReadPoint(sx_ex+colnm_comb,sy_ex+line_comb);//型号未知之前不要擅自修改读取程序！	
			pts++;			//以pts为指针将各个值存在buff里
		}
	}
	
	LCD_Fill(sx_in,sy_in,ex_in,ey_in,POOR); 							//显示衬底
	LCD_DrawBorder(sx_ex,sy_ex,ex_ex,ey_ex,STROKE,RED);		//显示外框，外框确保为红色
	
	y0-=(rows_heig)/2;//计算起始坐标
	x0-=len/2;
	y = y0;	/*初始化光标位置*/																									
	if(strlen((const char*)str)>ASCcharcnt)x=x0;
	else x=x0+(len-strlen((const char*)str)*halfsize)/2;			
	while(*str!=0)//数据未结束
	{ 
			if(!bHz){//判断是否为汉字模块，若不是汉字，bHz不变化，仍是0，若是汉字，则变其为1，并在下一个else中还原
				if(*str>0x80)bHz=1;//中文 0x80为128 大于此数之后是汉字
				else{ //else后的确为字符 按字符显示的方法进行显示    
					if(x>(x0+len-size/2)){	//不够写剩下一个字符了，索性换行			   
						y+=size;
						if(strlen((const char*)str)>ASCcharcnt)x=x0;
						else x=x0+(len-strlen((const char*)str)*halfsize)/2;
					}							    
					if(y>(y0+rows_heig-size))break;//越界返回      
					if(*str==13){ //换行符号        
						y+=size;
						if(strlen((const char*)str)>ASCcharcnt)x=x0;
						else x=x0+(len-strlen((const char*)str)*halfsize)/2;
						str++; 
					}  
					else LCD_ShowChar_Ex(x,y,*str,size,mode);//有效部分写入 
					str++; 
					x+=size/2; //字符,为全字的一半 
				}
			}else{//这个情况时此字的确是中文      
					bHz=0;//有汉字库（？）此处归零是为了下一个字符的判断，将这个flag清零    
					if(x>(x0+len-size)){	//换行    
							y+=size;
							if(strlen((const char*)str)>ASCcharcnt)x=x0;
							else x=x0+(len-strlen((const char*)str)*halfsize)/2;	  
					}
					if(y>(y0+rows_heig-size))break;//越界返回  						     
					Show_Font_Ex(x,y,str,size,mode); //显示这个汉字,空心显示 
					str+=2; 
					x+=size;//下一个汉字偏移	    
			}						 
	}//显示完毕
	errorTone();	//提示音
	delay_ms_plus(Holdms);
	
	//这一切之后，开始进行恢复工作
	pts = PreserveBuff;//指针归回栈顶，复位
	for(colnm_comb=0;colnm_comb<popover_len;colnm_comb++){
		for(line_comb=0;line_comb<popover_heig;line_comb++){
			LCD_Fast_DrawPoint(sx_ex+colnm_comb,sy_ex+line_comb,*pts);//数据类型都对，进行读点
			pts++;			//以pts为指针将各个值存在buff里
		}
	}
	myfree(SRAMEX,PreserveBuff);//释放内存，放马南山
}


//充电桩信息单条显示
//mode为显示模式，区分二者都显示同一种颜色或者分别
//mode=0，表示序号与状态显示同一种颜色
//mode=1，表示序号仍显示默认的笔色
//rval为返回值
//rval=0，表示当前序号的充电桩正被占用或故障
//rval=0，暗示当前位置不能选择
//rval=1，表示当前序号的充电桩可以使用
uint8_t ChargeNbr_RQ_PieceDisp(uint16_t x,uint16_t y,uint8_t seqNbr,uint8_t size,uint8_t mode)
{
	#ifndef _ChargeNbr_PieceDisp
	#define _ChargeNbr_PieceDisp
	
	#define GAP 						10  			//字间像素数为10
	#define SUB_RATIO 			0.48			//主字与附字的比例
	#define DROP_RATIO			0.37			//主字与附字之间的落差比例
	#define OCP_COLR				0xF800		//占用的颜色 红色
	#define ERR_COLR				0xF81F		//故障的颜色 紫色
	#define AVL_COLR				0x07E0		//空闲的颜色 绿色
	#define DFT_COLR				0XFE80		//默认笔色   黄色 设计色之立顿黄
	#endif
	unsigned char str[10];
	uint8_t rval=0;
	if(assert_param_seqNbr(seqNbr)){
		if(charge_Sync.stack[seqNbr].ucFlag&(1<<0)){
			POINT_COLOR = OCP_COLR;//工作标识位为占用
			Advanced_Show_Str(x+size+GAP,y+mult(size,DROP_RATIO),lcddev.width,size,"(占用)",mult(size,SUB_RATIO),0);
		}else if(charge_Sync.stack[seqNbr].ucFlag&(1<<4)){
			POINT_COLOR = ERR_COLR;//故障位标起
			Advanced_Show_Str(x+size+GAP,y+mult(size,DROP_RATIO),lcddev.width,size,"(故障)",mult(size,SUB_RATIO),0);
		}else{//标识位为空闲
			POINT_COLOR = AVL_COLR;
			Advanced_Show_Str(x+size+GAP,y+mult(size,DROP_RATIO),lcddev.width,size,"(空闲)",mult(size,SUB_RATIO),0);
			rval=1;//当前序号可以使用
		}
		if(mode)POINT_COLOR = DFT_COLR;
		
		sprintf(str,"%.2d",seqNbr);//显示两位整数，位数不够则在左侧补零 //忽略报错 /*缓冲区溢出了！！！！*/
		Advanced_Show_Str(x,y,lcddev.width,size,str,size,0);
	}
	return rval;
}

//充电桩号滚动条显示
//一般状态调用0
//充电桩号颜色指示其工作状态
//rval为返回值
//rval=0，表示当前序号的充电桩正被占用或故障
//rval=0，暗示当前位置不能选择
//rval=1，表示当前序号的充电桩可以使用
uint8_t ChargeNbr_RQ_ScrollDisp(uint16_t x,uint16_t y,uint8_t seqNbr,uint8_t size,uint8_t mode)
{
	uint8_t rval=0;
	u16 colorTemp = POINT_COLOR;
	LCD_Fill(x,y-size-GAP,lcddev.width-21,y+size*2+GAP,BACK_COLOR);
	Advanced_Show_Str(lcddev.width-mult(size,0.80)-20,y+mult(size,0.18),lcddev.width,size,"←",mult(size,0.80),0);//显示箭头
	rval = ChargeNbr_RQ_PieceDisp(x,y,seqNbr,size,mode);
	ChargeNbr_RQ_PieceDisp(x,y-size-GAP,seqNbr-1,size,mode);
	ChargeNbr_RQ_PieceDisp(x,y+size+GAP,seqNbr+1,size,mode);
	POINT_COLOR = colorTemp;//还原颜色
	return rval;
}

//显示用户信息
//uint8_t* ucArray_ID为另一个可选参数
//自动确定参数的个数和用途过于复杂
//还是老老实实地写入参数吧
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
	if(ucArray_ID!=NULL){//读入了数据
		LCD_Fill(0,0,lcddev.width,300,POOR);			//填充POOR色
		sprintf( cStr, "卡号：%02X%02X%02X%02X",ucArray_ID [0], ucArray_ID [1], ucArray_ID [2],ucArray_ID [3] );//低字节代表高位数据！
		Advanced_Show_Str(30,70,lcddev.width,48,cStr,48,0);
	}
	sprintf ( cStr, "余额：%.2f 元", (float)((double)readValue/100));			//若只是更新余额，则说明不需要重新填入颜色				 										 	         
	Advanced_Show_Str(30,140,lcddev.width,48,cStr,48,0);
	va_end(arg_ptr);
	BACK_COLOR = backColorTemp;//恢复背景描写的颜色
}


//在任意位置作局限于一个方形内的任意来源的点阵图案
//输入的点阵图案务必为方形，会将其按照方形的格式进行相应的处理
//输入参数：
//x,y为该点阵图案的起始像素的位置
//pServohead为伺服磁头，用于读取数据
//size为点阵图案的像素大小
//color为该单色图案的着色
//mode为点阵图案是否为叠加显示
//mode为0：非叠加显示，无像素处着以背景色
//mode为1：叠加显示，无像素处按透明直接处理，不打扰此处的显存数据
void Draw_Random_MAP_withinSquare(uint16_t x, uint16_t y, uint8_t* pServohead, uint8_t size, uint16_t color ,uint8_t mode)
{
	u16 temp,tbit,tbyte;//防溢出
	u16 y0=y;	//用来记述点阵头的起始位置，方便在达到size边界的时侯返回
	
	u16 csize=(size/8+((size%8)?1:0))*size;		//得到点阵所占的字节数	按照方形的取模规律
	
	for(tbyte=0;tbyte<csize;tbyte++){//一个字符所占有的字节数 只读取足够的字节数即可，真正的索引是字节数！！可以预先设好充足的字节数，只要在主控时不读即可！！
																		//随着读字节磁头的行进，将字节数据一个一个地读进来	
		temp = pServohead[tbyte];	//这个绝妙的简单的语句包含了传递，只要是以字节为单位进行索引，就可以忽略原始数组的结构差异
		
		for(tbit=0;tbit<8;tbit++){		
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,color);//当前点有值
			else if((mode&0x01)==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);//着背景色，若mode为0，即非叠加模式，在当前点为0时着背景色
																												//若mode为1，则直接跳过透明的点
			temp<<=1;//向高位移一位
			y++;//取模方式为从上到下，自左至右
			if(y>=lcddev.height)return;		//超区域了
			
			if((y-y0)==size)//此处自动将字限制在相应的范围内，到size的范围就将显示阶段
			{//比如12号字体的比特数不是字节的整数倍，但是其表示的比特数却为16位，为了不将下面无意义的内容显示出来
				y=y0;
				x++;
				if(x>=lcddev.width)return;	//超区域了
				break;
			}
		}  	 
	}//将输入的点阵数据全部操作完，点阵MAP对应的图案就会全部显现出来  	    	
	return;
}

//作连缀的等大方阵点阵图案串
//pdotSqr为存放图案的首地址
//strlen为连缀的图案数
//在这个阶段，mode的最高位表示是否进行居中化处理
//mode[7]为1表示居中,输入的x，y认为是对角中心
//mode[7]为0表示常规
//其余参数为寻常的
//其用途按寻常方式处理
void Draw_Composed_dotMapSqr_String(uint16_t x, uint16_t y,u16 width,u16 height,u8 strlen,u8* pdotSqr,u8 size,u16 color,u8 mode)
{
	u16 x0;//(x0,y0)始终为作图案的起始角
	u16 y0;	
	uint8_t cnt;
	uint16_t csize = (size/8+((size%8)?1:0))*size;//获得单个图案的字节数
	if(mode&0x80){//mode[7]为1表示居中,输入的x，y认为是对角中心
		x -= size*strlen/2;
		y -= size/2;
		if((x>lcddev.width)||(y>lcddev.height))return;//但凡位置不对，超域，就终止函数
	}
	x0 = x;
	y0 = y;
	for(cnt = 0;cnt<strlen; cnt++){
		if(x>(x0+width-size)){	//换行    
							y+=size;
							x=x0;		  
					}
					if(y>(y0+height-size))break;//越界返回，放弃显示  						     
					Draw_Random_MAP_withinSquare(x,y,pdotSqr,size,color,mode&0x01); //显示这个图案,空心显示 
					pdotSqr+=csize; //让指针直接跨越一个图案的距离
					x+=size;//下一个汉字偏移	    
	}//作完strlen个图案，即可结束返回
	return;
}

//绘制单色充电桩图标
//推荐使用颜色 GBLUE 0X07FF
//mode为点阵图案是否为叠加显示
//mode为0：非叠加显示，无像素处着以背景色
//mode为1：叠加显示，无像素处按透明直接处理，不打扰此处的显存数据
void draw_mono_charger_icos(uint16_t x, uint16_t y,uint16_t color, uint8_t mode)
{
	Draw_Random_MAP_withinSquare(x,y,(uint8_t*)_CHARGER_ICO114_114,114,color,mode);//使用时间数码色
	return;
}



//代替显示引导语的放大字体显示
void draw_enlarged_GuideStr(uint8_t strTag)
{
	uint8_t* pChar = NULL;
	switch(strTag){//选择标识语句
		case GUIDE__WELCOME: 				pChar = (uint8_t*)_GUIDE__WELCOME;				break;
		case GUIDE__OP_SUCCESS:			pChar = (uint8_t*)_GUIDE__OP_SUCCESS;			break;
		case GUIDE__SWIPE_CARD_PLS: pChar = (uint8_t*)_GUIDE__SWIPE_CARD_PLS;	break;
	}
	if(pChar==NULL)return;	//未找到字库则直接退出
	//语句都只用4个2字符来实现，因此具有共同性
//	Draw_Composed_dotMapSqr_String(240,386,lcddev.width,lcddev.height,4,pChar,96,POINT_COLOR,0x80);//非叠加的居中显示
	Draw_Composed_dotMapSqr_String(240,410,lcddev.width,lcddev.height,4,pChar,96,POINT_COLOR,0x80);//非叠加的居中显示
}

uint16_t availTemp;
//单次模式下进行全面显示
//连续模式下只进行值的更改
void title_IdleDisp(uint8_t runAttribute)
{
	char str[20];
	uint16_t colorTemp = POINT_COLOR;
	POINT_COLOR = LGRAYBLUE;//LIGHTGREEN;//GBLUE;//ORANGE;
	if(runAttribute==0){
		draw_mono_charger_icos(lcddev.width-119,80,GBLUE,0x01);
		sprintf(str,"充电桩可用:%2d",charge_Sync.avail);
		Advanced_Show_Str(lcddev.width-114-50*6.5-8,112,480,50,(uint8_t*)str,50,0x00);
	}else{
		if(availTemp!=charge_Sync.avail){//更新之
			availTemp=charge_Sync.avail;
			sprintf(str,"%2d",availTemp);
			Advanced_Show_Str(lcddev.width-114-58,112,480,50,(uint8_t*)str,50,0x00);
		}
	}
	POINT_COLOR = colorTemp;
	return;
}

//定义一个结构体来存储相应的数据
struct gui_back_PreserveStruct{
	uint16_t* ptr_PreserveBuff;
	uint16_t  sx;
	uint16_t  sy;
	uint16_t	ex;
	uint16_t 	ey;
}gui_back_PreserveBuff;
//gui背景缓存
//(sx,sy)为起始对角线点，(ex,ey)为结束对角线点
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
	
	gui_back_PreserveBuff.ptr_PreserveBuff = (uint16_t*)mymalloc(SRAMEX,totalpoints*2);//申请外部内存池中的totalpoints*2个字节
	
//	if((pts = gui_back_PreserveBuff.ptr_PreserveBuff)==NULL){//初始化
//		printf("\r\n显存申请失败\r\n");			
//		return;
//	}
	
	if(gui_back_PreserveBuff.ptr_PreserveBuff==NULL){//初始化
		printf("\r\n显存申请失败\r\n");			
		return;
	}else {
		printf("\r\n显存申请成功\r\n");
		pts = gui_back_PreserveBuff.ptr_PreserveBuff;//我竟然用野指针写了半天！！！程序没出什么问题已经是万幸了！
	}		
	
	for(colnm_comb=0;colnm_comb<popover_len;colnm_comb++){
		for(line_comb=0;line_comb<popover_heig;line_comb++){
			*pts =LCD_ReadPoint(sx+colnm_comb,sy+line_comb);//数据类型都对，进行读点
//			debug = *pts ;//观察存值,写入出现了问题
			pts++;			//以pts为指针将各个值存在buff里
		}
	}
	printf("\r\n显存写入成功\r\n");	
	debug ++;//无意义，纯粹只是防止报错
	return;
}

//背景图案恢复
//参数无需再次写入
void gui_back_Recover(void)
{
	u16 popover_len;
	u16 popover_heig;
	uint16_t* pts = NULL;
	uint16_t watch;
	
	uint16_t line_comb,colnm_comb; 
	
	pts = gui_back_PreserveBuff.ptr_PreserveBuff;//指针归回栈顶，复位
  if(pts == NULL){
		printf("\r\n找不到显存数据！\r\n");			
		return;
	}
	popover_len = gui_back_PreserveBuff.ex-gui_back_PreserveBuff.sx+1;
	popover_heig = gui_back_PreserveBuff.ey-gui_back_PreserveBuff.sy+1;	

	for(colnm_comb=0;colnm_comb<popover_len;colnm_comb++){
		for(line_comb=0;line_comb<popover_heig;line_comb++){
			LCD_Fast_DrawPoint(gui_back_PreserveBuff.sx+colnm_comb,gui_back_PreserveBuff.sy+line_comb,*pts);//数据类型都对，进行读点
//			watch = *pts;
			pts++;			//以pts为指针将各个值存在buff里
		}
	}
	printf("\r\n背景恢复成功！\r\n");	
	watch++;//无意义，纯粹为防止报错
	myfree(SRAMEX,gui_back_PreserveBuff.ptr_PreserveBuff);//释放内存，放马南山
	return;
}


//各页面语句的布局
//尚未做屏幕优化
//主页面
void HomePage_StaticLayout(void)
{
	Reset_Background(0x00);
//	Advanced_Show_Str_Mid(0,386,"欢迎使用",72,480,0);
	draw_enlarged_GuideStr(GUIDE__WELCOME);
	realTime_Display();
	HintBar_Display("解锁","充电");
	return;
}

//设置页面
//设置充电桩号与时长
void SettingPage_StaticLayout(void)
{
	Reset_Background(0x00);
	Advanced_Show_Str(40,150,480,50,"充电桩号：",50,1);
	Advanced_Show_Str(40,520,480,50,"充电时长：",50,1);
	Advanced_Show_Str(375,520,480,50,"min",50,1);
	HintBar_Display("返回","继续");
	return;
}

//充电开始页面
void Charge_CommencePage_StaticLayout(void)
{
	Reset_Background(0x00);
//	Advanced_Show_Str_Mid(0,386,"请刷卡！",72,480,0);
	draw_enlarged_GuideStr(GUIDE__SWIPE_CARD_PLS);
	HintBar_Display("取消","\0");
	return;
}

//充电结束页面
void Charge_TerminatePage_StaticLayout(void)
{
	Reset_Background(0x00);
//	Advanced_Show_Str_Mid(0,386,"请刷卡！",72,480,0);
	draw_enlarged_GuideStr(GUIDE__SWIPE_CARD_PLS);
	HintBar_Display("\0","取消");
	return;
}

