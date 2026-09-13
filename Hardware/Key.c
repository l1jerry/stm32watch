#include "stm32f10x.h"                  // Device header
#include "Delay.h"

uint8_t Key_Num;
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_6;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}

uint8_t Key_GetNum(void)
{
	uint8_t Temp;
	if(Key_Num)
	{
		Temp=Key_Num;
		Key_Num=0;
		return Temp;
	}
	else
	{
		return 0;
	}
}

int press_time;
void Key3_Tick(void)
{
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)
	{
		press_time++;
	}
	
	if((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 1))
	{
		press_time=0;
	}
};

uint8_t Key_GetState(void)
{
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0)
	{
		return 1;
	}
	else if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)==0)
	{
		return 2;
	}
	else if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)==0)
	{
		return 3;
	}
	else if ((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)&&press_time>1000)
	{
		return 4;
	}
	else
	{
		return 0;
	}
	
}

void Key_Tick(void)
{
	static uint8_t Count;
	static uint8_t CurrentState,PerState;
	Count++;
	if(Count>=20)
	{
		Count=0;
		PerState=CurrentState;
		CurrentState=Key_GetState();
		if(PerState!=0&&CurrentState==0)
		{
			Key_Num=PerState;
		}
			
	}
}

