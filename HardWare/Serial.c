#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>
uint8_t Serial_RxData;
uint8_t FLAG;
void Serial_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate=9600; //波特率
    USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode=USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity=USART_Parity_No; //校验模式
    USART_InitStructure.USART_StopBits=USART_StopBits_1;//停止位
    USART_InitStructure.USART_WordLength=USART_WordLength_8b;

    USART_Init(USART1,&USART_InitStructure);
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
    NVIC_Init(&NVIC_InitStructure);
    USART_Cmd(USART1,ENABLE);
}

void Serial_SendByte(uint8_t Byte){
    USART_SendData(USART1,Byte);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);

}
void Serial_SendArr(uint8_t* Array,uint16_t length){
    while(length--){
    Serial_SendByte(*Array++);
}
}
void Serial_SendStr(char*Str){
    while(*Str!='\0'){
        Serial_SendByte(*Str++);
    }
}
void Serial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendStr(String);
}
uint32_t Serial_Pow(uint32_t x, uint32_t y){
    uint32_t res=1;
    while(y--){
        res*=x;
    }
    return res;
}
void Serial_SendNum(uint32_t num,uint8_t length){

    uint8_t i;
    for(i=0;i<length;i++){
        Serial_SendByte(num/Serial_Pow(10,length-i-1)%10+'0');
    }
}
int fputc(int ch,FILE*f){
    Serial_SendByte(ch);
    return ch;
}

uint8_t Serial_GetRxFlag(void){
    if(FLAG==1){
        FLAG=0;
        return 1;
    }
    return 0;
}
uint8_t Serial_GetRxData(void){
    return Serial_RxData;
}
void USART1_IRQHandler(void){
    if(USART_GetFlagStatus(USART1,USART_IT_RXNE)==SET){
        Serial_RxData=USART_ReceiveData(USART1);
        FLAG=1;
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);

    }
}