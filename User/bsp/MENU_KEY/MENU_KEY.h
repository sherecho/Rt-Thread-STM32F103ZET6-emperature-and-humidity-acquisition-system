#ifndef __MENU_KEY_H
#define __MENU_KEY_H	 
#include "sys.h"
#include "bsp_lcd.h"



#define KEY0  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)//读取按键0 
#define KEY1  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2)//读取按键1
#define KEY2  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_3)//读取按键2 
#define WK_UP   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键3(WK_UP) 
#define KEY0_PRES 	1	//KEY0按下
#define WKUP_PRES   4	//KEY_UP按下(即WK_UP/KEY_UP)


#define OK 0
#define NO 1


void GPIO_MENU_Init(void);//IO初始化				
void K_MENU_Init(void);//外部中断初始化	


#endif

