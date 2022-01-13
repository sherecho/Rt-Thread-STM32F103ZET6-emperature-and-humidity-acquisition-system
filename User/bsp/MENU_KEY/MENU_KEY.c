#include "MENU_KEY.h"
#include "delay.h"
#include <rtthread.h>


 u8 choice=1;
 u8 c_state=NO;
 int adjust=0;
 int result3;
#define EVENT_FLAG6 (1 << 6)  //IO口中断触发事件
#define EVENT_FLAG7 (1 << 7)  //修改sampling 事件
extern struct rt_event memlcd_data_event;
extern u8 samp_set,humset,temset;
extern  uint8_t tem,hum,tem_flag,hum_flag;
extern struct rt_event changesamp_event;
extern  uint16_t samp_freq;
//外部中断0服务程序
void K_MENU_Init(void)
{
 //结构体init 
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
//GPIO init
    GPIO_MENU_Init();	 //	按键端口初始化
//时钟打开
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟
//mode choice
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
//GPIOC.2 中断线以及中断初始化配置   下降沿触发
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource2);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line2;	//KEY2
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

//GPIOC.3	  中断线以及中断初始化配置 下降沿触发 //KEY1
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource3);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line3;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
//GPIOE.4	  中断线以及中断初始化配置  下降沿触发	//KEY0
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource4);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

//GPIOA.0	  中断线以及中断初始化配置 上升沿触发 PA0  WK_UP
 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0); 
  	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_Init(&EXTI_InitStructure);		//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

//优先级配置
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//使能按键WK_UP所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//子优先级3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
		
		NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;			//使能按键KEY2所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级2
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			//使能按键KEY1所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//使能按键KEY0所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级0 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
		
		
}

void GPIO_MENU_Init(void) //IO初始化
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE| RCC_APB2Periph_GPIOC, ENABLE);//使能PORTA,PORTE时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//KEY0-KEY2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE2,3,4
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;//PC2 PC3 
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
	  GPIO_Init(GPIOC, &GPIO_InitStructure);
    //初始化 WK_UP-->GPIOA.0	  下拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	  
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.0

}

//外部中断0服务程序 
void EXTI0_IRQHandler(void)
{
	rt_kprintf(" in EXIT0\n");
	delay_ms(20);//消抖
	if(WK_UP==1)	 	 //WK_UP按键
	{				 
        if (choice < 6)
		{
            choice++;
			u8 j;
	        for(j=1;j<6;j++)
	        { if(j== choice)
               my_LCD_Color_Fill(0, choice * 30-20, 240, choice * 31, BLUE);
	        	else
	          my_LCD_Color_Fill(0, j * 30-20, 240, j * 31,GRAY) ;	
	        
	        }
	    }
        else
            choice =1;
				while(WK_UP==1)
			        ;
	//发送IO按键触发事件集
	result3=rt_event_send(&memlcd_data_event, EVENT_FLAG6);
	rt_kprintf(" in EXIT0\n");
		if (result3 != RT_EOK)
            {
                rt_kprintf("rt_event_send ERR\n");
            }
	
	}
	EXTI_ClearITPendingBit(EXTI_Line0); //清除LINE0上的中断标志位 
	
}
 
void EXTI4_IRQHandler(void)
{
	rt_kprintf(" in EXIT4\n");
	delay_ms(20);//消抖
	
	if(KEY0==0)	 //按键KEY0
	{
		 
		c_state= !c_state; 
		//更换背景
		if(c_state==OK)
		{
		LCD_Clear(GRAY);
	    LCD_ShowNum(10,300 ,c_state,5,16);

		}
		else		
		{
			
			LCD_Clear(GRAY);
			display_Mainlist();
		  LCD_ShowNum(10,300 ,c_state,5,16);	
		}
		while(KEY0==0)
			;
	//发送IO按键触发事件集
	result3=rt_event_send(&memlcd_data_event, EVENT_FLAG6);
	if (result3 != RT_EOK)
     {
        rt_kprintf("rt_event_send ERR\n");
     } 
		
	}		 
	EXTI_ClearITPendingBit(EXTI_Line4);  //清除LINE4上的中断标志位  
}
 //外部中断2服务程序
void EXTI2_IRQHandler(void)
{
	rt_kprintf(" in EXIT2\n");
	delay_ms(10);//消抖
	
	if(KEY1==0)	  //按键KEY2
	{
		 adjust+=1;
		 LCD_ShowNum(10,300 ,adjust,5,16);
		 result3=rt_event_send(&memlcd_data_event, EVENT_FLAG6);
	    if (result3 != RT_EOK)
        {
            rt_kprintf("rt_event_send ERR\n");
        } 
		
	}		 
	EXTI_ClearITPendingBit(EXTI_Line2);  //清除LINE2上的中断标志位  
}
//外部中断3服务程序
void EXTI3_IRQHandler(void)
{	rt_kprintf(" in EXIT3\n");
	delay_ms(10);//消抖
	
	if(KEY2==0)	 //按键KEY1
	{				 
		 adjust-=1;
		 LCD_ShowNum(10,300 ,adjust,5,16);
		result3=rt_event_send(&memlcd_data_event, EVENT_FLAG6);
	    if (result3 != RT_EOK)
        {
            rt_kprintf("rt_event_send ERR\n");
        } 
	}		 
	EXTI_ClearITPendingBit(EXTI_Line3);  //清除LINE3上的中断标志位  
}
