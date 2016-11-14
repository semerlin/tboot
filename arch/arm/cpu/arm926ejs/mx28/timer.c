#include "stddef.h"
#include "types.h"
#include "arch/arch-mx28/mx28_regs.h"
#include "arch/arch-mx28/regs_timrot.h"

/* 定时器是向下计数 */

/* TIMROT提供4组定时器，N选择定时器组 */
#define N		0

/* 时钟频率 */
#define CONFIG_SYS_HZ	1000

/* 最大固定计数值 */
#define TIMER_LOAD_VAL	0xffffffff


/* 当前时间戳,总运行时间 */
static uint32_t timestamp;

/* 上一次记录时间 */
static uint32_t lastdec;


/********************************************************************************
* 函数: static void reset_timer_masked(void)
* 描述: 复位当前计数值
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void reset_timer_masked(void)
{
    //当前tick计数值
	lastdec = REG_RD(REGS_TIMROT_BASE, HW_TIMROT_RUNNING_COUNTn(N));
	timestamp = 0;
}


/********************************************************************************
* 函数: static uint32_t get_timer_masked(void)
* 描述: 取得计数值
* 输入: none
* 输出: none
* 返回: 计数值
* 作者:
* 版本: V1.0
**********************************************************************************/
static uint32_t get_timer_masked(void)
{
	/* 取得当前计数值 */
	uint32_t now = REG_RD(REGS_TIMROT_BASE, HW_TIMROT_RUNNING_COUNTn(N));

	if (lastdec >= now)
	{
		/* 计数没有溢出 */
		timestamp += lastdec - now;	}
	else
	{
		/* 计数溢出 */
		timestamp += lastdec + TIMER_LOAD_VAL - now + 1;
	}
	lastdec = now;

	return timestamp;
}

/********************************************************************************
* 函数: int32_t timer_init(void)
* 描述: 初始化定时器
* 输入: none
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t timer_init(void)
{
	/*
	 * 复位定时器和旋转解码器模块
	 */

	/* 清零SFTRST */
	REG_CLR(REGS_TIMROT_BASE, HW_TIMROT_ROTCTRL, 1 << 31);
	while (REG_RD(REGS_TIMROT_BASE, HW_TIMROT_ROTCTRL) & (1 << 31));

	/* 清零CLKGATE */
	REG_CLR(REGS_TIMROT_BASE, HW_TIMROT_ROTCTRL, 1 << 30);

	/* 置位SFTRST，等待CLKGATE置位 */
	REG_SET(REGS_TIMROT_BASE, HW_TIMROT_ROTCTRL, 1 << 31);
	while (!(REG_RD(REGS_TIMROT_BASE, HW_TIMROT_ROTCTRL) & (1 << 30)));

	/* 清零SFTRST和CLKGATE */
	REG_CLR(REGS_TIMROT_BASE, HW_TIMROT_ROTCTRL, 1 << 31);
	REG_CLR(REGS_TIMROT_BASE, HW_TIMROT_ROTCTRL, 1 << 30);

	/*
	*  初始化定时器
	*/

	/* 清零fixed_count*/
	REG_WR(REGS_TIMROT_BASE, HW_TIMROT_FIXED_COUNTn(N), 0);

	/* 置位UPDATE和1KHz频率 */
	REG_WR(REGS_TIMROT_BASE, HW_TIMROT_TIMCTRLn(N),
		BM_TIMROT_TIMCTRLn_RELOAD | BM_TIMROT_TIMCTRLn_UPDATE |
		BV_TIMROT_TIMCTRLn_SELECT__1KHZ_XTAL);

	/* 设置fixed_count到最大值 */
	REG_WR(REGS_TIMROT_BASE, HW_TIMROT_FIXED_COUNTn(N), TIMER_LOAD_VAL);

	/* 初始化timestamp和lastdec */
	reset_timer_masked();

	return 0;
}


/********************************************************************************
* 函数: unsigned long long get_ticks(void)
* 描述: 取得tick值
* 输入: none
* 输出: none
* 返回: tick值
* 作者:
* 版本: V1.0
**********************************************************************************/
uint32_t get_ticks(void)
{
	return get_timer_masked();
}

/********************************************************************************
* 函数: uint32_t get_tbclk(void)
* 描述: 取得计数频率
* 输入: none
* 输出: none
* 返回: 计数频率
* 作者:
* 版本: V1.0
**********************************************************************************/
uint32_t get_tbclk(void)
{
	uint32_t tbclk;

	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}


/********************************************************************************
* 函数: void udelay(__in uint32_t usec)
* 描述: 微秒延时
* 输入: usec: 延时时间
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void udelay(__in uint32_t usec)
{
	uint32_t tmo, tmp;

	if (usec >= 1000)
	{
		tmo = usec / 1000;
		tmo *= CONFIG_SYS_HZ;
		tmo /= 1000;
	}
	else
	{
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000*1000);
	}

	tmp = get_timer_masked(); //获取当前时间戳
	if ((tmo + tmp + 1) < tmp)
		reset_timer_masked();
	else
		tmo += tmp;
	while (get_timer_masked() < tmo);
}


/********************************************************************************
* 函数: void mdelay(__in uint32_t usec)
* 描述: 毫秒延时
* 输入: msec: 延时时间
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void mdelay(__in uint32_t msec)
{
	uint32_t tmo, tmp;

	tmo = msec;

	tmp = get_timer_masked();  //当前时间戳

	if ((tmo + tmp + 1) < tmp)  //计数溢出
		reset_timer_masked();
	else
		tmo += tmp;

	while (get_timer_masked() < tmo);
}




/********************************************************************************
* 函数: void reset_timer(void)
* 描述: 复位定时计数
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void reset_timer(void)
{
	reset_timer_masked();
}


/********************************************************************************
* 函数: uint32_t get_timer(uint32_t base)
* 描述: 获取计数值
* 输入: base: 计数基准
* 输出: none
* 返回: 计数值
* 作者:
* 版本: V1.0
**********************************************************************************/
uint32_t get_timer(__in uint32_t base)
{
	return get_timer_masked() - base;
}


/********************************************************************************
* 函数: void set_timer(uint32_t t)
* 描述: 设置当前计数值
* 输入: t: 新计数值
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void set_timer(__in uint32_t t)
{
	timestamp = t;
}









