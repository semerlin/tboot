#include "stddef.h"
#include "mtd/mtd.h"
#include "errno.h"
#include "string.h"
#include "assert.h"
#include "log.h"

/* mtd分区表 */
struct mtd_info *mtd_table[MAX_MTD_DEVICES];


/********************************************************************************
* 函数: int32_t add_mtd_device(__in struct mtd_info *mtd)
* 描述: 添加mtd设备
* 输入: mtd: mtd设备句柄
* 输出: none
* 返回: 0: 添加成功
       -EINVAL: mtd参数无效
       -ENOMEM: mtd设备达到最大数量，无法继续添加
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t add_mtd_device(__in struct mtd_info *mtd)
{
    int32_t i = 0;

    if(!mtd)
        return -EINVAL;

    assert(mtd->writesize);

    for(i = 0; i < MAX_MTD_DEVICES; i++)
    {
        if(!mtd_table[i])
        {
            mtd_table[i] = mtd;
            mtd->index = i;
            mtd->usecount = 0;

            return 0;
        }
    }

    return -ENOMEM;
}

/********************************************************************************
* 函数: int32_t del_mtd_device(__in struct mtd_info *mtd)
* 描述: 删除mtd设备
* 输入: mtd: mtd设备句柄
* 输出: none
* 返回: 0:
       -ENODEV: 此mtd设备不能存在
       -EBUSY: 此mtd设备正在使用
       -EINVAL: mtd参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t del_mtd_device(__in struct mtd_info *mtd)
{
    if(!mtd)
        return -EINVAL;

    if(mtd_table[mtd->index] != mtd)
        return -ENODEV;
    else if(mtd->usecount)
    {
        printl(LOG_LEVEL_ERR, "[MTD:ERR] removing MTD device '%s(%d)' with use count %d\n",
               mtd->name, mtd->index, mtd->usecount);
        return -EBUSY;
    }
    else
    {
        mtd_table[mtd->index] = NULL;
        return 0;
    }
}

/********************************************************************************
* 函数: struct mtd_info *get_mtd_device(__in struct mtd_info *mtd,
                                       __in int32_t num)
* 描述: 获取指定的mtd设备的句柄，或检测指定的mtd设备是否还存在
* 输入: num: mtd设备的编号，为-1时检测输入的mtd设备是否还存在
       mtd: mtd设备句柄
* 输出: mtd: NULL
* 返回: -ENODEV: 设备不存在
       成功: mtd设备的句柄
* 作者:
* 版本: v1.0
**********************************************************************************/
struct mtd_info *get_mtd_device(__inout struct mtd_info *mtd, __in int32_t num)
{
    struct mtd_info *ret = NULL;
    int32_t i = 0;

    if(num == -1)
    {
        for(i = 0; i < MAX_MTD_DEVICES; i++)
        {
            if(mtd_table[i] == mtd)
                ret = mtd;
        }
    }
    else if(num < MAX_MTD_DEVICES)
    {
        ret = mtd_table[num];
        if(mtd && (mtd != ret))
            ret = NULL;
    }

    if(!ret)
        return (void *)(-ENODEV);

    ret->usecount++;

    return ret;
}

/********************************************************************************
* 函数: struct mtd_info *get_mtd_device_nm(__in const char *name)
* 描述: 按名称获取mtd设备
* 输入: name: mtd设备名称
* 输出: none
* 返回: -ENODEV: mtd设备不存在
       成功: 获取到的mtd设备的句柄
* 作者:
* 版本: v1.0
**********************************************************************************/
struct mtd_info *get_mtd_device_nm(__in const char *name)
{
    int32_t i = 0;
    struct mtd_info *mtd = NULL;

    for(i = 0; i < MAX_MTD_DEVICES; i++)
    {
        if(mtd_table[i] && !strcmp(name, mtd_table[i]->name))
        {
            mtd = mtd_table[i];
            break;
        }
    }

    if(!mtd)
        return (void *)(-ENODEV);

    mtd->usecount++;
    return mtd;
}

/********************************************************************************
* 函数: void put_mtd_device(__in struct mtd_info *mtd)
* 描述: 释放mtd设备
* 输入: mtd: 需要释的mtd设备句柄
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void put_mtd_device(__in struct mtd_info *mtd)
{
    if(mtd->usecount > 0)
        --mtd->usecount;
}

