#include "sys.h" 
#include "fontupd.h"
#include "w25qxx.h"
#include "lcd.h"
#include "text.h"	
#include "string.h"												    
#include "usart.h"												    
/************************************************
 ALIENTEK战舰STM32开发板
 
* 充电桩主控系统 * 
 
 作者：高恩铭 @GaoEnMing

* @date    2021-xx-xx
* @brief   防拔充电桩工程设计
* All rights reserved	
//参考正点原子的少数基础代码并自己编写
************************************************/
 
//code 字符指针开始
//从字库中查找出字模
//code 字符串的开始地址,GBK码
//mat  数据存放地址 (size/8+((size%8)?1:0))*(size) bytes大小	
//size:字体大小
void Get_HzMat(unsigned char *code,unsigned char *mat,u8 size)//unsigned char 就是u8
{		    
	unsigned char qh,ql;
	unsigned char i;					  
	unsigned long foffset; 
	u8 csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数	 
	qh=*code;
	ql=*(++code);
	if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//非 常用汉字
	{   		    
	    for(i=0;i<csize;i++)*mat++=0x00;//填充满格
	    return; //结束访问
	}          
	if(ql<0x7f)ql-=0x40;//注意!
	else ql-=0x41;
	qh-=0x81;   
	foffset=((unsigned long)190*qh+ql)*csize;	//得到字库中的字节偏移量  		  
	switch(size)
	{
		case 12:
			W25QXX_Read(mat,foffset+ftinfo.f12addr,csize);
			break;
		case 16:
			W25QXX_Read(mat,foffset+ftinfo.f16addr,csize);
			break;
		case 24:
			W25QXX_Read(mat,foffset+ftinfo.f24addr,csize);//ftinfo是font info，存放着关于字体的相关信息，在fontupd中定义
			break;//ftinfo.f24addr这里面存放了外部flash中存放的三大字库的位置，但是正点原子并没有透露给我们！
			
	}     												    
}

//从字库中查找出标准字源字模，默认为24*24的GBK
//code 字符串的开始地址,GBK码//code 字符指针开始
//mat  数据存放地址 (size/8+((size%8)?1:0))*(size) bytes大小	纵向取模，一列不足时按成一字节来计算，横向则真实的column
//size:字体大小
void Get_SrcHzMat(unsigned char *code,unsigned char *mat)//unsigned char 就是u8
{	
	#ifndef _STD_SRC_SIZE
	#define STD_SRC_SIZE 24	//以24为标准字源，以备缩放
	#endif
	unsigned char qh,ql;
	unsigned char i;					  
	unsigned long foffset; 
	u8 csize=(STD_SRC_SIZE/8+((STD_SRC_SIZE%8)?1:0))*(STD_SRC_SIZE);//得到字体一个汉字字符对应点阵集所占的字节数	 
	qh=*code;
	ql=*(++code);
	if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//非 常用汉字
	{   		    
	    for(i=0;i<csize;i++)*mat++=0x00;//填充满格
	    return; //结束访问
	}          
	if(ql<0x7f)ql-=0x40;//注意!
	else ql-=0x41;
	qh-=0x81;   
	foffset=((unsigned long)190*qh+ql)*csize;	//得到字库中的字节偏移量  		  
	W25QXX_Read(mat,foffset+ftinfo.f24addr,csize);//获取字体的数据存放地址    mat为接收数据的地方												    
}

uint8_t* Get_HzofArbitrarySize(unsigned char *code,uint8_t size)
{
	#ifndef _STD_SRC_SIZE
	#define STD_SRC_SIZE 24	//以24为标准字源，以备缩放
	#endif
	uint8_t SrcFont24[72];//对数组名取地址，其类型是uint8_t(*)[72]代表一个数组本身，要想正确传值需要强制类型转换
	Get_SrcHzMat(code,(uint8_t*)&SrcFont24);//而数组名本身的填入则当作了指向首元素的指针，类型是uint8_t*，与输入参数对上了
	return zoomChar(STD_SRC_SIZE,STD_SRC_SIZE,size,size,(uint8_t*)&SrcFont24,1);
}

/***********************缩放字体****************************/
//#define ZOOMMAXBUFF 16384
//uint8_t zoomBuff[ZOOMMAXBUFF] = {0};	//用于缩放的缓存，最大支持到128*128
//uint8_t zoomTempBuff[1024] = {0};

/**
 * @brief  缩放字模，缩放后的字模由1个像素点由8个数据位来表示
										0x01表示笔迹，0x00表示空白区
 * @param  in_width ：原始字符宽度
 * @param  in_heig ：原始字符高度
 * @param  out_width ：缩放后的字符宽度
 * @param  out_heig：缩放后的字符高度
 * @param  in_ptr ：字库输入指针	注意：1pixel 1bit
 * @param  out_ptr ：缩放后的字符输出指针 注意: 1pixel 8bit
 *		out_ptr实际上没有正常输出，改成了直接输出到全局指针zoomBuff中
 * @param  en_cn ：0为英文，1为中文
 * @retval 无
 */
uint8_t* zoomChar(uint16_t in_width,	//原始字符宽度
									uint16_t in_heig,		//原始字符高度
									uint16_t out_width,	//缩放后的字符宽度
									uint16_t out_heig,	//缩放后的字符高度
									uint8_t *in_ptr,	//字库输入指针	注意：1pixel 1bit
									/*uint8_t *out_ptr,*/ //缩放后的字符输出指针 注意: 1pixel 8bit
									uint8_t en_cn)		//0为英文，1为中文	
{
	#define ZOOMMAXBUFF 16384 //128*128
	#define ZOOMMAXBYTE (128/8)*128 //输出字符的最大存储空间
	//static uint8_t zoomBuff[ZOOMMAXBUFF] = {0};	//最大支持到128*128 静态变量，数据不会随着函数调用结束而释放
	static uint8_t zoomTempBuff[1024] = {0};//用于缩放的缓存，源字限制输入为32*32,一个成员对接一个像素
	static uint8_t objchar[ZOOMMAXBYTE] = {0};//缩放后字符所存储的位置，以字节的形式保存,先留出富余
	//在函数里封装以上的静态变量

	
	uint8_t *pts,*ots;
	//根据源字模及目标字模大小，设定运算比例因子，左移16是为了把浮点运算转成定点运算，从而能够体现精度
	unsigned int xrIntFloat_16=(in_width<<16)/out_width+1; 
  unsigned int yrIntFloat_16=(in_heig<<16)/out_heig+1;
	
	unsigned int srcx_16=0;
	unsigned int y,x;
	uint8_t *pSrcLine;
	uint8_t *pSrcColumn; //指向列的指针，因为素材字体是纵向取模
	
	uint16_t byteCount,bitCount;
	uint16_t ocsize;//输出字符的字节数
	//定义要在可执行语句之前，养成如此的写作习惯
	
	//静态变量在每次调用函数的时侯先清空
	memset(zoomTempBuff,0,sizeof(zoomTempBuff));
	memset(objchar,0,sizeof(objchar));//每次使用时预先清零
	//检查参数是否合法
	if(in_width >= 32) return NULL;												//字库不允许超过32像素
	if(in_width * in_heig == 0) return NULL;								//输入像素数据合法性检测
	if(in_width * in_heig >= 1024 ) return NULL; 					//限制输入最大 32*32
	
	if(out_width * out_heig == 0) return NULL;	
	if(out_width * out_heig >= ZOOMMAXBUFF ) return NULL; //限制最大缩放 128*128
	pts = (uint8_t*)&zoomTempBuff;										//转编码的暂存数组
	
	//为方便运算，字库的数据由1 pixel/1bit 映射到1pixel/8bit
	//0x01表示笔迹，0x00表示空白区
	if(en_cn == 0x00)//英文
	{ //因为总是纵向取模，因此需要对高度格外关注，高度的部分才需要调整和计算相应的字节
		//英文和中文字库上下边界不对，可在此处调整。需要注意tempBuff防止溢出
		ocsize = (out_heig/8+((out_heig%8)?1:0))*out_width;//输入参数应当是规范的
		for(byteCount=0;byteCount<(in_heig/8+((in_heig%8)?1:0))*in_width;byteCount++)	
			{//英文字符的长宽始终为2：1
				for(bitCount=0;bitCount<8;bitCount++)
					{						
						//把源字模数据由位映射到字节
						//in_ptr里bitX为1，则pts里整个字节值为1
						//in_ptr里bitX为0，则pts里整个字节值为0
						*pts++ = (in_ptr[byteCount] & (0x80>>bitCount))?1:0; //先自增再赋值
					}
			}				
	}
	else //中文
	{		ocsize = (out_heig/8+((out_heig%8)?1:0))*out_width;	//规范的中文字符长宽的比例为1：1
			for(byteCount=0;byteCount<(in_heig/8+((in_heig%8)?1:0))*in_width;byteCount++)	
			{
				for(bitCount=0;bitCount<8;bitCount++)
					{						
						//把源字模数据由位映射到字节
						//in_ptr里bitX为1，则pts里整个字节值为1
						//in_ptr里bitX为0，则pts里整个字节值为0
						*pts++ = (in_ptr[byteCount] & (0x80>>bitCount))?1:0; //自增运算大赏（姑且认为源字模的数据是正确的）
					}
			}		
	}

	//zoom缩放的过程
	pts = (uint8_t*)&zoomTempBuff;	//映射后的源数据指针
	//ots = (uint8_t*)&zoomBuff;	//输出数据的指针
	//定义指针的非常规的方式
	//ots是一个宏大的数组，每一个成员都是一个指针，指向这个点要画的点的数据所在的存储位置，一个成员所含的数据是关于一个像素的
	for (x=0,byteCount=0;x<out_width;x++)	/*列遍历*/
    {
				unsigned int srcy_16=0;//每一次大列循环时都需要更新这个行指针的数据
        pSrcColumn=(pts+in_heig*(srcx_16>>16));	//源列基地址，此中存放的是一个地址		
        for (y=0,bitCount=0;y<out_heig;y++,bitCount++) /*列内像素遍历*/
        {
						if(!(bitCount<8)){
							bitCount = 0;
							byteCount++;
						}//有方括号时，表示此处已经有过取址运算
            /*ots[y]=pSrcColumn[srcy_16>>16];*/ //把源字模数据复制到目标指针中
						if((pSrcColumn[srcy_16>>16])==1)objchar[byteCount]|=(0x80>>bitCount);//相应的bit置位
            srcy_16+=yrIntFloat_16;			//按比例偏移源像素点，在下一次偏移移位归正的过程中
        }
        srcx_16+=xrIntFloat_16;				  //按比例偏移源像素点
        ots+=out_heig;//每加一个一，不是数值加，而是表示地址的移动
				byteCount++;
    }
	/*！！！缩放后的字模数据直接存储到全局指针zoomBuff里了*/
	//out_ptr = (uint8_t*)&zoomBuff;	//out_ptr没有正确传出，后面调用直接改成了全局变量指针！
	
	/*实际中如果使用out_ptr不需要下面这一句！！！
		只是因为out_ptr没有使用，会导致warning。强迫症*/
	//out_ptr++; 
		
		
		
	ocsize++;
	pSrcLine++;
		
		return (uint8_t*)&objchar;
}			



//显示一个指定大小的汉字
//x,y :汉字的坐标
//font:汉字GBK码，如何使用GB2313码
//size:字体大小
//mode:0,正常显示,1,叠加显示	   
void Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1;
	u16 y0=y;
	u8 dzk[72];   
	u8 csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数 取模与取余，不满一字节者补全为一字节	 
	if(size!=12&&size!=16&&size!=24)return;	//不支持的size /*需要修改这里*/
	Get_HzMat(font,dzk,size);	//得到相应大小的点阵数据 24表示该字涉及到24个像素
	for(t=0;t<csize;t++)//csize是不同字体大小所对应的正确的字节数
	{   												   
		temp=dzk[t];			//得到点阵数据                          
 		for(t1=0;t1<8;t1++)//单位是字节，总是八位的，通过移位来判断当前像素点是否画点
		{
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR); 
			temp<<=1;
			y++;
			if((y-y0)==size)//一列写完，方才罢休，步进x，写下一列
			{
				y=y0;
				x++;
				break;
			}
		}  	 
	}  
}

//显示一个指定任意大小的汉字
//x,y :汉字的坐标
//font:汉字GBK码，如何使用GB2313码
//size:字体大小
//mode:0,正常显示,1,叠加显示，显示在图片上	
//修改历史，溢出错误！
void Show_Font_Ex(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u16 temp,t,t1;
	u16 y0=y;
	uint8_t* dzk;//存放字节	//y要防止数据溢出
	u16 csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数 取模与取余，不满一字节者补全为一字节	 
	dzk = Get_HzofArbitrarySize(font,size);	//得到相应大小的点阵数据 24表示该字涉及到24个像素，由此可以获取任意大小的字模数据
	for(t=0;t<csize;t++)//csize是不同字体大小所对应的正确的字节数
	{   										   
		temp=dzk[t];			//得到点阵数据                          
		for(t1=0;t1<8;t1++)//单位是字节，总是八位的，通过移位来判断当前像素点是否画点
		{
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR); 
			temp<<=1;
			y++;
			if((y-y0)==size)//一列写完，方才罢休，步进x，写下一列
			{
				y=y0;
				x++;
				break;
			}
		}
//	LCD_Fast_DrawPoint(x,y,POINT_COLOR);
//	LCD_Fill(x,y,x+10,y+10,RED);
	}
//	LCD_Fill(x,y,x+30,y+30,YELLOW);
//	sprintf(str,"%d",t);
////	LCD_ShowChar(x,y,t+0x30,16,0);
//	LCD_ShowString(x,y,40,16,16,str);
}

//在指定位置开始显示一个字符串	    
//支持自动换行
//(x,y):起始坐标
//width,height:区域
//str  :字符串
//size :字体大小
//mode:0,非叠加方式;1,叠加方式    	   		   
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     //字符或者中文  	    				    				  	  
	while(*str!=0)//数据未结束
	{ 
			if(!bHz){//判断是否为汉字模块，若不是汉字，bHz不变化，仍是0，若是汉字，则变其为1，并在下一个else中还原
				if(*str>0x80)bHz=1;//中文 0x80为128 大于此数之后是汉字
				else{ //else后的确为字符 按字符显示的方法进行显示    
					if(x>(x0+width-size/2)){	//换行			   
						y+=size;
						x=x0;	   
					}							    
					if(y>(y0+height-size))break;//越界返回      
					if(*str==13){ //换行符号        
						y+=size;
						x=x0;
						str++; 
					}  
					else LCD_ShowChar(x,y,*str,size,mode);//有效部分写入 
					str++; 
					x+=size/2; //字符,为全字的一半 
				}
			}else{//这个情况时此字的确是中文      
					bHz=0;//有汉字库（？）此处归零是为了下一个字符的判断，将这个flag清零    
					if(x>(x0+width-size)){	//换行    
							y+=size;
							x=x0;		  
					}
					if(y>(y0+height-size))break;//越界返回  						     
					Show_Font(x,y,str,size,mode); //显示这个汉字,空心显示 
					str+=2; 
					x+=size;//下一个汉字偏移	    
			}						 
	}   
}

//在指定位置开始显示一个字符串	    
//支持自动换行
//(x,y):起始坐标
//width,height:区域
//str  :字符串
//size :字体大小
//mode:0,非叠加方式;1,叠加方式    	   		   
void Advanced_Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     //字符或者中文  	    				    				  	  
	while(*str!=0)//数据未结束
	{ 
			if(!bHz){//判断是否为汉字模块，若不是汉字，bHz不变化，仍是0，若是汉字，则变其为1，并在下一个else中还原
				if(*str>0x80)bHz=1;//中文 0x80为128 大于此数之后是汉字
				else{ //else后的确为字符 按字符显示的方法进行显示    
					if(x>(x0+width-size/2)){	//换行			   
						y+=size;
						x=x0;	   
					}							    
					if(y>(y0+height-size))break;//越界返回      
					if(*str==13){ //换行符号        
						y+=size;
						x=x0;
						str++; 
					}  
					else LCD_ShowChar_Ex(x,y,*str,size,mode);//有效部分写入 
					str++; 
					x+=size/2; //字符,为全字的一半 
				}
			}else{//这个情况时此字的确是中文      
					bHz=0;//有汉字库（？）此处归零是为了下一个字符的判断，将这个flag清零    
					if(x>(x0+width-size)){	//换行    
							y+=size;
							x=x0;		  
					}
					if(y>(y0+height-size))break;//越界返回  						     
					Show_Font_Ex(x,y,str,size,mode); //显示这个汉字,空心显示 
					str+=2; 
					x+=size;//下一个汉字偏移	    
			}						 
	}   
}

//订制显示三位数字
void Advanced_Show_Nbr(u16 x,u16 y,u16 Nbr,u8 size)
{
	char str[10];
	sprintf(str,"%3d",Nbr);
	Advanced_Show_Str(x,y,lcddev.width,size,(unsigned char*)str,size,0);
}

//在指定宽度的中间显示字符串
//如果字符长度超过了len,则用Show_Str显示
//len:指定要显示的宽度			  
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u16 len)//居中显示？
{
	u16 strlenth=0;
   	strlenth=strlen((const char*)str);
	strlenth*=size/2;
	if(strlenth>len)Show_Str(x,y,lcddev.width,lcddev.height,str,size,1);
	else
	{
		strlenth=(len-strlenth)/2;
	    Show_Str(strlenth+x,y,lcddev.width,lcddev.height,str,size,1);
	}
}   

//在指定宽度的中间显示字符串，字体大小任意
//如果字符长度超过了len,则用Show_Str显示
//len:指定要显示的宽度			  
void Advanced_Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u16 len,uint8_t mode)
{
	u16 strlenth=0;
   	strlenth=strlen((const char*)str);//一个汉字占两个字符的宽度，strlen严格描述一个字符串的字符宽度
	strlenth*=size/2;
	if(strlenth>len)Advanced_Show_Str(x,y,lcddev.width,lcddev.height,str,size,mode);
	else
	{
		strlenth=(len-strlenth)/2;
	    Advanced_Show_Str(strlenth+x,y,lcddev.width,lcddev.height,str,size,mode);
	}
}   

//居中对齐显示字符串
//(x,y)为中心坐标
//len 两臂的总宽度，臂展
void Stellar_Show_Str_Mid(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len)
{
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     //字符或者中文 
	u8 mode=1;		//0 非叠加，1 叠加
	u8 halfsize = size/2;
	u16 strlenth=strlen((const char*)str);//得到字符数，一个汉字占两个字符
	u8  charcnt = len/size;								//得到一行能显示的汉字数
	u8  ASCcharcnt = charcnt*2;						//一行能显示的ASC字符数
	u8  rownum = strlenth/(charcnt*2)+((strlenth%(charcnt*2))?1:0);//得到以汉字为度量的行数
	u16 rows_heig = rownum*size;					//显示这些字符总共所需的行数总高
	if(len<size)return; 									//如果显示的行太窄，则返回
	if((rows_heig/2>((y0<lcddev.height-y0)?y0:lcddev.height-y0))
		||(len/2>((x0<lcddev.width-x0)?x0:lcddev.width-x0)))return;
		//目前的字数若超出了既已有的范围，则返回
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
	}
}

//附加边框的居中对齐显示字符串
//(x,y)为中心坐标
//len 两臂的总宽度，臂展
//有默认的内径与框粗
void Remounted_Show_Str(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len)
{
	#ifndef _REMOUNT
	#define _REMOUNT
	
	#define STROKE 						14				//裱框宽度的像素数 
	#define INNER_DIAM_LEN		202				//默认内径长像素数
	#define INNER_DIAM_HEIG		52				//默认内径高像素数
	
	#endif
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     			//字符或者中文 
	u8 mode=1;					//0 非叠加，1 叠加	
	u8 halfsize = size/2;
	
	u16 sx_in,ex_in=0;
	u16 sy_in,ey_in=0;		//外框与内框的二四象限对角点坐标定义
	u16 sx_ex,ex_ex=0;
	u16 sy_ex,ey_ex=0;		
	
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
	ey_ex=ey_in+STROKE; //确定了四个坐标
	LCD_Fill(sx_in,sy_in,ex_in,ey_in,POOR); 							//显示衬底
	LCD_DrawBorder(sx_ex,sy_ex,ex_ex,ey_ex,STROKE,RED);		//显示外框
	
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
	}
}























		  






