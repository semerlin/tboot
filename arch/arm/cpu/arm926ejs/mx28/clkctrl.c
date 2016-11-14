#include "stddef.h"
#include "math.h"
#include "errno.h"
#include "assert.h"
#include "clock.h"
#include "arch/arch-mx28/mx28_regs.h"
#include "arch/arch-mx28/regs_clkctrl.h"
#include "arch/arch-mx28/clkctrl.h"


/* 固定时钟频率 */
#define CLK_PLL0_1     (480000000)
#define CLK_PLL2       (50000000)
#define CLK_XTAL       (24000000)
#define CLK_32K        (32000)


/****************************** 空操作 *******************************************/

/********************************************************************************
* 函数: int32_t (*set_rate)(__in uint64_t rate);
* 描述: 设置clk设备的时钟
* 输入: rate: 时钟频率
* 输出: none
* 返回: 0: 设置成功
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t null_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    return 0;
}

/********************************************************************************
* 函数: uint64_t (*round_rate)(__in uint64_t rate);
* 描述: 根据输入的时钟频率计算可实现的clk设备的近似频率
* 输入: rate: 指定的频率
* 输出: none
* 返回: 可以实现的近似频率
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static uint64_t null_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    return 0;
}

/********************************************************************************
* 函数: int32_t (*enable)(void);
* 描述: 使能clk设备的时钟
* 输入: clk: 需要使能的clk设备的指针
* 输出: none
* 返回: 0: 成功
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t null_enable(__in struct clk *clk)
{
    return 0;
}

/********************************************************************************
* 函数: void (*disable)(void);
* 描述: 禁止clk设备时钟
* 输入: none
* 输出: none
* 返回: 0: 成功
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t null_disable(__in struct clk *clk)
{
    return 0;
}

/********************************************************************************
* 函数: int32_t (*set_parent)(__in struct clk *parent);
* 描述: 设置clk设备继承时钟的主设备
* 输入: parent: 继承时钟的主设备
* 输出: none
* 返回: 0: 设置成功
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t null_set_parent(__in struct clk *clk, __in struct clk *parent)
{
    return 0;
}




/********************************************************************************
* 函数: static int32_t check_busy(__in uint32_t reg, __in uint32_t busy_bit,
                                 __in uint32_t val)
* 描述: 检测忙标志位
* 输入: reg: 忙标志位寄存器
       busy_bit: 忙标志位
       val: 空闲值
* 输出: none
* 返回: 0: 空闲
       -ETIMEDOUT: 等待超时
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t check_busy(__in uint32_t reg, __in uint32_t busy_bit, __in uint32_t val)
{
    int32_t i = 0;
    for(i = 10000; i > 0; i--)
    {
        if((REG_RD(REGS_CLKCTRL_BASE, reg) & busy_bit) == val)
            break;
    }

    if(i <= 0)
        return -ETIMEDOUT;

    return 0;
}

#if 0
/********************************************************************************
* 函数: static bool cal_div_frac(__in uint64_t rootrate, __in uint64_t calrate,
                                __in uint32_t divmax, __out int32_t *out_div,
                                __out int32_t *out_frac)
* 描述: 计算小数分频的系数
* 输入: rootrate: 参考时钟频率
       calrate: 需要调整到的频率
       divmax: 最大分频系数
* 输出: out_div: 计算出的整数分频系数
       out_frac: 计算出的小数分频系数
* 返回: true: 找到合适的分频系数
       false: 没有合适的分频系数
* 作者:
* 版本: v1.0
**********************************************************************************/
static bool cal_div_frac(__in uint64_t rootrate, __in uint64_t calrate, __in uint32_t divmax,
                          __out int32_t *out_div, __out int32_t *out_frac)
{
    int32_t  min_frac, min_div, cur_frac, cur_div, reminder;
    uint64_t min_delta = 0xffffffffffffffffLL;
    int64_t cur_delta;
    bool found = false;

    if(0 == calrate)
        return false;

    /* 需要小数分频 */
    for(cur_div = 1; cur_div <= divmax; cur_div++)
    {
        cur_frac = rootrate * 18 / cur_div / calrate;
        reminder = (rootrate * 18) % (cur_div * calrate);

        /* 小数部分进行4舍5入 */
        if((reminder << 1) > (cur_div * calrate))
        {
            cur_frac ++;
            if((cur_frac < 18) || (cur_frac > 35))
                cur_frac --;
        }


        if((cur_frac < 18) || (cur_frac > 35))
            continue;

        cur_delta = rootrate * 18 / cur_div / cur_frac - calrate;

        if(abs(cur_delta) < min_delta)
        {
            found = true;
            min_delta = abs(cur_delta);
            min_frac = cur_frac;
            min_div = cur_div;
        }
    }

    if(found)
    {
        *out_div = min_div;
        *out_frac = min_frac;

        return true;
    }
    else
    {
        *out_div = 1;
        *out_frac = 18;
        return false;
    }


}
#endif

/****************************** ref clk ****************************************/

/********************************************************************************
* 函数: static uint64_t ref_xtal_get_rate(void)
* 描述: 获取晶振的频率
* 输入: clk: 指定的clk设备
* 输出: none
* 返回: 晶振的频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t ref_xtal_get_rate(__in struct clk *clk)
{
    return CLK_XTAL;
}


/********************************************************************************
* 函数: static uint64_t pll_get_rate(__in struct clk *clk)
* 描述: 获取pll时钟频率
* 输入: clk: 指定的pll时钟设备
* 输出: none
* 返回: pll时钟频率
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static uint64_t pll_clk_get_rate(__in struct clk *clk)
{
    if((clk == &pll_clk[0]) || (clk == &pll_clk[1]))
        return CLK_PLL0_1;
    else if(clk == &pll_clk[2])
        return CLK_PLL2;
    else
        return -1;
}

/********************************************************************************
* 函数: static int32_t pll_clk_enable(__in struct clk *clk)
* 描述: 使能pll，pll0为480MHz，可以作为clk_p,usb0的参考时钟
               pll1为480MHz，作为usb1的参考时钟
               pll2为50MHz，作为enet的参考时钟
* 输入: clk: 指定的pll时钟设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       -ETIMEDOUT: 失败，超时
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t pll_clk_enable(__in struct clk *clk)
{
    assert(clk);

    volatile int i = 0;

    if(clk == &pll_clk[0])
    {
        /* 使能pll0 */
        REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL0, BM_CLKCTRL_PLL0CTRL0_POWER);
        /* 延时10us以上 */
        for(i = 0; i < 240; i++);

        /* 锁定pll */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL1,
               (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL1) | BM_CLKCTRL_PLL0CTRL1_FORCE_LOCK));
        while((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL1) & BM_CLKCTRL_PLL0CTRL1_LOCK_COUNT)
                < 0X4B0);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL1,
               (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL1) & ~BM_CLKCTRL_PLL0CTRL1_FORCE_LOCK));

        /* 检测pll0是否成功开启 */
        if(!(REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL1) & BM_CLKCTRL_PLL0CTRL1_LOCK))
            return -ETIMEDOUT;

    }
    else if(clk == &pll_clk[1])
    {
        /* 使能pll1 */
        REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL0, BM_CLKCTRL_PLL1CTRL0_POWER);
        /* 延时10us以上 */
        for(i = 0; i < 240; i++);

        /* 锁定pll */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL1,
               (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL1) | BM_CLKCTRL_PLL1CTRL1_FORCE_LOCK));
        while((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL1) & BM_CLKCTRL_PLL1CTRL1_LOCK_COUNT)
               < 0X4B0);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL1,
               (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL1) & ~BM_CLKCTRL_PLL1CTRL1_FORCE_LOCK));

        /* 检测pll1是否成功开启 */
        if(!(REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL1) & BM_CLKCTRL_PLL1CTRL1_LOCK))
           return -ETIMEDOUT;

    }
    else if(clk == &pll_clk[2])
    {
         /* 使能pll0 */
        REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL2CTRL0, BM_CLKCTRL_PLL2CTRL0_POWER);
        /* 延时10us以上 */
        for(i = 0; i < 240; i++);

        /* 使能enet pll */
        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL2CTRL0, BM_CLKCTRL_PLL2CTRL0_CLKGATE);

    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_ON;
    return 0;
}

/********************************************************************************
* 函数: static int32_t pll_clk_disable(__in struct clk *clk)
* 描述: 禁止pll设备
* 输入: clk: 指定的pll时钟设备
* 输出: none
* 返回: 0: 成功
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t pll_clk_disable(__in struct clk *clk)
{
    assert(clk);

    if(clk == &pll_clk[0])
    {
        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL0, BM_CLKCTRL_PLL0CTRL0_POWER);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL1,
           (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL0CTRL1) & ~BM_CLKCTRL_PLL0CTRL1_FORCE_LOCK));
    }
    else if(clk == &pll_clk[1])
    {
        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL0, BM_CLKCTRL_PLL1CTRL0_POWER);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL1,
           (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL1CTRL1) & ~BM_CLKCTRL_PLL1CTRL1_FORCE_LOCK));
    }
    else if(clk == &pll_clk[2])
    {
        REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL2CTRL0, BM_CLKCTRL_PLL2CTRL0_CLKGATE);
        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_PLL2CTRL0, BM_CLKCTRL_PLL2CTRL0_POWER);
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_OFF;

    return 0;
}

/********************************************************************************
* 函数: static int32_t clk_set_rate(__in uint64_t rootrate,
                                   __in uint64_t calrate,
                                   __in uint32_t basereg, __in uint32_t scalpos)
* 描述: 设置参考时钟频率
* 输入: rootrate: 参考频率
       calrate: 需要设置到的频率
       basereg: 需要设置的寄存器
       scalpos: 需要设置的位置
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t clk_set_rate(__in uint64_t rootrate, __in uint64_t calrate,
                              __in uint32_t basereg, __in uint32_t scalpos)
{
    if(0 == calrate)
        return -EINVAL;

    uint32_t div = (rootrate / 1000) * 18 / (calrate / 1000);
    uint32_t reminder = (rootrate / 1000) * 18 % (calrate / 1000);
    uint32_t val = 0;

    /* 小数部分4舍5入 */
    if((reminder << 1) > (calrate / 1000))
    {
        div ++;
        if(div > 35)
            div --;
    }

    if((div < 18) || (div > 35))
        return -EINVAL;

    /* 写入分频系数 */
    val = REG_RD(REGS_CLKCTRL_BASE, basereg);
    val &= ~(0x3f << scalpos);
    val |= (div << scalpos);
    REG_WR(REGS_CLKCTRL_BASE, basereg, val);

    return 0;

}

/********************************************************************************
* 函数: static uint64_t clk_get_rate(__in uint64_t rootrate, __in uint32_t basereg,
                               __in uint32_t scalbits, __in uint32_t scalpos)
* 描述: 获取参考时钟的频率
* 输入: rootrate: 参考时钟频率
       basereg: 获取的寄存器
       scalbits: 使能检测位
       scalpos: 分频系数位置
* 输出: none
* 返回: 0: 时钟未使能
       成功: 参考时钟的频率
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static uint64_t clk_get_rate(__in uint64_t rootrate, __in uint32_t basereg,
                               __in uint32_t scalbits, __in uint32_t scalpos)
{
    if((REG_RD(REGS_CLKCTRL_BASE, basereg) & scalbits) != 0)
        return 0;

    uint32_t div = ((REG_RD(REGS_CLKCTRL_BASE, basereg) >> scalpos) & 0x3f);

    return (rootrate / div);
}


/********************************************************************************
* 函数: static uint64_t clk_round_rate(__in uint64_t rootrate,
                                      __in uint64_t calrate)
* 描述: 计算能设置到的最接近rate的频率
* 输入: rootrate: 参考时钟频率
       calrate: 需要设置到的频率
* 输出: none
* 返回: 能设置到的最接近的频率
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static uint64_t clk_round_rate(__in uint64_t rootrate, __in uint64_t calrate)
{
    if(0 == calrate)
        return (rootrate * 18 / 35);

    uint32_t div = (rootrate / 1000) * 18 / (calrate / 1000);
    uint32_t reminder = (rootrate / 1000) * 18 % (calrate / 1000);



    /* 小数部分4舍5入 */
    if((reminder << 1) > (calrate / 1000))
    {
        div ++;
        if(div > 35)
            div --;
    }

    if(div < 18)
        return rootrate;

    if(div > 35)
        div = 35;

    return (rootrate * 18 / div);

}


/********************************************************************************
* 函数: static void clk_enable(__in uint32_t basereg, __in uint32_t scalbits)
* 描述: 使能参考时钟
* 输入: basereg: 需要设置的寄存器
       scalbits: 需要设置的位
* 输出: none
* 返回: none
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static void clk_enable(__in uint32_t basereg, __in uint32_t scalbits)
{
    volatile int32_t i = 0;
    uint32_t val = REG_RD(REGS_CLKCTRL_BASE, basereg);
    val &= ~(scalbits);

    REG_WR(REGS_CLKCTRL_BASE, basereg, val);

    for(i = 1000; i > 0; i--);
}


/********************************************************************************
* 函数: static void clk_disable(__in uint32_t basereg, __in uint32_t scalbits)
* 描述: 禁止参考时钟
* 输入: basereg: 需要设置的寄存器
       scalbits: 需要设置的位
* 输出: none
* 返回: none
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static void clk_disable(__in uint32_t basereg, __in uint32_t scalbits)
{
    uint32_t val = REG_RD(REGS_CLKCTRL_BASE, basereg);
    val |= (scalbits);

    REG_WR(REGS_CLKCTRL_BASE, basereg, val);
}



/********************************************************************************
* ref_cpu设置
**********************************************************************************/
static int32_t ref_cpu_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_set_rate(clk->parent->get_rate(clk->parent), rate,
                         HW_CLKCTRL_FRAC0, BP_CLKCTRL_FRAC0_CPUFRAC);
}


static uint64_t ref_cpu_get_rate(__in struct clk *clk)
{
    assert(clk);

    return clk_get_rate(clk->parent->get_rate(clk->parent), HW_CLKCTRL_FRAC0,
                         BM_CLKCTRL_FRAC0_CLKGATECPU, BP_CLKCTRL_FRAC0_CPUFRAC);
}

static uint64_t ref_cpu_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_round_rate(clk->parent->get_rate(clk->parent), rate);
}

static int32_t ref_cpu_enable(__in struct clk *clk)
{
    clk_enable(HW_CLKCTRL_FRAC0, BM_CLKCTRL_FRAC0_CLKGATECPU);
    clk->status = CLK_STATUS_ON;

    return 0;
}


static int32_t ref_cpu_disable(__in struct clk *clk)
{
    clk_disable(HW_CLKCTRL_FRAC0, BM_CLKCTRL_FRAC0_CLKGATECPU);
    clk->status = CLK_STATUS_OFF;

    return 0;
}


/********************************************************************************
* ref_emi设置
**********************************************************************************/
static int32_t ref_emi_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_set_rate(clk->parent->get_rate(clk->parent), rate,
                         HW_CLKCTRL_FRAC0, BP_CLKCTRL_FRAC0_EMIFRAC);
}


static uint64_t ref_emi_get_rate(__in struct clk *clk)
{
    assert(clk);

    return clk_get_rate(clk->parent->get_rate(clk->parent), HW_CLKCTRL_FRAC0,
                         BM_CLKCTRL_FRAC0_CLKGATEEMI, BP_CLKCTRL_FRAC0_EMIFRAC);
}

static uint64_t ref_emi_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_round_rate(clk->parent->get_rate(clk->parent), rate);
}

static int32_t ref_emi_enable(__in struct clk *clk)
{
    clk_enable(HW_CLKCTRL_FRAC0, BM_CLKCTRL_FRAC0_CLKGATEEMI);
    clk->status = CLK_STATUS_ON;

    return 0;
}


static int32_t ref_emi_disable(__in struct clk *clk)
{
    clk_disable(HW_CLKCTRL_FRAC0, BM_CLKCTRL_FRAC0_CLKGATEEMI);
    clk->status = CLK_STATUS_OFF;

    return 0;
}


/********************************************************************************
* ref_io1设置
**********************************************************************************/
static int32_t ref_io1_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_set_rate(clk->parent->get_rate(clk->parent), rate,
                         HW_CLKCTRL_FRAC0, BP_CLKCTRL_FRAC0_IO1FRAC);
}


static uint64_t ref_io1_get_rate(__in struct clk *clk)
{
    assert(clk);

    return clk_get_rate(clk->parent->get_rate(clk->parent), HW_CLKCTRL_FRAC0,
                         BM_CLKCTRL_FRAC0_CLKGATEIO1, BP_CLKCTRL_FRAC0_IO1FRAC);
}

static uint64_t ref_io1_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_round_rate(clk->parent->get_rate(clk->parent), rate);
}

static int32_t ref_io1_enable(__in struct clk *clk)
{
    clk_enable(HW_CLKCTRL_FRAC0, BM_CLKCTRL_FRAC0_CLKGATEIO1);
    clk->status = CLK_STATUS_ON;

    return 0;
}


static int32_t ref_io1_disable(__in struct clk *clk)
{
    clk_disable(HW_CLKCTRL_FRAC0, BM_CLKCTRL_FRAC0_CLKGATEIO1);
    clk->status = CLK_STATUS_OFF;

    return 0;
}

/********************************************************************************
* ref_io0设置
**********************************************************************************/
static int32_t ref_io0_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_set_rate(clk->parent->get_rate(clk->parent), rate,
                         HW_CLKCTRL_FRAC0, BP_CLKCTRL_FRAC0_IO0FRAC);
}


static uint64_t ref_io0_get_rate(__in struct clk *clk)
{
    assert(clk);

    return clk_get_rate(clk->parent->get_rate(clk->parent), HW_CLKCTRL_FRAC0,
                         BM_CLKCTRL_FRAC0_CLKGATEIO0, BP_CLKCTRL_FRAC0_IO0FRAC);
}

static uint64_t ref_io0_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_round_rate(clk->parent->get_rate(clk->parent), rate);
}

static int32_t ref_io0_enable(__in struct clk *clk)
{
    clk_enable(HW_CLKCTRL_FRAC0, BM_CLKCTRL_FRAC0_CLKGATEIO0);
    clk->status = CLK_STATUS_ON;

    return 0;
}


static int32_t ref_io0_disable(__in struct clk *clk)
{
    clk_disable(HW_CLKCTRL_FRAC0, BM_CLKCTRL_FRAC0_CLKGATEIO0);
    clk->status = CLK_STATUS_OFF;

    return 0;
}


/********************************************************************************
* ref_gpmi设置
**********************************************************************************/
static int32_t ref_gpmi_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_set_rate(clk->parent->get_rate(clk->parent), rate,
                         HW_CLKCTRL_FRAC1, BP_CLKCTRL_FRAC1_GPMIFRAC);
}


static uint64_t ref_gpmi_get_rate(__in struct clk *clk)
{
    assert(clk);

    return clk_get_rate(clk->parent->get_rate(clk->parent), HW_CLKCTRL_FRAC1,
                         BM_CLKCTRL_FRAC1_CLKGATEGPMI, BP_CLKCTRL_FRAC1_GPMIFRAC);
}

static uint64_t ref_gpmi_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_round_rate(clk->parent->get_rate(clk->parent), rate);
}

static int32_t ref_gpmi_enable(__in struct clk *clk)
{
    clk_enable(HW_CLKCTRL_FRAC1, BM_CLKCTRL_FRAC1_CLKGATEGPMI);
    clk->status = CLK_STATUS_ON;

    return 0;
}


static int32_t ref_gpmi_disable(__in struct clk *clk)
{
    clk_disable(HW_CLKCTRL_FRAC1, BM_CLKCTRL_FRAC1_CLKGATEGPMI);
    clk->status = CLK_STATUS_OFF;

    return 0;
}


/********************************************************************************
* ref_hsadc设置
**********************************************************************************/
static int32_t ref_hsadc_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_set_rate(clk->parent->get_rate(clk->parent), rate,
                         HW_CLKCTRL_FRAC1, BP_CLKCTRL_FRAC1_HSADCFRAC);
}


static uint64_t ref_hsadc_get_rate(__in struct clk *clk)
{
    assert(clk);

    return clk_get_rate(clk->parent->get_rate(clk->parent), HW_CLKCTRL_FRAC1,
                         BM_CLKCTRL_FRAC1_CLKGATEHSADC, BP_CLKCTRL_FRAC1_HSADCFRAC);
}

static uint64_t ref_hsadc_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_round_rate(clk->parent->get_rate(clk->parent), rate);
}

static int32_t ref_hsadc_enable(__in struct clk *clk)
{
    clk_enable(HW_CLKCTRL_FRAC1, BM_CLKCTRL_FRAC1_CLKGATEHSADC);
    clk->status = CLK_STATUS_ON;

    return 0;
}


static int32_t ref_hsadc_disable(__in struct clk *clk)
{
    clk_disable(HW_CLKCTRL_FRAC1, BM_CLKCTRL_FRAC1_CLKGATEHSADC);
    clk->status = CLK_STATUS_OFF;

    return 0;
}


/********************************************************************************
* ref_pix设置
**********************************************************************************/
static int32_t ref_pix_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_set_rate(clk->parent->get_rate(clk->parent), rate,
                         HW_CLKCTRL_FRAC1, BP_CLKCTRL_FRAC1_PIXFRAC);
}


static uint64_t ref_pix_get_rate(__in struct clk *clk)
{
    assert(clk);

    return clk_get_rate(clk->parent->get_rate(clk->parent), HW_CLKCTRL_FRAC1,
                         BM_CLKCTRL_FRAC1_CLKGATEPIX, BP_CLKCTRL_FRAC1_PIXFRAC);
}

static uint64_t ref_pix_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    return clk_round_rate(clk->parent->get_rate(clk->parent), rate);
}

static int32_t ref_pix_enable(__in struct clk *clk)
{
    clk_enable(HW_CLKCTRL_FRAC1, BM_CLKCTRL_FRAC1_CLKGATEPIX);
    clk->status = CLK_STATUS_ON;

    return 0;
}


static int32_t ref_pix_disable(__in struct clk *clk)
{
    clk_disable(HW_CLKCTRL_FRAC1, BM_CLKCTRL_FRAC1_CLKGATEPIX);
    clk->status = CLK_STATUS_OFF;

    return 0;
}

/*************************** end of ref clk ************************************/






/********************************************************************************
* 函数: static int32_t pclk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设备clk_p的时钟频率
* 输入: clk: clk设备的指针
       rate: 需要设定的频率
* 输出: none
* 返回: 0: 设置成功
       -EINVAL: 参数无效
       ETIMEDOUT: 设置超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t pclk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);
    if(0 == rate)
        return -EINVAL;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;
    uint32_t val = 0;

    /* 频率过高 */
    if(0 == div)
        return -EINVAL;

    if(clk->parent == &ref_xtal)
    {
        /* 小数4舍5入 */
        if((reminder << 1) > rate)
        {
            div ++;
            if(div > 0x3ff)
                div --;
        }

        if(div > 0x3ff) /* 超出最大分频系数 */
            return -EINVAL;

        /* 禁止小数分频 */
        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CPU, BM_CLKCTRL_CPU_DIV_XTAL_FRAC_EN);

        /* 只有空闲时才写入系数 */
        if(check_busy(HW_CLKCTRL_CPU, BM_CLKCTRL_CPU_BUSY_REF_XTAL, 0) == -ETIMEDOUT)
            return -ETIMEDOUT;

        /* 写入整数分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_CPU) & ~BM_CLKCTRL_CPU_DIV_XTAL);
        val |= (div << BP_CLKCTRL_CPU_DIV_XTAL);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CPU, val);

        /* 等待分频系数写入完成 */
        return check_busy(HW_CLKCTRL_CPU, BM_CLKCTRL_CPU_BUSY_REF_XTAL, 0);
    }
    else if(clk->parent == &ref_cpu)
    {
        /* 小数4舍5入 */
        if((reminder << 1) > rate)
        {
            div ++;
            if(div > 0x3f)
                div --;
        }

        /* 不需要小数分频 */
        if(div > 0x3f) /* 超出最大分频系数 */
            return -EINVAL;

        /* 空闲时才写入分频系数 */
        if(check_busy(HW_CLKCTRL_CPU, BM_CLKCTRL_CPU_BUSY_REF_CPU, 0) == -ETIMEDOUT)
            return -ETIMEDOUT;

        /* 使能小数分频 */
        //REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CPU, BM_CLKCTRL_CPU_DIV_CPU_FRAC_EN);

        /* 写入整数分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_CPU) & ~BM_CLKCTRL_CPU_DIV_CPU);
        val |= (div << BP_CLKCTRL_CPU_DIV_CPU);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CPU, val);
        /* 等待分频系数写入完成 */
        return check_busy(HW_CLKCTRL_CPU, BM_CLKCTRL_CPU_BUSY_REF_CPU, 0);
    }
    else
        return -EINVAL;

}


/********************************************************************************
* 函数: uint64_t pclk_get_rate(__in struct clk *clk)
* 描述: 获取指定clk设备的频率
* 输入: clk: 指定的clk设备
* 输出: none
* 返回: 失败: -1
       成功: clk设备的时钟频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t pclk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    if(clk->parent == &ref_xtal)
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_CPU) & BM_CLKCTRL_CPU_DIV_XTAL) >> BP_CLKCTRL_CPU_DIV_XTAL);
    else if(clk->parent == &ref_cpu)
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_CPU) & BM_CLKCTRL_CPU_DIV_CPU) >> BP_CLKCTRL_CPU_DIV_CPU);
    else
        return -1;

    return root_rate / div;
}


/********************************************************************************
* 函数: static uint64_t pclk_round_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 计算指定clk设备能设置到的与rate最接近的值
* 输入: clk: 指定的clk设备
       rate: 需要设置的频率
* 输出: none
* 返回: 能设置到的最接近的频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t pclk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    if(0 == rate)
        return (root_rate * 18 / 35 / 0x3ff);

    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;

    if(clk->parent == &ref_xtal)
    {
        /* 需要设置的频率过高 */
        if(0 == div)
            return root_rate;

        if((reminder << 1) > rate)
        {
            div++;
            if(div > 0x3ff)
                div--;
        }

        if(div > 0x3ff) /* 超出最大分频系数 */
                div = 0x3ff;

        return (root_rate / div);
    }
    else if(clk->parent == &ref_cpu)
    {
        /* 需要设置的频率过高 */
        if(0 == div)
            return root_rate;

        /* 不需要小数分频 */

        if((reminder << 1) > rate)
        {
            div++;
            if(div > 0x3f)
                div--;
        }

        if(div > 0x3f) /* 超出最大分频系数 */
            div = 0x3f;

        return (root_rate / div);
    }
    else
        return -1;
}


/********************************************************************************
* 函数: static int32_t pclk_enable(__in struct clk *clk)
* 描述: 使能clk_p
* 输入: clk: 指向clk_p设备的clk指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: clk_p的参考时钟无效或者未设置
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t pclk_enable(__in struct clk *clk)
{
    assert(clk);

    if(clk->parent == &ref_xtal)
        REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_CPU);
    else if(clk->parent == &ref_cpu)
        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_CPU);
    else
        return -EINVAL;

    clk->status = CLK_STATUS_ON;

    return 0;
}


/********************************************************************************
* 函数: static int32_t pclk_set_parent(__in struct clk *clk,
                                      __in struct clk *parent)
* 描述: 设置clk_p的参考时钟
* 输入: clk: clk_p时钟设备指针
       parent: 参考时钟
* 输出: none
* 返回: 0: 设置成功
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t pclk_set_parent(__in struct clk *clk, __in struct clk *parent)
{
    assert(clk);

    if((parent != &ref_xtal) && (parent != &ref_cpu))
        return -EINVAL;

    clk->parent = parent;

    return 0;
}

/********************************************************************************
* 函数: static int32_t hclk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设置clk_h设备的频率
* 输入: clk: clk_h设备
       rate: 需要设置的频率
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       ETIMEDOUT: 设置超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t hclk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);
    if(0 == rate)
        return -EINVAL;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = root_rate / rate, reminder = root_rate % rate;
    uint32_t val = 0;

    /* 不支持小数分频 */
    REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_HBUS, BM_CLKCTRL_HBUS_DIV_FRAC_EN);

    if(0 == div)
        return -EINVAL;

    /* 小数部分进行4舍5入 */
    if((reminder << 1) > rate)
    {
        div ++;
        if(div > 0x0f)
            div --;
    }

    if(div > 0x0f)
        return -EINVAL;

    /* 空闲时才写入分频系数 */
    if(check_busy(HW_CLKCTRL_HBUS, BM_CLKCTRL_HBUS_ASM_BUSY, 0) == -ETIMEDOUT)
        return -ETIMEDOUT;
    /* 写入整数分频系数 */
    val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_HBUS) & ~BM_CLKCTRL_HBUS_DIV);
    val |= (div << BP_CLKCTRL_HBUS_DIV);
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_HBUS, val);

    /* 等待整数分频系数写入完成 */
    return check_busy(HW_CLKCTRL_HBUS, BM_CLKCTRL_HBUS_ASM_BUSY, 0);
}

/********************************************************************************
* 函数: static uint64_t hclk_get_rate(__in struct clk *clk)
* 描述: 获取clk_h的频率
* 输入: clk: clk_h设备指针
* 输出: none
* 返回: clk_h的频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t hclk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_HBUS) & BM_CLKCTRL_HBUS_DIV) >> BP_CLKCTRL_HBUS_DIV);

    return root_rate / div;
}


/********************************************************************************
* 函数: static uint64_t hclk_round_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 计算能设置的与rate最接近的频率
* 输入: clk: clk_h设备指针
       rate: 需要设置的频率
* 输出: none
* 返回: 能设置的最接近的频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t hclk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    if(0 == rate)
        return (root_rate / 0x0f);

    uint32_t div = root_rate / rate;

    if(0 == div)
        return root_rate;

    if(div > 0x0f)
        div = 0x0f;

    return root_rate / div;
}



/********************************************************************************
* 函数: static int32_t xclk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设置clk_x设备的频率
* 输入: clk: clk_x设备
       rate: 需要设置的频率
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       ETIMEDOUT: 设置超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t xclk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);
    if(0 == rate)
        return -EINVAL;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = root_rate / rate, reminder = root_rate % rate;
    uint32_t val = 0;

    /* 不支持小数分频 */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_XBUS, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_XBUS) & ~BM_CLKCTRL_XBUS_DIV_FRAC_EN));

    if(0 == div)
        return -EINVAL;

    /* 小数部分进行4舍5入 */
    if((reminder << 1) > rate)
    {
        div ++;
        if(div > 0x3ff)
            div --;
    }

    if(div > 0x3ff)
        return -EINVAL;

    /* 空闲时才写入分频系数 */
    if(check_busy(HW_CLKCTRL_XBUS, BM_CLKCTRL_XBUS_BUSY, 0) == -ETIMEDOUT)
        return -ETIMEDOUT;

    /* 写入整数分频系数 */
    val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_XBUS) & ~BM_CLKCTRL_XBUS_DIV);
    val |= (div << BP_CLKCTRL_XBUS_DIV);
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_XBUS, val);

    /* 等待整数分频系数写入完成 */
    return check_busy(HW_CLKCTRL_XBUS, BM_CLKCTRL_XBUS_BUSY, 0);
}

/********************************************************************************
* 函数: static uint64_t xclk_get_rate(__in struct clk *clk)
* 描述: 获取clk_x的频率
* 输入: clk: clk_x设备指针
* 输出: none
* 返回: clk_h的频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t xclk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_XBUS) & BM_CLKCTRL_XBUS_DIV) >> BP_CLKCTRL_XBUS_DIV);

    return root_rate / div;
}


/********************************************************************************
* 函数: static uint64_t xclk_round_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 计算能设置的与rate最接近的频率
* 输入: clk: clk_x设备指针
       rate: 需要设置的频率
* 输出: none
* 返回: 能设置的最接近的频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t xclk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    if(0 == rate)
        return (root_rate / 0x3ff);

    uint32_t div = root_rate / rate, reminder = root_rate % rate;

    if(0 == div)
        return root_rate;

    if(div > 0x3ff)
        div = 0x3ff;

    /* 小数部分进行4舍5入 */
    if((reminder << 1) > rate)
    {
        div ++;
        if(div > 0x3ff)
            div --;
    }

    return root_rate / div;
}

/********************************************************************************
* 函数: static uint64_t uart_clk_get_rate(__in struct clk *clk)
* 描述: 获取uart参考时钟频率
* 输入: clk: uart参考时钟设备指针
* 输出: none
* 返回: uart参考时钟频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t uart_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    int32_t div = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_XTAL) & BM_CLKCTRL_XTAL_DIV_UART);

    return (root_rate / div);
}


/********************************************************************************
* 函数: static int32_t uart_clk_enable(__in struct clk *clk)
* 描述: 使能uart参考时钟
* 输入: clk: uart参考时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t uart_clk_enable(__in struct clk *clk)
{
    REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_XTAL, BM_CLKCTRL_XTAL_UART_CLK_GATE);
    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: static uint64_t pwm_clk_get_rate(__in struct clk *clk)
* 描述: 获取pwm参考时钟频率
* 输入: clk: pwm参考时钟设备指针
* 输出: none
* 返回: pwm参考时钟频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t pwm_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    return (clk->parent->get_rate(clk->parent));
}


/********************************************************************************
* 函数: static int32_t pwm_clk_enable(__in struct clk *clk)
* 描述: 使能pwm参考时钟
* 输入: clk: pwm参考时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t pwm_clk_enable(__in struct clk *clk)
{
    REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_XTAL, BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);
    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: static uint64_t timrot_clk_get_rate(__in struct clk *clk)
* 描述: 获取timrot参考时钟频率
* 输入: clk: timrot参考时钟设备指针
* 输出: none
* 返回: timrot参考时钟频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t timrot_clk_get_rate(__in struct clk *clk)
{
    return CLK_32K;
}


/********************************************************************************
* 函数: static int32_t timrot_clk_enable(__in struct clk *clk)
* 描述: 使能timrot参考时钟
* 输入: clk: timrot参考时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t timrot_clk_enable(__in struct clk *clk)
{
    REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_XTAL, BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);
    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: static int32_t ssp_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设置ssp的时钟频率
* 输入: clk: ssp时钟设备指针
       rate: 需要设置的时钟频率
* 输出: none
* 返回: 0: 设置成功
       -EINVAL: 参数无效
       ETIMEDOUT: 设置超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t ssp_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);
    if(0 == rate)
        return -EINVAL;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = root_rate / rate, reminder = root_rate % rate;
    uint32_t val = 0;

    /* 频率过高 */
    if(0 == div)
        return -EINVAL;

    /* 小数部分进行4舍5入 */
    if((reminder << 1) > rate)
    {
        div ++;
        if(div > 0x1ff)
            div --;
    }

    if(div > 0x1ff) /* 超出最大分频系数 */
        return -EINVAL;

    if(clk == &clk_ssp[0])
    {
        if(clk->parent == &ref_xtal)
        {
            /* 禁止小数分频 */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) & ~BM_CLKCTRL_SSP0_DIV_FRAC_EN));
        }

        else if(clk->parent == &ref_io0)
        {
            /* 使能小数分频 */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) | BM_CLKCTRL_SSP0_DIV_FRAC_EN));
        }
        else
            return -EINVAL;

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) & ~BM_CLKCTRL_SSP0_CLKGATE));

        /* 只有空闲时才写入系数 */
        if(check_busy(HW_CLKCTRL_SSP0, BM_CLKCTRL_SSP0_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) | BM_CLKCTRL_SSP0_CLKGATE));
            return -ETIMEDOUT;
        }



        /* 写入整数分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) & ~BM_CLKCTRL_SSP0_DIV);
        val |= (div << BP_CLKCTRL_SSP0_DIV);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0, val);

        /* 等待分频系数写入完成 */
        if(check_busy(HW_CLKCTRL_CPU, BM_CLKCTRL_SSP0_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) | BM_CLKCTRL_SSP0_CLKGATE));
            return -ETIMEDOUT;
        }

        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) | BM_CLKCTRL_SSP0_CLKGATE));

    }
    else if(clk == &clk_ssp[1])
    {
        if(clk->parent == &ref_xtal)
        {
            /* 禁止小数分频 */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) & ~BM_CLKCTRL_SSP1_DIV_FRAC_EN));
        }
        else if(clk->parent == &ref_io0)
        {
            /* 使能小数分频 */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) | BM_CLKCTRL_SSP1_DIV_FRAC_EN));
        }
        else
            return -EINVAL;

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) & ~BM_CLKCTRL_SSP1_CLKGATE));

        /* 只有空闲时才写入系数 */
        if(check_busy(HW_CLKCTRL_SSP1, BM_CLKCTRL_SSP1_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) | BM_CLKCTRL_SSP1_CLKGATE));
            return -ETIMEDOUT;
        }


        /* 写入整数分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) & ~BM_CLKCTRL_SSP1_DIV);
        val |= (div << BP_CLKCTRL_SSP1_DIV);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1, val);

        /* 等待分频系数写入完成 */
        if(check_busy(HW_CLKCTRL_CPU, BM_CLKCTRL_SSP1_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) | BM_CLKCTRL_SSP1_CLKGATE));
            return -ETIMEDOUT;
        }

        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) | BM_CLKCTRL_SSP1_CLKGATE));

    }
    else if(clk == &clk_ssp[2])
    {
        if(clk->parent == &ref_xtal)
        {
            /* 禁止小数分频 */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) & ~BM_CLKCTRL_SSP2_DIV_FRAC_EN));
        }
        else if(clk->parent == &ref_io1)
        {
            /* 使能小数分频 */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) | BM_CLKCTRL_SSP2_DIV_FRAC_EN));
        }
        else
            return -EINVAL;

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) & ~BM_CLKCTRL_SSP2_CLKGATE));

        /* 只有空闲时才写入系数 */
        if(check_busy(HW_CLKCTRL_SSP2, BM_CLKCTRL_SSP2_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) | BM_CLKCTRL_SSP2_CLKGATE));
            return -ETIMEDOUT;
        }



        /* 写入整数分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) & ~BM_CLKCTRL_SSP2_DIV);
        val |= (div << BP_CLKCTRL_SSP2_DIV);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2, val);

        /* 等待分频系数写入完成 */
        if(check_busy(HW_CLKCTRL_CPU, BM_CLKCTRL_SSP2_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) | BM_CLKCTRL_SSP2_CLKGATE));
            return -ETIMEDOUT;
        }

        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) | BM_CLKCTRL_SSP2_CLKGATE));

    }
    else if(clk == &clk_ssp[3])
    {
        if(clk->parent == &ref_xtal)
        {
            /* 禁止小数分频 */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) & ~BM_CLKCTRL_SSP3_DIV_FRAC_EN));
        }
        else if(clk->parent == &ref_io1)
        {
            /* 使能小数分频 */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) | BM_CLKCTRL_SSP3_DIV_FRAC_EN));
        }
        else
            return -EINVAL;

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) & ~BM_CLKCTRL_SSP3_CLKGATE));

        /* 只有空闲时才写入系数 */
        if(check_busy(HW_CLKCTRL_SSP3, BM_CLKCTRL_SSP3_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) | BM_CLKCTRL_SSP3_CLKGATE));
            return -ETIMEDOUT;
        }



        /* 写入整数分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) & ~BM_CLKCTRL_SSP3_DIV);
        val |= (div << BP_CLKCTRL_SSP3_DIV);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3, val);

        /* 等待分频系数写入完成 */
        if(check_busy(HW_CLKCTRL_CPU, BM_CLKCTRL_SSP3_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) | BM_CLKCTRL_SSP3_CLKGATE));
            return -ETIMEDOUT;
        }

        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) | BM_CLKCTRL_SSP3_CLKGATE));

    }
    else
        return -EINVAL;

    return 0;
}

/********************************************************************************
* 函数: static uint64_t ssp_clk_get_rate(__in struct clk *clk)
* 描述: 取得指定ssp设备的频率
* 输入: clk: 需要取得的频率的ssp设备
* 输出: none
* 返回: 失败: -1
       成功: ssp设备的频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t ssp_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    if(clk == &clk_ssp[0])
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) & BM_CLKCTRL_SSP0_DIV) >> BP_CLKCTRL_SSP0_DIV);
    }
    else if(clk == &clk_ssp[1])
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) & BM_CLKCTRL_SSP1_DIV) >> BP_CLKCTRL_SSP1_DIV);
    }
    else if(clk == &clk_ssp[2])
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) & BM_CLKCTRL_SSP2_DIV) >> BP_CLKCTRL_SSP2_DIV);
    }
    else if(clk == &clk_ssp[3])
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) & BM_CLKCTRL_SSP3_DIV) >> BP_CLKCTRL_SSP3_DIV);
    }
    else
        return -1;

    return (root_rate / div);
}

/********************************************************************************
* 函数: static uint64_t ssp_clk_round_rate(__in struct clk *clk,
                                          __in uint64_t rate)
* 描述: 计算能实现的与rate最接近的ssp的频率
* 输入: clk: 指定的ssp设备的指针
       rate: 需要设置的频率
* 输出: none
* 返回: 能设置的最靠近的频率
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t ssp_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);

    if(0 == rate)
        return (root_rate / 0x1ff);

    uint32_t div = root_rate / rate, reminder = root_rate % rate;


    /* 频率过高 */
    if(0 == div)
        return root_rate;

    /* 小数部分进行4舍5入 */
    if((reminder << 1) > rate)
    {
        div ++;
        if(div > 0x1ff)
            div --;
    }

    if(div > 0x1ff) /* 超出最大分频系数 */
        div = 0x1ff;

    return (root_rate / div);
}

/********************************************************************************
* 函数: static int32_t ssp_clk_enable(__in struct clk *clk)
* 描述: 使能指定的clk设备时钟
* 输入: clk: clk设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t ssp_clk_enable(__in struct clk *clk)
{
    assert(clk);

    if(clk == &clk_ssp[0])
    {
        if(clk->parent == &ref_xtal)
        {
            REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SSP0);
        }
        else if(clk->parent == &ref_io0)
        {
            REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SSP0);
        }
        else
            return -EINVAL;

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) & ~BM_CLKCTRL_SSP0_CLKGATE));
    }
    else if(clk == &clk_ssp[1])
    {
        if(clk->parent == &ref_xtal)
        {
            REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SSP1);
        }
        else if(clk->parent == &ref_io0)
        {
            REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SSP1);
        }
        else
            return -EINVAL;

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) & ~BM_CLKCTRL_SSP1_CLKGATE));
    }
    else if(clk == &clk_ssp[2])
    {
        if(clk->parent == &ref_xtal)
        {
            REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SSP2);
        }
        else if(clk->parent == &ref_io1)
        {
            REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SSP2);
        }
        else
            return -EINVAL;

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) & ~BM_CLKCTRL_SSP2_CLKGATE));
    }
    else if(clk == &clk_ssp[3])
    {
        if(clk->parent == &ref_xtal)
        {
            REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SSP3);
        }
        else if(clk->parent == &ref_io1)
        {
            REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SSP3);
        }
        else
            return -EINVAL;

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) & ~BM_CLKCTRL_SSP3_CLKGATE));
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: static int32_t ssp_clk_disable(__in struct clk *clk)
* 描述: 禁止ssp时钟
* 输入: clk: ssp时钟设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t ssp_clk_disable(__in struct clk *clk)
{
    assert(clk);

    if(clk == &clk_ssp[0])
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0,
               (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP0) | BM_CLKCTRL_SSP0_CLKGATE));
    }
    else if(clk == &clk_ssp[1])
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1,
               (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP1) | BM_CLKCTRL_SSP1_CLKGATE));
    }
    else if(clk == &clk_ssp[2])
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2,
               (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP2) | BM_CLKCTRL_SSP2_CLKGATE));
    }
    else if(clk == &clk_ssp[3])
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3,
               (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SSP3) | BM_CLKCTRL_SSP3_CLKGATE));
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_OFF;

    return 0;
}

/********************************************************************************
* 函数: static int32_t ssp_clk_set_parent(__in struct clk *clk,
                                         __in struct clk *parent)
* 描述: 设置ssp设备的参考时钟设备
* 输入: clk: 需要设置的设备
       parent: 参考的设备
* 输出: none
* 返回: 0: 设置成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t ssp_clk_set_parent(__in struct clk *clk, __in struct clk *parent)
{
    assert(clk);

    if((clk == &clk_ssp[0]) || (clk == &clk_ssp[1]) ||
        (clk == &clk_ssp[2]) || (clk == &clk_ssp[3]))
    {
        if((parent == &ref_xtal) || (parent == &ref_io0) || (parent == &ref_io1))
            clk->parent = parent;
        else
            return -EINVAL;
    }
    else
        return -EINVAL;

    return 0;
}

/********************************************************************************
* 函数: static int32_t gpmi_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设置gpmi时钟频率
* 输入: clk: gpmi时钟设备指针
       rate: 需要设置的频率
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       -ETIMEDOUT: 设置时间超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);
    if(0 == rate)
        return -EINVAL;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;
    uint32_t val = 0;

    /* 频率过高 */
    if(0 == div)
        return -EINVAL;

    if((reminder << 1) > rate)
    {
        div ++;
        if(div > 0x3ff);
            div--;
    }

    /* 不需要小数分频 */
    if(div > 0x3ff) /* 超出最大分频系数 */
        return -EINVAL;

    if(clk->parent == &ref_xtal)
    {
        /* 禁止小数分频 */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) & ~BM_CLKCTRL_GPMI_DIV_FRAC_EN);
    }
    else if(clk->parent == &ref_gpmi)
    {
        /* 使能小数分频 */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) & ~BM_CLKCTRL_GPMI_DIV_FRAC_EN);
    }
    else
        return -EINVAL;


    /* 关闭CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) & ~BM_CLKCTRL_GPMI_CLKGATE);

    /* 只有空闲时才写入系数 */
    if(check_busy(HW_CLKCTRL_GPMI, BM_CLKCTRL_GPMI_BUSY, 0) == -ETIMEDOUT)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) | BM_CLKCTRL_GPMI_CLKGATE);
        return -ETIMEDOUT;
    }

    /* 写入整数分频系数 */
    val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) & ~BM_CLKCTRL_GPMI_DIV);
    val |= (div << BP_CLKCTRL_GPMI_DIV);
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, val);

    /* 等待分频系数写入完成 */
    if(check_busy(HW_CLKCTRL_GPMI, BM_CLKCTRL_GPMI_BUSY, 0) == -ETIMEDOUT)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) | BM_CLKCTRL_GPMI_CLKGATE);
        return -ETIMEDOUT;
    }


    /* 打开CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) | BM_CLKCTRL_GPMI_CLKGATE);


    return 0;


}



/********************************************************************************
* 函数: static uint64_t gpmi_clk_get_rate(__in struct clk *clk)
* 描述: 获取gpmi设备的时钟
* 输入: clk: gpmi时钟设备指针
* 输出: none
* 返回: 成功: gpmi设备时钟频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t gpmi_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    if((clk->parent == &ref_xtal) || (clk->parent == &ref_gpmi))
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) & BM_CLKCTRL_GPMI_DIV) >> BP_CLKCTRL_GPMI_DIV);
        return root_rate / div;
    }
    else
        return -1;
}

/********************************************************************************
* 函数: static uint64_t gpmi_clk_round_rate(__in struct clk *clk,
                                           __in uint64_t rate)
* 描述: 计算能实现的最接近rate的时钟频率
* 输入: clk: gpmi时钟设备的指针
       rate: 需要实现的频率
* 输出: none
* 返回: 成功: 能实现的最接近的频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint64_t gpmi_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);

    if(0 == rate)
        return (root_rate * 18 / 35 / 0x3ff);

    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;

    if((clk->parent == &ref_xtal) || (clk->parent == &ref_gpmi))
    {
        /* 需要设置的频率过高 */
        if(0 == div)
            return root_rate;

        if((reminder << 1) > rate)
        {
            div++;
            if(div > 0x3ff) /* 超出最大分频系数 */
                div--;
        }

        if(div > 0x3ff) /* 超出最大分频系数 */
            div = 0x3ff;

        return (root_rate / div);
    }
    else
        return -1;
}

/********************************************************************************
* 函数: static int32_t gpmi_clk_enable(__in struct clk *clk)
* 描述: 使能gpmi时钟
* 输入: clk: gpmi时钟设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_clk_enable(__in struct clk *clk)
{
    assert(clk);

    /* 打开CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) | BM_CLKCTRL_GPMI_CLKGATE);

    if(clk->parent == &ref_xtal)
    {
        REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);
    }
    else if(clk->parent == &ref_gpmi)
    {
        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);
    }
    else
        return -EINVAL;

    /* 关闭CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) & ~BM_CLKCTRL_GPMI_CLKGATE);

    clk->status = CLK_STATUS_ON;

    return 0;
}


/********************************************************************************
* 函数: static int32_t gpmi_clk_disable(__in struct clk *clk)
* 描述: 禁止gpmi时钟
* 输入: clk: gpmi时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_clk_disable(__in struct clk *clk)
{
    /* 打开CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI, (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_GPMI) | BM_CLKCTRL_GPMI_CLKGATE));
    clk->status = CLK_STATUS_OFF;

    return 0;
}


/********************************************************************************
* 函数: static int32_t gpmi_clk_set_parent(__in struct clk *clk,
                                          __in struct clk *parent)
* 描述: 设置gpmi的参考时钟
* 输入: clk: gpmi时钟设备指针
       parent: gpmi时钟的参考时钟
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_clk_set_parent(__in struct clk *clk, __in struct clk *parent)
{
    assert(clk);

    if((clk == &ref_xtal) || (clk == &ref_gpmi))
    {
        clk->parent = parent;
        return 0;
    }
    else
        return -EINVAL;
}

/********************************************************************************
* 函数: uint64_t spdif_clk_get_rate(__in struct clk *clk)
* 描述: 获取spdif设备的时钟频率
* 输入: clk: spdif时钟设备指针
* 输出: none
* 返回: spdif时钟设备的频率
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t spdif_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    return clk->parent->get_rate(clk->parent);
}



/********************************************************************************
* 函数: int32_t spdif_clk_enable(__in struct clk *clk)
* 描述: 使能spdif时钟设备
* 输入: clk: spdif时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t spdif_clk_enable(__in struct clk *clk)
{
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SPDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SPDIF) & ~BM_CLKCTRL_SPDIF_CLKGATE);
    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: int32_t spdif_clk_disable(__in struct clk *clk)
* 描述: 禁止spdif时钟设备
* 输入: clk: spdif时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t spdif_clk_disable(__in struct clk *clk)
{
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SPDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SPDIF) | BM_CLKCTRL_SPDIF_CLKGATE);
    clk->status = CLK_STATUS_OFF;

    return 0;
}

/********************************************************************************
* 函数: int32_t spdif_clk_set_parent(__in struct clk *clk,
                                    __in struct clk *parent)
* 描述: 设置spdif设备的参考时钟设备
* 输入: clk: spdif设备的指针
       parent: spdif设备的参考时钟
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t spdif_clk_set_parent(__in struct clk *clk, __in struct clk *parent)
{
    assert(clk);

    clk->parent = parent;

    return 0;
}

/********************************************************************************
* 函数: int32_t emi_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设置emi设备时钟频率
* 输入: clk: emi时钟设备指针
       rate: 需要设置的频率
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       -ETIMEDOUT: 设置超时
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t emi_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);
    if(0 == rate)
        return -EINVAL;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;
    volatile int32_t i;
    uint32_t val = 0;

    /* 禁止同步 */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & ~BM_CLKCTRL_EMI_SYNC_MODE_EN);
    for(i = 1000; i > 0; i--);

    /* 频率过高 */
    if(0 == div)
        return -EINVAL;

    if(clk->parent == &ref_xtal)
    {

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & ~BM_CLKCTRL_EMI_CLKGATE);

        if((reminder << 1) > rate)
        {
            div ++;
            if(div > 0x0f)
                div--;
        }

        /* 不需要小数分频 */
        if(div > 0x0f) /* 超出最大分频系数 */
            return -EINVAL;

        /* 只有空闲时才写入系数 */
        if(check_busy(HW_CLKCTRL_EMI, BM_CLKCTRL_EMI_BUSY_REF_XTAL, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) | BM_CLKCTRL_EMI_CLKGATE);
            return -ETIMEDOUT;
        }

        /* 写入整数分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & ~BM_CLKCTRL_EMI_DIV_XTAL);
        val |= (div << BP_CLKCTRL_EMI_DIV_XTAL);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, val);

        /* 等待分频系数写入完成 */
        if(check_busy(HW_CLKCTRL_EMI, BM_CLKCTRL_EMI_BUSY_REF_XTAL, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) | BM_CLKCTRL_EMI_CLKGATE);
            return -ETIMEDOUT;
        }

        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) | BM_CLKCTRL_EMI_CLKGATE);

        return 0;
    }
    else if(clk->parent == &ref_emi)
    {

        /* 不需要小数分频 */

        if(div > 0x3f) /* 超出最大分频系数 */
            return -EINVAL;

        if((reminder << 1) > rate)
        {
            div ++;
            if(div > 0x3f)
                div--;
        }

        /* 空闲时才写入分频系数 */
        if(check_busy(HW_CLKCTRL_EMI, BM_CLKCTRL_EMI_BUSY_REF_EMI, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) | BM_CLKCTRL_EMI_CLKGATE);
            return -ETIMEDOUT;
        }

        /* 写入整数分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & ~BM_CLKCTRL_EMI_DIV_EMI);
        val |=(div << BP_CLKCTRL_EMI_DIV_EMI);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, val);
        /* 等待分频系数写入完成 */
        if(check_busy(HW_CLKCTRL_EMI, BM_CLKCTRL_EMI_BUSY_REF_EMI, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) | BM_CLKCTRL_EMI_CLKGATE);
            return -ETIMEDOUT;
        }

        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) | BM_CLKCTRL_EMI_CLKGATE);
        return 0;
    }
    else
        return -EINVAL;


}

/********************************************************************************
* 函数: uint64_t emi_clk_get_rate(__in struct clk *clk)
* 描述: 获取emi设备时钟频率
* 输入: clk: emi设备时钟指针
* 输出: none
* 返回: 成功: emi设备时钟频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t emi_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    if(clk->parent == &ref_xtal)
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & BM_CLKCTRL_EMI_DIV_XTAL) >> BP_CLKCTRL_EMI_DIV_XTAL);
    }
    else if(clk->parent == &ref_emi)
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & BM_CLKCTRL_EMI_DIV_EMI) >> BP_CLKCTRL_EMI_DIV_EMI);
    }
    else
        return -1;

    return root_rate / div;
}

/********************************************************************************
* 函数: uint64_t emi_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 计算能设置的与rate最接近的时钟频率
* 输入: clk: emi设备的指针
       rate: 需要设置到的频率
* 输出: none
* 返回: 成功: 能设置到的最接近的频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t emi_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);

    if(0 == rate)
        return (root_rate * 18 / 35 / 0x3f);

    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;

    /* 需要设置的频率过高 */
    if(0 == div)
        return root_rate;


    if(clk->parent == &ref_xtal)
    {
        if((reminder << 1) > rate)
        {
            div ++;
            if(div > 0x0f)
                div--;
        }

        if(div > 0x0f) /* 超出最大分频系数 */
            div = 0x0f;


    }
    else if(clk->parent == &ref_emi)
    {
        if((reminder << 1) > rate)
        {
            div ++;
            if(div > 0x3f)
                div--;
        }

        if(div > 0x3f) /* 超出最大分频系数 */
            div = 0x3f;
    }
    else
        return -1;

    return (root_rate / div);
}

/********************************************************************************
* 函数: int32_t emi_clk_enable(__in struct clk *clk)
* 描述: 使能emi时钟
* 输入: clk: emi时钟设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t emi_clk_enable(__in struct clk *clk)
{
    assert(clk);

    /* 禁止同步 */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & ~BM_CLKCTRL_EMI_SYNC_MODE_EN);

    if(clk->parent == &ref_xtal)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) | BM_CLKCTRL_EMI_CLKGATE);

        REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_EMI);

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & ~BM_CLKCTRL_EMI_CLKGATE);
    }
    else if(clk->parent == &ref_emi)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) | BM_CLKCTRL_EMI_CLKGATE);

        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_EMI);

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & ~BM_CLKCTRL_EMI_CLKGATE);
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: int32_t emi_clk_disable(__in struct clk *clk)
* 描述: 禁止emi时钟设备
* 输入: clk: emi时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t emi_clk_disable(__in struct clk *clk)
{
    /* 禁止同步 */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) & ~BM_CLKCTRL_EMI_SYNC_MODE_EN);

    /* 打开CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_EMI) | BM_CLKCTRL_EMI_CLKGATE);

    clk->status = CLK_STATUS_OFF;

    return 0;
}

/********************************************************************************
* 函数: int32_t emi_clk_set_parent(__in struct clk *clk, __in struct clk *parent)
* 描述: 设置emi时钟频率
* 输入: clk: emi时钟设备指针
       parent: emi时钟的参考时钟
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t emi_clk_set_parent(__in struct clk *clk, __in struct clk *parent)
{
    assert(clk);

    if((clk->parent == &ref_xtal) || (clk->parent == &ref_emi))
    {
        clk->parent = clk;
        return 0;
    }

    return -EINVAL;
}

/********************************************************************************
* 函数: int32_t saif_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设置saif设备时钟频率
* 输入: clk: saif时钟设备指针
       rate: 需要设置的频率
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       -ETIMEDOUT: 设置时间超时
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t saif_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    if(0 == rate)
        return -EINVAL;
    uint32_t div = root_rate / rate;
    uint32_t val = 0;

    if(0 == div)
        return -EINVAL;

    if(div > 0xffff)
        return -EINVAL;

    if(clk == &clk_saif[0])
    {
        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) & ~BM_CLKCTRL_SAIF0_CLKGATE);

        /* 使能小数分频(为了正常工作) */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) | BM_CLKCTRL_SAIF0_DIV_FRAC_EN);

        /* 清零bypass(为了正常工作)*/
        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SAIF0);

        /* 空闲时才写入分频系数 */
        if(check_busy(HW_CLKCTRL_SAIF0, BM_CLKCTRL_SAIF0_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) | BM_CLKCTRL_SAIF0_CLKGATE);
            return -ETIMEDOUT;
        }

        /* 写入分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) & ~BM_CLKCTRL_SAIF0_DIV);
        val |= (div << BP_CLKCTRL_SAIF0_DIV);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0, val);
        /* 等待写入完成 */
        if(check_busy(HW_CLKCTRL_SAIF0, BM_CLKCTRL_SAIF0_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) | BM_CLKCTRL_SAIF0_CLKGATE);
            return -ETIMEDOUT;
        }

        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) | BM_CLKCTRL_SAIF0_CLKGATE);
    }
    else if(clk == &clk_saif[1])
    {
        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1) & ~BM_CLKCTRL_SAIF1_CLKGATE);

        /* 使能小数分频(为了正常工作) */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1) | BM_CLKCTRL_SAIF1_DIV_FRAC_EN);

        /* 清零bypass(为了正常工作)*/
        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_SAIF1);

        /* 空闲时才写入分频系数 */
        if(check_busy(HW_CLKCTRL_SAIF1, BM_CLKCTRL_SAIF1_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1) | BM_CLKCTRL_SAIF1_CLKGATE);
            return -ETIMEDOUT;
        }

        /* 写入分频系数 */
        val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1) & ~BM_CLKCTRL_SAIF1_DIV);
        val |= (div << BP_CLKCTRL_SAIF1_DIV);
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1, val);
        /* 等待写入完成 */
        if(check_busy(HW_CLKCTRL_SAIF1, BM_CLKCTRL_SAIF1_BUSY, 0) == -ETIMEDOUT)
        {
            /* 打开CLKGATE */
            REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1) | BM_CLKCTRL_SAIF1_CLKGATE);
            return -ETIMEDOUT;
        }

        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1) | BM_CLKCTRL_SAIF1_CLKGATE);
    }
    else
        return -EINVAL;

    return 0;

}

/********************************************************************************
* 函数: uint64_t saif_clk_get_rate(__in struct clk *clk)
* 描述: 获取saif设备时钟频率
* 输入: clk: saif设备指针
* 输出: none
* 返回: 成功: saif设备时钟频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t saif_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    if(clk == &clk_saif[0])
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) & BM_CLKCTRL_SAIF0_DIV) >> BP_CLKCTRL_SAIF0_DIV);
    }
    else if(clk == &clk_saif[1])
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) & BM_CLKCTRL_SAIF0_DIV) >> BP_CLKCTRL_SAIF0_DIV);
    }
    else
        return -1;

    return (root_rate / div);
}

/********************************************************************************
* 函数: uint64_t saif_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 计算能实现的最接近rate的时钟频率
* 输入: clk: saif设备指针
       rate: 需要设置的时钟频率
* 输出: none
* 返回: 能设置到的最接近的时钟频率
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t saif_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);

    if(0 == rate)
        return root_rate / 0xffff;

    uint32_t div = root_rate / rate;
    uint32_t reminder = root_rate % rate;

    if((reminder << 1) > rate)
    {
        div++;
        if(div > 0xffff)
            div--;
    }

    if(div > 0xffff)
        div = 0xffff;

    return (root_rate / div);
}

/********************************************************************************
* 函数: int32_t saif_clk_enable(__in struct clk *clk)
* 描述: 使能saif设备的时钟
* 输入: clk: saif设备时钟指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t saif_clk_enable(__in struct clk *clk)
{
    assert(clk);

    if(clk == &clk_saif[0])
    {
        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) & ~BM_CLKCTRL_SAIF0_CLKGATE);
    }
    else if(clk == &clk_saif[1])
    {
        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1) & ~BM_CLKCTRL_SAIF1_CLKGATE);
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: int32_t saif_clk_disable(__in struct clk *clk)
* 描述: 禁止saif时钟
* 输入: clk: saif时钟设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t saif_clk_disable(__in struct clk *clk)
{
    assert(clk);

    if(clk == &clk_saif[0])
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF0) | BM_CLKCTRL_SAIF0_CLKGATE);
    }
    else if(clk == &clk_saif[1])
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_SAIF1) | BM_CLKCTRL_SAIF1_CLKGATE);
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_OFF;

    return 0;
}


/********************************************************************************
* 函数: int32_t lcdif_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设置lcdif设备时钟频率
* 输入: clk: lcdif时钟设备指针
       rate: 需要设置的频率
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       -ETIMEDOUT: 设置超时
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t lcdif_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);
    if(0 == rate)
        return -EINVAL;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;

    uint32_t val = 0;

    /* 频率过高 */
    if(0 == div)
        return -EINVAL;

    if(clk->parent == &ref_xtal)
    {
        /* 禁止小数分频 */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF,
               REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) & ~BM_CLKCTRL_DIS_LCDIF_DIV_FRAC_EN);
    }
    else if(clk->parent == &ref_pix)
    {
        /* 使能小数分频 */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF,
               REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) | BM_CLKCTRL_DIS_LCDIF_DIV_FRAC_EN);
    }
    else
        return -EINVAL;

    /* 关闭CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) & ~BM_CLKCTRL_DIS_LCDIF_CLKGATE);

    if((reminder << 1) > rate)
    {
        div ++;
        if(div > 0x1ff)
            div--;
    }

     /* 不需要小数分频 */
    if(div > 0x1ff) /* 超出最大分频系数 */
        return -EINVAL;

    /* 只有空闲时才写入系数 */
    if(check_busy(HW_CLKCTRL_DIS_LCDIF, BM_CLKCTRL_DIS_LCDIF_BUSY, 0) == -ETIMEDOUT)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) | BM_CLKCTRL_DIS_LCDIF_CLKGATE);
        return -ETIMEDOUT;
    }

    /* 写入整数分频系数 */
    val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) & ~BM_CLKCTRL_DIS_LCDIF_DIV);
    val |= (div << BP_CLKCTRL_DIS_LCDIF_DIV);
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, val);

    /* 等待分频系数写入完成 */
    if(check_busy(HW_CLKCTRL_DIS_LCDIF, BM_CLKCTRL_DIS_LCDIF_BUSY, 0) == -ETIMEDOUT)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) | BM_CLKCTRL_DIS_LCDIF_CLKGATE);
        return -ETIMEDOUT;
    }

    /* 打开CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) | BM_CLKCTRL_DIS_LCDIF_CLKGATE);

    return 0;

}

/********************************************************************************
* 函数: uint64_t lcdif_clk_get_rate(__in struct clk *clk)
* 描述: 获取lcdif设备时钟频率
* 输入: clk: lcdif设备时钟指针
* 输出: none
* 返回: 成功: lcdif设备时钟频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t lcdif_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    if((clk->parent == &ref_xtal) || (clk->parent == &ref_pix))
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) & BM_CLKCTRL_DIS_LCDIF_DIV) >> BP_CLKCTRL_DIS_LCDIF_DIV);
    }
    else
        return -1;

    return root_rate / div;
}

/********************************************************************************
* 函数: uint64_t lcdif_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 计算能设置的与rate最接近的时钟频率
* 输入: clk: lcdif设备的指针
       rate: 需要设置到的频率
* 输出: none
* 返回: 成功: 能设置到的最接近的频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t lcdif_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);

    if(0 == rate)
        return (root_rate * 18 / 35 / 0x1ff);

    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;

    if((clk->parent == &ref_xtal) || (clk->parent == &ref_pix))
    {
        /* 需要设置的频率过高 */
        if(0 == div)
            return root_rate;

        if((reminder << 1) > rate)
        {
            div++;
            if(div > 0x1ff)
                div--;
        }

        /* 不需要小数分频 */
        if(div > 0x1ff) /* 超出最大分频系数 */
            div = 0x1ff;

        return (root_rate / div);
    }
    else
        return -1;
}

/********************************************************************************
* 函数: int32_t emi_clk_enable(__in struct clk *clk)
* 描述: 使能emi时钟
* 输入: clk: emi时钟设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t lcdif_clk_enable(__in struct clk *clk)
{
    assert(clk);

    if(clk->parent == &ref_xtal)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) | BM_CLKCTRL_DIS_LCDIF_CLKGATE);

        REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF);

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) & ~BM_CLKCTRL_DIS_LCDIF_CLKGATE);
    }
    else if(clk->parent == &ref_pix)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) | BM_CLKCTRL_DIS_LCDIF_CLKGATE);

        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF);

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) & ~BM_CLKCTRL_DIS_LCDIF_CLKGATE);;
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: int32_t lcdif_clk_disable(__in struct clk *clk)
* 描述: 禁止lcdif时钟设备
* 输入: clk: lcdif时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t lcdif_clk_disable(__in struct clk *clk)
{
    /* 打开CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_DIS_LCDIF) | BM_CLKCTRL_DIS_LCDIF_CLKGATE);
    clk->status = CLK_STATUS_OFF;

    return 0;
}

/********************************************************************************
* 函数: int32_t lcdif_clk_set_parent(__in struct clk *clk, __in struct clk *parent)
* 描述: 设置lcdif时钟频率
* 输入: clk: lcdif时钟设备指针
       parent: lcdif时钟的参考时钟
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t lcdif_clk_set_parent(__in struct clk *clk, __in struct clk *parent)
{
    assert(clk);

    if((clk->parent == &ref_xtal) || (clk->parent == &ref_pix))
    {
        clk->parent = clk;
        return 0;
    }

    return -EINVAL;
}

/********************************************************************************
* 函数: int32_t etm_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设置etm设备时钟频率
* 输入: clk: etm时钟设备指针
       rate: 需要设置的频率
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       -ETIMEDOUT: 设置超时
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t etm_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);
    if(0 == rate)
        return -EINVAL;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;
    uint32_t val = 0;

    /* 频率过高 */
    if(0 == div)
        return -EINVAL;

    /* 不支持小数分频 */
    if(clk->parent == &ref_xtal)
    {
        /* 禁止小数分频 */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) & ~BM_CLKCTRL_ETM_DIV_FRAC_EN);
    }
    else if(clk->parent == &ref_cpu)
    {
        /* 使能小数分频 */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) | BM_CLKCTRL_ETM_DIV_FRAC_EN);
    }
    else
        return -EINVAL;


    /* 关闭CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) & ~BM_CLKCTRL_ETM_CLKGATE);

    if((reminder << 1) > rate)
    {
        div ++;
        if(div > 0x7f)
            div --;
    }

    /* 不需要小数分频 */
    if(div > 0x7f) /* 超出最大分频系数 */
        return -EINVAL;

    /* 只有空闲时才写入系数 */
    if(check_busy(HW_CLKCTRL_ETM, BM_CLKCTRL_ETM_BUSY, 0) == -ETIMEDOUT)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) | BM_CLKCTRL_ETM_CLKGATE);
        return -ETIMEDOUT;
    }


    /* 写入整数分频系数 */
    val = (REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) & ~BM_CLKCTRL_ETM_DIV);
    val |= (div << BP_CLKCTRL_ETM_DIV);
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, val);

    /* 等待分频系数写入完成 */
    if(check_busy(HW_CLKCTRL_DIS_LCDIF, BM_CLKCTRL_ETM_BUSY, 0) == -ETIMEDOUT)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) | BM_CLKCTRL_ETM_CLKGATE);
        return -ETIMEDOUT;
    }

    /* 打开CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) | BM_CLKCTRL_ETM_CLKGATE);

    return 0;

}

/********************************************************************************
* 函数: uint64_t etm_clk_get_rate(__in struct clk *clk)
* 描述: 获取etm设备时钟频率
* 输入: clk: etm设备时钟指针
* 输出: none
* 返回: 成功: etm设备时钟频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t etm_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    if((clk->parent == &ref_xtal) || (clk->parent == &ref_cpu))
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) & BM_CLKCTRL_ETM_DIV) >> BP_CLKCTRL_ETM_DIV);

        return root_rate / div;
    }
    else
        return -1;
}

/********************************************************************************
* 函数: uint64_t lcdif_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 计算能设置的与rate最接近的时钟频率
* 输入: clk: lcdif设备的指针
       rate: 需要设置到的频率
* 输出: none
* 返回: 成功: 能设置到的最接近的频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t etm_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);

    if(0 == rate)
        return (root_rate / 0x7f);

    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;

    if((clk->parent == &ref_xtal) || (clk->parent == &ref_cpu))
    {
        /* 需要设置的频率过高 */
        if(0 == div)
            return root_rate;

        /* 不需要小数分频 */
        if((reminder << 1) > rate)
        {
            div++;
            if(div > 0x7f)
                div--;
        }

        if(div > 0x7f) /* 超出最大分频系数 */
            div = 0x7f;

        return (root_rate / div);
    }
    else
        return -1;
}

/********************************************************************************
* 函数: int32_t emi_clk_enable(__in struct clk *clk)
* 描述: 使能emi时钟
* 输入: clk: emi时钟设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t etm_clk_enable(__in struct clk *clk)
{
    assert(clk);


    if(clk->parent == &ref_xtal)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) | BM_CLKCTRL_ETM_CLKGATE);

        REG_SET(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_ETM);

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) & ~BM_CLKCTRL_ETM_CLKGATE);
    }
    else if(clk->parent == &ref_cpu)
    {
        /* 打开CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) | BM_CLKCTRL_ETM_CLKGATE);

        REG_CLR(REGS_CLKCTRL_BASE, HW_CLKCTRL_CLKSEQ, BM_CLKCTRL_CLKSEQ_BYPASS_ETM);

        /* 关闭CLKGATE */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) & ~BM_CLKCTRL_ETM_CLKGATE);
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: int32_t lcdif_clk_disable(__in struct clk *clk)
* 描述: 禁止lcdif时钟设备
* 输入: clk: lcdif时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t etm_clk_disable(__in struct clk *clk)
{
    /* 打开CLKGATE */
    REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM, REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_ETM) | BM_CLKCTRL_ETM_CLKGATE);
    clk->status = CLK_STATUS_OFF;

    return 0;
}

/********************************************************************************
* 函数: int32_t lcdif_clk_set_parent(__in struct clk *clk, __in struct clk *parent)
* 描述: 设置lcdif时钟频率
* 输入: clk: lcdif时钟设备指针
       parent: lcdif时钟的参考时钟
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t etm_clk_set_parent(__in struct clk *clk, __in struct clk *parent)
{
    assert(clk);

    if((clk->parent == &ref_xtal) || (clk->parent == &ref_cpu))
    {
        clk->parent = clk;
        return 0;
    }

    return -EINVAL;
}



/********************************************************************************
* 函数: int32_t hsadc_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 设置hsadc设备时钟频率
* 输入: clk: hsadc时钟设备指针
       rate: 需要设置的频率
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       -ETIMEDOUT: 设置超时
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t hsadc_clk_set_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);
    if(0 == rate)
        return -EINVAL;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;
    uint32_t val = 0;

    if(clk->parent == &ref_hsadc)
    {
        /* 频率过高 */
        if(0 == div)
            return -EINVAL;

        if((reminder << 1) > rate)
        {
            div++;
        }

        val = REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_HSADC);
        val &= ~BM_CLKCTRL_HSADC_FREQDIV;
        val |= BM_CLKCTRL_HSADC_RESETB;

        switch(div)
        {
        case 9:
            val |= (0 << BP_CLKCTRL_HSADC_FREQDIV);
            break;
        case 18:
            val |= (1 << BP_CLKCTRL_HSADC_FREQDIV);
            break;
        case 36:
            val |= (2 << BP_CLKCTRL_HSADC_FREQDIV);
            break;
        case 72:
            val |= (3 << BP_CLKCTRL_HSADC_FREQDIV);
            break;
        default:
            return -EINVAL;
            break;
        }

        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_HSADC, val);

    }
    else
        return -EINVAL;


    return 0;

}

/********************************************************************************
* 函数: uint64_t etm_clk_get_rate(__in struct clk *clk)
* 描述: 获取etm设备时钟频率
* 输入: clk: etm设备时钟指针
* 输出: none
* 返回: 成功: etm设备时钟频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t hsadc_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint32_t div = 1;

    if(clk->parent == &ref_hsadc)
    {
        div = ((REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_HSADC) & BM_CLKCTRL_HSADC_FREQDIV) >> BP_CLKCTRL_HSADC_FREQDIV);
        switch(div)
        {
        case 0:
            div = 9;
            break;
        case 1:
            div = 18;
            break;
        case 2:
            div = 36;
            break;
        case 3:
            div = 72;
            break;
        default:
            break;
        }

        return root_rate / div;
    }
    else
        return -1;
}

/********************************************************************************
* 函数: uint64_t hsadc_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
* 描述: 计算能设置的与rate最接近的时钟频率
* 输入: clk: hsadc设备的指针
       rate: 需要设置到的频率
* 输出: none
* 返回: 成功: 能设置到的最接近的频率
       失败: -1
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t hsadc_clk_round_rate(__in struct clk *clk, __in uint64_t rate)
{
    assert(clk);

    if(0 == rate)
        return -1;

    uint64_t root_rate = clk->parent->get_rate(clk->parent);
    uint64_t reminder = root_rate % rate;
    uint32_t div = root_rate / rate;

    if(clk->parent == &ref_hsadc)
    {
        /* 频率过高 */
        if(0 == div)
            return (root_rate / 9);

        if((reminder << 1) > rate)
        {
            div++;
        }

        if(div % 9 == 0)
            return root_rate / div;
        else
            return -1;

    }
    else
        return -1;
}

/********************************************************************************
* 函数: int32_t hsadc_clk_enable(__in struct clk *clk)
* 描述: 使能hsadc时钟
* 输入: clk: hsadc时钟设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t hsadc_clk_enable(__in struct clk *clk)
{
    assert(clk);

    if(clk->parent == &ref_hsadc)
    {
        /* 关闭RESETB */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_HSADC,
               REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_HSADC) & ~BM_CLKCTRL_HSADC_RESETB);
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_ON;

    return 0;
}

/********************************************************************************
* 函数: int32_t hsadc_clk_disable(__in struct clk *clk)
* 描述: 禁止hsadc时钟设备
* 输入: clk: hsadc时钟设备指针
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t hsadc_clk_disable(__in struct clk *clk)
{
    assert(clk);

    if(clk->parent == &ref_hsadc)
    {
        /* 打开RESETB */
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_HSADC,
               REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_HSADC) | BM_CLKCTRL_HSADC_RESETB);
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_OFF;

    return 0;
}



/********************************************************************************
* 函数: uint64_t flexcan_clk_get_rate(__in struct clk *clk)
* 描述: 获取flexcan设备时钟频率
* 输入: clk: flexcan设备时钟指针
* 输出: none
* 返回: 设备频率
* 作者:
* 版本: v1.0
**********************************************************************************/
uint64_t flexcan_clk_get_rate(__in struct clk *clk)
{
    assert(clk);

    return (clk->parent->get_rate(clk->parent));
}


/********************************************************************************
* 函数: int32_t flexcan_clk_enable(__in struct clk *clk)
* 描述: 使能flexcan时钟
* 输入: clk: flexcan时钟设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者: hy
* 版本: v1.0
**********************************************************************************/
int32_t flexcan_clk_enable(__in struct clk *clk)
{
    assert(clk);

    if(clk == &clk_flexcan[0])
    {
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_FLEXCAN,
               REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_FLEXCAN) & ~BM_CLKCTRL_FLEXCAN_STOP_CAN0);
    }
    else if(clk == &clk_flexcan[1])
    {
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_FLEXCAN,
               REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_FLEXCAN) & ~BM_CLKCTRL_FLEXCAN_STOP_CAN1);
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_ON;

    return 0;
}


/********************************************************************************
* 函数: int32_t flexcan_clk_disable(__in struct clk *clk)
* 描述: 禁止flexcan时钟
* 输入: clk: flexcan设备指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
* 作者: hy
* 版本: v1.0
**********************************************************************************/
int32_t flexcan_clk_disable(__in struct clk *clk)
{
    assert(clk);

    if(clk == &clk_flexcan[0])
    {
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_FLEXCAN,
               REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_FLEXCAN) | BM_CLKCTRL_FLEXCAN_STOP_CAN0);
    }
    else if(clk == &clk_flexcan[1])
    {
        REG_WR(REGS_CLKCTRL_BASE, HW_CLKCTRL_FLEXCAN,
               REG_RD(REGS_CLKCTRL_BASE, HW_CLKCTRL_FLEXCAN) | BM_CLKCTRL_FLEXCAN_STOP_CAN1);
    }
    else
        return -EINVAL;

    clk->status = CLK_STATUS_OFF;

    return 0;
}









/**********************************************************************
* 系统参考时钟
***********************************************************************/
struct clk ref_xtal =
{
    .parent = NULL,
    .name = "ref_xtal",
    .flags = CLK_FLG_RATE_FIXED | CLK_FLG_ALWAYS_ENABLE,
    .status = CLK_STATUS_ON,
    .set_rate = null_set_rate,
    .get_rate = ref_xtal_get_rate,
    .round_rate = null_round_rate,
    .enable = null_enable,
    .disable = null_disable,
    .set_parent = null_set_parent,
};

struct clk pll_clk[] =
{
    {
        .parent = NULL,
        .name = "pll0_clk",
        .flags = CLK_FLG_RATE_FIXED,
        .status = CLK_STATUS_OFF,
        .set_rate = null_set_rate,
        .get_rate = pll_clk_get_rate,
        .round_rate = null_round_rate,
        .enable = pll_clk_enable,
        .disable = pll_clk_disable,
        .set_parent = null_set_parent,
    },

    {
        .parent = NULL,
        .name = "pll1_clk",
        .flags = CLK_FLG_RATE_FIXED,
        .status = CLK_STATUS_OFF,
        .set_rate = null_set_rate,
        .get_rate = pll_clk_get_rate,
        .round_rate = null_round_rate,
        .enable = pll_clk_enable,
        .disable = pll_clk_disable,
        .set_parent = null_set_parent,
    },

    {
        .parent = NULL,
        .name = "pll2_clk",
        .flags = CLK_FLG_RATE_FIXED,
        .status = CLK_STATUS_OFF,
        .set_rate = null_set_rate,
        .get_rate = pll_clk_get_rate,
        .round_rate = null_round_rate,
        .enable = pll_clk_enable,
        .disable = pll_clk_disable,
        .set_parent = null_set_parent,
    }

};


struct clk ref_cpu =
{
    .parent = &pll_clk[0],
    .name = "ref_cpu",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = ref_cpu_set_rate,
    .get_rate = ref_cpu_get_rate,
    .round_rate = ref_cpu_round_rate,
    .enable = ref_cpu_enable,
    .disable = ref_cpu_disable,
    .set_parent = null_set_parent,
};

struct clk ref_emi =
{
    .parent = &pll_clk[0],
    .name = "ref_emi",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = ref_emi_set_rate,
    .get_rate = ref_emi_get_rate,
    .round_rate = ref_emi_round_rate,
    .enable = ref_emi_enable,
    .disable = ref_emi_disable,
    .set_parent = null_set_parent,
};

struct clk ref_io1 =
{
    .parent = &pll_clk[0],
    .name = "ref_io1",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = ref_io1_set_rate,
    .get_rate = ref_io1_get_rate,
    .round_rate = ref_io1_round_rate,
    .enable = ref_io1_enable,
    .disable = ref_io1_disable,
    .set_parent = null_set_parent,
};

struct clk ref_io0 =
{
    .parent = &pll_clk[0],
    .name = "ref_io0",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = ref_io0_set_rate,
    .get_rate = ref_io0_get_rate,
    .round_rate = ref_io0_round_rate,
    .enable = ref_io0_enable,
    .disable = ref_io0_disable,
    .set_parent = null_set_parent,
};

struct clk ref_gpmi =
{
    .parent = &pll_clk[0],
    .name = "ref_gpmi",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = ref_gpmi_set_rate,
    .get_rate = ref_gpmi_get_rate,
    .round_rate = ref_gpmi_round_rate,
    .enable = ref_gpmi_enable,
    .disable = ref_gpmi_disable,
    .set_parent = null_set_parent,
};

struct clk ref_hsadc =
{
    .parent = &pll_clk[0],
    .name = "ref_hsadc",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = ref_hsadc_set_rate,
    .get_rate = ref_hsadc_get_rate,
    .round_rate = ref_hsadc_round_rate,
    .enable = ref_hsadc_enable,
    .disable = ref_hsadc_disable,
    .set_parent = null_set_parent,
};

struct clk ref_pix =
{
    .parent = &pll_clk[0],
    .name = "ref_pix",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = ref_pix_set_rate,
    .get_rate = ref_pix_get_rate,
    .round_rate = ref_pix_round_rate,
    .enable = ref_pix_enable,
    .disable = ref_pix_disable,
    .set_parent = null_set_parent,
};



/**********************************************************************
* 运行时钟
***********************************************************************/
struct clk clk_p =
{
    .parent = &ref_cpu,
    .name = "clk_p",
    .flags = CLK_FLG_ALWAYS_ENABLE,
    .status = CLK_STATUS_ON,
    .set_rate = pclk_set_rate,
    .get_rate = pclk_get_rate,
    .round_rate = pclk_round_rate,
    .enable = pclk_enable,
    .disable = null_disable,
    .set_parent = pclk_set_parent,
};

struct clk clk_h =
{
    .parent = &clk_p,
    .name = "clk_h",
    .flags = CLK_FLG_ALWAYS_ENABLE,
    .status = CLK_STATUS_ON,
    .set_rate = hclk_set_rate,
    .get_rate = hclk_get_rate,
    .round_rate = hclk_round_rate,
    .enable = null_enable,
    .disable = null_disable,
    .set_parent = null_set_parent,
};

struct clk clk_x =
{
    .parent = &ref_xtal,
    .name = "clk_x",
    .flags = CLK_FLG_ALWAYS_ENABLE,
    .status = CLK_STATUS_ON,
    .set_rate = xclk_set_rate,
    .get_rate = xclk_get_rate,
    .round_rate = xclk_round_rate,
    .enable = null_enable,
    .disable = null_disable,
    .set_parent = null_set_parent,
};

struct clk clk_uart =
{
    .parent = &ref_xtal,
    .name = "clk_uart",
    .flags = CLK_FLG_ALWAYS_ENABLE,
    .status = CLK_STATUS_OFF,
    .set_rate = null_set_rate,
    .get_rate = uart_clk_get_rate,
    .round_rate = null_round_rate,
    .enable = uart_clk_enable,
    .disable = null_disable,
    .set_parent = null_set_parent,
};

struct clk clk_pwm =
{
    .parent = &ref_xtal,
    .name = "clk_pwm",
    .flags = CLK_FLG_ALWAYS_ENABLE,
    .status = CLK_STATUS_OFF,
    .set_rate = null_set_rate,
    .get_rate = pwm_clk_get_rate,
    .round_rate = null_round_rate,
    .enable = pwm_clk_enable,
    .disable = null_disable,
    .set_parent = null_set_parent,
};

struct clk clk_timrot =
{
    .parent = NULL,
    .name = "clk_timrot",
    .flags = CLK_FLG_ALWAYS_ENABLE,
    .status = CLK_STATUS_OFF,
    .set_rate = null_set_rate,
    .get_rate = timrot_clk_get_rate,
    .round_rate = null_round_rate,
    .enable = timrot_clk_enable,
    .disable = null_disable,
    .set_parent = null_set_parent,
};


struct clk clk_ssp[] =
{
    {
        .parent = &ref_io0,
        .name = "clk_ssp0",
        .flags = 0,
        .status = CLK_STATUS_OFF,
        .set_rate = ssp_clk_set_rate,
        .get_rate = ssp_clk_get_rate,
        .round_rate = ssp_clk_round_rate,
        .enable = ssp_clk_enable,
        .disable = ssp_clk_disable,
        .set_parent = ssp_clk_set_parent,
    },

    {
        .parent = &ref_io0,
        .name = "clk_ssp1",
        .flags = 0,
        .status = CLK_STATUS_OFF,
        .set_rate = ssp_clk_set_rate,
        .get_rate = ssp_clk_get_rate,
        .round_rate = ssp_clk_round_rate,
        .enable = ssp_clk_enable,
        .disable = ssp_clk_disable,
        .set_parent = ssp_clk_set_parent,
    },

    {
        .parent = &ref_io1,
        .name = "clk_ssp2",
        .flags = 0,
        .status = CLK_STATUS_OFF,
        .set_rate = ssp_clk_set_rate,
        .get_rate = ssp_clk_get_rate,
        .round_rate = ssp_clk_round_rate,
        .enable = ssp_clk_enable,
        .disable = ssp_clk_disable,
        .set_parent = ssp_clk_set_parent,
    },

    {
        .parent = &ref_io1,
        .name = "clk_ssp3",
        .flags = 0,
        .status = CLK_STATUS_OFF,
        .set_rate = ssp_clk_set_rate,
        .get_rate = ssp_clk_get_rate,
        .round_rate = ssp_clk_round_rate,
        .enable = ssp_clk_enable,
        .disable = ssp_clk_disable,
        .set_parent = ssp_clk_set_parent,
    },
};


struct clk clk_gpmi =
{
    .parent = &ref_xtal,
    .name = "clk_gpmi",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = gpmi_clk_set_rate,
    .get_rate = gpmi_clk_get_rate,
    .round_rate = gpmi_clk_round_rate,
    .enable = gpmi_clk_enable,
    .disable = gpmi_clk_disable,
    .set_parent = gpmi_clk_set_parent,
};


struct clk clk_spdif =
{
    .parent = &ref_xtal,
    .name = "clk_spdif",
    .flags = CLK_FLG_RATE_FIXED,
    .status = CLK_STATUS_OFF,
    .set_rate = null_set_rate,
    .get_rate = spdif_clk_get_rate,
    .round_rate = null_round_rate,
    .enable = spdif_clk_enable,
    .disable = spdif_clk_disable,
    .set_parent = spdif_clk_set_parent,
};

struct clk clk_emi =
{
    .parent = &ref_emi,
    .name = "clk_emi",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = emi_clk_set_rate,
    .get_rate = emi_clk_get_rate,
    .round_rate = emi_clk_round_rate,
    .enable = emi_clk_enable,
    .disable = emi_clk_disable,
    .set_parent = emi_clk_set_parent,
};

struct clk clk_saif[] =
{
    {
        .parent = &pll_clk[0],
        .name = "clk_saif0",
        .flags = 0,
        .status = CLK_STATUS_OFF,
        .set_rate = saif_clk_set_rate,
        .get_rate = saif_clk_get_rate,
        .round_rate = saif_clk_round_rate,
        .enable = saif_clk_enable,
        .disable = saif_clk_disable,
        .set_parent = null_set_parent,
    },

    {
        .parent = &pll_clk[0],
        .name = "clk_saif1",
        .flags = 0,
        .status = CLK_STATUS_OFF,
        .set_rate = saif_clk_set_rate,
        .get_rate = saif_clk_get_rate,
        .round_rate = saif_clk_round_rate,
        .enable = saif_clk_enable,
        .disable = saif_clk_disable,
        .set_parent = null_set_parent,
    },
};


struct clk clk_lcdif =
{
    .parent = &ref_pix,
    .name = "clk_lcdif",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = lcdif_clk_set_rate,
    .get_rate = lcdif_clk_get_rate,
    .round_rate = lcdif_clk_round_rate,
    .enable = lcdif_clk_enable,
    .disable = lcdif_clk_disable,
    .set_parent = lcdif_clk_set_parent,
};


struct clk clk_etm =
{
    .parent = &ref_cpu,
    .name = "clk_etm",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = etm_clk_set_rate,
    .get_rate = etm_clk_get_rate,
    .round_rate = etm_clk_round_rate,
    .enable = etm_clk_enable,
    .disable = etm_clk_disable,
    .set_parent = etm_clk_set_parent,
};

struct clk clk_hsadc =
{
    .parent = &ref_hsadc,
    .name = "clk_hsadc",
    .flags = 0,
    .status = CLK_STATUS_OFF,
    .set_rate = hsadc_clk_set_rate,
    .get_rate = hsadc_clk_get_rate,
    .round_rate = hsadc_clk_round_rate,
    .enable = hsadc_clk_enable,
    .disable = hsadc_clk_disable,
    .set_parent = null_set_parent,
};


struct clk clk_flexcan[] =
{
    {
        .parent = &ref_hsadc,
        .name = "clk_flexcan0",
        .flags = 0,
        .status = CLK_STATUS_OFF,
        .set_rate = null_set_rate,
        .get_rate = flexcan_clk_get_rate,
        .round_rate = null_round_rate,
        .enable = flexcan_clk_enable,
        .disable = flexcan_clk_disable,
        .set_parent = null_set_parent,
    },

    {
        .parent = &ref_hsadc,
        .name = "clk_flexcan1",
        .flags = 0,
        .status = CLK_STATUS_OFF,
        .set_rate = null_set_rate,
        .get_rate = flexcan_clk_get_rate,
        .round_rate = null_round_rate,
        .enable = flexcan_clk_enable,
        .disable = flexcan_clk_disable,
        .set_parent = null_set_parent,
    },
};


