/**
  ******************************************************************************
  * @file    bsp_usart.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   重定向c库printf函数到usart端口
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火STM32 F103-霸道 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
	
#include "bsp_usart3.h"
#include "rtthread.h"
#include "string.h"
extern struct rt_event DHT11_data_event;
#define EVENT_FLAG8 (1 << 8)

/* 外部定义信号量控制块 */
//extern rt_sem_t test_sem3;
rt_sem_t test_sem3;
 /**
  * @brief  配置嵌套向量中断控制器NVIC
  * @param  无
  * @retval 无
  */
 u8 Res;
 u16 USART_RX_STA=0;       //中断·标记·
 u8 USART_RX_BUF[200];     //接收缓冲,最大USART_REC_LEN个字节.
static void NVIC_Configuration2(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* 嵌套向量中断控制器组选择 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* 配置USART为中断源 */
  NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART3_IRQ;
  /* 抢断优先级*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  /* 子优先级 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  /* 使能中断 */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* 初始化配置NVIC */
  NVIC_Init(&NVIC_InitStructure);
}

 /**
  * @brief  USART GPIO 配置,工作参数配置
  * @param  无
  * @retval 无
  */
void USART3_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	

	// 打开串口GPIO的时钟
	DEBUG_USART3_GPIO_APBxClkCmd(DEBUG_USART3_GPIO_CLK, ENABLE);
	
	// 打开串口外设的时钟
	DEBUG_USART3_APBxClkCmd(DEBUG_USART3_CLK, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART3_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART3_TX_GPIO_PORT, &GPIO_InitStructure);

  // 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART3_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART3_RX_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //
	GPIO_SetBits(GPIOB,GPIO_Pin_1);                  //485EN
	
	
	
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = DEBUG_USART3_BAUDRATE;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = 
	USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(DEBUG_USART3, &USART_InitStructure);
	// 串口中断优先级配置
	NVIC_Configuration2();
	// 开启 串口空闲IDEL 中断
	USART_ITConfig(DEBUG_USART3, USART_IT_IDLE, ENABLE);  
  // 开启串口DMA接收
	//USART_DMACmd(DEBUG_USART3, USART_DMAReq_Rx, ENABLE); 
	// 使能串口
	USART_Cmd(DEBUG_USART3, ENABLE);	    
}

char Usart3_Rx_Buf[USART3_RBUFF_SIZE];

//终于搞定串口2的DMA接收  但是为什么串口1就不需要中断配置呢  很费解！！！！
void USART3_IRQHandler(void)
{
	
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)  // 空闲中断
	Res =USART_ReceiveData(USART3);	//读取接收到的数据]
	//发送事件
	 rt_event_send(& DHT11_data_event, EVENT_FLAG8);
	

	//rt_kprintf("res :%d\n",Res);
	//LCD_ShowNum(10, 230,Res, 5, 16);
    /*if((USART_RX_STA&0x8000)==0)//接收未完成
			{
			if(USART_RX_STA&0x4000)//接收到了0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
				}
			else //还没收到0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(200-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
					}		 
				}
			} */

    /*{
        Uart3_DMA_Rx_Data();

        USART_ReceiveData( USART3 ); // Clear IDLE interrupt flag bit
    }*/
}

/*
void Uart3_DMA_Rx_Data(void)
{
   // 关闭DMA ，防止干扰
   DMA_Cmd(USART3_RX_DMA_CHANNEL, DISABLE);      
   // 清DMA标志位
	 DMA_ClearFlag( DMA1_FLAG_GL3 ); 
   ///DMA_ClearFlag( DMA1_FLAG_GL5 );          
   //  重新赋值计数值，必须大于等于最大可能接收到的数据帧数目
   USART3_RX_DMA_CHANNEL->CNDTR = USART3_RBUFF_SIZE;    
   DMA_Cmd(USART3_RX_DMA_CHANNEL, ENABLE);       
   //给出二值信号量 ，发送接收到新数据标志，供前台程序查询
   rt_sem_release(test_sem3);  	
  /* 
    DMA 开启，等待数据。注意，如果中断发送数据帧的速率很快，MCU来不及处理此次接收到的数据，
    中断又发来数据的话，这里不能开启，否则数据会被覆盖。有2种方式解决：

    1. 在重新开启接收DMA通道之前，将LumMod_Rx_Buf缓冲区里面的数据复制到另外一个数组中，
    然后再开启DMA，然后马上处理复制出来的数据。

    2. 建立双缓冲，在LumMod_Uart_DMA_Rx_Data函数中，重新配置DMA_MemoryBaseAddr 的缓冲区地址，
    那么下次接收到的数据就会保存到新的缓冲区中，不至于被覆盖。
  
}*/


/*****************  发送一个字节 **********************/
void Usart3_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

/*******************************************************************************  
* 发送字节 
*******************************************************************************/    
void uart3_send_char(u8 temp)      
{        
    USART_SendData(USART3,(u8)temp);        
    while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET); //USART_FLAG_TXE 发送数据寄存器空标志位        
}    

/*******************************************************************************  
* 发送字符串  
*******************************************************************************/    
void uart3_send_buff(u8 *buf,u8 len)     
{    
    u8 t;    
    for(t=0;t<len;t++){    
    USART_SendData(USART3,buf[t]);
	while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET); //USART_FLAG_TXE 发送数据寄存器空标志位
		}			
}
/****************** 发送8位的数组 ************************/
void Usart3_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num)
{
  uint8_t i;
	
	for(i=0; i<num; i++)
  {
	    /* 发送一个字节数据到USART */
	    Usart3_SendByte(pUSARTx,array[i]);	
  
  }
	/* 等待发送完成 */
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);
}

/*****************  发送字符串 **********************/
void Usart3_SendString( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
  do 
  {
      Usart3_SendByte( pUSARTx, *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* 等待发送完成 */
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {}
}

/*****************  发送一个16位数 **********************/
void Usart3_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;
	
	/* 取出高八位 */
	temp_h = (ch&0XFF00)>>8;
	/* 取出低八位 */
	temp_l = ch&0XFF;
	
	/* 发送高八位 */
	USART_SendData(pUSARTx,temp_h);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	
	/* 发送低八位 */
	USART_SendData(pUSARTx,temp_l);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}


/***********************************************************
函数名称：int RS485_SendCmd(u8 *cmd,u8 len,int x,int wait)
函数功能：RS485问询与返回指令
入口参数：cmd：问询指令
         len：数据长度
           x：数据起始位
        wait：问询延时
出口参数：数据点
备 注：
rt_kprintf("[NBiot_SendCmd] %s\r\n", cmd);
  rt_thread_delay(100);   延时100个tick 	
***********************************************************/
 /*
int RS485_SendCmd(u8 *cmd,u8 len,int max_len,int x,int wait)
{   
  int Val = 0; 
	unsigned short CRC_Tmp;
	unsigned short crc;
	memset(Usart3_Rx_Buf,0,USART3_RBUFF_SIZE);//先清空缓冲区
  //rt_kprintf("[RS485_SendCmd] %d\r\n",len);
	GPIO_SetBits(GPIOB,GPIO_Pin_1);
	rt_thread_delay(2);
  uart3_send_buff(cmd, len);
	rt_thread_delay(2);//稍稍延时一下，原因去查看sp3485的手册吧
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);//
  rt_thread_delay(wait);
 if (strcmp(Usart3_Rx_Buf, "")!= 0) //返回值不为空if (strcmp(Usart2_Rx_Buf, "") == 0) //返回值为空
  {			
//      for(i=0;i<len+1;i++)  //打印出来接收的包共9个数据
//      {
//      rt_kprintf("%X@",Usart3_Rx_Buf[i]);	
//      }		
//			rt_kprintf("返回数据2; %d\r\n",strlen(Usart3_Rx_Buf));
		  GPIO_SetBits(GPIOB,GPIO_Pin_1);
		  rt_thread_delay(2);//稍稍延时一下，原因去查看sp3485的手册吧
			crc = ((unsigned short)Usart3_Rx_Buf[max_len-2]<<8) +(unsigned short)Usart3_Rx_Buf[max_len-1]; //收到数据的crc校验值
			CRC_Tmp = CRC_16_HEX((unsigned char *)Usart3_Rx_Buf,(unsigned char)(max_len-2)); //处理除去最后两位的数据CRC校验，算出crc校验值
//			rt_kprintf("%X\r\n",crc);
//			rt_kprintf("%X\r\n",CRC_Tmp);
			if (CRC_Tmp == crc){   //比较CRC校验值是否相等，相等则进行下一步处理
      Val = (Usart3_Rx_Buf[x]*256) + (Usart3_Rx_Buf[x+1]*1);
			return Val;
			}
			return Val;
    }  
}




*/
////重定向c库函数printf到串口，重定向后可使用printf函数
//int fputc(int ch, FILE *f)
//{
//		/* 发送一个字节数据到串口 */
//		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
//		
//		/* 等待发送完毕 */
//		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
//	
//		return (ch);
//}

/////重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
//int fgetc(FILE *f)
//{
//		/* 等待串口输入数据 */
//		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

//		return (int)USART_ReceiveData(DEBUG_USARTx);
//}

