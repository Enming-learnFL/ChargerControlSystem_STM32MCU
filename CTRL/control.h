#ifndef _CONTROL_H
#define _CONTROL_H

#include "sys.h"

/************************************************
 ALIENTEKս��STM32������
 
* ���׮����ϵͳ * 
 
 ���ߣ��߶��� @GaoEnMing

* @date    2021-xx-xx
* @brief   ���γ��׮�������
************************************************/
/////////////////////////////////////////////////////////////////////
//CRUCIAL CONTROL ����ϵͳ
//@Auther: Gao Enming
/////////////////////////////////////////////////////////////////////

/**-----------------------------------------------------------------------------------
 **
 *	��break;���λ��switch��ʱֻ������switch���飬����������while����
 *	�������������Կ��Ա�дUI�Ĳ�������
 *	����ֵ���ʽ��ֵ�Ǳ���ֵ�Ķ����ֵ������if(a=assign(whatever))������������ȿ����ж�
 *	����assign(whatever)�ķ���ֵ���ֿ��԰����ֵ����һ������������Ӷ����ݸ���һ������
 *	����Ϊ���Ͻ�����Ҫ�ڸ�����������һ���жϣ����շ����ʽ�ļ��㣡
 *	�������Ĺ����ǣ���������Ҫ�������ֵ��һ��ʹ��return����������ָ��������������߳���
 *	��		����˼�����Ǵ���Ĵ��룬����һ��ָ�룬��Ȼ��ͬ����ʱ���ֶ��������һ��ȡַ�������õ������ݺ�������
 *		�ֲ���ÿ��ͬ�����ݶ������ԭ����������
 *		STMFLASH_Write(SYNC_SHUTTLE_BASE,(uint16_t*)&pchargeStruct,sizeof(_Charge_Info)/2);
 *		ȥ��ȡַ���ܵõ���ȷ�Ĵ��룡���ע���ֽڵĵ�ַ���룡��
 */
/* The Programmer's Progress ----------------------------------------------------- */

#define bill(min) ((uint32_t)((0.75*min/(double)15)*100)) //�����üƼ�Ϊÿ15����0.75Ԫ����ÿСʱ3�� ����100��תΪ������

#define TEST		0X01	//����ģʽ
#define NORM		0X00	//Ѱ��ģʽ
//
#define TOKEN		0XAAAA	 //֡ͷ���������ж�EEPROM����������Ƿ���� ������Ϊ1100 1100 1100 1100������16λbit
#define nbrOfCharger 16  //ԭ�ͻ��ܹ�16�����׮�����0-15��[15:0]

/*------------------------------------------------------------------------------------
	SYNC_SHUTTLE ����ͬ����λ��STM32����FLASH�У���ַΪ0x0807F800
	ѡַ�����洢����ҳ255��ʼ0x0807F800��ҳ��ַ��ΧΪ 0x0807F800-0x0807FFFF����2K�ֽ�
-----------------------------------------------------------------------------------*/
#define SYNC_SHUTTLE_BASE 0x0807F800   //����֡��ͬ����λ��EEPROM�Ļ�ַ

#define seqNbr_default 2		 //Ĭ��ͣ��#2�ų��λ
#define time_default	 180	 //Ĭ��ͣ��180����

//�������ڴ�flash�д洢�����ݽṹ��
typedef struct Charge_State{
	uint16_t	ucFlag;									//���Լ�¼��Ӧ�ı�־λ�����ֽڱ��������ֽ����Լ�¼״̬
	uint32_t 	ucArray_ID;							//��ǰ���ڳ���׮��
	uint32_t 	timeStamp;							//������ʱ��ʱ���
} _State;
/* 
ucFlag��ʶ�ֽ� 16λ ��2�ֽ�
	\\*   ���ֽڱ��� Ϊ0X00   *     *���ֽ����Ա��ʹ�����*
		|15|14|13|12|11|10| 9| 8|   |7 |6 |5 |4 | 3| 2| 1| 0|
[7] 	ģʽλ		1:����ģʽ�� 0:��ʽ��չģʽʹ��

[6:5] (��������ʼΪ0)

[4]		����λ 	1:ά���� 0:����ʹ��

[3:1] (��������ʼΪ0)

[0]		����λ		1:ʹ���� 0:����״̬

ucFlag��ʼ״̬ӦΪ 0X0080 = 0000 0000 1000 0000(B)
���£����ֽ�д����׮��ţ�
*/



//��������֡�ṹ��
typedef volatile struct Charge_Info{
	uint16_t 		token;								//ͷ����ǣ������жϽṹ���Ƿ���Ȼ����
	uint16_t 		avail;								//��ǰ���õĳ��׮��Ŀ
	_State			stack[nbrOfCharger];	//�ṹ�����飬���ڴ�Ÿ������׮������
} _Charge_Info;
/**
		���׮����֡�ṹ�����ɲ���
	1.	token = 0XAAAA (1100 1100 1100 1100);
	2.	avail = 0x0000 ��ʼΪnbrOfCharger����0X0010����ʾ��ǰ���õĳ��׮����
	3.	stack[nbrOfCharger] ջ�����Դ��nbrOfCharger�����׮��ǰ��״̬
			ջ�����е�ÿ��Ԫ�ذ���Ҫ�������λ
		1) ucFlag = 0X0080��������λΪ����λ������Ĭ�ϲ���״̬����������״̬Ϊ0X0000
		2) ucArray_ID Ϊ��ID����ʼΪȫ��
		3) timeStamp Ϊ�����ʱ���������ʱ��ʼΪȫ�㣬����ΪȫF 0xFFFF FFFF
	
	@Attention:seqNbrΪ���׮��ţ�����Ҫ����������
*/

//������ԣ������ж���������ĺϷ���
#define IS_VALID_SEQNBR(seqNbr) ((seqNbr!=0xFF)&&(seqNbr<nbrOfCharger))
#define assert_param_seqNbr(seqNbr)	IS_VALID_SEQNBR(seqNbr)

extern _Charge_Info charge_Sync;

extern uint8_t 		seqNbr_Selected;				//�Ѿ�ȷ���ĺϷ����׮��
extern uint16_t 	chargeTime_Selected;		//�Ѿ�ȷ���ĺϷ����ʱ��

extern uint8_t		seqNbr_thbwhlSwitch;		//��ǰָ��ĳ��׮��
extern uint16_t		chargeTime_designated;	//��ǰѡ��ĳ��ʱ��

//�������ͣ�_Charge_Info���Խṹ��Ϊʵ�����ö���ĺ�����������intrude�ģ�����Χ�Ƶģ������Ĳ�������Χ�Ƶ�
void _Charge_Info_Init(_Charge_Info* pchargeStruct, uint8_t mode);
void _Charge_Info_Synchronize(_Charge_Info* pchargeStruct);
void _Charge_Info_unitReset(_Charge_Info* pchargeStruct,uint8_t seqNbr);
void _Charge_Info_unitSet(_Charge_Info* pchargeStruct,uint8_t seqNbr,uint16_t chargeTime_Selected,uint8_t* ucArray_ID);
void _Charge_Info_Proofread(_Charge_Info* pchargeStruct);
uint8_t _Charge_Info_Load(_Charge_Info* pchargeStruct);
uint8_t _Match_Info_UserID(_Charge_Info* pchargeStruct, uint8_t* ucArray_ID);

uint8_t Charge_Status_Check(_Charge_Info* pchargeStruct);
void Charger_MonitorHandler(void);//���׮���ӷ�������װ����:RTC_IRQHandler();

void SettingsBeforeCharge(void);
void theMainProcess(void);


//Ӳ���ԽӲ���
void Charger_ShutDown(uint8_t seqNbr);
void Charger_PowerOn(uint8_t seqNbr);
void Charger_UnLock(uint8_t seqNbr);
void Charger_LockUp(uint8_t seqNbr);
uint8_t Charger_Terminate(uint8_t seqNbr);
uint8_t Charger_Commence(uint8_t seqNbr);
#endif

