#include "sys.h" 
#include "fontupd.h"
#include "w25qxx.h"
#include "lcd.h"
#include "text.h"	
#include "string.h"												    
#include "usart.h"												    
/************************************************
 ALIENTEKս��STM32������
 
* ���׮����ϵͳ * 
 
 ���ߣ��߶��� @GaoEnMing

* @date    2021-xx-xx
* @brief   ���γ��׮�������
* All rights reserved	
//�ο�����ԭ�ӵ������������벢�Լ���д
************************************************/
 
//code �ַ�ָ�뿪ʼ
//���ֿ��в��ҳ���ģ
//code �ַ����Ŀ�ʼ��ַ,GBK��
//mat  ���ݴ�ŵ�ַ (size/8+((size%8)?1:0))*(size) bytes��С	
//size:�����С
void Get_HzMat(unsigned char *code,unsigned char *mat,u8 size)//unsigned char ����u8
{		    
	unsigned char qh,ql;
	unsigned char i;					  
	unsigned long foffset; 
	u8 csize=(size/8+((size%8)?1:0))*(size);//�õ�����һ���ַ���Ӧ������ռ���ֽ���	 
	qh=*code;
	ql=*(++code);
	if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//�� ���ú���
	{   		    
	    for(i=0;i<csize;i++)*mat++=0x00;//�������
	    return; //��������
	}          
	if(ql<0x7f)ql-=0x40;//ע��!
	else ql-=0x41;
	qh-=0x81;   
	foffset=((unsigned long)190*qh+ql)*csize;	//�õ��ֿ��е��ֽ�ƫ����  		  
	switch(size)
	{
		case 12:
			W25QXX_Read(mat,foffset+ftinfo.f12addr,csize);
			break;
		case 16:
			W25QXX_Read(mat,foffset+ftinfo.f16addr,csize);
			break;
		case 24:
			W25QXX_Read(mat,foffset+ftinfo.f24addr,csize);//ftinfo��font info������Ź�������������Ϣ����fontupd�ж���
			break;//ftinfo.f24addr�����������ⲿflash�д�ŵ������ֿ��λ�ã���������ԭ�Ӳ�û��͸¶�����ǣ�
			
	}     												    
}

//���ֿ��в��ҳ���׼��Դ��ģ��Ĭ��Ϊ24*24��GBK
//code �ַ����Ŀ�ʼ��ַ,GBK��//code �ַ�ָ�뿪ʼ
//mat  ���ݴ�ŵ�ַ (size/8+((size%8)?1:0))*(size) bytes��С	����ȡģ��һ�в���ʱ����һ�ֽ������㣬��������ʵ��column
//size:�����С
void Get_SrcHzMat(unsigned char *code,unsigned char *mat)//unsigned char ����u8
{	
	#ifndef _STD_SRC_SIZE
	#define STD_SRC_SIZE 24	//��24Ϊ��׼��Դ���Ա�����
	#endif
	unsigned char qh,ql;
	unsigned char i;					  
	unsigned long foffset; 
	u8 csize=(STD_SRC_SIZE/8+((STD_SRC_SIZE%8)?1:0))*(STD_SRC_SIZE);//�õ�����һ�������ַ���Ӧ������ռ���ֽ���	 
	qh=*code;
	ql=*(++code);
	if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//�� ���ú���
	{   		    
	    for(i=0;i<csize;i++)*mat++=0x00;//�������
	    return; //��������
	}          
	if(ql<0x7f)ql-=0x40;//ע��!
	else ql-=0x41;
	qh-=0x81;   
	foffset=((unsigned long)190*qh+ql)*csize;	//�õ��ֿ��е��ֽ�ƫ����  		  
	W25QXX_Read(mat,foffset+ftinfo.f24addr,csize);//��ȡ��������ݴ�ŵ�ַ    matΪ�������ݵĵط�												    
}

uint8_t* Get_HzofArbitrarySize(unsigned char *code,uint8_t size)
{
	#ifndef _STD_SRC_SIZE
	#define STD_SRC_SIZE 24	//��24Ϊ��׼��Դ���Ա�����
	#endif
	uint8_t SrcFont24[72];//��������ȡ��ַ����������uint8_t(*)[72]����һ�����鱾��Ҫ����ȷ��ֵ��Ҫǿ������ת��
	Get_SrcHzMat(code,(uint8_t*)&SrcFont24);//�����������������������ָ����Ԫ�ص�ָ�룬������uint8_t*�����������������
	return zoomChar(STD_SRC_SIZE,STD_SRC_SIZE,size,size,(uint8_t*)&SrcFont24,1);
}

/***********************��������****************************/
//#define ZOOMMAXBUFF 16384
//uint8_t zoomBuff[ZOOMMAXBUFF] = {0};	//�������ŵĻ��棬���֧�ֵ�128*128
//uint8_t zoomTempBuff[1024] = {0};

/**
 * @brief  ������ģ�����ź����ģ��1�����ص���8������λ����ʾ
										0x01��ʾ�ʼ���0x00��ʾ�հ���
 * @param  in_width ��ԭʼ�ַ����
 * @param  in_heig ��ԭʼ�ַ��߶�
 * @param  out_width �����ź���ַ����
 * @param  out_heig�����ź���ַ��߶�
 * @param  in_ptr ���ֿ�����ָ��	ע�⣺1pixel 1bit
 * @param  out_ptr �����ź���ַ����ָ�� ע��: 1pixel 8bit
 *		out_ptrʵ����û������������ĳ���ֱ�������ȫ��ָ��zoomBuff��
 * @param  en_cn ��0ΪӢ�ģ�1Ϊ����
 * @retval ��
 */
uint8_t* zoomChar(uint16_t in_width,	//ԭʼ�ַ����
									uint16_t in_heig,		//ԭʼ�ַ��߶�
									uint16_t out_width,	//���ź���ַ����
									uint16_t out_heig,	//���ź���ַ��߶�
									uint8_t *in_ptr,	//�ֿ�����ָ��	ע�⣺1pixel 1bit
									/*uint8_t *out_ptr,*/ //���ź���ַ����ָ�� ע��: 1pixel 8bit
									uint8_t en_cn)		//0ΪӢ�ģ�1Ϊ����	
{
	#define ZOOMMAXBUFF 16384 //128*128
	#define ZOOMMAXBYTE (128/8)*128 //����ַ������洢�ռ�
	//static uint8_t zoomBuff[ZOOMMAXBUFF] = {0};	//���֧�ֵ�128*128 ��̬���������ݲ������ź������ý������ͷ�
	static uint8_t zoomTempBuff[1024] = {0};//�������ŵĻ��棬Դ����������Ϊ32*32,һ����Ա�Խ�һ������
	static uint8_t objchar[ZOOMMAXBYTE] = {0};//���ź��ַ����洢��λ�ã����ֽڵ���ʽ����,����������
	//�ں������װ���ϵľ�̬����

	
	uint8_t *pts,*ots;
	//����Դ��ģ��Ŀ����ģ��С���趨����������ӣ�����16��Ϊ�˰Ѹ�������ת�ɶ������㣬�Ӷ��ܹ����־���
	unsigned int xrIntFloat_16=(in_width<<16)/out_width+1; 
  unsigned int yrIntFloat_16=(in_heig<<16)/out_heig+1;
	
	unsigned int srcx_16=0;
	unsigned int y,x;
	uint8_t *pSrcLine;
	uint8_t *pSrcColumn; //ָ���е�ָ�룬��Ϊ�ز�����������ȡģ
	
	uint16_t byteCount,bitCount;
	uint16_t ocsize;//����ַ����ֽ���
	//����Ҫ�ڿ�ִ�����֮ǰ��������˵�д��ϰ��
	
	//��̬������ÿ�ε��ú�����ʱ�������
	memset(zoomTempBuff,0,sizeof(zoomTempBuff));
	memset(objchar,0,sizeof(objchar));//ÿ��ʹ��ʱԤ������
	//�������Ƿ�Ϸ�
	if(in_width >= 32) return NULL;												//�ֿⲻ������32����
	if(in_width * in_heig == 0) return NULL;								//�����������ݺϷ��Լ��
	if(in_width * in_heig >= 1024 ) return NULL; 					//����������� 32*32
	
	if(out_width * out_heig == 0) return NULL;	
	if(out_width * out_heig >= ZOOMMAXBUFF ) return NULL; //����������� 128*128
	pts = (uint8_t*)&zoomTempBuff;										//ת������ݴ�����
	
	//Ϊ�������㣬�ֿ��������1 pixel/1bit ӳ�䵽1pixel/8bit
	//0x01��ʾ�ʼ���0x00��ʾ�հ���
	if(en_cn == 0x00)//Ӣ��
	{ //��Ϊ��������ȡģ�������Ҫ�Ը߶ȸ����ע���߶ȵĲ��ֲ���Ҫ�����ͼ�����Ӧ���ֽ�
		//Ӣ�ĺ������ֿ����±߽粻�ԣ����ڴ˴���������Ҫע��tempBuff��ֹ���
		ocsize = (out_heig/8+((out_heig%8)?1:0))*out_width;//�������Ӧ���ǹ淶��
		for(byteCount=0;byteCount<(in_heig/8+((in_heig%8)?1:0))*in_width;byteCount++)	
			{//Ӣ���ַ��ĳ���ʼ��Ϊ2��1
				for(bitCount=0;bitCount<8;bitCount++)
					{						
						//��Դ��ģ������λӳ�䵽�ֽ�
						//in_ptr��bitXΪ1����pts�������ֽ�ֵΪ1
						//in_ptr��bitXΪ0����pts�������ֽ�ֵΪ0
						*pts++ = (in_ptr[byteCount] & (0x80>>bitCount))?1:0; //�������ٸ�ֵ
					}
			}				
	}
	else //����
	{		ocsize = (out_heig/8+((out_heig%8)?1:0))*out_width;	//�淶�������ַ�����ı���Ϊ1��1
			for(byteCount=0;byteCount<(in_heig/8+((in_heig%8)?1:0))*in_width;byteCount++)	
			{
				for(bitCount=0;bitCount<8;bitCount++)
					{						
						//��Դ��ģ������λӳ�䵽�ֽ�
						//in_ptr��bitXΪ1����pts�������ֽ�ֵΪ1
						//in_ptr��bitXΪ0����pts�������ֽ�ֵΪ0
						*pts++ = (in_ptr[byteCount] & (0x80>>bitCount))?1:0; //����������ͣ�������ΪԴ��ģ����������ȷ�ģ�
					}
			}		
	}

	//zoom���ŵĹ���
	pts = (uint8_t*)&zoomTempBuff;	//ӳ����Դ����ָ��
	//ots = (uint8_t*)&zoomBuff;	//������ݵ�ָ��
	//����ָ��ķǳ���ķ�ʽ
	//ots��һ���������飬ÿһ����Ա����һ��ָ�룬ָ�������Ҫ���ĵ���������ڵĴ洢λ�ã�һ����Ա�����������ǹ���һ�����ص�
	for (x=0,byteCount=0;x<out_width;x++)	/*�б���*/
    {
				unsigned int srcy_16=0;//ÿһ�δ���ѭ��ʱ����Ҫ���������ָ�������
        pSrcColumn=(pts+in_heig*(srcx_16>>16));	//Դ�л���ַ�����д�ŵ���һ����ַ		
        for (y=0,bitCount=0;y<out_heig;y++,bitCount++) /*�������ر���*/
        {
						if(!(bitCount<8)){
							bitCount = 0;
							byteCount++;
						}//�з�����ʱ����ʾ�˴��Ѿ��й�ȡַ����
            /*ots[y]=pSrcColumn[srcy_16>>16];*/ //��Դ��ģ���ݸ��Ƶ�Ŀ��ָ����
						if((pSrcColumn[srcy_16>>16])==1)objchar[byteCount]|=(0x80>>bitCount);//��Ӧ��bit��λ
            srcy_16+=yrIntFloat_16;			//������ƫ��Դ���ص㣬����һ��ƫ����λ�����Ĺ�����
        }
        srcx_16+=xrIntFloat_16;				  //������ƫ��Դ���ص�
        ots+=out_heig;//ÿ��һ��һ��������ֵ�ӣ����Ǳ�ʾ��ַ���ƶ�
				byteCount++;
    }
	/*���������ź����ģ����ֱ�Ӵ洢��ȫ��ָ��zoomBuff����*/
	//out_ptr = (uint8_t*)&zoomBuff;	//out_ptrû����ȷ�������������ֱ�Ӹĳ���ȫ�ֱ���ָ�룡
	
	/*ʵ�������ʹ��out_ptr����Ҫ������һ�䣡����
		ֻ����Ϊout_ptrû��ʹ�ã��ᵼ��warning��ǿ��֢*/
	//out_ptr++; 
		
		
		
	ocsize++;
	pSrcLine++;
		
		return (uint8_t*)&objchar;
}			



//��ʾһ��ָ����С�ĺ���
//x,y :���ֵ�����
//font:����GBK�룬���ʹ��GB2313��
//size:�����С
//mode:0,������ʾ,1,������ʾ	   
void Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1;
	u16 y0=y;
	u8 dzk[72];   
	u8 csize=(size/8+((size%8)?1:0))*(size);//�õ�����һ���ַ���Ӧ������ռ���ֽ��� ȡģ��ȡ�࣬����һ�ֽ��߲�ȫΪһ�ֽ�	 
	if(size!=12&&size!=16&&size!=24)return;	//��֧�ֵ�size /*��Ҫ�޸�����*/
	Get_HzMat(font,dzk,size);	//�õ���Ӧ��С�ĵ������� 24��ʾ�����漰��24������
	for(t=0;t<csize;t++)//csize�ǲ�ͬ�����С����Ӧ����ȷ���ֽ���
	{   												   
		temp=dzk[t];			//�õ���������                          
 		for(t1=0;t1<8;t1++)//��λ���ֽڣ����ǰ�λ�ģ�ͨ����λ���жϵ�ǰ���ص��Ƿ񻭵�
		{
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR); 
			temp<<=1;
			y++;
			if((y-y0)==size)//һ��д�꣬���Ű��ݣ�����x��д��һ��
			{
				y=y0;
				x++;
				break;
			}
		}  	 
	}  
}

//��ʾһ��ָ�������С�ĺ���
//x,y :���ֵ�����
//font:����GBK�룬���ʹ��GB2313��
//size:�����С
//mode:0,������ʾ,1,������ʾ����ʾ��ͼƬ��	
//�޸���ʷ���������
void Show_Font_Ex(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u16 temp,t,t1;
	u16 y0=y;
	uint8_t* dzk;//����ֽ�	//yҪ��ֹ�������
	u16 csize=(size/8+((size%8)?1:0))*(size);//�õ�����һ���ַ���Ӧ������ռ���ֽ��� ȡģ��ȡ�࣬����һ�ֽ��߲�ȫΪһ�ֽ�	 
	dzk = Get_HzofArbitrarySize(font,size);	//�õ���Ӧ��С�ĵ������� 24��ʾ�����漰��24�����أ��ɴ˿��Ի�ȡ�����С����ģ����
	for(t=0;t<csize;t++)//csize�ǲ�ͬ�����С����Ӧ����ȷ���ֽ���
	{   										   
		temp=dzk[t];			//�õ���������                          
		for(t1=0;t1<8;t1++)//��λ���ֽڣ����ǰ�λ�ģ�ͨ����λ���жϵ�ǰ���ص��Ƿ񻭵�
		{
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR); 
			temp<<=1;
			y++;
			if((y-y0)==size)//һ��д�꣬���Ű��ݣ�����x��д��һ��
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

//��ָ��λ�ÿ�ʼ��ʾһ���ַ���	    
//֧���Զ�����
//(x,y):��ʼ����
//width,height:����
//str  :�ַ���
//size :�����С
//mode:0,�ǵ��ӷ�ʽ;1,���ӷ�ʽ    	   		   
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     //�ַ���������  	    				    				  	  
	while(*str!=0)//����δ����
	{ 
			if(!bHz){//�ж��Ƿ�Ϊ����ģ�飬�����Ǻ��֣�bHz���仯������0�����Ǻ��֣������Ϊ1��������һ��else�л�ԭ
				if(*str>0x80)bHz=1;//���� 0x80Ϊ128 ���ڴ���֮���Ǻ���
				else{ //else���ȷΪ�ַ� ���ַ���ʾ�ķ���������ʾ    
					if(x>(x0+width-size/2)){	//����			   
						y+=size;
						x=x0;	   
					}							    
					if(y>(y0+height-size))break;//Խ�緵��      
					if(*str==13){ //���з���        
						y+=size;
						x=x0;
						str++; 
					}  
					else LCD_ShowChar(x,y,*str,size,mode);//��Ч����д�� 
					str++; 
					x+=size/2; //�ַ�,Ϊȫ�ֵ�һ�� 
				}
			}else{//������ʱ���ֵ�ȷ������      
					bHz=0;//�к��ֿ⣨�����˴�������Ϊ����һ���ַ����жϣ������flag����    
					if(x>(x0+width-size)){	//����    
							y+=size;
							x=x0;		  
					}
					if(y>(y0+height-size))break;//Խ�緵��  						     
					Show_Font(x,y,str,size,mode); //��ʾ�������,������ʾ 
					str+=2; 
					x+=size;//��һ������ƫ��	    
			}						 
	}   
}

//��ָ��λ�ÿ�ʼ��ʾһ���ַ���	    
//֧���Զ�����
//(x,y):��ʼ����
//width,height:����
//str  :�ַ���
//size :�����С
//mode:0,�ǵ��ӷ�ʽ;1,���ӷ�ʽ    	   		   
void Advanced_Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     //�ַ���������  	    				    				  	  
	while(*str!=0)//����δ����
	{ 
			if(!bHz){//�ж��Ƿ�Ϊ����ģ�飬�����Ǻ��֣�bHz���仯������0�����Ǻ��֣������Ϊ1��������һ��else�л�ԭ
				if(*str>0x80)bHz=1;//���� 0x80Ϊ128 ���ڴ���֮���Ǻ���
				else{ //else���ȷΪ�ַ� ���ַ���ʾ�ķ���������ʾ    
					if(x>(x0+width-size/2)){	//����			   
						y+=size;
						x=x0;	   
					}							    
					if(y>(y0+height-size))break;//Խ�緵��      
					if(*str==13){ //���з���        
						y+=size;
						x=x0;
						str++; 
					}  
					else LCD_ShowChar_Ex(x,y,*str,size,mode);//��Ч����д�� 
					str++; 
					x+=size/2; //�ַ�,Ϊȫ�ֵ�һ�� 
				}
			}else{//������ʱ���ֵ�ȷ������      
					bHz=0;//�к��ֿ⣨�����˴�������Ϊ����һ���ַ����жϣ������flag����    
					if(x>(x0+width-size)){	//����    
							y+=size;
							x=x0;		  
					}
					if(y>(y0+height-size))break;//Խ�緵��  						     
					Show_Font_Ex(x,y,str,size,mode); //��ʾ�������,������ʾ 
					str+=2; 
					x+=size;//��һ������ƫ��	    
			}						 
	}   
}

//������ʾ��λ����
void Advanced_Show_Nbr(u16 x,u16 y,u16 Nbr,u8 size)
{
	char str[10];
	sprintf(str,"%3d",Nbr);
	Advanced_Show_Str(x,y,lcddev.width,size,(unsigned char*)str,size,0);
}

//��ָ����ȵ��м���ʾ�ַ���
//����ַ����ȳ�����len,����Show_Str��ʾ
//len:ָ��Ҫ��ʾ�Ŀ��			  
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u16 len)//������ʾ��
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

//��ָ����ȵ��м���ʾ�ַ����������С����
//����ַ����ȳ�����len,����Show_Str��ʾ
//len:ָ��Ҫ��ʾ�Ŀ��			  
void Advanced_Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u16 len,uint8_t mode)
{
	u16 strlenth=0;
   	strlenth=strlen((const char*)str);//һ������ռ�����ַ��Ŀ�ȣ�strlen�ϸ�����һ���ַ������ַ����
	strlenth*=size/2;
	if(strlenth>len)Advanced_Show_Str(x,y,lcddev.width,lcddev.height,str,size,mode);
	else
	{
		strlenth=(len-strlenth)/2;
	    Advanced_Show_Str(strlenth+x,y,lcddev.width,lcddev.height,str,size,mode);
	}
}   

//���ж�����ʾ�ַ���
//(x,y)Ϊ��������
//len ���۵��ܿ�ȣ���չ
void Stellar_Show_Str_Mid(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len)
{
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     //�ַ��������� 
	u8 mode=1;		//0 �ǵ��ӣ�1 ����
	u8 halfsize = size/2;
	u16 strlenth=strlen((const char*)str);//�õ��ַ�����һ������ռ�����ַ�
	u8  charcnt = len/size;								//�õ�һ������ʾ�ĺ�����
	u8  ASCcharcnt = charcnt*2;						//һ������ʾ��ASC�ַ���
	u8  rownum = strlenth/(charcnt*2)+((strlenth%(charcnt*2))?1:0);//�õ��Ժ���Ϊ����������
	u16 rows_heig = rownum*size;					//��ʾ��Щ�ַ��ܹ�����������ܸ�
	if(len<size)return; 									//�����ʾ����̫խ���򷵻�
	if((rows_heig/2>((y0<lcddev.height-y0)?y0:lcddev.height-y0))
		||(len/2>((x0<lcddev.width-x0)?x0:lcddev.width-x0)))return;
		//Ŀǰ�������������˼����еķ�Χ���򷵻�
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
	}
}

//���ӱ߿�ľ��ж�����ʾ�ַ���
//(x,y)Ϊ��������
//len ���۵��ܿ�ȣ���չ
//��Ĭ�ϵ��ھ�����
void Remounted_Show_Str(uint16_t x,uint16_t y,uint8_t* str, uint8_t size, uint16_t len)
{
	#ifndef _REMOUNT
	#define _REMOUNT
	
	#define STROKE 						14				//�ѿ��ȵ������� 
	#define INNER_DIAM_LEN		202				//Ĭ���ھ���������
	#define INNER_DIAM_HEIG		52				//Ĭ���ھ���������
	
	#endif
	u16 x0=x;
	u16 y0=y;							  	  
  u8 bHz=0;     			//�ַ��������� 
	u8 mode=1;					//0 �ǵ��ӣ�1 ����	
	u8 halfsize = size/2;
	
	u16 sx_in,ex_in=0;
	u16 sy_in,ey_in=0;		//������ڿ�Ķ������޶Խǵ����궨��
	u16 sx_ex,ex_ex=0;
	u16 sy_ex,ey_ex=0;		
	
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
	ey_ex=ey_in+STROKE; //ȷ�����ĸ�����
	LCD_Fill(sx_in,sy_in,ex_in,ey_in,POOR); 							//��ʾ�ĵ�
	LCD_DrawBorder(sx_ex,sy_ex,ex_ex,ey_ex,STROKE,RED);		//��ʾ���
	
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
	}
}























		  






