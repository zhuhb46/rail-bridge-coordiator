[目录说明]
.
├── Applications 	驱动LTE模块需要用到的相关驱动库文件
├── Libraries 	    STM32F10x库文件
├── Project         MDK 工程
├── User	    	用户程序开发入口

[驱动库文件说明]
./Applications
├── gpio	GPIO驱动库，用于LTE模块PEN引脚控制，复位LTE模块
├── lte		LTE模块的驱动库，用于驱动LTE模块初始化，联网，数据交互相关操作
├── systick	嘀嗒定时器驱动库，用来实现延时操作
├── tim		定时器驱动库文件，用于串口接收空闲计时与心跳包发送计时
├── uart	串口驱动库文件，串口1打印Debug信息，串口2与LTE模块通信

[移植说明]
1.例程所使用的是STM32F10x标准库（V3.5版本）实现，若移植其它MCU平台需要根据做相应的修改
2.移植文件主要需要移植目录下的Applications 文件夹中相关驱动库文件
3.移植后，需要在Options for target配置中的C/C++选项卡勾选C99 Mode，否则程序无法正常编译以及运行。