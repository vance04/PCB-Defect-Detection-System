#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include <stdint.h>

// 补充：定义传感器数据结构体（和main.c中的数据对应）
typedef struct
{
    
    uint8_t humi;        // 温湿度的湿度值
    uint16_t light;      // 光敏值
    float thermal_temp;  // 热敏温度值（热值）
		int8_t Speed;				 //电机
		float Angle;				 //舵机
    
} SensorData_t;

extern SensorData_t SensorData;  // 声明全局传感器数据结构体
extern char Serial_RxPacket[];
extern uint8_t Serial_RxFlag;

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
void Serial_SendSensorData(void);  // 新增：发送传感器数据的函数声明

#endif
