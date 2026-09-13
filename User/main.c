#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "meau.h"
#include "Timer.h"
#include "Key.h"
#include "Dino.h"


int main(void)
{
	/*OLED初始化*/
	OLED_Init();
	OLED_Clear();
	Peripheral_Init();
	Timer_Init();
	int clkfalg1;

	while (1)
	{	
		//		OLED_ShowNum(64,0,press_time,4,OLED_6X8);
		//		OLED_ShowNum(64,8,Key_Num,1,OLED_6X8);
		//		OLED_Update();
		
		clkfalg1=First_Page_Clock();
		if(clkfalg1==1){Meau();}
		else if(clkfalg1==2){SettingPage();}
		
		
	}
		
}

//定时器中断
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
		Key_Tick();
		StopWatch_Tick();
		Dino_Tick();
		Key3_Tick();
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}




