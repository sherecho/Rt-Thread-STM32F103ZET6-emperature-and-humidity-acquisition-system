#include "Menu.h"
#include <rtthread.h>
#include "board.h"
#include "stdio.h"
#include <string.h>
#include <ctype.h>
u8 timerset;
extern u8 mem_slice;
extern rt_uint8_t lcd_draw_tem[48];
extern rt_uint8_t lcd_drawn_hum[48];
//外部变量定义
extern u8 c_state;
extern u8 choice;
u8 change_tem=0;
u8 change_freq=0;
u8 change_hum=0;
u8 humset=0,temset=0;
u16 samp_set;
extern u8 index;
extern  int adjust;
extern rt_timer_t DH11_timer ;//定时器控制块
extern  u16 POINT_COLOR ;	//画笔颜色
extern  u16 BACK_COLOR ;  //背景色 

extern  rt_uint8_t *ptr[48];
extern  rt_uint8_t *ptr2[48];
extern  uint16_t samp_freq;
extern  uint8_t tem,hum,tem_flag,hum_flag;
extern struct rt_event memlcd_data_event;
extern struct rt_event changesamp_event;
extern struct rt_messagequeue mq_tem_memlcd;
#define EVENT_FLAG2 (1 << 2)  //DHT11LCD可读取
#define EVENT_FLAG7 (1 << 7)  //修改sampling 事件

struct menu_sta  deal_key(void)
{
	struct menu_sta re;
	if (c_state ==OK)
	{
		re.verify_sta = BRANCH_STA;
		re.menu_loc_now = choice;
	}
    
	if (c_state == NO)
	{  
		re.verify_sta = MAIN_STA;
		re.menu_loc_now = choice;
	}
	return re;

}
void key_choice_handle(void)
{
	u8 j;
	struct menu_sta menu = deal_key();
	if (menu.verify_sta == BRANCH_STA)
	{ 
		switch (menu.menu_loc_now)
		{
		case 1: 
		{tem_set_display();
         change_tem=1;
		if (c_state ==NO)
		  {
			LCD_Clear(GRAY);
			if(change_tem)
			{
            tem_flag=temset;
		    temset=0;
		    adjust=0;
			change_tem=0;
			}
		  }
		}
		break;
		case 2: {hum_set_display();
		         change_hum=1;
			if (c_state == NO)
			{
				if(change_hum)
				{
			    hum_flag=humset;
				humset=0;
				adjust=0;
				change_hum=0;
				}
				LCD_Clear(GRAY);
			}
		}
	    break;

		case 3: {samp_set_display();
		    change_freq=1;
			if (c_state == NO)
			{
      
				if(change_freq)
				{
			    samp_freq=samp_set;
				samp_set=0;
				adjust=0;			
				//发送修改采样频率的事件
				//rt_event_send(&changesamp_event, EVENT_FLAG7);
				rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_TIME, (void *)samp_freq);	
				//rt_kprintf("change freq\n");
				change_freq=0;
				}
				LCD_Clear(GRAY);
	           
			}
			}
		 break;

		case 4: { lcd_showwave();
			if (c_state == NO)
				LCD_Clear(GRAY);
		}
	   break;
	   case 5: { 
		   humtem_infor_display();
			if (c_state == NO)
				LCD_Clear(GRAY);
		}
	   break;
		default:
			 break;
		}

    
	}
	else
	{
		display_Mainlist();
        if(change_tem)
			{
            tem_flag=temset;
		    temset=0;
		    adjust=0;
			change_tem=0;

			}
		if(change_hum)
				{
			    hum_flag=humset;
				humset=0;
				adjust=0;
				change_hum=0;
				}
		if(change_freq)
				{
			  samp_freq=samp_set;
				samp_set=0;
				adjust=0;			
				//发送修改采样频率的事件
				//rt_event_send(&changesamp_event, EVENT_FLAG7);	
				
				//rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_TIME, (void *)&samp_freq);
				//rt_timer_stop(DH11_timer);
				//rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_PERIODIC, (void *)&samp_freq);
				//rt_timer_start(DH11_timer);
				//rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_PERIODIC , (void *)&samp_freq);
				//rt_kprintf("timer set %d \n",timerset);				
						
				
				change_freq=0;
				}
		if(c_state==OK)
			 LCD_Clear(GRAY);
  }

}


//主菜单显示
void display_Mainlist( void)
{
	//整体背景 grey  每一行的宽度 30
	//LCD_Clear(GRAY);//在main 里面写
	//选中背景 蓝色
	

	//字符显示
	LCD_ShowString(10, 10, 200, 12, 12, "1.temperature threhold set");
	LCD_ShowString(10, 40, 200, 12, 12, "2.humidity threhold set");
	LCD_ShowString(10, 70, 200, 12, 12, "3.sampling frequence set");
	LCD_ShowString(10, 100, 200, 12,12, "4.wave information display");
	LCD_ShowString(10, 130, 200, 12, 12,"5.num information display");

}
void tem_set_display(void)
{
   LCD_ShowString(10, 10, 200, 12, 12, " set the threadhold of temperature");
	LCD_ShowString(10, 30, 200, 12, 12, "1. the threadhold of tem is: ");
	LCD_ShowNum(180, 30, tem_flag, 5, 12);
	temset=tem_flag+adjust;
	LCD_ShowString(10, 50, 200, 12, 12, " the figure you set is : ");
	LCD_ShowNum(180, 50, temset, 5, 12);

}
void hum_set_display(void)
{

	LCD_ShowString(10, 10, 200, 12, 12, " set the threadhold of humidity");
	LCD_ShowString(10, 30, 200, 12, 12, "1. the threahold of hum is: ");
    LCD_ShowNum(180, 30, hum_flag, 5, 12);
    
	humset=hum_flag+adjust;
	LCD_ShowString(10, 50, 200, 12, 12, " the figure you set is : ");
	LCD_ShowNum(180, 50, humset, 5, 12);
	 

}
void samp_set_display(void)
{
	LCD_ShowString(10, 10, 200, 12, 12, " set the sampling frequence");
	LCD_ShowString(10, 30, 200, 12, 12, "1. the sampling freq is: ");
    LCD_ShowNum(180, 30, samp_freq, 6, 12);
	samp_set=samp_freq+adjust*10;
	LCD_ShowString(10, 50, 200, 12, 12, " the figure you set is : ");
	LCD_ShowNum(180, 50, samp_set, 6, 12);
}
void humtem_infor_display(void)
{

    LCD_ShowString(10, 10, 200, 12, 12, " the current figure  display ");
	LCD_ShowString(10, 30, 200, 14, 14, " the temperature is: ");
    LCD_ShowNum(180, 30, tem, 5, 12);
	
	LCD_ShowString(10, 50, 200, 14, 14, " the humidity is: ");
	LCD_ShowNum(180, 50, hum, 5, 12);


}



//lcd 显示曲线信息
void show_tem_lcdwave(int index)
{
	int n=0,m=0;
   // rt_kprintf("drawning\n");
	int step=4,base_height1=120,base_height2=240;		
			LCD_Clear(GRAY);
			for(m=0;m<48;m++)
			{				
				//绘制温度曲线
				 //rt_kprintf("draw ptr %d \n",ptr[index][m]);
				 //rt_kprintf("index %d \n",index);
				
				//LCD_DrawLine(10+m*step,base_height1-ptr[index][m],10+(m+1)*step,base_height1-ptr[index][m+1]);
				LCD_DrawLine(10+m*step,base_height1-lcd_draw_tem[m],10+(m+1)*step,base_height1-lcd_draw_tem[m+1]);	
				//绘制湿度曲线
				//rt_kprintf("draw ptr2 %d \n",lcd_drawn_hum[m]);
				LCD_DrawLine(10+m*step,base_height2-lcd_drawn_hum[m],10+(m+1)*step,base_height2-lcd_drawn_hum[m+1]);			
			}
			LCD_DrawLine_red(0,base_height1-tem_flag,240,base_height1-tem_flag);//flag
			LCD_ShowString(10, 120, 200, 12, 12, " temperature cave");
			LCD_DrawLine_red(0,base_height2-hum_flag,240,base_height2-hum_flag);//flag
			LCD_ShowString(10, 240, 200, 12, 12, " humidity cave");
			
}
void lcd_showwave(void)
{	
    
    		
			
		//等待事件
        if (rt_event_recv(& memlcd_data_event, (EVENT_FLAG2),
                      RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER,RT_NULL ) == RT_EOK)
         {
	  
		show_tem_lcdwave(index);	
         }	
		 		   


}
