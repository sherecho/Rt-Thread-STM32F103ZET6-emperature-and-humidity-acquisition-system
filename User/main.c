
/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/ 

//#include "all_header.h"
#include <rtthread.h>
#include "board.h"
#include "stdio.h"
#include <string.h>
#include <ctype.h>
//bsp

#include "bsp_dht11.h"
#include "bsp_lcd.h"
#include "bsp_led.h"
#include "MENU.h"
#include "MENU_KEY.h"
#include "sys.h"
#include "delay.h"
//SD卡  
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
//时间戳
#include "rtc.h" 
/*
**********************************************
               变量

**********************************************
*/
uint16_t samp_freq=200;
uint8_t tem,hum;
uint8_t tem_flag=30,hum_flag=0; //温湿度报警阈值，初始化为30，50（用户可重新设定)
u8 index;//使用信号量防止冲突
int i=0;
u8 t=0;	
extern  u16 samp_set;
extern u8 change_freq;		
extern  int adjust;			

/*
*************************************************************************
*                           SD 卡FATFS配置
*************************************************************************
*/
FATFS fatsd;
FATFS fatflash;
FIL fileobj;
FRESULT fr;//定义返回值
UINT brs;
/*
*************************************************************************
*                          RTC配置
*************************************************************************
*/


/*
**********************************************
                定时器变量

**********************************************
*/

rt_timer_t DH11_timer ;//定时器控制块
//温湿度读取变量
static  uint8_t tem,hum;


/*
**********************************************
                消息队列变量

**********************************************
*/
	
	/* 定义消息队列来发送温度传感器消息 */
static struct rt_messagequeue mq_DHT11_tem;
static struct rt_messagequeue mq_DHT11_hum;
/* 温度传感器消息内存池 */
static rt_uint8_t msg_DHT11_pool_tem[1024];
static rt_uint8_t msg_DHT11_pool_hum[1024];
	/* 定义消息队列来向LCD已经处理该内存块，SD卡可以写入并销毁该内存块*/
struct rt_messagequeue mq_tem_memlcd;	
static rt_uint8_t mq_tem_pool_memlcd[256];


/*
**********************************************
                事件变量

**********************************************
*/
#define EVENT_FLAG1 (1 << 1)  //DHT11温度
#define EVENT_FLAG2 (1 << 2)  //DHT11LCD可读取
#define EVENT_FLAG3 (1 << 3)  //超温事件
//#define EVENT_FLAG4 (1 << 4)  //超湿度事件
#define EVENT_FLAG5 (1 << 5)  //DHT11LCD可读取
#define EVENT_FLAG6 (1 << 6)  //IO口中断触发事件
#define EVENT_FLAG7 (1 << 7)  //修改采样频率
#define EVENT_FLAG8 (1 << 8)  //要求串口发送事件的信息



/* 定义事件控制块 */

struct rt_event DHT11_data_event;
struct rt_event memlcd_data_event;
struct rt_event memlcd2_data_event;
struct rt_event changesamp_event;
struct rt_event senddata_event;

//ALIGN(RT_ALIGN_SIZE)
/*
**********************************************
               任务线程

**********************************************
*/

/* 定义事件任务*/
static rt_thread_t uart_deal_event = RT_NULL;  //事件处理线程


//入口函数串口发送温度
	static void uart_deal_thread_entry(void* parameter)
{	
    //rt_kprintf("thread in uartsend\n");
    while (1)
    {
		  //rt_uint32_t e;  
			//rt_kprintf("thread in\n");
			
     if (rt_event_recv(&DHT11_data_event, (EVENT_FLAG1|EVENT_FLAG8),
                      RT_EVENT_FLAG_AND|RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER,RT_NULL ) == RT_EOK)
    {
			
        rt_kprintf("tem: %d\n",tem);
        rt_kprintf("hum: %d\n",hum);			
    }			
			
     
			   
    }
}

//温湿度消息队列接收并存储任务线程
//定义用于保存温度和湿度历史信息的内存池
rt_uint8_t lcd_draw_tem[48]=0;
rt_uint8_t lcd_drawn_hum[48]=0;
rt_uint8_t *ptr [48]; //最多保存48*mem_slice 数据
rt_uint8_t *ptr2 [48]; //最多保存48*mem_slice 数据
static rt_uint8_t temmempool[1024]; //4K
static rt_uint8_t hummempool[1024];
static struct rt_mempool tem_mp;
static struct rt_mempool hum_mp;
//获取传读取温度序列，保存在内存池之中，并与LCD显示等线程同步

#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5
static rt_thread_t recevDHT11 = RT_NULL;
static rt_thread_t thread_mp_release = RT_NULL;
u8 mem_slice=25;
//入口函数
static void recevDHT11_entry(void *parameter)
{
   
    int cnt=0;
    int cnt2=0;
    int i=0;
    int j=0;
    uint8_t tem_get,hum_get,result;
    while (1)
    {
			  i=i%48;
        j=j%48;
       // rt_kprintf("thread in recevDHT11\n");
        if (rt_mq_recv(&mq_DHT11_hum, &hum_get, sizeof(hum_get), RT_WAITING_NO) == RT_EOK)
        {
           // rt_kprintf("rt_mq_recv mq_DHT11_hum \n");	
					  //申请内存池
            if(cnt2%mem_slice==0)
						{
					   
					  ptr2[j] = rt_mp_alloc(&hum_mp, RT_WAITING_NO);
           
            if (ptr2[j] != RT_NULL)
						{ //rt_kprintf("allocate tempool success\n");	
						   
						
							//向LCD显示任务线程发内存块数据可读消息（保存可读的idex）以及可处理事件标志
							if(j!=0)
							{
								
                index=j;
						    //内存处理向LCD发送event(发送的可读取区域为0~(i-1)
								result=rt_event_send(&memlcd2_data_event, EVENT_FLAG5);
		            if (result != RT_EOK)
                {
                    rt_kprintf("rt_event_send ERR\n");
                }
		           
						
						  }	
              j=j+1;
							
						}
					  else
							   rt_kprintf("error no pool left \n");				
						
						}
						//接收消息并保存到内存池中
					 if (ptr2[j-1] != RT_NULL)
					 {
						//接收消息并保存到内存池中
					//	rt_kprintf(" recv from msg queue, the content:%d\n", hum);
						ptr2[j-1][cnt2%mem_slice]=hum_get;
            cnt2+=1;
					 }
           lcd_drawn_hum[cnt2%48]=hum_get;
       
        }
      
        if (rt_mq_recv(&mq_DHT11_tem, &tem_get, sizeof(tem_get), RT_WAITING_NO) == RT_EOK)
        {
					  //申请内存池
					//  rt_kprintf("receive mq_DHT11_tem \n");
            if(cnt%mem_slice==0)
						{
					  
					  ptr[i] = rt_mp_alloc(&tem_mp, RT_WAITING_NO);
           
            if (ptr[i] != RT_NULL)
						{// rt_kprintf("allocate tempool success\n");	
						   
							
							//向LCD显示任务线程发内存块数据可读消息（保存可读的idex）以及可处理事件标志
							if(i!=0)
							{
								
                index=i;
						    //内存处理向LCD发送event(发送的可读取区域为0~(i-1)
								result=rt_event_send(&memlcd_data_event, EVENT_FLAG2);
               // rt_kprintf("send memlcd_data_event  event success  \n");
                //rt_kprintf("index %d  \n",i);
		            if (result != RT_EOK)
                {
                    rt_kprintf("rt_event_send ERR\n");
                }
		            else
                    rt_kprintf("send memlcd_data_event  event success  \n");
                  
						
						  }
						  i=i+1;
						}
					  else
							   rt_kprintf("error no pool left \n");				
						
						}
						//接收消息并保存到内存池中
					 if (ptr[i-1] != RT_NULL)
					 {
						//接收消息并保存到内存池中
					//	rt_kprintf(" recv from msg queue, the content: %d \n", tem);
						ptr[i-1][cnt%mem_slice]=tem_get;
            cnt+=1;
					 }
           lcd_draw_tem[cnt%48]=tem_get;
        
        }
   

		}
   
		
		
}
//自定义十位数转字符串
u16 DectoStr (u8 *pSrc, u16 SrcLen, u8 *pObj)
{
    u16 i=0;

    for(i=0;    i<SrcLen;   i++)
    {
        sprintf((char *)(pObj + i * 2), "%02d", *(pSrc + i));
    }
    *(pObj + i * 2) = '\0';
    return  (i * 2);
}
//销毁内存池，写入SD卡线程将LCD已经动态显示过的内存释放，将数据写入SD卡中保存（接收LCD显示曲线处理线程发送的消息队列)

static void thread_mp_release_entry(void *parameter)
{
    int buf;
    char name[11];
    u8 cnt=0;
	  u8 num;
    while(1)
    {
	 //接收消息
     
       
	  if (rt_mq_recv(&mq_tem_memlcd, &buf, sizeof(buf), RT_WAITING_FOREVER) == RT_EOK)
     {
          if(buf>=1)
        {
          buf=buf-1;
         // rt_kprintf("writing to SD Card\n");
          //rt_kprintf("index %d\n",buf);
          //生成文件名称
		       char str[40];  
           char week[12];   
           u8 hour;
           u8 min;
           u8 sec;   
           u8 month;
           u8 day;  	
	         strcpy (str,"0:data");
	         DectoStr (&cnt, 1, &num);
           strcat (str,&num);
           strcat (str,".csv");           
           fr=f_open(&fileobj,(const TCHAR*)str, 	FA_CREATE_ALWAYS | FA_WRITE | FA_READ);//open the file	   
          // rt_kprintf("file name: %s \n", &str);    
           //rt_kprintf("num: %d \n", cnt);   
          
		     	//接收当前内存可写入SD卡内存块位置    
           f_printf(&fileobj,"tem,hum,time");
          // rt_kprintf("fr %d\n",fr);   
	         f_putc( '\n',&fileobj);
          //获取时间
          switch(calendar.week)
		          	{
		          		case 0:
		          			strcpy (week,"Sunday");
		          			break;
		          		case 1:
		          			strcpy (week,"Monday");
		          			break;
		          		case 2:
		          			strcpy (week,"Tuesday");
		          			break;
		          		case 3:
		          	  	strcpy (week,"Wednesday");
		          			break;
		          		case 4:
		          			strcpy (week,"Thursday");
		          			break;
		          		case 5:
		          		  strcpy (week,"Friday");
		          			break;
		          		case 6:
		          			strcpy (week,"Saturday");
		          			break;  
		          	}
           		
           DectoStr ((u8*)&calendar.w_month,1,&month);
           DectoStr ((u8*)&calendar.w_date,1,&day);	
			     DectoStr ((u8*)&calendar.hour,1,&hour);		
           DectoStr ((u8*)&calendar.min,1,&min);
           DectoStr ((u8*)&calendar.sec,1,&sec);	          								  
           strcat (str,&month); 
           strcat (str,"-");
           strcat (str,&day); 
           strcat (str,"-");
           strcat (str,week);   
           strcat (str,":"); 
           strcat (str,&hour); 
			     strcat (str,":"); 
           strcat (str,&min); 
           strcat (str,":"); 
           strcat (str,&sec);
         	for (i=0;i<mem_slice;i++)
	          {
		     		
             rt_kprintf(" ptr: %d, %d ,%s\n",ptr [buf][i],ptr2 [buf][i],&str);  			 
	           f_printf(&fileobj, "%d,%d,%s",ptr [buf][i], ptr2 [buf][i],&str);
		     		 f_putc( '\n',&fileobj);
		     		
		     		 
		     	 }
            rt_kprintf(" try to release block\n");
            rt_kprintf(" time %s\n",&str);
            rt_mp_free(ptr[buf]);
            ptr[buf] = RT_NULL;
            ptr2[buf] = RT_NULL;
            f_close(&fileobj);
            cnt=cnt+1;
      }
          
     }
    } 

   
}


/*
**********************************************
               定时器入口函数

**********************************************
*/
//定时器超时函数
static void timeout1(void *parameter)
{ 
   u8 freq;
	  rt_err_t result,result2,result3;  
   // rt_kprintf("geting tem and hum from sensor \n ");

    /* 读取温湿度传感器信息*/
      DHT11_Read_Data(&tem,&hum);	
	
  	  //rt_kprintf("get tem from timer \n");
	   //	rt_kprintf("get hum from timer \n");
	  /*发送消息队列*/
	   
//	      			
	 //温度发送队列
        result = rt_mq_send(&mq_DHT11_tem, &tem, sizeof(tem));        
            if (result != RT_EOK)
            {
                rt_kprintf("rt_mq_send_tem ERR\n");
            }

            //rt_kprintf(" send messageque tem - %d\n", tem);
   //湿度发送队列
						result2 = rt_mq_send(&mq_DHT11_hum, &hum, sizeof(hum));        
            if (result2 != RT_EOK)
            {
                rt_kprintf("rt_mq_send_hum ERR\n");
            }

          //  rt_kprintf(" send messageque hum - %d\n", hum);
      
    
	
	  /*发送事件集*/				 
    result3=rt_event_send(&DHT11_data_event, EVENT_FLAG1);
		if (result3 != RT_EOK)
            {
                rt_kprintf("rt_event_send ERR\n");
            }
		/*else
     rt_kprintf("sendDHT11 data  event success  \n");*/
   // rt_kprintf("sampling once  \n");
   //rt_timer_control(DH11_timer, RT_TIMER_CTRL_GET_TIME , (void *)&freq);
   
						
		/*if(change_freq)
				{
			  samp_freq=samp_set;
				samp_set=0;
				adjust=0;			
				//发送修改采样频率的事件
				//rt_event_send(&changesamp_event, EVENT_FLAG7);	
				
				rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_TIME, (void *)&samp_freq);
				//rt_timer_stop(DH11_timer);
				//rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_PERIODIC, (void *)&samp_freq);
				//rt_timer_start(DH11_timer);
				//rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_PERIODIC , (void *)&samp_freq);
				//rt_kprintf("timer set %d \n",timerset);								
				change_freq=0;
				}*/
				rt_kprintf("sampfreq:%d \n",samp_freq);
				
   
}


/*
*************************************************************************
*                             LCD显示线程
*************************************************************************
*/


static rt_thread_t lcd_show = RT_NULL;
static void lcd_show_entry(void* parameter)
{	
    int buf =0;
    int result;

    while (1)
    {		
      
      rt_kprintf("thread in LCDSHOW\n");
     // key_choice_handle()	;
      
      if (rt_event_recv(& memlcd_data_event, (EVENT_FLAG2|EVENT_FLAG6),
                      RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER ,&buf ) == RT_EOK)
         
         {      
           //仅仅是被事件6唤醒
            if(buf==EVENT_FLAG6)
            {
                  key_choice_handle()	;
                   rt_kprintf(" wakeup by event6 \n");
            }
            else
            {
                key_choice_handle()	;
                //发送显示完毕消息队列
		            result=rt_mq_send(& mq_tem_memlcd, &index, sizeof(index));
                rt_kprintf(" wakeup by event2 %d \n",buf);
	              rt_kprintf(" sending  mq_tem_memlcd \n");
                rt_kprintf("index %d  \n",index);
	               if (result!= RT_EOK)
                {
                    rt_kprintf(" mq_tem_memlcd ERR\n");
                  
                    
                }
            }
         

			   
         }	        
              
    }


}





/*温度报警处理线程，采用高优先级*/
/* 定义事件任务*/
//线程控制块
static rt_thread_t over_temhum = RT_NULL;  //超温超湿度事件线程

//入口函数
static void overtem_entry(void* parameter)
{
	  //LED1_ON	
	   //等待事件触发
    
	while(1)
	{
      
     // rt_kprintf("thread in TEMOVER\n");
     	u2_printf("tem: %d\n",tem);
      u2_printf("hum: %d\n",hum);	
      if (rt_event_recv(&DHT11_data_event , (EVENT_FLAG1),
                      RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER,RT_NULL ) == RT_EOK)
			{
				if(tem>tem_flag)		
				{					
				  LED=0;
           rt_kprintf("over tem\n");
					
				}
				else
				{
					LED=1;		//当温度恢复正常的时候熄灭
				}
				
				if(hum>hum_flag)		
				{					
				   LED=0;
           rt_kprintf("over hum\n");
				}
				else
				{
					 LED=1;		//当温度恢复正常的时候熄灭
           
				}
				
       /* //线程挂起
        if(tem<tem_flag&&hum<hum_flag)	
           rt_thread_suspend(over_temhum);/* 挂起线程 */
			}					
	}
	
}

/*
*************************************************************************
*                             wifi线程
*************************************************************************
*/

//线程控制块
static rt_thread_t wifi = RT_NULL;
static void wifi_entry(void* parameter)
{
     
     //接收事件
     while (1)
    {
		 
     rt_kprintf("in wifi \n");
			
     if (rt_event_recv(&DHT11_data_event, (EVENT_FLAG1),
                      RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER,RT_NULL ) == RT_EOK)
    {
			
        		u2_printf("tem: %d\n",tem);
            u2_printf("hum: %d\n",hum);	
    }			
			
     
			   
    }
    


}






/*
*************************************************************************
*                             main 函数
*************************************************************************
*/
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */



int main(void)
{	
    /* 
	 * 开发板硬件初始化，RTT系统初始化已经在main函数之前完成，
	 * 即在component.c文件中的rtthread_startup()函数中完成了。
	 * 所以在main函数中，只需要创建线程和启动线程即可。
	 */
  /*连接云平台*/

	u2_printf("AT+ATKCLDSTA=\"39581907267257811324\",\"12345678\"\r\n");
  rt_kprintf("** 温度检测系统**\r\n");
	rt_kprintf("RT0S：RT-thread  \r\n");
	rt_kprintf("MCU：STM32F103ZET6 \r\n");
	
  uint8_t result;
  rt_err_t flag;
	int j=0;	
  /***********************挂载SD卡并打开文件*******************************/
	while(SD_Init())//检测不到SD卡
	{
		LCD_ShowString(30,150,200,16,16,"SD Card Error!");				
		LCD_ShowString(50,150,200,16,16,"Please Check! ");
		
	}
   //打开文件
  f_mount(&fatsd,"0:",1); 					//挂载SD卡 1:立即挂载


  //*****************消息队列创建	*****************************************/
  //申请内存池
	//初始化控制块
    rt_mp_init(&tem_mp, "temmp", &temmempool[0], sizeof(temmempool), mem_slice);
	  rt_mp_init(&hum_mp, "hummp", &hummempool[0], sizeof(hummempool), mem_slice);
	    rt_err_t result2;
    /* 初始化消息队列 */
    result= rt_mq_init(& mq_DHT11_tem,
                        " mq_DHT11_tem",
                        &msg_DHT11_pool_tem[0],               /* msg_pool */
                        sizeof(tem),                          /* 每个消息的大小*/
                        sizeof(msg_DHT11_pool_tem),                     /* msg_pool大小*/
                        RT_IPC_FLAG_FIFO); 
    result2 = rt_mq_init(& mq_DHT11_hum,
                        " mq_DHT11_hum",
                        &msg_DHT11_pool_hum[0],               /* msg_pool */
                        sizeof(hum),                          /* 每个消息的大小*/
                        sizeof(msg_DHT11_pool_hum),                     /* msg_pool大小*/
                        RT_IPC_FLAG_FIFO); 
    												

    if (result != RT_EOK)
    {
        rt_kprintf("init message_tem queue failed.\n");
        //return -1;
    }
		if (result2 != RT_EOK)
    {
        rt_kprintf("init message_hum queue failed.\n");
        //return -1;
    }
		
		result2 = rt_mq_init(& mq_tem_memlcd,
                        " mq_tem_memlcd",
                        &mq_tem_pool_memlcd[0],               /* msg_pool */
                        sizeof(int),                          /* 每个消息的大小*/
                        sizeof(mq_tem_pool_memlcd),                     /* msg_pool大小*/
                        RT_IPC_FLAG_FIFO); 
     if (result2 != RT_EOK)
    {
        rt_kprintf("initmq_tem_memlcd failed.\n");
       
    }
		
	//事件创建
	  //  creat_myevents();
    /* 初始化事件对象（静态） */
    result = rt_event_init(&DHT11_data_event, "DHT11_data_event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event dht11 failed.\n");
        return -1;
    }

	  result = rt_event_init(&memlcd_data_event, "memlcd_data_event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event memlcd_data_event  failed.\n");
        return -1;
    }	
		result = rt_event_init(&memlcd2_data_event, "memlcd_data_event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event memlcd_data_event  failed.\n");
        return -1;
    }	

    
    result = rt_event_init(&senddata_event, "senddata_event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event senddata_event  failed.\n");
        return -1;
    }
    
		
		//定时器
    /* DHT11 TIMER */		
    DH11_timer = rt_timer_create("DHT11_get", timeout1,
                             RT_NULL, samp_freq,
                             RT_TIMER_FLAG_PERIODIC);
    /* 启动*/
    if (DH11_timer  != RT_NULL) 
			  flag=rt_timer_start(DH11_timer );	
	  if(flag==RT_EOK)
			 rt_kprintf("DH11_timer start successfull\n");	

	//***********************事件处理线程*******************************************/
		uart_deal_event =                          /* 线程控制块指针 */
    rt_thread_create( "usart_deal_event",              /* 线程名字 */
                      uart_deal_thread_entry,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      256,                 /* 线程栈大小 */
                      2,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
    if (uart_deal_event != RT_NULL)
        rt_thread_startup(uart_deal_event);
    else
         rt_kprintf("led_thread_creat_error");
		
////*******************创建内存任务线程***********************************************************//
   for (j = 0; j < 48; j++) ptr[j] = RT_NULL;

    //创建线程
    recevDHT11 = rt_thread_create("recevDHT11", recevDHT11_entry, RT_NULL,
                            512,
                            4 ,20);
		
    if (recevDHT11 != RT_NULL)
         rt_thread_startup(recevDHT11);

//*******************创建释放内存堆写入SD卡线程***********************************************************//	
    thread_mp_release = rt_thread_create("thread_mp_release", thread_mp_release_entry, RT_NULL,
                            1024,
                            4, 20);
    if (thread_mp_release != RT_NULL)
        rt_thread_startup(thread_mp_release);
		
///*******************创LCD 显示绘图线程***********************************************************//	
		lcd_show= rt_thread_create("lcd_showwave", lcd_show_entry, RT_NULL,
                            512,
                            3, 30);
    if (lcd_show != RT_NULL)
         rt_thread_startup(lcd_show );
//*******************超温处理线程的创建***********************************************************//			
		
		over_temhum = rt_thread_create("over_temhum ", overtem_entry, RT_NULL,
                            256,
                            1, 20);
    if (lcd_show != RT_NULL)
         rt_thread_startup(over_temhum  );		
//*******************wifi线程创建***********************************************************//			
		
		wifi= rt_thread_create("wifi ", wifi_entry, RT_NULL,
                            128,
                            2, 20);
    if (wifi != RT_NULL)
         rt_thread_startup(wifi);	
  
     
//*************测试wifi模块************************************************************************//



  
  
 
		
	}



/********************************END OF FILE****************************/
