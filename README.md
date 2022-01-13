# Design-of-temperature-and-humidity-acquisition-system-based-on Rt-Thread-STM32F103ZET6
Design of temperature and humidity acquisition system based on Rt-Thread-STM32F103ZET6

## 系统功能组成

系统涉及到的硬件由以下几部分组成：

1.交互输入部分：按键
\2. 输出部分：TFTLCD屏幕显示，SD卡存储，led灯报警
\3. 传感器采集：DHT11温湿度采集
\4. 通讯部分：串口，wifi模块
\5. 计时部分：RTC时钟计时
系统框图：![在这里插入图片描述](https://img-blog.csdnimg.cn/1103049961fb4366814044bb4d6fbd20.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBAd2VpeGluXzQ1OTM0ODY5,size_20,color_FFFFFF,t_70,g_se,x_16#pic_center)

## 系统软件设计

软件程序结构：
![在这里插入图片描述](https://img-blog.csdnimg.cn/f6d420da521e4adeaedbbc7a89f54595.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBAd2VpeGluXzQ1OTM0ODY5,size_20,color_FFFFFF,t_70,g_se,x_16#pic_center)

其中在定时器超函数中周期性采集温湿度信息，并发送消息队列到处理缓存传感器信息的线程，和超温处理线程，同时发布采集到一次传感器的事件。超温处理线程接收消息判断是否超过温度，wifi模块通过串口2将温湿度信息发送给wifi模块，由wifi模块实时上传到服务器。同时串口温度上传任务同时接收定时器发送的事件和PC机通过串口1发送的获取温度标志所触发的事件。当采集到传感器信息且上位机要求获取温湿度信息的事情同时发生时，通过串口1向PC发送温湿度信息。由于SD卡存储缓慢，所以由温湿度信息缓存线程在内存池中申请内存，并将获取到的温湿度信息缓存到内存中，同时更新LCD曲线绘制的历史温湿度信息缓存。LCD处理线程处理IO口中断发送过来的标志信息，并据此显示相应的菜单，温度湿度信息，显示所设定采样频率与阈值的情况等。SD卡存储任务作为后台任务，从缓存存储温湿度信息的内存中读取温湿度信息，并通过FATFS文件系统将相应的温湿度信息存入SD卡，同时根据每片内存的头部时间信息以及定时器采样频率推算本片内存中其他数据的时间信息，并将采样时间一起保存到SD卡中。在存储完一片内存片后，释放该内存到内存池之中。流程图如下所示：
![在这里插入图片描述](https://img-blog.csdnimg.cn/9962edb6b8754c698319a28bc2c06f1c.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBAd2VpeGluXzQ1OTM0ODY5,size_20,color_FFFFFF,t_70,g_se,x_16#pic_center)

## 部分工程效果展示

### 主菜单显示：
主菜单分为5个功能：1.温度阈值设定2.湿度阈值设定3.采样频率设定
4.温湿度曲线显示 5.温湿度实时数值显示

![在这里插入图片描述](https://img-blog.csdnimg.cn/3ac01edc4e6947d5b0db3f0de9a4a202.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBAd2VpeGluXzQ1OTM0ODY5,size_6,color_FFFFFF,t_70,g_se,x_16#pic_center)

### 温湿度，采样频率设定：
![在这里插入图片描述](https://img-blog.csdnimg.cn/4d47a750efd842c69d13e7d2e6b8ba06.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBAd2VpeGluXzQ1OTM0ODY5,size_9,color_FFFFFF,t_70,g_se,x_16#pic_center)

### 温湿度曲线：

![在这里插入图片描述](https://img-blog.csdnimg.cn/e9fce93df53d40d79992e62863220b7e.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBAd2VpeGluXzQ1OTM0ODY5,size_6,color_FFFFFF,t_70,g_se,x_16#pic_center)

### 在原子云端查看wifi发送的信息：

![在这里插入图片描述](https://img-blog.csdnimg.cn/8ec580d03f214153b2118b8392a57c0b.png#pic_center)

### SD卡csv温湿度信息文件存储效果：
![在这里插入图片描述](https://img-blog.csdnimg.cn/61b4966f30eb47c68b9aafb8ea1af939.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBAd2VpeGluXzQ1OTM0ODY5,size_6,color_FFFFFF,t_70,g_se,x_16#pic_center)
