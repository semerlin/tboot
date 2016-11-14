#ifndef _CLOCK_H_
  #define _CLOCK_H_

#include "stddef.h"
#include "list.h"

/*****************************
* 时钟标记
******************************/
/* 时钟不可关闭，一直使能 */
#define CLK_FLG_ALWAYS_ENABLE   (0x00000001)
/* 时钟频率固定，不可修改，不可进入其他模式 */
#define CLK_FLG_RATE_FIXED      (0x00000002)
/* 时钟频率可以动态调整，可以进入低功耗等其他模式*/
#define CLK_FLG_RATE_DYNAMIC    (0x00000004)


/*****************************
* 时钟状态
******************************/
/* 时钟开启 */
#define CLK_STATUS_ON              (0x00000001)
/* 时钟关闭 */
#define CLK_STATUS_OFF             (0x00000002)
/* 时钟处于空闲状态 */
#define CLK_STATUS_IDLE            (0x00000004)
/* 时钟处于睡眠状态 */
#define CLK_STATUS_SLEEP           (0x00000008)

/*****************************
* 时钟模式
******************************/
/* 时钟开启 */
#define CLK_MODE_ON              (0x00000001)
/* 时钟关闭 */
#define CLK_MODE_OFF             (0x00000002)
/* 时钟处于空闲模式 */
#define CLK_MODE_IDLE            (0x00000004)
/* 时钟处于睡眠模式 */
#define CLK_MODE_SLEEP           (0x00000008)

/* 时钟模块结构体 */
struct clk
{
    /* 此始终的参考时钟 */
    struct clk *parent;

    /* 此时钟id，id小的时钟需要先初始化 */
    //uint8_t id;

    /* 此时钟名字 */
    int8_t name[16];

    /* 此时钟设备标记位 */
    uint32_t flags;

    /* 时钟当前状态 */
    uint32_t status;

    /********************************************************************************
    * 函数: int32_t (*set_rate)(__in uint64_t rate);
    * 描述: 设置clk设备的时钟
    * 输入: rate: 时钟频率
    * 输出: none
    * 返回: 0: 设置成功
    * 作者:
    * 版本: v1.0
    **********************************************************************************/
    int32_t (*set_rate)(__in struct clk *clk, __in uint64_t rate);

    /********************************************************************************
    * 函数: uint64_t (*get_rate)(void);
    * 描述: 取得clk设备的时钟
    * 输入: none
    * 输出: none
    * 返回: clk设备的时钟频率
    * 作者:
    * 版本: v1.0
    **********************************************************************************/
    uint64_t (*get_rate)(__in struct clk *clk);

    /********************************************************************************
    * 函数: uint64_t (*round_rate)(__in uint64_t rate);
    * 描述: 根据输入的时钟频率计算可实现的clk设备的近似频率
    * 输入: rate: 指定的频率
    * 输出: none
    * 返回: 可以实现的近似频率
    * 作者:
    * 版本: v1.0
    **********************************************************************************/
    uint64_t (*round_rate)(__in struct clk *clk, __in uint64_t rate);

    /********************************************************************************
    * 函数: int32_t (*enable)(void);
    * 描述: 使能clk设备的时钟
    * 输入: clk: 需要使能的clk设备的指针
    * 输出: none
    * 返回: 0: 成功
    * 作者:
    * 版本: v1.0
    **********************************************************************************/
    int32_t (*enable)(__in struct clk *clk);

    /********************************************************************************
    * 函数: void (*disable)(void);
    * 描述: 禁止clk设备时钟
    * 输入: none
    * 输出: none
    * 返回: 0: 成功
    * 作者:
    * 版本: v1.0
    **********************************************************************************/
    int32_t (*disable)(__in struct clk *clk);

    /********************************************************************************
    * 函数: int32_t (*set_parent)(__in struct clk *parent);
    * 描述: 设置clk设备继承时钟的主设备
    * 输入: parent: 继承时钟的主设备
    * 输出: none
    * 返回: 0: 设置成功
    * 作者:
    * 版本: v1.0
    **********************************************************************************/
    int32_t (*set_parent)(__in struct clk *clk, __in struct clk *parent);

    /* 时钟节点链表 */
    struct list_head list;
};



extern int32_t clk_register(__in struct clk *clk);
extern void clk_unregister(__in const int8_t *name);
extern struct clk *clk_get_by_name(__in const int8_t *name);

#ifdef CONFIG_POWER_CONTROL
extern int32_t clk_dynamic(int32_t mode);
#endif

extern int32_t clk_init(void);











#endif
