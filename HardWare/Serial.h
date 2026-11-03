#ifndef __SERIAL_H
#define __SERIAL_H
#include <stdio.h>
void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArr(uint8_t* Array,uint16_t length);
void Serial_SendStr(char*Str);
void Serial_Printf(char *format, ...);
void Serial_SendNum(uint32_t num,uint8_t length);
uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxData(void);
#endif
