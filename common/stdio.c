#include "config.h"
#include "common.h"
#include "malloc.h"
#include "stdio_dev.h"
#include "serial.h"
#include "global_data.h"


DECLARE_GLOBAL_DATA_PTR;

/********************************************************
* 静态变量
*********************************************************/
static struct stdio_dev devs;  //stdio设备的链表头

/********************************************************
* 全局变量
*********************************************************/
struct stdio_dev *stdio_devices[] = {NULL, NULL, NULL};   //当前正在使用的设备
int8_t *stdio_names[MAX_FILES] = {"stdin", "stdout", "stderr"};



/********************************************************
* 无输入输出设备
*********************************************************/
#ifdef CONFIG_SYS_DEVICE_NULLDEV
void nulldev_putc(const int8_t c)
{

}

void nulldev_puts(const pint8_t s)
{

}

int nulldev_input(void)
{
    return 0;
}
#endif


/********************************************************************************
* 函数: static void stdio_system_init(void)
* 描述: 初始化输入输出设备基本信息
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void stdio_system_init(void)
{
    struct stdio_dev dev;
    memset(&dev, 0, sizeof(dev));

    strcpy(dev.name, "serial");
    dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_OUTPUT | DEV_FLAGS_SYSTEM;
    dev.putc = serial_putc;
    dev.puts = serial_puts;
    dev.getc = serial_getc;
    dev.tstc = serial_tstc;

    stdio_register(&dev);   //注册窗口设备



#ifdef CONFIG_SYS_DEVICE_NULLDEV
    memset(&dev, 0, sizeof(dev));

    strcpy(dev.name, "nulldev");
    dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_OUTPUT | DEV_FLAGS_SYSTEM;
    dev.putc = nulldev_putc;
    dev.puts = nulldev_puts;
    dev.gets = nulldev_input;
    dec.tstc = nulldev_input;

    stdio_register(&dev); //注册空设备
#endif

}


/********************************************************************************
* 函数: struct list_head *stdio_get_list(void)
* 描述: 取得io设备的链表头地址
* 输入: none
* 输出: none
* 返回: io设备链表头地址
* 作者:
* 版本: v1.0
**********************************************************************************/
struct list_head *stdio_get_list(void)
{
    return &(devs.list);
}


/********************************************************************************
* 函数: struct stdio_dev *stdio_get_by_name(const int8_t *name)
* 描述: 通过名字取得io设备结构体的地址
* 输入: name: 设备的名字
* 输出: none
* 返回: 成功: 指向io设备结构体的指针
       失败: NULL
* 作者:
* 版本: v1.0
**********************************************************************************/
struct stdio_dev *stdio_get_by_name(__in const int8_t *name)
{
    struct list_head *pos;
    struct stdio_dev *pdev;

    list_for_each(pos, &(devs.list))
    {
        pdev = list_entry(pos, struct stdio_dev, list);
        if(strcmp(pdev->name, name) == 0)
            return pdev;
    }

    return NULL;
}


/********************************************************************************
* 函数: struct stdio_dev *stdio_clone(struct stdio_dev *pdev)
* 描述: 克隆一个io设备
* 输入: pdev: 需要克隆的io设备结构体地址
* 输出: none
* 返回: 成功: 指向新克隆的io设备地址
       失败: NULL
* 作者:
* 版本: v1.0
**********************************************************************************/
struct stdio_dev *stdio_clone(__in struct stdio_dev *pdev)
{
    struct stdio_dev *_pdev = NULL;

    if(!pdev)
        return NULL;

    _pdev = dlcalloc(1, sizeof(struct stdio_dev));

    if(!_pdev)
        return NULL;

    memcpy(_pdev, pdev, sizeof(struct stdio_dev));
    strncpy(_pdev->name, pdev->name, 16);

    return _pdev;
}



/********************************************************************************
* 函数: int32_t stdio_register(struct stdio_dev *pdev)
* 描述: 注册一个io设备
* 输入: pdev: 需要注册的io设备信息
* 输出: none
* 返回: 成功: 0
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t stdio_register(__in struct stdio_dev *pdev)
{
    struct stdio_dev *_pdev;

    _pdev = stdio_clone(pdev);
    if(!_pdev)
        return -1;

    list_add_tail(&(_pdev->list), &(devs.list));

    return 0;
}



/********************************************************************************
* 函数: int32_t stdio_deregister(__in const int8_t *pname)
* 描述: 卸载一个io设备
* 输入: pname: 设备的名称
* 输出:
* 返回: 成功: 0
        失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
#ifdef CONFIG_SYS_STDIO_DEREGISTER
int32_t stdio_deregister(__in const int8_t *pname)
{
    int32_t i = 0;
    struct stdio_dev *pdev = NULL;
    struct list_head *pos = NULL;
    int8_t temp_names[3][16];

    pdev = stdio_get_by_name(pname);
    if(!pdev)
        return -1;

    for(i = 0; i < MAX_FILES; i++)
    {
        //设备被分配，不能释放
        if(stdio_devices[i] == pdev)
            return -1;

        memcpy(&temp_name[i][0], stdio_devices[i]->name, sizeof(stdio_dev[i]->name)));
    }

    list_del(&(pdev->list));
    dlfree(pdev);

    list_for_each(pos, &(devs.list))
    {
        pdev = list_entry(pos, STDIO_DEV, list);
        for(i = 0; i < MAX_FILES; i++)
        {
            //重新分配设备
            if(strcmp(pdev->name, temp_name[i]) == 0)
                stdio_devices[i] = pdev;
        }
    }

    return 0;
}
#endif



/********************************************************************************
* 函数: int32_t stdio_init(void)
* 描述: 初始化输入输出设备
* 输入: none
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t stdio_init(void)
{
#ifndef CONFIG_ARM
    int32_t i;
    for(i = 0; i < (sizeof(stdio_names) / sizeof(int8_t *)); ++i)
    {
        stdio_names[i] = (int8_t *)(stdio_names[i] + gd->reloc_off);
    }
#endif


    INIT_LIST_HEAD(&(devs.list));

    stdio_system_init();

    return 0;
}









