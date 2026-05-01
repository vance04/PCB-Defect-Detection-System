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
//#include "LightSensor.h"  // 修改：使用LightSensor头文件（已包含ADC初始化）
#include "Motor.h"
#include "Servo.h"
#include "Serial.h"

//DHT11_Data_TypeDef DHT11_Data;
//char oled_Temp[16],oled_TempThr[16];
//char oled_Hum[16],oled_HumThr[16];
//char oled_Light[16];      // 新增：OLED显示光照强度缓冲区
uint8_t key_value = 0;
//uint8_t Temp_Thr = 30;
//uint8_t Hum_Thr = 70;
char PUBLIS_BUF[256];
const char devPubTopic[] = "$sys/d8IdNXtA66/Test1/thing/property/post";
const char *devSubTopic[] = {"$sys/d8IdNXtA66/Test1/thing/property/set"};
unsigned char *dataPtr = NULL;
uint16_t TimeCount = 0;
//uint16_t Light = 0;// 新增：光照强度变量（单位：勒克斯Lux，范围0-999）
uint8_t DefectFlag = 0;  // 瑕疵检测标志，1表示检测到瑕疵
char *p;
uint16_t DefectTotalCount = 0;      // 累积缺陷产品数量
//char DefectDetail[100] = {0};       // 本次瑕疵详情字符串，格式如 "class1:2,class2:1"
// 新增：三种瑕疵类型的累计数量
uint16_t OpenCircuit = 0;   // 开路瑕疵累计
uint16_t Short = 0;         // 短路瑕疵累计
uint16_t Spur = 0;          // 毛刺瑕疵累计
char *token;
char *colon;
char *className;
int count;
//typedef enum{
//		
//	MAIN_MENU, //主菜单界面
//	TEMP_SET,  //温度阈值设置界面
//	HUM_SET,   //湿度阈值设置界面
//}DisplayState;


//DisplayState currentState = MAIN_MENU;

/*
---------------------------------------------------------------------------------------------------------
*	函 数 名: Bsp_init
*	功能说明: 各个模块的初始化函数(串口、OELD、按键等)
*	参    数：无
*	返 回 值: 无
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
//		LightSensor_Init();  // 修改：使用LightSensor初始化（包含ADC和GPIO初始化）
		Motor_Init();		//直流电机初始化
		Servo_Init();
		Serial_Init();	
}
/*
---------------------------------------------------------------------------------------------------------
*	函 数 名:  Oled_Show
*	功能说明: OLED显示屏显示温湿度数据信息
*	参    数：无
*	返 回 值: 无
---------------------------------------------------------------------------------------------------------
*/

void Oled_show()
{
//		 OLED_ShowCH(5,0,"温湿度采集系统");
//	
//		 if(DHT11_Read_TempAndHumidity(&DHT11_Data) == 1)
//		 {
//			 sprintf(oled_Temp,"Temp:%d.%d",DHT11_Data.temp_int,DHT11_Data.temp_deci);
//			 OLED_ShowCH(20,2,(char*)oled_Temp);
//			 sprintf(oled_Hum,"Hum:%d%%",DHT11_Data.humi_int);
//			 OLED_ShowCH(20,4,(char*)oled_Hum);		
//			 // 新增：获取并显示光照强度（勒克斯）
//			 Light = LightSensor_GetValue();  // 获取光照强度值（0-999 Lux）
//			 sprintf(oled_Light,"Light:%dLux",Light);  // 格式化显示
//			 OLED_ShowCH(20,6,(char*)oled_Light);			 
//		 }
		
}
/*
---------------------------------------------------------------------------------------------------------
*	函 数 名:  Oled_Show1
*	功能说明: OLED显示屏显示设置温度阈值界面
*	参    数：无
*	返 回 值: 无
---------------------------------------------------------------------------------------------------------
*/
void Oled_show1()
{
//		OLED_ShowCH(30,0,"温度阈值");
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
*	函 数 名:  Oled_Show2
*	功能说明: OLED显示屏显示设置湿度阈值界面
*	参    数：无
*	返 回 值: 无
---------------------------------------------------------------------------------------------------------
*/
void Oled_show2()
{
//		OLED_ShowCH(30,0,"湿度阈值");
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
*	函 数 名: Oled_Switch()
*	功能说明: OLED界面切换函数
*	参    数：无
*	返 回 值: 无
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
		OLED_ShowCH(20,3,"网络连接中..");	
		ESP8266_Init();
	
		while(OneNet_DevLink())//连接Onenet平台,如果失败等待500ms继续尝试。一直失败会死循环
		{
			DelayXms(500);
		}
		
		OLED_Clear();		
		OLED_ShowCH(20,3,"网络连接成功");
		//DelayXms(3000);
		//OLED_Clear();		
		/*订阅主题*/
		OneNet_Subscribe(devSubTopic,1);
		while(1)
		{
			
//			Light = LightSensor_GetValue();  // 获取光照强度值（0-999 Lux）
			//电机控制
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
			
			
				Oled_Switch();			//空函数
				if(++TimeCount >= 25)/*Oled_Switch大概需要耗时10ms,每5s进一次这个逻辑*/
				{
					JsonValue();
					OneNet_Publish(devPubTopic, PUBLIS_BUF);
					ESP8266_Clear();
					TimeCount = 0;
				}					
				dataPtr = ESP8266_GetIPD(2);
				if(dataPtr != NULL)
					OneNet_RevPro(dataPtr);

				//================================UART串口=======================
				// 解析串口指令（Qt发送的@1\r\n指令）
				if (Serial_RxFlag == 1)  // 串口收到完整数据包
				{
					// 调试：打印原始接收数据（查看实际收到的内容）
					Serial_Printf("原始接收数据：%s\r\n", Serial_RxPacket);					
					// 新增：解析瑕疵检测指令（指令格式 "@D1\r\n" 或 "@D0\r\n"）							
					if (Serial_RxPacket[0] == 'D') {
							if (Serial_RxPacket[1] == '1')
							{
												DefectFlag = 1;												
												DefectTotalCount++;// 累积缺陷产品计数 +1
												// 解析详情字符串（格式：D1,OpenCircuit:2,Short:1,Spur:3）
												p = strchr((char*)Serial_RxPacket, ',');
												if (p != NULL) {
															p++;  // 跳过逗号															
															token = strtok(p, ",");// 按逗号分割各个瑕疵项
															while (token != NULL) {																	
															colon = strchr(token, ':');// 按冒号分割类别和数量
															if (colon != NULL) {
																	*colon = '\0';  // 将冒号替换为结束符，左边为类别名
																	className = token;
																	count = atoi(colon + 1);													
													// 根据类别名累加到对应的变量
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
													// 可扩展其他类别
											}
											token = strtok(NULL, ",");
														}
												}
												Serial_Printf("收到瑕疵指令：总计数=%d, OpenCircuit=%d, Short=%d, Spur=%d\r\n",
														DefectTotalCount, OpenCircuit, Short, Spur);
								} 
								else if (Serial_RxPacket[1] == '0') 
								{
									DefectFlag = 0;
									Serial_Printf("收到瑕疵指令：无瑕疵\r\n");
								}
						}					
						// 忽略@符号，匹配"1"（推荐）
						else if (Serial_RxPacket[0] == '1')  
						{							 
								Serial_Printf("收到指令：电机/舵机已更新\r\n");
						}    
						
					Serial_RxPacket[0] = '\0'; // 清空接收缓存（避免残留）
					Serial_RxFlag = 0;         // 清除接收标志					
				}				
				
				// ========== 瑕疵检测舵机控制（独立于按键） ==========
        if (DefectFlag == 1)  // 检测到瑕疵
        {
            // 舵机动作：快速摆动（0度 -> 180度 -> 0度）
            Servo_SetAngle(180);    // 转到180度
						//Motor_SetSpeed(0);			// 电机停止
            DelayXms(300);          // 等待300ms
            Servo_SetAngle(0);      // 转回0度
					
            //DelayXms(1000);
           // Motor_SetSpeed(30);                    
            
            DefectFlag = 0;  // 执行完毕后清除标志，避免重复动作
            Serial_Printf("瑕疵产品剔除操作完成\r\n");
        }
				//=======================串口===================================
				
		}
}





