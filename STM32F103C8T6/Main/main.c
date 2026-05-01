#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "bsp_delay.h"
#include "bsp_key.h"
#include "bsp_oled.h"
//#include "bsp_dht11.h"
#include "bsp_Alarm.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "esp8266.h"
#include "onenet.h"
//#include "LightSensor.h"  // �޸ģ�ʹ��LightSensorͷ�ļ����Ѱ���ADC��ʼ����
#include "Motor.h"
#include "Servo.h"
#include "Serial.h"

//DHT11_Data_TypeDef DHT11_Data;
//char oled_Temp[16],oled_TempThr[16];
//char oled_Hum[16],oled_HumThr[16];
//char oled_Light[16];      // ������OLED��ʾ����ǿ�Ȼ�����
uint8_t key_value = 0;
//uint8_t Temp_Thr = 30;
//uint8_t Hum_Thr = 70;
char PUBLIS_BUF[256];
const char devPubTopic[] = "$sys/d8IdNXtA66/Test1/thing/property/post";
const char *devSubTopic[] = {"$sys/d8IdNXtA66/Test1/thing/property/set"};
unsigned char *dataPtr = NULL;
uint16_t TimeCount = 0;
//uint16_t Light = 0;// ����������ǿ�ȱ�������λ���տ�˹Lux����Χ0-999��
uint8_t DefectFlag = 0;  // 覴ü���־��1��ʾ��⵽覴�
char *p;
uint16_t DefectTotalCount = 0;      // �ۻ�ȱ�ݲ�Ʒ����
//char DefectDetail[100] = {0};       // ����覴������ַ�������ʽ�� "class1:2,class2:1"
// ����������覴����͵��ۼ�����
uint16_t OpenCircuit = 0;   // ��·覴��ۼ�
uint16_t Short = 0;         // ��·覴��ۼ�
uint16_t Spur = 0;          // ë��覴��ۼ�
char *token;
char *colon;
char *className;
int count;
//typedef enum{
//		
//	MAIN_MENU, //���˵�����
//	TEMP_SET,  //�¶���ֵ���ý���
//	HUM_SET,   //ʪ����ֵ���ý���
//}DisplayState;


//DisplayState currentState = MAIN_MENU;

/*
---------------------------------------------------------------------------------------------------------
*	�� �� ��: Bsp_init
*	����˵��: ����ģ��ĳ�ʼ������(���ڡ�OELD��������)
*	��    ������
*	�� �� ֵ: ��
---------------------------------------------------------------------------------------------------------
*/
void Bsp_init()
{
		//DHT11_Init();
		OLED_Init();
		Delay_Init();
		Key_Init();		
		Usart_Init();
		LED_Init();
//		LightSensor_Init();  // �޸ģ�ʹ��LightSensor��ʼ��������ADC��GPIO��ʼ����
		Motor_Init();		//ֱ�������ʼ��
		Servo_Init();
		Serial_Init();	
}
/*
---------------------------------------------------------------------------------------------------------
*	�� �� ��:  Oled_Show
*	����˵��: OLED��ʾ����ʾ��ʪ��������Ϣ
*	��    ������
*	�� �� ֵ: ��
---------------------------------------------------------------------------------------------------------
*/

void Oled_show()
{
//		 OLED_ShowCH(5,0,"��ʪ�Ȳɼ�ϵͳ");
//	
//		 if(DHT11_Read_TempAndHumidity(&DHT11_Data) == 1)
//		 {
//			 sprintf(oled_Temp,"Temp:%d.%d",DHT11_Data.temp_int,DHT11_Data.temp_deci);
//			 OLED_ShowCH(20,2,(char*)oled_Temp);
//			 sprintf(oled_Hum,"Hum:%d%%",DHT11_Data.humi_int);
//			 OLED_ShowCH(20,4,(char*)oled_Hum);		
//			 // ��������ȡ����ʾ����ǿ�ȣ��տ�˹��
//			 Light = LightSensor_GetValue();  // ��ȡ����ǿ��ֵ��0-999 Lux��
//			 sprintf(oled_Light,"Light:%dLux",Light);  // ��ʽ����ʾ
//			 OLED_ShowCH(20,6,(char*)oled_Light);			 
//		 }
		
}
/*
---------------------------------------------------------------------------------------------------------
*	�� �� ��:  Oled_Show1
*	����˵��: OLED��ʾ����ʾ�����¶���ֵ����
*	��    ������
*	�� �� ֵ: ��
---------------------------------------------------------------------------------------------------------
*/
void Oled_show1()
{
//		OLED_ShowCH(30,0,"�¶���ֵ");
//		if(key_value == 2)
//		{
//				if(Temp_Thr < 100)
//				{
//					Temp_Thr++;
//				}
//		}
//		else if(key_value == 3)
//		{
//				if(Temp_Thr > 0)
//				{
//					Temp_Thr--;
//				}			
//		}
//		sprintf(oled_TempThr,"Temp:%d",Temp_Thr);
//		OLED_ShowCH(30,4,(u8*)oled_TempThr);		
}
/*
---------------------------------------------------------------------------------------------------------
*	�� �� ��:  Oled_Show2
*	����˵��: OLED��ʾ����ʾ����ʪ����ֵ����
*	��    ������
*	�� �� ֵ: ��
---------------------------------------------------------------------------------------------------------
*/
void Oled_show2()
{
//		OLED_ShowCH(30,0,"ʪ����ֵ");
//		if(key_value == 2)
//		{
//				if(Hum_Thr < 100)
//				{
//					Hum_Thr++;
//				}
//		}
//		else if(key_value == 3)
//		{
//				if(Hum_Thr > 0)
//				{
//					Hum_Thr--;
//				}			
//		}
//		sprintf(oled_HumThr,"Hum:%d",Hum_Thr);
//		OLED_ShowCH(30,4,(u8*)oled_HumThr);		
}

/*
---------------------------------------------------------------------------------------------------------
*	�� �� ��: Oled_Switch()
*	����˵��: OLED�����л�����
*	��    ������
*	�� �� ֵ: ��
---------------------------------------------------------------------------------------------------------
*/
void Oled_Switch()
{
//	 key_value = Key_Scan(0);
//	 if(key_value == 1)
//	 {
//			currentState = (currentState + 1) % 3;	
//			OLED_Clear();
//	 }

//	 switch(currentState)
//	 {
//			case MAIN_MENU:
//					Oled_show();
//					break;
//			case TEMP_SET:
//					Oled_show1();
//					break;
//			case HUM_SET:	
//					Oled_show2();
//					break; 
//	 }
}

void JsonValue()
{
    memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
	
		sprintf(PUBLIS_BUF,"{\"id\":\"123\",\"params\":{\"DefectTotalCount\":{\"value\":%d},\"OpenCircuit\":{\"value\":%d},\"Short\":{\"value\":%d},\"Spur\":{\"value\":%d} }}",
					DefectTotalCount,OpenCircuit,Short,Spur);	
	
}

int main()
{
		Bsp_init();
		OLED_ShowCH(20,3,"����������..");	
		ESP8266_Init();
	
		while(OneNet_DevLink())//����Onenetƽ̨,���ʧ�ܵȴ�500ms�������ԡ�һֱʧ�ܻ���ѭ��
		{
			DelayXms(500);
		}
		
		OLED_Clear();		
		OLED_ShowCH(20,3,"�������ӳɹ�");
		//DelayXms(3000);
		//OLED_Clear();		
		/*��������*/
		OneNet_Subscribe(devSubTopic,1);
		while(1)
		{
			
//			Light = LightSensor_GetValue();  // ��ȡ����ǿ��ֵ��0-999 Lux��
			//�������
			key_value = Key_Scan(0);
			if(key_value==1)
			{
				Motor_SetSpeed(0); 
			}else if(key_value==2)
			{				
				Motor_SetSpeed(15); 
			}else if(key_value==3)
			{
			Motor_SetSpeed(30); 
			}
			
			
				Oled_Switch();			//�պ���
				if(++TimeCount >= 25)/*wait 25ms*/
				{
					JsonValue();
					OneNet_Publish(devPubTopic, PUBLIS_BUF);
					//ESP8266_Clear();
					TimeCount = 0;
				}					
				dataPtr = ESP8266_GetIPD(2);//wait 10ms 
				if(dataPtr != NULL)
					OneNet_RevPro(dataPtr);

				//================================UART����=======================
				// ��������ָ�Qt���͵�@1\r\nָ�
				if (Serial_RxFlag == 1)  // �����յ��������ݰ�
				{
					// ���ԣ���ӡԭʼ�������ݣ��鿴ʵ���յ������ݣ�
					Serial_Printf("ԭʼ�������ݣ�%s\r\n", Serial_RxPacket);					
					// ����������覴ü��ָ�ָ���ʽ "@D1\r\n" �� "@D0\r\n"��							
					if (Serial_RxPacket[0] == 'D') {
							if (Serial_RxPacket[1] == '1')
							{
												DefectFlag = 1;												
												DefectTotalCount++;// �ۻ�ȱ�ݲ�Ʒ���� +1
												// ���������ַ�������ʽ��D1,OpenCircuit:2,Short:1,Spur:3��
												p = strchr((char*)Serial_RxPacket, ',');
												if (p != NULL) {
															p++;  // ��������															
															token = strtok(p, ",");// �����ŷָ����覴���
															while (token != NULL) {																	
															colon = strchr(token, ':');// ��ð�ŷָ���������
															if (colon != NULL) {
																	*colon = '\0';  // ��ð���滻Ϊ�����������Ϊ�����
																	className = token;
																	count = atoi(colon + 1);													
													// ����������ۼӵ���Ӧ�ı���
													if (strcmp(className, "open_circuit") == 0) {
															//OpenCircuit += count;
																OpenCircuit += count;
													} else if (strcmp(className, "short") == 0) {
															//Short += count;
																Short += count;
													} else if (strcmp(className, "spur") == 0) {
															//Spur += count;
																Spur += count;
													}
													// ����չ�������
											}
											token = strtok(NULL, ",");
														}
												}
												Serial_Printf("�յ�覴�ָ��ܼ���=%d, OpenCircuit=%d, Short=%d, Spur=%d\r\n",
														DefectTotalCount, OpenCircuit, Short, Spur);
								} 
								else if (Serial_RxPacket[1] == '0') 
								{
									DefectFlag = 0;
									Serial_Printf("�յ�覴�ָ���覴�\r\n");
								}
						}					
						// ����@���ţ�ƥ��"1"���Ƽ���
						else if (Serial_RxPacket[0] == '1')  
						{							 
								Serial_Printf("�յ�ָ����/����Ѹ���\r\n");
						}    
						
					Serial_RxPacket[0] = '\0'; // ��ս��ջ��棨���������
					Serial_RxFlag = 0;         // ������ձ�־					
				}				
				
				// ========== 覴ü�������ƣ������ڰ����� ==========
        if (DefectFlag == 1)  // ��⵽覴�
        {
            // ������������ٰڶ���0�� -> 180�� -> 0�ȣ�
            Servo_SetAngle(180);    // ת��180��
						//Motor_SetSpeed(0);			// ���ֹͣ
            DelayXms(300);          // �ȴ�300ms
            Servo_SetAngle(0);      // ת��0��
					
            //DelayXms(1000);
           // Motor_SetSpeed(30);                    
            
            DefectFlag = 0;  // ִ����Ϻ������־�������ظ�����
            Serial_Printf("覴ò�Ʒ�޳��������\r\n");
        }
				//=======================����===================================
				
		}
}





