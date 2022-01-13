/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   led应用函数接口
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火 F103-霸道 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "bsp_led.h"   

 /**
  * @brief  初始化控制LED的IO
  * @param  无
  * @retval 无
  */
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);	 //????PB,PE?????±??
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //LED0-->PB.5 ????????
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //???ì????
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO????????50MHz
 //GPIO_Init(GPIOB, &GPIO_InitStructure);					 //?ù???è?¨??????????GPIOB.5
 //GPIO_SetBits(GPIOB,GPIO_Pin_5);						 //PB.5 ??????

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    		 //LED1-->PE.5 ????????, ???ì????
 GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 //???ì???? ??IO????????50MHz
 GPIO_ResetBits(GPIOE,GPIO_Pin_5); 						 //PE.5 ?????? 
}

/*********************************************END OF FILE**********************/
