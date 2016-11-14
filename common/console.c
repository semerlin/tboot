#include "malloc.h"
#include "stdio_dev.h"
#include "common.h"
#include "serial.h"
#include "global_data.h"


DECLARE_GLOBAL_DATA_PTR;



/* 检测ctrl+c状态 */
static int32_t ctrlc_disabled = 0;	/* ctrlc使能 */
static int32_t ctrlc_was_pressed = 0;  /* ctrlc是否按下 */



/********************************************************************************
* 函数: int32_t disable_ctrlc(__in int32_t disable)
* 描述: ctrlc使能检测
* 输入: disable: 0: 检测
                1: 不检测
* 输出: none
* 返回: 上一次的使能状态
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t disable_ctrlc(__in int32_t disable)
{
    int32_t prev = ctrlc_disabled;

    ctrlc_disabled = disable;

    return prev;
}


/********************************************************************************
* 函数: int32_t had_ctrlc(void)
* 描述: 检测ctrl+c是否被按下
* 输入: none
* 输出: none
* 返回: 0: 没有按下
       1: 按下
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t had_ctrlc(void)
{
    return ctrlc_was_pressed;
}

/********************************************************************************
* 函数: void clear_ctrlc(void)
* 描述: 清除ctrl+c按下状态
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void clear_ctrlc(void)
{
    ctrlc_was_pressed = 0;
}

/********************************************************************************
* 函数: static int32_t console_setfile(__in int32_t file,
                                      __in struct stdio_dev *pDev)
* 描述: 绑定concole设备
* 输入: file: stdin: 标准输入设备
             stdout: 标准输出设备
             stderr: 标准错误设备
* 输出: none
* 返回: 0: 设置成功
       -1: 设置失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t console_setfile(__in int32_t file, __in struct stdio_dev *pDev)
{
    int rVal = 0;

    if(!pDev)
        return -1;

    switch(file)
    {
    case stdin:
    case stdout:
    case stderr:
        if(pDev->start)
        {
            rVal = pDev->start();
            if(rVal < 0)  //启动stdio设备失败
                break;
        }

        stdio_devices[file] = pDev;  //绑定设备

        break;

    default:
        rVal = -1;
        break;
    }

    return rVal;
}


#if defined(CONFIG_CONSOLE_MUX)  //统一类型节点下存在多个console设备时使用

/* 统一类型节点下存在多个console设备时使用 */
static PSTDIO_DEV tstcdev;  //记录存在有效数据的节点
PSTDIO_DEV *console_devices[MAX_FILES]; //二维数组指针，指向从设备
int cd _count[MAX_FILE];   //记录每个设备下从设备的数量

/********************************************************************************
* 函数: static int32_t console_getc(__in int32_t file)
* 描述: 取得file指定设备中的数据
* 输入: file: 取得数据的设备
* 输出: none
* 返回: 取得的数据
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t console_getc(__in int32_t file)
{
    int32_t ret = 0;

    //从有数据的节点取得数据
    ret = tstcdev->getc();
    tstcdev = NULL;

    return ret;
}


/********************************************************************************
* 函数: static int32_t console_tstc(__in int32_t file)
* 描述: 检测console设备中是否有有效的数据
* 输入: file： 检测的设备名
* 输出: none
* 返回: 0: 没有设备有有效数据
       1： 有有效数据
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t console_tstc(__in int32_t file)
{
    int32_t ret = 0;
    PSTDIO_DEV pDev;

    disable_ctrlc(1);

    //主节点下的所有从节点都测试
    for(int32_t i = 0; i < cd_count[file]; i++)
    {
        pDev = concole_devices[file][i];
        if(pDev->tstc)
        {
            ret = pDev->tstc();
            if(ret > 0)
            {
                tstcdev = pDev;
                disable_ctrlc(0);
                return ret;
            }
        }
    }

    disable_ctrlc(0);

    return 0;
}



/********************************************************************************
* 函数: static void console_putc(__in int32_t file, __in const char c)
* 描述: 通过console设备发送一个字节的数据
* 输入: file: 设备名称 stdin stdout stderr
       c: 发送的字节
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void console_putc(__in int32_t file, __in const int8_t c)
{
    PSTDIO_DEV pDev;

    for(int32_t i = 0; i < cd_count[file]; i++)
    {
        pDev = console_devices[file][i];
        if(pDev->putc)
        {
            pDev->putc(c);
        }
    }
}


/********************************************************************************
* 函数: static void console_puts(__in int32_t file, __in const char *s)
* 描述: 通过console设备发送一串数据
* 输入: file: 设备名称 stdin stdout stderr
       s: 需要发送的字符串
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void console_puts(__in int32_t file, __in const int8_t *s)
{
    PSTDIO_DEV pDev;

    for(int32_t i = 0; i < cd_count[file]; i++)
    {
        pDev = console_devices[file][i];
        if(pDev->puts)
        {
            pDev->puts(s);
        }
    }
}


/********************************************************************************
* 函数: static inline void console_printdevs(int file)
* 描述: 打印指定console类型设备名称
* 输入: file: 设备类型
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void console_printdevs(__in int32_t file)
{
    iomux_printdevs(file);
}

/********************************************************************************
* 函数: static inline void console_doenv(int file, PSTDIO_DEV dev)
* 描述: 重新注册指定类型console设备
* 输入: file: 设备类型
       dev: 设备指针
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void console_doenv(__in int32_t file, PSTDIO_DEV dev)
{
    iomux_doenv(file, dev->name);
}

#else

/********************************************************************************
* 函数: static int32_t console_getc(__in int32_t file)
* 描述: 取得file指定设备中的数据
* 输入: file: 取得数据的设备
* 输出: none
* 返回: 取得的数据
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline int console_getc(__in int32_t file)
{
	return stdio_devices[file]->getc();
}

/********************************************************************************
* 函数: static int32_t console_tstc(__in int32_t file)
* 描述: 检测console设备中是否有有效的数据
* 输入: file： 检测的设备名
* 输出: none
* 返回: 0: 没有设备有有效数据
       1： 有有效数据
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline int console_tstc(__in int32_t file)
{
	return stdio_devices[file]->tstc();
}

/********************************************************************************
* 函数: static void console_putc(__in int32_t file, __in const char c)
* 描述: 通过console设备发送一个字节的数据
* 输入: file: 设备名称 stdin stdout stderr
       c: 发送的字节
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void console_putc(__in int32_t file, __in const int8_t c)
{
	stdio_devices[file]->putc(c);
}

/********************************************************************************
* 函数: static void console_puts(__in int32_t file, __in const char *s)
* 描述: 通过console设备发送一串数据
* 输入: file: 设备名称 stdin stdout stderr
       s: 需要发送的字符串
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void console_puts(__in int32_t file, __in const int8_t *s)
{
	stdio_devices[file]->puts(s);
}

/********************************************************************************
* 函数: static inline void console_printdevs(int file)
* 描述: 打印指定console类型设备名称
* 输入: file: 设备类型
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void console_printdevs(__in int32_t file)
{
	printf("%s\n", stdio_devices[file]->name);
}

/********************************************************************************
* 函数: static inline void console_doenv(int file, PSTDIO_DEV dev)
* 描述: 重新注册指定类型console设备
* 输入: file: 设备类型
       dev: 设备指针
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void console_doenv(__in int32_t file, __in struct stdio_dev *dev)
{
	console_setfile(file, dev);
}

#endif




/********************************************************************************
* 函数: PSTDIO_DEV search_device(int32_t flag, char *name)
* 描述: 寻找指定的设备
* 输入: flag: 设备类别
       name: 设备名字
* 输出: none
* 返回: 成功: 设备的指针
       失败: NULL
* 作者:
* 版本: v1.0
**********************************************************************************/
struct stdio_dev *search_device(__in int32_t flag, __in int8_t *name)
{
    struct stdio_dev *pDev;

    pDev = stdio_get_by_name(name);

    if(pDev && (pDev->flags & flag))
        return pDev;

    return NULL;
}


/********************************************************************************
* 函数: int32_t console_assign(int32_t file, int8_t *name)
* 描述: 绑定console设备
* 输入: file: 设备类型
       name: 设备名称
* 输出: none
* 返回: -1: 绑定失败
        0: 绑定成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t console_assign(__in int32_t file, __in int8_t *name)
{
    int flag;
    struct stdio_dev *pDev;

    switch(file)
    {
    case stdin:
        flag = DEV_FLAGS_INPUT;
        break;
    case stdout:
    case stderr:
        flag = DEV_FLAGS_OUTPUT;
        break;
    default:
        return -1;
    }

    pDev = search_device(flag, name);

    if(pDev)
        return console_setfile(file, pDev);

    return -1;
}

/********************************************************************************
* 函数: void serial_printf(const int8_t *fmt, ...)
* 描述: 通过串口格式化数据输出
* 输入: fmt: 参数列表
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void serial_printf(__in const int8_t *fmt, ...)
{
    va_list args;

    int8_t printbuf[CONFIG_SYS_PBSIZE];

    va_start(args, fmt);

    vsprintf(printbuf, fmt, args);

    va_end(args);

    serial_puts(printbuf);
}

/********************************************************************************
* 函数: int32_t fgetc(int32_t file)
* 描述: 取得一个字节
* 输入: file： 设备类型
* 输出: none
* 返回: 失败: -1
       成功: 数据字节
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t fgetc(__in int32_t file)
{
    if(file < MAX_FILES)
    {
#if defined(CONFIG_CONSOLE_MUX)
        for(;;)
        {
            if(tstcdev != NULL)
                return console_getc(file);
            console_tstc(file);

#ifdef CONFIG_WATCHDOG
            udelay(1);
#endif
        }
#else
        return console_getc(file);
#endif
    }

    return -1;
}

/********************************************************************************
* 函数: int32_t ftstc(__in int32_t file)
* 描述: 检测console设备是否有有效数据
* 输入: file: 设备类型
* 输出: none
* 返回: 1: 有有效数据
       0: 没有有效数据
      -1: 错误
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t ftstc(__in int32_t file)
{
    if(file < MAX_FILES)
        return console_tstc(file);

    return -1;
}

/********************************************************************************
* 函数: void fputc(__in int32_t file, __in const char c)
* 描述: 发送一个字节的数据
* 输入: file: 设备类型
          c: 需要发送的字节
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void fputc(__in int32_t file, __in const int8_t c)
{
    if(file < MAX_FILES)
        console_putc(file, c);
}

/********************************************************************************
* 函数: void fputs(__in int32_t file, __in const int8_t *s)
* 描述: 发送字符串
* 输入: file: 设备类型
          s: 字符串
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void fputs(__in int32_t file, __in const int8_t *s)
{
    if(file < MAX_FILES)
        console_puts(file, s);
}

/********************************************************************************
* 函数: void fprintf(__in int32_t file, __in const char *fmt, ...)
* 描述: 格式化打印数据
* 输入: file: 设备类型
        fmt: 数据列表
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void fprintf(__in int32_t file, __in const char *fmt, ...)
{
    va_list args;
    int8_t printbuf[CONFIG_SYS_PBSIZE];

    va_start(args, fmt);

    vsprintf(printbuf, fmt, args);

    va_end(args);

    fputs(file, printbuf);

}

/********************************************************************************
* 函数: int32_t getc(void)
* 描述: 通过输入设备获取一个字节的数据
* 输入: none
* 输出: none
* 返回: 失败: -1
       成功: 获取到的数据
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t getc(void)
{
#ifdef CONFIG_DISABLE_CONSOLE
    if(gd->flags & GD_FLG_DISABLE_CONSOLE)
        return 0;
#endif

    if(gd->flags & GD_FLG_DEVINIT)
        return fgetc(stdin);

    return serial_getc();
}

/********************************************************************************
* 函数: int32_t tstc(void)
* 描述: 测试输入输入设备中是否有有效数据
* 输入: none
* 输出: none
* 返回: 1: 有有效数据
       0: 没有有效数据
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t tstc(void)
{
#ifdef CONFIG_DISABLE_CONSOLE
    if(gd->flags & GD_FLG_DISABLE_CONSOLE)
        return 0;
#endif

    if(gd->flags & GD_FLG_DEVINIT)
    {
        return ftstc(stdin);
    }

    return serial_tstc();
}

/********************************************************************************
* 函数: void putc(__in const int8_t c)
* 描述: 发送一个字节的数据
* 输入: c: 需要发送的字节
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void putc(__in const int8_t c)
{
#ifdef CONFIG_SILENT_CONSOLE
    if(gd->flags & GD_FLG_SILENT)
        return ;
#endif

#ifdef CONFIG_DISABLE_CONSOLE
    if(gd->flags & GD_FLG_DISABLE_CONSOLE)
        return ;
#endif

    if(gd->flags & GD_FLG_DEVINIT)
        fputc(stdout, c);
    else
        serial_putc(c);
}

/********************************************************************************
* 函数: void puts(__in const int8_t *s)
* 描述: 发送一串字符串
* 输入: s: 需要发送的字符串
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void puts(__in const int8_t *s)
{
#ifdef CONFIG_SILENT_CONSOLE
    if(gd->flags & GD_FLG_SILENT)
        return ;
#endif

#ifdef CONFIG_DISABLE_CONSOLE
    if(gd->flags & GD_FLG_DISABLE_CONSOLE)
        return ;
#endif

    if(gd->flags & GD_FLG_DEVINIT)
        fputs(stdout, s);
    else
        serial_puts(s);
}

/********************************************************************************
* 函数: void printf(const int8_t *fmt, ...)
* 描述: 格式化打印输出字符串
* 输入: fmt: 需要打印的数据
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void printf(__in const int8_t *fmt, ...)
{
    va_list args;

    int8_t printbuf[CONFIG_SYS_PBSIZE];

    va_start(args, fmt);

    vsprintf(printbuf, fmt, args);

    va_end(args);

    puts(printbuf);
}

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
void vprintf(__in const int8_t *fmt, va_list args)
{
	char printbuf[CONFIG_SYS_PBSIZE];
	vsprintf(printbuf, fmt, args);
	puts(printbuf);
}

/********************************************************************************
* 函数: int32_t ctrlc(void)
* 描述: 检测ctrlc是否按下
* 输入: none
* 输出: none
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t ctrlc(void)
{
    if(!ctrlc_disabled && gd->have_console)
    {
        if(tstc())
        {
            switch (getc())
            {
            case 0x03:
                ctrlc_was_pressed = 1;
                break;
            default:
                break;
            }
        }
    }

    return 0;
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/






















