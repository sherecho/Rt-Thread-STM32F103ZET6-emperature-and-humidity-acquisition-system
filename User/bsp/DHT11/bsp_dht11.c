#include "bsp_dht11.h"
#include "delay.h"

 //////////////////////////////////////////////////////////////////////////////////	 
//±¾³ÌÐòÖ»¹©Ñ§Ï°Ê¹ÓÃ£¬Î´¾­×÷ÕßÐí¿É£¬²»µÃÓÃÓÚÆäËüÈÎºÎÓÃÍ¾
//ALIENTEKÕ½½¢STM32¿ª·¢°å
//DHT11Êý×ÖÎÂÊª¶È´«¸ÐÆ÷Çý¶¯´úÂë	   
//ÕýµãÔ­×Ó@ALIENTEK
//¼¼ÊõÂÛÌ³:www.openedv.com
//ÐÞ¸ÄÈÕÆÚ:2012/9/12
//°æ±¾£ºV1.0
//°æÈ¨ËùÓÐ£¬µÁ°æ±Ø¾¿¡£
//Copyright(C) ¹ãÖÝÊÐÐÇÒíµç×Ó¿Æ¼¼ÓÐÏÞ¹«Ë¾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
      
//¸´Î»DHT11
void DHT11_Rst(void)	   
{                 
	DHT11_IO_OUT(); 	//SET OUTPUT
    DHT11_DQ_OUT=0; 	//À­µÍDQ
    delay_ms(20);    	//À­µÍÖÁÉÙ18ms
    DHT11_DQ_OUT=1; 	//DQ=1 
	delay_us(30);     	//Ö÷»úÀ­¸ß20~40us
}
//µÈ´ýDHT11µÄ»ØÓ¦
//·µ»Ø1:Î´¼ì²âµ½DHT11µÄ´æÔÚ
//·µ»Ø0:´æÔÚ
u8 DHT11_Check(void) 	   
{   
	u8 retry=0;
	DHT11_IO_IN();//SET INPUT	 
    while (DHT11_DQ_IN&&retry<100)//DHT11»áÀ­µÍ40~80us
	{
		retry++;
		delay_us(1);
	};	 
	if(retry>=100)return 1;
	else retry=0;
    while (!DHT11_DQ_IN&&retry<100)//DHT11À­µÍºó»áÔÙ´ÎÀ­¸ß40~80us
	{
		retry++;
		delay_us(1);
	};
	if(retry>=100)return 1;	    
	return 0;
}
//´ÓDHT11¶ÁÈ¡Ò»¸öÎ»
//·µ»ØÖµ£º1/0
u8 DHT11_Read_Bit(void) 			 
{
 	u8 retry=0;
	while(DHT11_DQ_IN&&retry<100)//µÈ´ý±äÎªµÍµçÆ½
	{
		retry++;
		delay_us(1);
	}
	retry=0;
	while(!DHT11_DQ_IN&&retry<100)//µÈ´ý±ä¸ßµçÆ½
	{
		retry++;
		delay_us(1);
	}
	delay_us(40);//µÈ´ý40us
	if(DHT11_DQ_IN)return 1;
	else return 0;		   
}
//´ÓDHT11¶ÁÈ¡Ò»¸ö×Ö½Ú
//·µ»ØÖµ£º¶Áµ½µÄÊý¾Ý
u8 DHT11_Read_Byte(void)    
{        
    u8 i,dat;
    dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT11_Read_Bit();
    }						    
    return dat;
}
//´ÓDHT11¶ÁÈ¡Ò»´ÎÊý¾Ý
//temp:ÎÂ¶ÈÖµ(·¶Î§:0~50¡ã)
//humi:Êª¶ÈÖµ(·¶Î§:20%~90%)
//·µ»ØÖµ£º0,Õý³£;1,¶ÁÈ¡Ê§°Ü
u8 DHT11_Read_Data(u8 *temp,u8 *humi)    
{        
 	u8 buf[5];
	u8 i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)//¶ÁÈ¡40Î»Êý¾Ý
		{
			buf[i]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			*humi=buf[0];
			*temp=buf[2];
		}
	}else return 1;
	return 0;	    
}
//³õÊ¼»¯DHT11µÄIO¿Ú DQ Í¬Ê±¼ì²âDHT11µÄ´æÔÚ
//·µ»Ø1:²»´æÔÚ
//·µ»Ø0:´æÔÚ    	 
u8 DHT11_Init(void)
{	 
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);	 //Ê¹ÄÜPG¶Ë¿ÚÊ±ÖÓ
	
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 //PG11¶Ë¿ÚÅäÖÃ
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //ÍÆÍìÊä³ö
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOG, &GPIO_InitStructure);				 //³õÊ¼»¯IO¿Ú
 	GPIO_SetBits(GPIOG,GPIO_Pin_11);						 //PG11 Êä³ö¸ß
			    
	DHT11_Rst();  //¸´Î»DHT11
	return DHT11_Check();//µÈ´ýDHT11µÄ»ØÓ¦
} 







