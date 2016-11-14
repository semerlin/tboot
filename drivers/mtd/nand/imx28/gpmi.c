#include "stddef.h"
#include "errno.h"
#include "arch/arch-mx28/clkctrl.h"
#include "arch/arch-mx28/dma_apbh.h"
#include "arch/arch-mx28/mx28_regs.h"
#include "arch/arch-mx28/regs_gpmi.h"
#include "arch/arch-mx28/regs_bch.h"
#include "arch/arch-mx28/gpmi.h"
#include "mtd/nand/nand_device_info.h"
#include "mtd/mtd.h"
#include "mtd/nand/nand.h"
#include "assert.h"
#include "math.h"
#include "malloc.h"
#include "string.h"
#include "log.h"

/* gpmi使用到dma描述器的数量 */
#define GPMI_DMA_DESC_CNT         (8)

/* dma描述器指针数组 */
static struct dma_desc *gpmi_dma_desc[GPMI_DMA_DESC_CNT];


/* 最大的DLL延时 */
#define MAX_DLL_CLOCK_PERIOD_IN_NS     (32)
#define MAX_DLL_DELAY_IN_NS            (16)
#define MAX_GPMI_SETUP_IN_NS           (4)



/********************************************************************************
* 函数: static int32_t set_geometry(__in struct mtd_info *mtd)
* 描述: 设置nandflash布局图
* 输入: mtd: nandflash设备的父类的mtd设备
* 输出: none
* 返回: 0: 成功
       -ETIMEDOUT： 设置超时
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t set_geometry(__in struct mtd_info *mtd)
{
    assert(mtd);

    uint32_t block_cnt;
    uint32_t block_size;
    uint32_t metadata_size;
    uint32_t page_size;
    uint32_t ecc_strength;
    int32_t i;

    block_cnt = mtd->writesize / GPMI_ECC_BLOCK_SIZE - 1;
    block_size = GPMI_ECC_BLOCK_SIZE;
    metadata_size = GPMI_ECC_METADATA_SIZE;

    /* 计算ecc位数 */
    if(2048 == mtd->writesize)
        ecc_strength = 8;
    else if(4096 == mtd->writesize)
    {
        if(128 == mtd->oobsize)
            ecc_strength = 8;
        else if(218 == mtd->oobsize)
            ecc_strength = 16;
        else
            ecc_strength = 0;
    }
    else
        ecc_strength = 0;

    page_size = mtd->writesize + mtd->oobsize;

    /* 复位BCH模块 */
    REG_CLR(REGS_BCH_BASE, HW_BCH_CTRL, BM_BCH_CTRL_SFTRST);
    mdelay(2);
    REG_CLR(REGS_BCH_BASE, HW_BCH_CTRL, BM_BCH_CTRL_CLKGATE);
    REG_SET(REGS_BCH_BASE, HW_BCH_CTRL, BM_BCH_CTRL_SFTRST);
    for(i = 1000000; i > 0; --i)
    {
        if(REG_RD(REGS_BCH_BASE, HW_BCH_CTRL) & BM_BCH_CTRL_CLKGATE)
            break;

        udelay(1);
    }
    /* 复位超时 */
    if(i <= 0)
    {
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] reset bch block timeout.\n");
        return -ETIMEDOUT;
    }

    REG_CLR(REGS_BCH_BASE, HW_BCH_CTRL, BM_BCH_CTRL_SFTRST);
    mdelay(2);
    REG_CLR(REGS_BCH_BASE, HW_BCH_CTRL, BM_BCH_CTRL_CLKGATE);

    for(i = 1000000; i > 0; --i)
    {
        if(!(REG_RD(REGS_BCH_BASE, HW_BCH_CTRL) & BM_BCH_CTRL_CLKGATE))
            break;

        udelay(1);
    }
    /* 复位超时 */
    if(i <= 0)
    {
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] reset bch block timeout.\n");
        return -ETIMEDOUT;
    }


    /* 设置nandflash布局 */
    REG_WR(REGS_BCH_BASE, HW_BCH_FLASH0LAYOUT0, (BF_BCH_FLASH0LAYOUT0_NBLOCKS(block_cnt) |
                                                 BF_BCH_FLASH0LAYOUT0_META_SIZE(metadata_size) |
                                                 BF_BCH_FLASH0LAYOUT0_ECC0(ecc_strength >> 1) |
                                                 BF_BCH_FLASH0LAYOUT0_DATA0_SIZE(block_size)));

    REG_WR(REGS_BCH_BASE, HW_BCH_FLASH0LAYOUT1, (BF_BCH_FLASH0LAYOUT1_PAGE_SIZE(page_size) |
                                                 BF_BCH_FLASH0LAYOUT1_ECCN(ecc_strength >> 1) |
                                                 BF_BCH_FLASH0LAYOUT1_DATAN_SIZE(block_size)));
    /* 所有芯片都使用0的布局 */
    REG_WR(REGS_BCH_BASE, HW_BCH_LAYOUTSELECT, 0);

    /* 使能中断 */
    REG_SET(REGS_BCH_BASE, HW_BCH_CTRL, BM_BCH_CTRL_COMPLETE_IRQ_EN);

    return 0;
}

/********************************************************************************
* 函数: static void clear_bch_irq(void)
* 描述: 清除bch中断标志位
* 输入: none
* 输出: none
* 返回: none
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static void clear_bch_irq(void)
{
    REG_CLR(REGS_BCH_BASE, HW_BCH_CTRL, BM_BCH_CTRL_COMPLETE_IRQ);
}


/********************************************************************************
* 函数: int32_t wait_for_bch_completion(__in uint32_t time)
* 描述: 等待bch完成
* 输入: time: 等待时间
* 输出: none
* 返回: 0: bch计算完成
       -ETIMEDOUT: bch计算超时
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t wait_for_bch_completion(__in uint32_t time)
{
    while((!(REG_RD(REGS_BCH_BASE, HW_BCH_CTRL) & BM_BCH_CTRL_COMPLETE_IRQ)) && --time);

    return (time > 0) ? 0 : -ETIMEDOUT;
}


/********************************************************************************
* 函数: static int32_t is_ready(__in uint32_t chipnum)
* 描述: 检测nandflash R/D引脚状态
* 输入: chipnum: 芯片号
* 输出: none
* 返回: 0: 芯片空闲
       !0: 芯片忙
* 作者: hy
* 版本: v1.0
**********************************************************************************/
static int32_t is_ready(__in uint32_t chipnum)
{
    uint32_t mask;

    mask = ((1 << chipnum) << BP_GPMI_STAT_READY_BUSY);

    return (REG_RD(REGS_GPMI_BASE, HW_GPMI_STAT) & mask);
}

/********************************************************************************
* 函数: static uint32_t ns_to_cyles(__in uint32_t time, __in uint32_t period,
                                   __in uint32_t min)
* 描述: 计算给定的时间最少等于多少个时钟周期
* 输入: time: 需要计算的时间
       period: 时钟周期的时间
       min: 最小参考值
* 输出: none
* 返回: 时钟周期的数目
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint32_t ns_to_cyles(__in uint32_t time, __in uint32_t period, __in uint32_t min)
{
    uint32_t val = 0;

    /* 小数进位 */
    val = (time + period - 1) / period;

    return max_t(uint32_t, val, min);
}





/********************************************************************************
* 函数: static int32_t gpmi_init(void)
* 描述: 初始化gpmi模块
* 输入: none
* 输出: none
* 返回: 0: 成功
       -ENOMEM:
       -EINVAL: channel参数无效
       -EBUSY: 通道已经被分配
       -ETIMEDOUT: 复位失败,超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_init(void)
{
    int32_t i = 0, err = 0;;

    /* 分配gpmi使用的dma资源 */
    for(i = 0; i< GPMI_DMA_DESC_CNT; i++)
    {
        gpmi_dma_desc[i] = dma_alloc_desc();

        /* 分配描述器失败，释放所有描述器 */
        if(NULL == gpmi_dma_desc[i])
        {
            for(i -= 1; i >=0; i--)
                dma_free_desc(gpmi_dma_desc[i]);

            printl(LOG_LEVEL_ERR, "[GPMI:ERR] allocate gpmi dma descriptor failed.\n");
            return -ENOMEM;
        }
    }

    for(i = DMA_CHANNEL_AHB_APBH_GPMI0; i <= DMA_CHANNEL_AHB_APBH_GPMI7; i++)
    {
        err = dma_init(i);
        if(!err)
        {
            printl(LOG_LEVEL_ERR, "[GPMI:ERR] init dma channel gpmi%d failed, code = %d.\n", i, err);
            return err;
        }
    }

    /* 复位gpmi模块, 超时时间1s */
    REG_CLR(REGS_GPMI_BASE, HW_GPMI_CTRL0, BM_GPMI_CTRL0_SFTRST);
    mdelay(2);
    REG_CLR(REGS_GPMI_BASE, HW_GPMI_CTRL0, BM_GPMI_CTRL0_CLKGATE);
    REG_SET(REGS_GPMI_BASE, HW_GPMI_CTRL0, BM_GPMI_CTRL0_SFTRST);
    for(i = 1000000; i > 0; --i)
    {
        if(REG_RD(REGS_GPMI_BASE, HW_GPMI_CTRL0) & BM_GPMI_CTRL0_CLKGATE)
            break;

        udelay(1);
    }
    /* 复位超时 */
    if(i <= 0)
    {
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] reset gpmi block timeout.\n");
        return -ETIMEDOUT;
    }

    REG_CLR(REGS_GPMI_BASE, HW_GPMI_CTRL0, BM_GPMI_CTRL0_SFTRST);
    mdelay(2);
    REG_CLR(REGS_GPMI_BASE, HW_GPMI_CTRL0, BM_GPMI_CTRL0_CLKGATE);

    for(i = 1000000; i > 0; --i)
    {
        if(!(REG_RD(REGS_GPMI_BASE, HW_GPMI_CTRL0) & BM_GPMI_CTRL0_CLKGATE))
            break;

        udelay(1);
    }
    /* 复位超时 */
    if(i <= 0)
    {
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] reset gpmi block timeout.\n");
        return -ETIMEDOUT;
    }


    /* 选择为nand模式 */
    REG_CLR(REGS_GPMI_BASE, HW_GPMI_CTRL1, BM_GPMI_CTRL1_GPMI_MODE);

    /* 设置IRQ */
    REG_SET(REGS_GPMI_BASE, HW_GPMI_CTRL1, BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY);

    /* 禁止写保护 */
    REG_SET(REGS_GPMI_BASE, HW_GPMI_CTRL1, BM_GPMI_CTRL1_DEV_RESET);

    /* 选择BCH ECC */
    REG_SET(REGS_GPMI_BASE, HW_GPMI_CTRL1, BM_GPMI_CTRL1_BCH_MODE);

    return 0;
}


/********************************************************************************
* 函数: static void gpmi_deinit(void)
* 描述: 释放gpmi模块
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
#if 0
static void gpmi_deinit(void)
{
    int32_t i;

    for(i = 0; i <  GPMI_DMA_DESC_CNT; i++)
        dma_free_desc(gpmi_dma_desc[i]);

}
#endif

/********************************************************************************
* 函数: static int32_t set_hw_timing(__in nand_chip *chip,
                                    __in struct nand_timing *timing)
* 描述: 设置gpmi时序
* 输入: chip: nand设备的自身指针
       timing: 需要设置的时序
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t set_hw_timing(__in struct nand_chip *chip, __in struct nand_timing *timing)
{
    uint32_t clk_rate_in_hz;
    bool improved_timing_available = false;
    uint32_t clk_perid_in_ns;
    uint32_t rp_in_ns;
    uint32_t delay_in_ns;
    uint32_t delta_data_setup_in_ns = 0;

    uint32_t data_setup_in_cyles;
    uint32_t data_hold_in_cyles;
    uint32_t address_setup_in_cyles;
    uint32_t sample_delay_factor;
    bool dll_use_half_peroids;
    bool dll_enable;
    uint32_t dll_wait_time_in_us;

    uint32_t val = 0;

    /* 芯片数量增多时，延长一些时间*/
    if(chip->numchips > 2)
    {
        timing->data_setup_in_ns += 10;
        timing->data_hold_in_ns += 10;
        timing->address_setup_in_ns += 10;
    }

    /* 更多的时钟信息,有助于提高带宽 */
    improved_timing_available = ((timing->tREA_in_ns > 0) &&
                                (timing->tRHOH_in_ns > 0) &&
                                (timing->tRLOH_in_ns > 0));

    /* gpmi时钟周期时间 */
    clk_rate_in_hz = clk_gpmi.get_rate(&clk_gpmi);
    clk_perid_in_ns = 1000000000 / clk_rate_in_hz;


    /* 计算延时 */
    if(improved_timing_available)
    {
        /* dll处理 */
        if(clk_perid_in_ns > MAX_DLL_CLOCK_PERIOD_IN_NS)
        {
            dll_enable = false;
        }
        else
        {
            if((timing->tREA_in_ns + MAX_GPMI_SETUP_IN_NS) < timing->data_setup_in_ns)
            {
                /* 需要gpmi延时 */
                dll_enable = true;

                if(clk_perid_in_ns > ((MAX_DLL_CLOCK_PERIOD_IN_NS) >> 1))
                {
                    rp_in_ns = (clk_perid_in_ns >> 1);
                    dll_use_half_peroids = true;
                }
                else
                {
                    rp_in_ns = clk_perid_in_ns;
                    dll_use_half_peroids = false;
                }


                /* 计算延时 */
                delay_in_ns = timing->tREA_in_ns + MAX_GPMI_SETUP_IN_NS - timing->data_setup_in_ns;

                if(delay_in_ns >= MAX_DLL_DELAY_IN_NS)
                {
                    delta_data_setup_in_ns = timing->tREA_in_ns + MAX_GPMI_SETUP_IN_NS - MAX_DLL_DELAY_IN_NS + 1;
                    timing->data_hold_in_ns += (delta_data_setup_in_ns - timing->data_setup_in_ns);
                    timing->address_setup_in_ns += (delta_data_setup_in_ns - timing->data_setup_in_ns);
                    timing->data_setup_in_ns = delta_data_setup_in_ns;
                }


                /* 寻找合适的系数 */
                do
                {
                    delay_in_ns = timing->tREA_in_ns + MAX_GPMI_SETUP_IN_NS - delta_data_setup_in_ns;
                    sample_delay_factor = (delay_in_ns * 8 + rp_in_ns - 1) / rp_in_ns;
                    delta_data_setup_in_ns += 1;
                }while(sample_delay_factor > 15);

                /* 计算最终时间 */
                delta_data_setup_in_ns -= 1;
                timing->data_hold_in_ns += (delta_data_setup_in_ns - timing->data_setup_in_ns);
                timing->address_setup_in_ns += (delta_data_setup_in_ns - timing->data_setup_in_ns);
                timing->data_setup_in_ns = delta_data_setup_in_ns;
                timing->gpmi_sample_delay_in_ns = (sample_delay_factor * rp_in_ns + 7) / 8;
            }
            else
            {
                /* 不需要gpmi延时 */
                dll_enable = false;
            }
        }
    }
    else
    {
        /* dll处理 */
        dll_enable = false;
    }


    /* 计算需要几个周期 */
    /* 0时为最大值，所以最小值要设置为1 */
    data_setup_in_cyles = ns_to_cyles(timing->data_setup_in_ns, clk_perid_in_ns, 1);
    data_hold_in_cyles = ns_to_cyles(timing->data_hold_in_ns, clk_perid_in_ns, 1);

    address_setup_in_cyles = ns_to_cyles(timing->address_setup_in_ns, clk_perid_in_ns, 0);



    /* 设置延时时间 */
    val = REG_RD(REGS_GPMI_BASE, HW_GPMI_TIMING0);
    val = (data_setup_in_cyles << BP_GPMI_TIMING0_DATA_SETUP) |
          (data_hold_in_cyles << BP_GPMI_TIMING0_DATA_HOLD) |
          (address_setup_in_cyles << BP_GPMI_TIMING0_ADDRESS_SETUP);
    REG_WR(REGS_GPMI_BASE, HW_GPMI_TIMING0, val);


    /* 设置dll */
    /* 关闭dll */
    REG_CLR(REGS_GPMI_BASE, HW_GPMI_CTRL1, BM_GPMI_CTRL1_DLL_ENABLE);

    /* 清零dll参数 */
    REG_CLR(REGS_GPMI_BASE, HW_GPMI_CTRL1, BM_GPMI_CTRL1_HALF_PERIOD);
    REG_CLR(REGS_GPMI_BASE, HW_GPMI_CTRL1, BM_GPMI_CTRL1_RDN_DELAY);

    /* dll状态 */
    if(dll_enable)
    {
        if(dll_use_half_peroids)
            REG_SET(REGS_GPMI_BASE, HW_GPMI_CTRL1, BM_GPMI_CTRL1_HALF_PERIOD);

        REG_SET(REGS_GPMI_BASE, HW_GPMI_CTRL1, (sample_delay_factor << BP_GPMI_CTRL1_RDN_DELAY));

        /* 使能dll */
        REG_SET(REGS_GPMI_BASE, HW_GPMI_CTRL1, BM_GPMI_CTRL1_DLL_ENABLE);

        /* 等待64个时钟周期之后使用GPMI */
        dll_wait_time_in_us = (clk_perid_in_ns * 64) / 1000;
        if(!dll_wait_time_in_us)
            dll_wait_time_in_us = 1;

        /* 开始延时 */
        udelay(dll_wait_time_in_us);
    }

    return 0;

}

/********************************************************************************
* 函数: static int32_t send_command(__in uint32_t chipnum,
                                   __in uint32_t buffer,
                                   __in uint32_t length)
* 描述: 发送指令
* 输入: chipnum: 芯片号
       buffer: 命令地址缓冲区
       length: 命令长度
* 输出: none
* 返回: 0: 成功
       -ETIMEDOUT: 执行失败，超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t send_command(__in uint32_t chipnum, __in uint32_t buffer,
                              __in uint32_t length)
{
    int32_t dma_channel;
    struct dma_desc **d = gpmi_dma_desc;
    uint32_t command_mode;
    uint32_t address;
    int32_t error;


    /* 写命令 */
    command_mode = BV_GPMI_CTRL0_COMMAND_MODE__WRITE;
    address = BV_GPMI_CTRL0_ADDRESS__NAND_CLE;

    /* 确定dma通道 */
    dma_channel = DMA_CHANNEL_AHB_APBH_GPMI0 + chipnum;

    /* 写dma命令 */
    (*d)->cmd.cmd.data = 0;

    (*d)->cmd.cmd.bits.command = DMA_READ;
    (*d)->cmd.cmd.bits.chain = 1;
    (*d)->cmd.cmd.bits.irq_complete = 1;
    (*d)->cmd.cmd.bits.nand_lock = 0;
    (*d)->cmd.cmd.bits.nand_wait4ready = 0;
    (*d)->cmd.cmd.bits.dec_sem = 1;
    (*d)->cmd.cmd.bits.cmd_wait4end = 1;
    (*d)->cmd.cmd.bits.halt_on_terminate = 0;
    (*d)->cmd.cmd.bits.num_pio_words = 3;
    (*d)->cmd.cmd.bits.num_trans_bytes = length;

    (*d)->cmd.bufaddr = buffer;

    (*d)->cmd.pio_words[0] =
        BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chipnum)                |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		/* 一个周期后从CLE转到ALE，方便发送命令之后接着发送地址，实现CLE到ALE的快速转换 */
		BM_GPMI_CTRL0_ADDRESS_INCREMENT          |
		BF_GPMI_CTRL0_XFER_COUNT(length)         ;

    /* 禁止BCH/ECC */
    (*d)->cmd.pio_words[1] = 0;
    (*d)->cmd.pio_words[2] = 0;

    dma_desc_append(dma_channel, (*d));
    error = dma_go(dma_channel);

    if(error)
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] send command dma error, code = %d!\n", -error);

    return error;
}

/********************************************************************************
* 函数: static int32_t send_data(__in uint32_t chipnum, __in uint32_t buffer,
                                __in uint32_t length)
* 描述: 写原始数据到nandflash, 地址可以是一页内的任何地址
* 输入: chipnum: 芯片号
       buffer: 需要写的数据缓冲区
       length: 数据长度
* 输出: none
* 返回: 0: 成功
       -ETIMEDOUT: 写数据超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t send_data(__in uint32_t chipnum, __in uint32_t buffer,
                           __in uint32_t length)
{
    int32_t dma_channel;
    struct dma_desc **d = gpmi_dma_desc;
    uint32_t command_mode;
    uint32_t address;
    int32_t error;


    /* 写数据 */
    command_mode = BV_GPMI_CTRL0_COMMAND_MODE__WRITE;
    address = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;

    /* 确定dma通道 */
    dma_channel = DMA_CHANNEL_AHB_APBH_GPMI0 + chipnum;

    /* 写dma命令 */
    (*d)->cmd.cmd.data = 0;

    (*d)->cmd.cmd.bits.command = DMA_READ;
    (*d)->cmd.cmd.bits.chain = 0;
    (*d)->cmd.cmd.bits.irq_complete = 1;
    (*d)->cmd.cmd.bits.nand_lock = 0;
    (*d)->cmd.cmd.bits.nand_wait4ready = 0;
    (*d)->cmd.cmd.bits.dec_sem = 1;
    (*d)->cmd.cmd.bits.cmd_wait4end = 1;
    (*d)->cmd.cmd.bits.halt_on_terminate = 0;
    (*d)->cmd.cmd.bits.num_pio_words = 4;
    (*d)->cmd.cmd.bits.num_trans_bytes = length;

    (*d)->cmd.bufaddr = buffer;

    (*d)->cmd.pio_words[0] =
        BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chipnum)                   |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		BF_GPMI_CTRL0_XFER_COUNT(length)         ;

    /* 禁止BCH/ECC */
    (*d)->cmd.pio_words[1] = 0;
    (*d)->cmd.pio_words[2] = 0;
    (*d)->cmd.pio_words[3] = 0;

    dma_desc_append(dma_channel, (*d));
    error = dma_go(dma_channel);

    if(error)
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] send data dma error, code = %d!\n", -error);

    return error;
}


/********************************************************************************
* 函数: static int32_t read_data(__in uint32_t chipnum, __out uint32_t buffer,
                           __in uint32_t length)
* 描述: 读nandflash原始数据，地址可以是一页内的任何地址
* 输入: chipnum: 芯片号
       length: 需要读取的数据长度
* 输出: buffer: 数据输出缓冲区
* 返回: 0: 成功
       -ETIMEDOUT: 读数据超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t read_data(__in uint32_t chipnum, __in uint32_t buffer,
                           __in uint32_t length)
{
    int32_t dma_channel;
    struct dma_desc **d = gpmi_dma_desc;
    uint32_t command_mode;
    uint32_t address;
    int32_t error;


    /* 写数据 */
    command_mode = BV_GPMI_CTRL0_COMMAND_MODE__READ;
    address = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;

    /* 确定dma通道 */
    dma_channel = DMA_CHANNEL_AHB_APBH_GPMI0 + chipnum;

    /* 写dma命令 */
    (*d)->cmd.cmd.data = 0;

    (*d)->cmd.cmd.bits.command = DMA_WRITE;
    (*d)->cmd.cmd.bits.chain = 1;
    (*d)->cmd.cmd.bits.irq_complete = 0;
    (*d)->cmd.cmd.bits.nand_lock = 0;
    (*d)->cmd.cmd.bits.nand_wait4ready = 0;
    (*d)->cmd.cmd.bits.dec_sem = 1;
    (*d)->cmd.cmd.bits.cmd_wait4end = 1;
    (*d)->cmd.cmd.bits.halt_on_terminate = 0;
    (*d)->cmd.cmd.bits.num_pio_words = 1;
    (*d)->cmd.cmd.bits.num_trans_bytes = length;

    (*d)->cmd.bufaddr = buffer;

    (*d)->cmd.pio_words[0] =
        BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chipnum)                   |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		BF_GPMI_CTRL0_XFER_COUNT(length)         ;


    dma_desc_append(dma_channel, (*d));
    d++;

    command_mode = BV_GPMI_CTRL0_COMMAND_MODE__WAIT_FOR_READY;
	address      = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;

	(*d)->cmd.cmd.data = 0;
	(*d)->cmd.cmd.bits.command = NO_DMA_XFER;
	(*d)->cmd.cmd.bits.chain = 0;
	(*d)->cmd.cmd.bits.irq_complete = 1;
	(*d)->cmd.cmd.bits.nand_lock = 0;
	(*d)->cmd.cmd.bits.nand_wait4ready = 1;
	(*d)->cmd.cmd.bits.dec_sem = 1;
	(*d)->cmd.cmd.bits.cmd_wait4end = 1;
	(*d)->cmd.cmd.bits.halt_on_terminate = 0;
	(*d)->cmd.cmd.bits.num_pio_words = 4;
	(*d)->cmd.cmd.bits.num_trans_bytes = 0;

	(*d)->cmd.bufaddr = 0;

	(*d)->cmd.pio_words[0] =
		BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chipnum)                   |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		BF_GPMI_CTRL0_XFER_COUNT(0)              ;

    /* 禁止BCH/ECC */
	(*d)->cmd.pio_words[1] = 0;
	(*d)->cmd.pio_words[2] = 0;
	(*d)->cmd.pio_words[3] = 0;

	dma_desc_append(dma_channel, (*d));

    error = dma_go(dma_channel);

    if(error)
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] read data dma error, code = %d!\n", -error);

    return error;
}



/********************************************************************************
* 函数: static int32_t send_page(__in struct mtd_info *mtd, __in uint32_t chipnum,
                                __in uint32_t payload, __in uint32_t auxiliary)
* 描述: 写一页经过bch/ecc校验布局的数据到nandflash，地址为页起始地址
* 输入: mtd: nandflash设备的父类
       chipnum: 芯片号
       payload: data区数据
       auxiliary: oob或metadata区数据
* 输出: none
* 返回: 0: 成功
       -ETIMEDOUT: 写数据超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t send_page(__in struct mtd_info *mtd, __in uint32_t chipnum,
                           __in uint32_t payload, __in uint32_t auxiliary)
{
    int32_t dma_channel;
    struct dma_desc **d = gpmi_dma_desc;
    uint32_t command_mode;
    uint32_t address;
    uint32_t ecc_command;
    uint32_t buffer_mask;
    int32_t error;

    dma_channel = DMA_CHANNEL_AHB_APBH_GPMI0 + chipnum;

    command_mode = BV_GPMI_CTRL0_COMMAND_MODE__WRITE;
    address = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;
    ecc_command = BV_GPMI_ECCCTRL_ECC_CMD__ENCODE;
    buffer_mask = BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE;

    (*d)->cmd.cmd.data = 0;
	(*d)->cmd.cmd.bits.command = NO_DMA_XFER;
	(*d)->cmd.cmd.bits.chain = 0;
	(*d)->cmd.cmd.bits.irq_complete = 1;
	(*d)->cmd.cmd.bits.nand_lock = 0;
	(*d)->cmd.cmd.bits.nand_wait4ready = 0;
	(*d)->cmd.cmd.bits.dec_sem = 1;
	(*d)->cmd.cmd.bits.cmd_wait4end = 1;
	(*d)->cmd.cmd.bits.halt_on_terminate = 0;
	(*d)->cmd.cmd.bits.num_pio_words = 6;
	(*d)->cmd.cmd.bits.num_trans_bytes = 0;

    (*d)->cmd.bufaddr = 0;

    (*d)->cmd.pio_words[0] =
		BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chipnum)                   |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		BF_GPMI_CTRL0_XFER_COUNT(0)              ;

	(*d)->cmd.pio_words[1] = 0;

	(*d)->cmd.pio_words[2] =
		BM_GPMI_ECCCTRL_ENABLE_ECC               |
		BF_GPMI_ECCCTRL_ECC_CMD(ecc_command)     |
		BF_GPMI_ECCCTRL_BUFFER_MASK(buffer_mask) ;

	(*d)->cmd.pio_words[3] = (mtd->writesize + mtd->oobsize);
	(*d)->cmd.pio_words[4] = payload;
	(*d)->cmd.pio_words[5] = auxiliary;

	dma_desc_append(dma_channel, (*d));
	d++;

    error = dma_go(dma_channel);

     if(error)
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] send page dma error, code = %d!\n", -error);

    error = wait_for_bch_completion(10000);
    if(error)
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] send page bch error, code = %d!\n", -error);

    clear_bch_irq();

    return error;
}

/********************************************************************************
* 函数: static static int32_t read_page(__in struct mtd_info *mtd,
                                       __in uint32_t chipnum,
                                       __out uint32_t payload,
                                       __out uint32_t auxiliary)
* 描述: 读一页经过bch/ecc校验布局的nandflash数据到内存，地址是页起始地址
* 输入: mtd: nandflash设备的父类
       chipnum: 芯片号
* 输出: payload: data区数据
       auxiliary: oob或metadata区数据
* 返回: 0: 成功
       -ETIMEDOUT: 读数据超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t read_page(__in struct mtd_info *mtd, __in uint32_t chipnum,
                           __out uint32_t payload, __out uint32_t auxiliary)
{
    int32_t dma_channel;
    struct dma_desc **d = gpmi_dma_desc;
    uint32_t command_mode;
    uint32_t address;
    uint32_t ecc_command;
    uint32_t buffer_mask;
    int32_t error;
    uint32_t page_size = mtd->writesize + mtd->oobsize;

    dma_channel = DMA_CHANNEL_AHB_APBH_GPMI0 + chipnum;


    /* 等待nandflash准备完成 */
    command_mode = BV_GPMI_CTRL0_COMMAND_MODE__WRITE;
    address = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;

    (*d)->cmd.cmd.data = 0;
	(*d)->cmd.cmd.bits.command = NO_DMA_XFER;
	(*d)->cmd.cmd.bits.chain = 1;
	(*d)->cmd.cmd.bits.irq_complete = 0;
	(*d)->cmd.cmd.bits.nand_lock = 0;
	(*d)->cmd.cmd.bits.nand_wait4ready = 1;
	(*d)->cmd.cmd.bits.dec_sem = 1;
	(*d)->cmd.cmd.bits.cmd_wait4end = 1;
	(*d)->cmd.cmd.bits.halt_on_terminate = 0;
	(*d)->cmd.cmd.bits.num_pio_words = 1;
	(*d)->cmd.cmd.bits.num_trans_bytes = 0;

    (*d)->cmd.bufaddr = 0;

    (*d)->cmd.pio_words[0] =
		BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chipnum)                |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		BF_GPMI_CTRL0_XFER_COUNT(0)              ;

	dma_desc_append(dma_channel, (*d));
    d++;

    /* BCH读数据 */
    command_mode = BV_GPMI_CTRL0_COMMAND_MODE__READ;
    address = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;
    ecc_command = BV_GPMI_ECCCTRL_ECC_CMD__DECODE;
    buffer_mask = BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE;

    (*d)->cmd.cmd.data = 0;
	(*d)->cmd.cmd.bits.command = NO_DMA_XFER;
	(*d)->cmd.cmd.bits.chain = 1;
	(*d)->cmd.cmd.bits.irq_complete = 0;
	(*d)->cmd.cmd.bits.nand_lock = 0;
	(*d)->cmd.cmd.bits.nand_wait4ready = 0;
	(*d)->cmd.cmd.bits.dec_sem = 1;
	(*d)->cmd.cmd.bits.cmd_wait4end = 1;
	(*d)->cmd.cmd.bits.halt_on_terminate = 0;
	(*d)->cmd.cmd.bits.num_pio_words = 6;
	(*d)->cmd.cmd.bits.num_trans_bytes = 0;

    (*d)->cmd.bufaddr = 0;

    (*d)->cmd.pio_words[0] =
		BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chipnum)                   |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		BF_GPMI_CTRL0_XFER_COUNT(page_size)              ;

	(*d)->cmd.pio_words[1] = 0;

	(*d)->cmd.pio_words[2] =
		BM_GPMI_ECCCTRL_ENABLE_ECC               |
		BF_GPMI_ECCCTRL_ECC_CMD(ecc_command)     |
		BF_GPMI_ECCCTRL_BUFFER_MASK(buffer_mask) ;

	(*d)->cmd.pio_words[3] = page_size;
	(*d)->cmd.pio_words[4] = payload;
	(*d)->cmd.pio_words[5] = auxiliary;

	dma_desc_append(dma_channel, (*d));
	d++;

    /* 禁止BCH/ECC，等待读取完成 */
	command_mode = BV_GPMI_CTRL0_COMMAND_MODE__WAIT_FOR_READY;
	address = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;

	(*d)->cmd.cmd.data = 0;
	(*d)->cmd.cmd.bits.command = NO_DMA_XFER;
	(*d)->cmd.cmd.bits.chain = 1;
	(*d)->cmd.cmd.bits.irq_complete = 0;
	(*d)->cmd.cmd.bits.nand_lock = 0;
	(*d)->cmd.cmd.bits.nand_wait4ready = 1;
	(*d)->cmd.cmd.bits.dec_sem = 1;
	(*d)->cmd.cmd.bits.cmd_wait4end = 1;
	(*d)->cmd.cmd.bits.halt_on_terminate = 0;
	(*d)->cmd.cmd.bits.num_pio_words = 3;
	(*d)->cmd.cmd.bits.num_trans_bytes = 0;

	(*d)->cmd.bufaddr = 0;

	(*d)->cmd.pio_words[0] =
		BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chipnum)                   |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		BF_GPMI_CTRL0_XFER_COUNT(page_size) ;

	(*d)->cmd.pio_words[1] = 0;
	(*d)->cmd.pio_words[2] = 0;

	dma_desc_append(dma_channel, (*d));
	d++;

	/* 不选中nandflash */
	(*d)->cmd.cmd.data = 0;
	(*d)->cmd.cmd.bits.command = NO_DMA_XFER;
	(*d)->cmd.cmd.bits.chain = 0;
	(*d)->cmd.cmd.bits.irq_complete = 1;
	(*d)->cmd.cmd.bits.nand_lock = 0;
	(*d)->cmd.cmd.bits.nand_wait4ready = 0;
	(*d)->cmd.cmd.bits.dec_sem = 1;
	(*d)->cmd.cmd.bits.cmd_wait4end = 0;
	(*d)->cmd.cmd.bits.halt_on_terminate = 0;
	(*d)->cmd.cmd.bits.num_pio_words = 0;
	(*d)->cmd.cmd.bits.num_trans_bytes = 0;

	(*d)->cmd.bufaddr = 0;

	dma_desc_append(dma_channel, (*d));
	d++;

	error = dma_go(dma_channel);
    if(error)
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] read page dma error, code = %d!\n", -error);

	error = wait_for_bch_completion(10000);
    if(error)
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] read page bch error, code = %d!\n", -error);

    clear_bch_irq();

    return error;
}





/********************************************************************************
---------------------------------------------------------------------------------
---------------------------------------------------------------------------------
*********************************************************************************/



/********************************************************************************
* 函数: static void gpmi_cmd_ctrl(__in struct mtd_info *mtd, __in int32_t data,
                                 __in uint32_t ctrl)
* 描述: 通过gpmi发送命令
* 输入: mtd: nandflash设备的父类
       data: 命令包含的数据
       ctrl: 命令类型
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void gpmi_cmd_ctrl(__in struct mtd_info *mtd, __in int32_t data, __in uint32_t ctrl)
{
    struct nand_chip *chip = mtd->priv;
    struct gpmi_info *gpmi = chip->priv;
    uint32_t error;

    /* 用static变量把所有命令数据和地址数据组织在一起发送 */
    static uint8_t *cmd_queue = NULL;
    static uint32_t cmd_q_len;

    /* 第一次初始化cmd_queue */
    if(!cmd_queue)
    {
        cmd_queue = dlmemalign(DMA_ALIGNMENT, GPMI_COMMAND_BUFFER_SIZE);

        if(!cmd_queue)
        {
            printl(LOG_LEVEL_ERR, "[GPMI:ERR] failed to alloc command queue buffer\n");
            return ;
        }

        memset(cmd_queue, 0, GPMI_COMMAND_BUFFER_SIZE);
        cmd_q_len = 0;
    }

    /* 组织命令 */
    if((ctrl & (NAND_ALE | NAND_CLE)))
    {
        if(data != NAND_CMD_NONE)
            cmd_queue[cmd_q_len++] = data;

        return ;
    }

    if(!cmd_q_len)
        return ;

    error = send_command(gpmi->cur_chip, (uint32_t)cmd_queue, cmd_q_len);

    if(error)
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] send command failed, error = %d", error);

    cmd_q_len = 0;
}

/********************************************************************************
* 函数: static int32_t gpmi_dev_ready(__in struct mtd_info *mtd)
* 描述: 检测gpmi是否空闲
* 输入: mtd: nandflash设备的父类
* 输出: none
* 返回: 0: 空闲
       1: 不空闲
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_dev_ready(__in struct mtd_info *mtd)
{
    struct gpmi_info *gpmi = ((struct nand_chip *)(mtd->priv))->priv;
    return (is_ready(gpmi->cur_chip) ? 1 : 0);
}

/********************************************************************************
* 函数: static void gpmi_select_chip(__in struct mtd_info *mtd,
                                    __in int32_t chipnum)
* 描述: 选择nandflash芯片
* 输入: mtd: nandflash设备的父类
       chipnum: 芯片号
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void gpmi_select_chip(__in struct mtd_info *mtd, __in int32_t chipnum)
{
    struct gpmi_info *gpmi = ((struct nand_chip *)(mtd->priv))->priv;

//    set_hw_timing(mtd);

    gpmi->cur_chip = chipnum;
}


/********************************************************************************
* 函数: static void gpmi_read_buf(__in struct mtd_info *mtd,
                                 __out uint8_t *buf, __in int32_t len)
* 描述: 读取nandflash原始数据，地址可以是一页内的任何地址
* 输入: mtd: nandflash的mtd父类
       len: 数据长度
* 输出: buf: 数据输出缓冲区
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void gpmi_read_buf(__in struct mtd_info *mtd, __out uint8_t *buf, __in int32_t len)
{
    struct nand_chip *chip = mtd->priv;
    struct gpmi_info *gpmi = chip->priv;

    if(len > NAND_MAX_PAGESIZE)
        printl(LOG_LEVEL_WARN, "[GPMI:WARN] len too long\n");

    if(!buf)
        printl(LOG_LEVEL_WARN, "[GPMI:WARN] buffer point is NULL\n");


    read_data(gpmi->cur_chip, (uint32_t)buf, len);
}


/********************************************************************************
* 函数: static void gpmi_write_buf(__in struct mtd_info *mtd, __in uint8_t *buf,
                                  __in int32_t len)
* 描述: 写nandflash原始数据，地址可以时一页内的任何地址
* 输入: mtd: nandflash设备的mtd设备父类
       buf: 需要写入的数据缓冲区
       len: 需要写入的数据长度
* 输出: none
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
static void gpmi_write_buf(__in struct mtd_info *mtd, __in const uint8_t *buf, __in int32_t len)
{
    struct gpmi_info *gpmi = ((struct nand_chip *)(mtd->priv))->priv;

    if(len > NAND_MAX_PAGESIZE)
        printl(LOG_LEVEL_WARN, "[GPMI:WARN] len too long\n");

    if(!buf)
        printl(LOG_LEVEL_WARN, "[GPMI:WARN] buffer point is NULL\n");

    send_data(gpmi->cur_chip, (uint32_t)buf, len);
}


/********************************************************************************
* 函数: static uint8_t gpmi_read_byte(__in struct mtd_info *mtd)
* 描述: 读取nandflash一页数据中的一个字节，地址可以是一页内的任何地址
* 输入: mtd: nandflash设备的mtd设备父类
* 输出: none
* 返回: 读取到的字节
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint8_t gpmi_read_byte(__in struct mtd_info *mtd)
{
    uint8_t byte;

    gpmi_read_buf(mtd, (uint8_t *)&byte, 1);

    return byte;
}

/********************************************************************************
* 函数: static int32_t gpmi_ecc_read_page(__in struct mtd_info *mtd,
                                         __out uint8_t *buf)
* 描述: 读nandflash一页经过ecc校验的数据，地址必须时页起始地址
* 输入: mtd: nandflash设备的父类
* 输出: buf: 取出来的数据缓冲区
* 返回: 0: 成功
       -ETIMEDOUT: 读数据超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_ecc_read_page(__in struct mtd_info *mtd, __out uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    struct gpmi_info *gpmi = this->priv;
    uint32_t error;
    uint32_t failed = 0;
    uint32_t corrected = 0;
    uint8_t *status;
    int32_t i = 0;

    error = read_page(mtd, gpmi->cur_chip, (uint32_t)(gpmi->data_buf), (uint32_t)(gpmi->oob_buf));

    if(error)
    {
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] read ecc based page failed, error = %d", error);
        return error;
    }

    status = gpmi->oob_buf + gpmi->aux_status_ofs;

    for(i = 0; i < gpmi->ecc_chunk_cnt; i++)
    {
        if((*status == 0x00) || (*status == 0xff))
            continue;

        if(*status == 0xfe)
        {
            failed++;
            continue;
        }

        corrected += *status;
    }

    /* 设置mtd层参数 */
    mtd->ecc_stats.failed += failed;
    mtd->ecc_stats.corrected += corrected;

    memset(this->oob_poi, 0xff, mtd->oobsize);
    this->oob_poi[0] = gpmi->oob_buf[0];

    memcpy(buf, gpmi->data_buf, mtd->writesize);

    return error;
}


/********************************************************************************
* 函数: static int32_t gpmi_ecc_write_page(__in struct mtd_info *mtd,
                                          __out uint8_t *buf)
* 描述: 写nandflash一页经过ecc校验的数据，地址必须时页起始地址
* 输入: mtd: nandflash设备的父类
       buf: 需要写的数据缓冲区
* 输出: none
* 返回: 0: 成功
       -ETIMEDOUT: 读数据超时
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_ecc_write_page(__in struct mtd_info *mtd, __in const uint8_t *buf)
{
    uint32_t error = 0;
    struct nand_chip *this = mtd->priv;
    struct gpmi_info *gpmi = this->priv;


    uint8_t *data_buf = gpmi->data_buf;
    uint8_t *oob_buf = gpmi->oob_buf;

    memcpy(data_buf, buf, mtd->writesize);
    memcpy(oob_buf, this->oob_poi, mtd->oobsize);

    error = send_page(mtd, gpmi->cur_chip, (uint32_t)gpmi->data_buf, (uint32_t)gpmi->oob_buf);

    if(error)
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] write ecc based page failed, error = %d", error);

    return error;
}


/********************************************************************************
* 函数: static int32_t gpmi_alloc_buf(__in struct gpmi_info *gpmi)
* 描述: 分配gpmi使用的缓冲区空间
* 输入: gpmi: gpmi信息结构体
* 输出: none
* 返回: 0: 成功
       -ENOMEM: 内存分配失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_alloc_buf(__in struct gpmi_info *gpmi)
{
    uint8_t *pBuf = NULL;

    pBuf = (uint8_t *)dlmemalign(DMA_ALIGNMENT, NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE);

    if(!pBuf)
    {
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] failed to allocate buffer\n");
        return -ENOMEM;
    }
    memset(pBuf, 0, NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE);

    gpmi->data_buf = pBuf;
    gpmi->oob_buf = pBuf + NAND_MAX_PAGESIZE;

    return 0;
}


/********************************************************************************
* 函数: static int32_t gpmi_scan_bbt(mtd_info *mtd)
* 描述: gpmi层扫描bbt，在调用上层之前处理一些具体数据
* 输入: mtd: nandflash设备自身指针
* 输出: none
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t gpmi_scan_bbt(struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;
    struct gpmi_info *gpmi = this->priv;
    int32_t error;

    /* gpmi具体参数设置 */
    gpmi->ecc_chunk_cnt = mtd->writesize / GPMI_ECC_BLOCK_SIZE;
	gpmi->ecc_strength = gpmi_nfc_get_ecc_strength(mtd->writesize, mtd->oobsize);

	/* 计算块标记信息 */
	gpmi->aux_status_ofs = ((GPMI_ECC_METADATA_SIZE + 0x03) & ~0x03);

	/* 设置nandflash布局图 */
	set_geometry(mtd);

    /* 重新设置新的时序 */
    error = set_hw_timing(this, this->timing);

	if (error)
		return error;

    return nand_default_bbt(mtd);
}


/********************************************************************************
* 函数: int32_t board_nand_init(__in struct nand_chip *chip)
* 描述: 初始化板载的nandflash芯片
* 输入: chip: nandflash设备自身指针
* 输出: none
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t board_nand_init(__in struct nand_chip *chip)
{
    struct gpmi_info *gpmi = NULL;
    struct nand_device_info *type = NULL;
    int32_t error;

    gpmi = dlmalloc(sizeof(struct gpmi_info));
    if(!gpmi)
    {
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] failed to allocate gpmi_info\n");
        return -ENOMEM;
    }

    memset(gpmi, 0, sizeof(struct gpmi_info));

    if(gpmi_alloc_buf(gpmi))
    {
        printl(LOG_LEVEL_ERR, "[GPMI:ERR] failed to allocate gpmi buffer\n");
        return -ENOMEM;
    }

    /* 初始化gpmi */
    gpmi_init();

    chip->priv = gpmi;

    chip->cmd_ctrl = gpmi_cmd_ctrl;
    chip->dev_ready = gpmi_dev_ready;

    chip->select_chip = gpmi_select_chip;
    chip->read_byte = gpmi_read_byte;
    chip->read_buf = gpmi_read_buf;
    chip->write_buf = gpmi_write_buf;

    chip->ecc_ctrl.read_page = gpmi_ecc_read_page;
    chip->ecc_ctrl.write_page = gpmi_ecc_write_page;

    chip->options |= NAND_NO_SUBPAGE_WRITE;

    chip->ecc_ctrl.mode = NAND_ECC_HW;
    chip->ecc_ctrl.ecc_bytes_per_step = 9;
    chip->ecc_ctrl.data_size_per_step = 512;

    chip->scan_bbt = gpmi_scan_bbt;


    /* 预设一个芯片数量，下面具体扫描时会更新 */
    chip->numchips = 1;
	/* 设置时序，设置一个安全时序，为下面扫面设备做准备  */
	type = nand_device_get_safenand_info();
	error = set_hw_timing(chip, &(type->timing));

	if (error)
		return error;

    return 0;
}


