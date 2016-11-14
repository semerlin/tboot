#include "serial.h"
#include "stdio_dev.h"
#include "string.h"

static struct serial_device *serial_devices = NULL;  //串口设备链表，第一个串口设备
static struct serial_device *serial_current = NULL;  //当前使用的串口设备


struct serial_device serial_default;


/********************************************************************************
* 函数: int32_t serial_register(struct serial_device *pDev)
* 描述: 注册串口设备
* 输入: pDev: 新串口设备信息结构体
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t serial_register(struct serial_device *pDev)
{
    pDev->next = serial_devices;
    serial_devices = pDev;

    return 0;
}


/********************************************************************************
* 函数: void serial_initialize(void)
* 描述: 初始化串口设备
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void serial_initialize(void)
{
    serial_register(&serial_default);
    serial_bind(&serial_default);
    serial_assign(serial_default.name);
}


/********************************************************************************
* 函数: void serial_stdio_init(void)
* 描述: 添加现有串口设备到stdio链表
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void serial_stdio_init(void)
{
    struct stdio_dev dev;
    struct serial_device *s = serial_devices;

    while(s)
    {
        memset(&dev, 0, sizeof(dev));

        strcpy(dev.name, s->name);
        dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;

        dev.start = s->init;
        dev.stop = s->deinit;
        dev.putc = s->putc;
        dev.puts = s->puts;
        dev.getc = s->getc;
        dev.tstc = s->tstc;

        stdio_register(&dev);

        s = s->next;
    }
}


/********************************************************************************
* 函数: int serial_assign(const char *name)
* 描述: 指定当前使用的串口
* 输入: name: 串口设备的名字
* 输出: none
* 返回: 0: 指定成功
       1: 指定失败
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t serial_assign(const int8_t *name)
{
    struct serial_device *s;
    for(s = serial_devices; s; s = s->next)
    {
        if(strcmp(s->name, name) == 0)
        {
            serial_current = s;
            return 0;
        }
    }

    return 1;
}


/********************************************************************************
* 函数: void serial_reinit_all(void)
* 描述: 重新初始化所有串口设备
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void serial_reinit_all(void)
{
    struct serial_device *s;

    for(s = serial_devices; s; s = s->next)
    {
        s->init();
    }
}


/********************************************************************************
* 函数: void serial_setbrg(void)
* 描述: 设置使用串口的寄存器
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void serial_deinit(void)
{
    if(!serial_current)
    {
        //struct serial_device *pDev = default_serial_console();
        //pDev->setbrg();
        serial_default.deinit();
        return ;
    }

    serial_current->setbrg();
}

/********************************************************************************
* 函数: void serial_setbrg(void)
* 描述: 设置使用串口的寄存器
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void serial_setbrg(void)
{
    if(!serial_current)
    {
        //struct serial_device *pDev = default_serial_console();
        //pDev->setbrg();
        serial_default.setbrg();
        return ;
    }

    serial_current->setbrg();
}



/********************************************************************************
* 函数: int32_t serial_getc(void)
* 描述: 从串口设备取得一个字节
* 输入: none
* 输出: none
* 返回: 取得的字节
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t serial_getc(void)
{
	if (!serial_current)
	{
		//struct serial_device *dev = default_serial_console ();
		//return dev->getc ();
		return serial_default.getc();
	}

	return serial_current->getc ();
}


/********************************************************************************
* 函数: int32_t serial_tstc(void)
* 描述: 测试串口设备一个字节是否准备好
* 输入: none
* 输出: none
* 返回: 0: 没有字节准备好
       1: 有字节准备好
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t serial_tstc(void)
{
	if (!serial_current)
	{
		//struct serial_device *dev = default_serial_console ();
		//return dev->tstc ();
		serial_default.tstc();
	}

	return serial_current->tstc ();
}


/********************************************************************************
* 函数: void serial_putc (const int8_t c)
* 描述: 通过串口设备发送一个字节
* 输入: c: 需要发送的字节
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void serial_putc(const int8_t c)
{
	if (!serial_current)
	{
		//struct serial_device *dev = default_serial_console ();
		//dev->putc (c);
		serial_default.putc(c);
		return;
	}

	serial_current->putc (c);
}


/********************************************************************************
* 函数: void serial_puts (const int8_t *s)
* 描述: 通过串口发送字符串
* 输入: s: 需要发送的字符串
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void serial_puts(const int8_t *s)
{
	if (!serial_current)
	{
		//struct serial_device *dev = default_serial_console ();
		//dev->puts (s);
		serial_default.puts(s);
		return;
	}

	serial_current->puts (s);
}












