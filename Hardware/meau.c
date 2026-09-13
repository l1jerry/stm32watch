#include "stm32f10x.h"                  // Device header
#include "MyRTC.h"
#include "OLED.h"
#include "key.h"
#include "LED.h"
#include "SetTime.h"
#include "meau.h"
#include "MPU6050.h"
#include "Delay.h"
#include <math.h>
#include "Dino.h" 
#include "AD.h"

uint8_t KeyNum;
void Peripheral_Init(void)
{
	MyRTC_Init();
	Key_Init();
	LED_Init();
	MPU6050_Init();
	AD_Init();
}

///*-------------------------------------首页时钟------------------------------*\

uint16_t ADValue;
float VBAT;
int Battery_Capacity;
//显示电池图标和电量
void Show_Battery(void)
{
	int sum;
	for(int i=0;i<3000;i++)
	{
		ADValue=AD_GetValue();
		sum+=ADValue;
		
	}
	ADValue=sum/3000;
	VBAT=(float)ADValue/4095*3.3;
	Battery_Capacity=(ADValue-3276)*100/819;
	if(Battery_Capacity<0)Battery_Capacity=0;
	
	//OLED_ShowNum(64,0,ADValue,4,OLED_6X8);
	//OLED_Printf(64,8,OLED_6X8,"VBAT:%.2f",VBAT);
	OLED_ShowNum(85,4,Battery_Capacity,3,OLED_6X8);
	OLED_ShowChar(103,4,'%',OLED_6X8);
	
	if(Battery_Capacity==100)OLED_ShowImage(110,0,16,16,Battery);
	else if(Battery_Capacity>=10&&Battery_Capacity<100)
	{
		OLED_ShowImage(110,0,16,16,Battery);
		OLED_ClearArea((112+Battery_Capacity/10),5,(10-Battery_Capacity/10),6);
		OLED_ClearArea(85,4,6,8);
	}
	
	else
	{
		OLED_ShowImage(110,0,16,16,Battery);
		OLED_ClearArea(112,5,10,6);
		OLED_ClearArea(85,4,12,8);
	}
};

void Show_Clock_UI(void)
{
	Show_Battery();
	MyRTC_ReadTime();
	OLED_Printf(0,0,OLED_6X8,"%d-%d-%d",MyRTC_Time[0],MyRTC_Time[1],MyRTC_Time[2]);
	OLED_Printf(16,16,OLED_12X24,"%02d:%02d:%02d",MyRTC_Time[3],MyRTC_Time[4],MyRTC_Time[5]);
	OLED_ShowString(0,48,"菜单",OLED_8X16);
	OLED_ShowString(96,48,"设置",OLED_8X16);
}

int clkflag=1;
int First_Page_Clock(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
	
		if(KeyNum==1)//上一项
		{
			clkflag--;
			if(clkflag<=0)clkflag=2;
		}
		else if(KeyNum==2)//下一项
		{
			clkflag++;
			if(clkflag>=3)clkflag=1;
		}
		else if(KeyNum==3)//确定
		{
			OLED_Clear();
			OLED_Update();
			return clkflag;
		}
		else if(KeyNum==4)
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_13);//长按Key3,KeyNum等于4，拉低CTL引脚（PB13),单片机关机
			GPIO_SetBits(GPIOB, GPIO_Pin_12);//拉高BAT_ADC_EN引脚（PB12),ADC检测电路断开
		};
		switch(clkflag)
	{
		case 1:
			Show_Clock_UI();
			OLED_ReverseArea(0,48,32,16);
			OLED_Update();
			break;
		
		case 2:
			Show_Clock_UI();
			OLED_ReverseArea(96,48,32,16);
			OLED_Update();
			break;
	}
	}	
}

///*-------------------------------------设置界面------------------------------*\

void Show_SettingPage_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(0,16,"日期时间设置",OLED_8X16);
}

int setfalg=1;
int SettingPage(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t setflag_temp=0;
		if(KeyNum==1)//上一项
		{
			setfalg--;
			if(setfalg<=0)setfalg=2;
		}
		else if(KeyNum==2)//下一项
		{
			setfalg++;
			if(setfalg>=3)setfalg=1;
		}
		else if(KeyNum==3)//确定
		{
			OLED_Clear();
			OLED_Update();
			setflag_temp=setfalg;
		}
		if(setflag_temp==1){return 0;}
		else if(setflag_temp==2){SetTime();}
		
		switch(setfalg)
	{
		case 1:
			Show_SettingPage_UI();
			OLED_ReverseArea(0,0,16,16);
			OLED_Update();
			break;
		
		case 2:
			Show_SettingPage_UI();
			OLED_ReverseArea(0,16,96,16);
			OLED_Update();
			break;
	}
	}	
}


///*-------------------------------------滑动菜单界面------------------------------*\

uint8_t pre_selection;//上次选择的选项
uint8_t target_selection;//目标选项
uint8_t x_pre=48;//上次选项的x坐标
uint8_t Speed=4;//速度
uint8_t move_flag;//开始移动标志伟，1表示开始移动，0表示停止移动

void Meau_Ainmation(void)
{
	OLED_Clear();
	OLED_ShowImage(42,10,44,44,Frame);
	
	if(pre_selection<target_selection)
	{
		x_pre-=Speed;
		if(x_pre==0)
		{
			pre_selection++;
			move_flag=0;
			x_pre=48;
		}
	}
	if(pre_selection>target_selection)
	{
		x_pre+=Speed;
		if(x_pre==96)
		{
			pre_selection-- ;
			move_flag=0;
			x_pre=48;
		}
	}
	
	if(pre_selection>=1)
	{
		OLED_ShowImage(x_pre-48,16,32,32,Menu_Graph[pre_selection-1]);
	}
	if(pre_selection>=2)
	{
		OLED_ShowImage(x_pre-96,16,32,32,Menu_Graph[pre_selection-2]);
	}
	
	OLED_ShowImage(x_pre,16,32,32,Menu_Graph[pre_selection]);
	OLED_ShowImage(x_pre+48,16,32,32,Menu_Graph[pre_selection+1]);
	OLED_ShowImage(x_pre+96,16,32,32,Menu_Graph[pre_selection+2]);
	
	OLED_Update();
		
}

void Set_Selection(uint8_t move_flag,uint8_t Pre_Selection,uint8_t Target_Selection)
{
	if(move_flag==1)
	{
		pre_selection=Pre_Selection;
		target_selection=Target_Selection;
		
	}
	Meau_Ainmation();
}

void MeauToFunction(void)
{
	for(uint8_t i=0;i<=6;i++)
	{
		OLED_Clear();
		if(pre_selection>=1)
			{
				OLED_ShowImage(x_pre-48,16+8*i,32,32,Menu_Graph[pre_selection-1]);
			}
			
			OLED_ShowImage(x_pre,16+8*i,32,32,Menu_Graph[pre_selection]);
			OLED_ShowImage(x_pre+48,16+8*i,32,32,Menu_Graph[pre_selection+1]);
			
			OLED_Update();
			}
}

uint8_t meau_flag=1;
int Meau(void)
{
	while(1)
	{
		Meau_Ainmation();
		uint8_t DirectFlag;//置1：移动到上一项；置2：移动到下一项
		KeyNum=Key_GetNum();
		uint8_t meau_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			DirectFlag=1;
			move_flag=1;
			meau_flag--;
			if(meau_flag<=0)meau_flag=7;
		}
		else if(KeyNum==2)//下一项
		{
			DirectFlag=2;
			move_flag=1;
			meau_flag++;
			if(meau_flag>=8)meau_flag=1;
		}
		else if(KeyNum==3)//确定
		{
			OLED_Clear();
			OLED_Update();
			meau_flag_temp=meau_flag;
		}
		if(meau_flag_temp==1){return 0;}
		else if(meau_flag_temp==2){MeauToFunction();StopWatch();}
		else if(meau_flag_temp==3){MeauToFunction();LED();}
		else if(meau_flag_temp==4){MeauToFunction();MPU6050();}
		else if(meau_flag_temp==5){MeauToFunction();Game();}
		else if(meau_flag_temp==6){MeauToFunction();Emoji();}
		else if(meau_flag_temp==7){MeauToFunction();Gradienter();}
		
//		switch(meau_flag)
//	{
//		case 1:
//			if(DirectFlag==1)Set_Selection(move_flag,1,0);
//			else if(DirectFlag==2)Set_Selection(move_flag,0,0);
//			break;
//		
//		case 2:
//			if(DirectFlag==1)Set_Selection(move_flag,2,1);
//			else if(DirectFlag==2)Set_Selection(move_flag,0,1);
//			break;
//		
//		case 3:
//			if(DirectFlag==1)Set_Selection(move_flag,3,2);
//			else if(DirectFlag==2)Set_Selection(move_flag,1,2);
//			break;
//		
//		case 4:
//			if(DirectFlag==1)Set_Selection(move_flag,4,3);
//			else if(DirectFlag==2)Set_Selection(move_flag,2,3);
//			break;
//		
//		case 5:
//			if(DirectFlag==1)Set_Selection(move_flag,5,4);
//			else if(DirectFlag==2)Set_Selection(move_flag,3,4);
//			break;
//		
//		case 6:
//			if(DirectFlag==1)Set_Selection(move_flag,6,5);
//			else if(DirectFlag==2)Set_Selection(move_flag,4,5);
//			break;
//		
//		case 7:
//			if(DirectFlag==1)Set_Selection(move_flag,7,6);
//			else if(DirectFlag==2)Set_Selection(move_flag,5,6);
//			break;
//	}

		if(meau_flag==1)
		{
			if(DirectFlag==1)Set_Selection(move_flag,1,0);
			else if(DirectFlag==2)Set_Selection(move_flag,0,0);
		}
		else
		{
			if(DirectFlag==1)Set_Selection(move_flag,meau_flag,meau_flag-1);
			else if(DirectFlag==2)Set_Selection(move_flag,meau_flag-2,meau_flag-1);
		}
	}	
	}


	///*-------------------------------------秒表-----------------------------*\
	
uint8_t hour,min,sec;
void	Show_StopWatch_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_Printf(32,20,OLED_8X16,"%02d:%02d:%02d",hour,min,sec);
	OLED_ShowString(8,44,"开始",OLED_8X16);
	OLED_ShowString(48,44,"停止",OLED_8X16);
	OLED_ShowString(88,44,"清除",OLED_8X16);
}
	
uint8_t start_timing_flag;//1：开始，0：停止
void StopWatch_Tick(void)
{
	static uint16_t Count;
	Count++;
	if(Count>=1000)
	{
		Count=0;
		if(start_timing_flag==1)
		{
			sec++;
			if(sec>=60)
			{
				sec=0;
				min++;
				if(min>=60)
				{
					min=0;
					hour++;
					if(hour>=99)hour=0;
				}
			}
		}
	}
}

uint8_t stopwatch_falg=1;
int StopWatch(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t stopwatch_falg_temp=0;
		if(KeyNum==1)//上一项
		{
			stopwatch_falg--;
			if(stopwatch_falg<=0)stopwatch_falg=4;
		}
		else if(KeyNum==2)//下一项
		{
			stopwatch_falg++;
			if(stopwatch_falg>=5)stopwatch_falg=1;
		}
		else if(KeyNum==3)//确定
		{
			OLED_Clear();
			OLED_Update();
			stopwatch_falg_temp=stopwatch_falg;
		}
		if(stopwatch_falg_temp==1){return 0;}
		
		switch(stopwatch_falg)
	{
		case 1:
			Show_StopWatch_UI();
			OLED_ReverseArea(0,0,16,16);
			OLED_Update();
			break;
		
		case 2:
			Show_StopWatch_UI();
			start_timing_flag=1;
			OLED_ReverseArea(8,44,32,16);
			OLED_Update();
			break;
		
		case 3:
			Show_StopWatch_UI();
			start_timing_flag=0;
			OLED_ReverseArea(48,44,32,16);
			OLED_Update();
			break;
		
		case 4:
			Show_StopWatch_UI();
			start_timing_flag=0;
			hour=min=sec=0;  
			OLED_ReverseArea(88,44,32,16);
			OLED_Update();
			break;
	}
	}	
}

	///*-------------------------------------手电筒-----------------------------*\

void Show_LED_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(20,20,"OFF",OLED_12X24);
	OLED_ShowString(72,20,"ON",OLED_12X24);
}

uint8_t led_flag=1;
int LED(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t led_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			led_flag--;
			if(led_flag<=0)led_flag=3;
		}
		else if(KeyNum==2)//下一项
		{
			led_flag++;
			if(led_flag>=4)led_flag=1;
		}
		else if(KeyNum==3)//确定
		{
			OLED_Clear();
			OLED_Update();
			led_flag_temp=led_flag;
		}
		if(led_flag_temp==1){return 0;}
		
		switch(led_flag)
	{
		case 1:
			Show_LED_UI();
			OLED_ReverseArea(0,0,16,16);
			OLED_Update();
			break;
		
		case 2:
			Show_LED_UI();
			LED_OFF();
			OLED_ReverseArea(20,20,36,24);
			OLED_Update();
			break;
		
		case 3:
			Show_LED_UI();
			LED_ON();
			OLED_ReverseArea(72,20,24,24);
			OLED_Update();
			break;

	}
	}	

}

	///*-------------------------------------MPU6050姿态-----------------------------*\

int16_t ax,ay,az,gx,gy,gz;//MPU6050测得的三轴加速度和角速度
float roll_g,pitch_g,yaw_g;//陀螺仪解算欧拉角
float roll_a,pitch_a;//加速度解算的欧拉角
float Roll,Pitch,Yaw;//互补滤波后的欧拉角
float a=0.9;//互补滤波系数
float Delta_t=0.005;//采样周期
double pi=3.1415927;

void MPU6050_Calculation(void)
{
	Delay_ms(5);
	MPU6050_GetData(&ax,&ay,&az,&gx,&gy,&gz);
	
		//通过陀螺仪解算欧拉角
	roll_g=Roll+(float)gx*Delta_t;
	pitch_g=Pitch+(float)gy*Delta_t;
	yaw_g=Yaw+(float)gz*Delta_t;
	
	//通过加速度计解算欧拉角
	pitch_a=atan2((-1)*ax,az)*180/pi;
	roll_a=atan2(ay,az)*180/pi;
	
	//通过互补滤波器进行数据融合
	Roll=a*roll_g+(1-a)*roll_a;
	Pitch=a*pitch_g+(1-a)*pitch_a;
	Yaw=a*yaw_g;
}

void Show_MPU6050_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_Printf(0,16,OLED_8X16,"Roll:%.2f",Roll);
	OLED_Printf(0,32,OLED_8X16,"Pitch:%.2f",Pitch);
	OLED_Printf(0,48,OLED_8X16,"Yaw:%.2f",Yaw);
}

int MPU6050(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		
		OLED_Clear();
		MPU6050_Calculation();
		Show_MPU6050_UI();
		OLED_ReverseArea(0,0,16,16);
		OLED_Update();
	}
}

///*-------------------------------------谷歌小恐龙-----------------------------*\

void Show_Game_UI(void)
{
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowString(0,16,"谷歌小恐龙",OLED_8X16);
}
	
uint8_t game_flag=1;
int Game(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		uint8_t game_flag_temp=0;
		if(KeyNum==1)//上一项
		{
			game_flag--;
			if(game_flag<=0)game_flag=2;
		}
		else if(KeyNum==2)//下一项
		{
			game_flag++;
			if(game_flag>=3)game_flag=1;
		}
		else if(KeyNum==3)//确定
		{
			OLED_Clear();
			OLED_Update();
			game_flag_temp=game_flag;
		}
		if(game_flag_temp==1){return 0;}
		else if(game_flag_temp==2){DinoGame_Pos_Init();DinoGame_Animation();}
		
		switch(game_flag)
	{
		case 1:
			Show_Game_UI();
			OLED_ReverseArea(0,0,16,16);
			OLED_Update();
			break;
		
		case 2:
			Show_Game_UI();
			OLED_ReverseArea(0,16,80,16);
			OLED_Update();
			break;
		

	}
	}	

}

///*-------------------------------------动态表情包-----------------------------*\

void Show_Emoji_UI(void)
{
	/*闭眼*/
	for(uint8_t i=0;i<3;i++)
	{
		OLED_Clear();
		OLED_ShowImage(30,10+i,16,16,Eyebrow[0]);//左眉毛
		OLED_ShowImage(82,10+i,16,16,Eyebrow[1]);//右眉毛
		OLED_DrawEllipse(40,32,6,6-i,1);//左眼
		OLED_DrawEllipse(88,32,6,6-i,1);//右眼
		OLED_ShowImage(54,40,20,20,Mouth);
		OLED_Update();
		Delay_ms(100);
	}
	
	/*睁眼*/
	for(uint8_t i=0;i<3;i++)
	{
		OLED_Clear();
		OLED_ShowImage(30,12-i,16,16,Eyebrow[0]);//左眉毛
		OLED_ShowImage(82,12-i,16,16,Eyebrow[1]);//右眉毛
		OLED_DrawEllipse(40,32,6,4+i,1);//左眼
		OLED_DrawEllipse(88,32,6,4+i,1);//右眼
		OLED_ShowImage(54,40,20,20,Mouth);
		OLED_Update();
		Delay_ms(100);
	}
	Delay_ms(500);
	
}

int Emoji(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		Show_Emoji_UI();
	}
}

///*-------------------------------------水平仪-----------------------------*\

void Show_Gradienter_UI(void)
{
	MPU6050_Calculation();
	OLED_DrawCircle(64,32,30,0);
	OLED_DrawCircle(64-Roll,32+Pitch,4,1);
}

int Gradienter(void)
{
	while(1)
	{
		KeyNum=Key_GetNum();
		if(KeyNum==3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		OLED_Clear();
		Show_Gradienter_UI();
		OLED_Update();
	}
}

