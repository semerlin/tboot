#include "stddef.h"
#include "log.h"
#include "errno.h"
#include "malloc.h"
#include "arch/arch-mx28/mx28_regs.h"
#include "arch/arch-mx28/regs_dma_apbh.h"
#include "arch/arch-mx28/dma_apbh.h"



//DMA链结构体数组
static struct dma_chan dma_channels[DMA_MAX_CHANNELS];

/* DMA模块复位标志 */
static bool dma_reset_flag = false;



/********************************************************************************
* 函数: static int32_t dma_apbh_enable(__in struct dma_chan *pchan,
                                      __in uint32_t chan)
* 描述: 使能一个dma操作链,写入第一次操作的cmd链地址,cmd在chan内部已通过链表链接好
* 输入: pchan:需要使能的dma命令操作链
       chan: 链使用的通道号
* 输出: none
* 返回: 0: 成功
       -EFAULT: 失败
* 作者:
* 版本: V1.0
**********************************************************************************/
static int32_t dma_apbh_enable(__in struct dma_chan *pchan, __in uint32_t chan)
{
    uint32_t sem = 0;
    struct dma_desc *pdesc;

    /* 第一个活动的DMA地址 */
    pdesc = list_first_entry(&pchan->active, struct dma_desc, node);
    if(NULL == pdesc)
        return -EFAULT;

    /* 取得当前信号量的计数值 */
    sem = REG_RD(REGS_APBH_BASE, HW_APBH_CHn_SEMA(chan));
    sem = (sem & BM_APBH_CHn_SEMA_PHORE) >> BP_APBH_CHn_SEMA_PHORE;

    /* chan已经使能正在使用 */
    if(pchan->flags & DMA_FLAGS_BUSY)
    {
        /* 正在执行最后一条指令 */
        if(pdesc->cmd.cmd.bits.chain == 0)
            return 0;

        if(sem < 2)
        {
            if(!sem)
                return 0; /* 指令执行完毕 */

            /* 正在执行原先运行指令的最后一条，载入新加入的指令的第一条 */
            pdesc = list_entry(pdesc->node.next, struct dma_desc, node);

            /* 设置下一条指令地址 */
            REG_WR(REGS_APBH_BASE, HW_APBH_CHn_NXTCMDAR(chan), dma_cmd_address(pdesc));
        }

        sem = pchan->pending_num;
        pchan->pending_num = 0;
        REG_WR(REGS_APBH_BASE, HW_APBH_CHn_SEMA(chan), sem);  /* 设置DMA信号量计数器 */
        pchan->active_num += sem;
        return 0;
    }

    /* chan还没有使能 */
    pchan->active_num += pchan->pending_num;
    pchan->pending_num = 0;
    REG_WR(REGS_APBH_BASE, HW_APBH_CHn_NXTCMDAR(chan), dma_cmd_address(pdesc));  /* 设置初始指令地址 */
    REG_WR(REGS_APBH_BASE, HW_APBH_CHn_SEMA(chan), pchan->active_num);  /* 设置DMA信号量计数器 */
    REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL0, 1 << chan);  /* 使能时钟 */

    return 0;
}



/********************************************************************************
* 函数: static void dma_apbh_disable(__in uint32_t chan)
* 描述: 禁止dma链
* 输入: chan: dma通道号
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void dma_apbh_disable(__in uint32_t chan)
{
    /* 关闭时钟 */
    REG_SET(REGS_APBH_BASE, HW_APBH_CTRL0, 1 << chan);
}


/********************************************************************************
* 函数: static void dma_apbh_reset(__in uint32_t chan)
* 描述: 复位dma通道
* 输入: chan: 通道号
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void dma_apbh_reset(__in uint32_t chan)
{
    /* 复位dma通道 */
    REG_SET(REGS_APBH_BASE, HW_APBH_CHANNEL_CTRL, 1 << (chan + BP_APBH_CTRL0_CLKGATE_CHANNEL));
}


/********************************************************************************
* 函数: static void dma_apbh_freeze(__in uint32_t chan)
* 描述: 冻结dma通道
* 输入: chan: 通道号
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void dma_apbh_freeze(__in uint32_t chan)
{
    /* 冻结dma通道 */
    REG_SET(REGS_APBH_BASE, HW_APBH_CHANNEL_CTRL, 1 << chan);
}

/********************************************************************************
* 函数: static void dma_apbh_unfreeze(__in uint32_t chan)
* 描述: 解冻dma通道
* 输入: chan: 通道号
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void dma_apbh_unfreeze(__in uint32_t chan)
{
    /* 解冻dma通道 */
    REG_CLR(REGS_APBH_BASE, HW_APBH_CHANNEL_CTRL, 1 << chan);
}

/********************************************************************************
* 函数: static void dma_apbh_info(__in uint32_t chan, __out dma_info *pInfo)
* 描述: 获取dma指定通道的信息
* 输入: chan: 通道号
* 输出: pInfo: 获取到的信息结构体
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void dma_apbh_info(__in uint32_t chan, __out struct dma_info *pInfo)
{
    uint32_t reg = 0;

    if(!pInfo)
        return ;

    reg = REG_RD(REGS_APBH_BASE, HW_APBH_CTRL2);
    pInfo->status = ((reg >> chan) & 0x01);
    pInfo->buf_addr = REG_RD(REGS_APBH_BASE, HW_APBH_CHn_BAR(chan));  //需要执行的数据缓冲区的地址
}

/********************************************************************************
* 函数: static uint32_t dma_apbh_read_semaphore(__in uint32_t chan)
* 描述: 取得dma通道当前信号量值
* 输入: chan: 通道号
* 输出: none
* 返回: 通道信号量值
* 作者:
* 版本: V1.0
**********************************************************************************/
static uint32_t dma_apbh_read_semaphore(__in uint32_t chan)
{
     return ((REG_RD(REGS_APBH_BASE, HW_APBH_CHn_SEMA(chan))
             & BM_APBH_CHn_SEMA_PHORE) >> BP_APBH_CHn_SEMA_PHORE);
}

/********************************************************************************
* 函数: static dma_apbh_enable_irq(__in uint32_t chan, __in bool enable)
* 描述: 使能/禁止dma通道中断
* 输入: chan: 通道号
       enable: true: 使能
               false: 禁止
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void dma_apbh_enable_irq(__in uint32_t chan, __in bool enable)
{
    if(enable)
        REG_SET(REGS_APBH_BASE, HW_APBH_CTRL1, 1 << (chan + 16));
    else
        REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL1, 1 << (chan + 16));
}

/********************************************************************************
* 函数: static uint32_t dma_apbh_irq_is_pending(__in uint32_t chan)
* 描述: 检测dma是否有指令完成/错误中断产生
* 输入: chan: dma通道号
* 输出: none
* 返回: 0: 没有中断
       0x01: 有正常中断
       0x02: 有错误中断
       0x03: 有正常和错误中断
* 作者:
* 版本: V1.0
**********************************************************************************/
static uint32_t dma_apbh_irq_is_pending(__in uint32_t chan)
{
    uint32_t reg = 0;

    reg = ((REG_RD(REGS_APBH_BASE, HW_APBH_CTRL1) >> chan) & 0x01);  //正常中断
    reg |= (((REG_RD(REGS_APBH_BASE, HW_APBH_CTRL2) >> chan) & 0x01) << 1);  //错误中断


    return reg;
}

/********************************************************************************
* 函数: static void dma_apbh_ack_irq(__in uint32_t chan)
* 描述: 清除指定dma通道中断标志
* 输入: chan: dma通道
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void dma_apbh_ack_irq(__in uint32_t chan)
{
    REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL1, 1 << chan);
    REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL2, 1 << chan);
}


/*************************************************************************************************************
*********************************************外部接口***********************************************************
**************************************************************************************************************/


/********************************************************************************
* 函数: static int32_t dma_request(__in uint32_t channel)
* 描述: 请求dma通道
* 输入: channel: 通道名
* 输出: none
* 返回: 0: 成功
       -EINVAL: channel参数无效
       -ENODEV: 没有设备注册使用通道
       -EBUSY: 通道已经被分配
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_request(__in uint32_t channel)
{
    struct dma_chan *pchan;

    if(channel >= DMA_MAX_CHANNELS)
        return -EINVAL;

    pchan = dma_channels + channel;

    /* 通道没有设备 */
    if(!(pchan->flags & DMA_FLAGS_VALID))
        return -ENODEV;

    /* 通道已经被分配 */
    if(pchan->flags & DMA_FLAGS_ALLOCATED)
        return -EBUSY;

    pchan->flags |= DMA_FLAGS_ALLOCATED;
    pchan->active_num = 0;
    pchan->pending_num = 0;

    INIT_LIST_HEAD(&pchan->active);
    INIT_LIST_HEAD(&pchan->done);

    return 0;
}

/********************************************************************************
* 函数: void dma_release(__in uint32_t channel)
* 描述: 释放dma通道
* 输入: channel: 通道名称
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void dma_release(__in uint32_t channel)
{
    struct dma_chan *pchan;

    if(channel >= DMA_MAX_CHANNELS)
        return ;

    pchan = dma_channels + channel;

    //通道没有设备
    if(!(pchan->flags & DMA_FLAGS_VALID))
        return ;

    //通道没有被分配
    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return ;

    //通道正在使用
    if(pchan->flags & DMA_FLAGS_BUSY)
        return ;

    pchan->dev = 0;
    pchan->active_num = 0;
    pchan->pending_num = 0;
    //pchan->flags &= ~DMA_FLAGS_ALLOCATED;
}



/********************************************************************************
* 函数: uint32_t dma_enable(__in uint32_t channel)
* 描述: 使能指定dma通道开始工作
* 输入: channel: 通道号
* 输出: none
* 返回: 0: 成功
       -EINVAL: 通道参数无效
       -ENODEV: 通道没有设备注册
       -EFAULT: 通道未被分配使用
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_enable(__in uint32_t channel)
{
    int32_t ret = 0;

    struct dma_chan *pchan;

    if(channel >= DMA_MAX_CHANNELS)
        return -EINVAL;

    pchan = dma_channels + channel;

    //通道没有设备注册
    if(!(pchan->flags & DMA_FLAGS_VALID))
        return -ENODEV;

    //通道未被分配
    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return -EFAULT;

    //有待执行的dma指令
    if(pchan->pending_num)
        ret = dma_apbh_enable(pchan, channel);

    pchan->flags |= DMA_FLAGS_BUSY;

    return ret;


}


/********************************************************************************
* 函数: void dma_disable(__in uint32_t channel)
* 描述: 停止dma通道的执行
* 输入: channel: 通道号
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void dma_disable(__in uint32_t channel)
{
    struct dma_chan *pchan;

    if(channel >= DMA_MAX_CHANNELS)
        return ;

    pchan = dma_channels + channel;

    //通道没有设备注册
    if(!(pchan->flags & DMA_FLAGS_VALID))
        return ;

    //通道没有被分配
    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return ;

    //通道没有使用
    if(!(pchan->flags & DMA_FLAGS_BUSY))
        return ;

    dma_apbh_disable(channel);

    pchan->flags &= ~DMA_FLAGS_BUSY;
    pchan->active_num = 0;
    pchan->pending_num = 0;

    //把还未执行的添加到已执行的头部
    list_splice_init(&pchan->active, &pchan->done);
}


/********************************************************************************
* 函数: int32_t dma_get_info(__in uint32_t channel, __out struct dam_info *info)
* 描述: 取得dma通道的信息
* 输入: channel: 通道号
* 输出: info: 通道信息
* 返回: 0: 成功
       -ENODEV: 通道没有设备注册
       -EINVAL: channel或info参数无效
       -EFAULT: 通道未被申请分配使用
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_get_info(__in uint32_t channel, __out struct dma_info *info)
{
    struct dma_chan *pchan;
    if(!info)
        return -EINVAL;

    if(channel >= DMA_MAX_CHANNELS)
        return -EINVAL;

    pchan = dma_channels + channel;

    //通道没有设备注册
    if(!(pchan->flags & DMA_FLAGS_VALID))
        return -ENODEV;

    //通道未被分配使用
    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return -EFAULT;

    dma_apbh_info(channel, info);

    return 0;
}


/********************************************************************************
* 函数: int32_t dma_cooked(__in int32_t channel, __in struct list_head *head)
* 描述: 更新active链表，把已经执行的dma指令移除
* 输入: channel: 需要更新的通道号
       head: NULL: 已执行的添加到done链表
             不为NULL: 已执行的添加到head链表
* 输出: none
* 返回: 0: 成功
       -ENODEV: 通道没有设备注册
       -EINVAL: channel参数无效
       -EFAULT: 通道未被申请分配使用
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_cooked(__in int32_t channel, __in struct list_head *head)
{
    int32_t sem = 0;
    struct dma_chan *pchan;
    struct dma_desc *pdesc;
    struct list_head *p, *q;

    if(channel >= DMA_MAX_CHANNELS)
        return -EINVAL;

    pchan = dma_channels + channel;

    /* 通道没有设备注册 */
    if(!(pchan->flags & DMA_FLAGS_VALID))
        return -ENODEV;

    /* 通道未被分配使用 */
    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return -EFAULT;

    sem = dma_apbh_read_semaphore(channel);


    /* 还没有指令执行，不需要移除 */
    if(sem == pchan->active_num)
        return 0;

    list_for_each_safe(p, q, &pchan->active)
    {
        /* 没有执行完毕的命令了，剩下的命令正在或者待执行 */
        if(pchan->active_num <= sem)
            break;

        /* 移动指令到链表尾 */
        pdesc = list_entry(p, struct dma_desc, node);
        pdesc->flags &= ~DMA_DESC_READY;
        if(head)
            list_move_tail(p, head);
        else
            list_move_tail(p, &pchan->done);

        /* */
        //if(pdesc->flags & DMA_DESC_LAST)
        pchan->active_num--;
    }

    if(sem == 0)
        pchan->flags &= ~DMA_FLAGS_BUSY;

    return 0;
}


/********************************************************************************
* 函数: void dma_reset(__in uint32_t channel)
* 描述: dma通道复位
* 输入: channel: 通道号
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void dma_reset(__in uint32_t channel)
{
    struct dma_chan *pchan;

    if(channel >= DMA_MAX_CHANNELS)
        return ;

    pchan = dma_channels + channel;

    if(!(pchan->flags & DMA_FLAGS_VALID))
        return ;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return ;

    dma_apbh_reset(channel);
}

/********************************************************************************
* 函数: void dma_freeze(__in uint32_t channel)
* 描述: 冻结dma通道
* 输入: channel: 通道号
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void dma_freeze(__in uint32_t channel)
{
    struct dma_chan *pchan;

    if(channel >= DMA_MAX_CHANNELS)
        return ;

    pchan = dma_channels + channel;

    if(!(pchan->flags & DMA_FLAGS_VALID))
        return ;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return ;

    dma_apbh_freeze(channel);
}

/********************************************************************************
* 函数: void dma_unfreeze(__in uint32_t channel)
* 描述: 解冻dma通道
* 输入: channel: 通道号
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void dma_unfreeze(__in uint32_t channel)
{
    struct dma_chan *pchan;

    if(channel >= DMA_MAX_CHANNELS)
        return ;

    pchan = dma_channels + channel;

    if(!(pchan->flags & DMA_FLAGS_VALID))
        return ;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return ;

    dma_apbh_unfreeze(channel);
}

/********************************************************************************
* 函数: int32_t dma_read_semaphore(__in uint32_t channel)
* 描述: 读取通道当前信号量计数值
* 输入: channel: 通道号
* 输出: none
* 返回: 成功: 信号量当前计数值
       -EINVAL: channel值无效
       -ENODEV: 通道没有设备注册使用
       -EFAULT: 通道没有分配使用
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_read_semaphore(__in uint32_t channel)
{
	int ret = 0;
	struct dma_chan *pchan;

	if(channel >= DMA_MAX_CHANNELS)
        return -EINVAL;

	pchan = dma_channels + channel;

    if(!(pchan->flags & DMA_FLAGS_VALID))
        return -ENODEV;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return -EFAULT;

	ret = dma_apbh_read_semaphore(channel);

	return ret;
}

/********************************************************************************
* 函数: void dma_enable_irq(__in uint32_t channel, __in bool enable)
* 描述: 使能/禁止指定通道dma中断
* 输入: channel: 通道号
       enable: true: 使能
               false: 禁止
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void dma_enable_irq(__in uint32_t channel, __in bool enable)
{
    struct dma_chan *pchan;

    if(channel >= DMA_MAX_CHANNELS)
        return ;

    pchan = dma_channels + channel;

    if(!(pchan->flags & DMA_FLAGS_VALID))
        return ;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return ;

    dma_apbh_enable_irq(channel, enable);
}


/********************************************************************************
* 函数: int32_t dma_irq_is_pending(__in int32_t channel)
* 描述: 检测指定通道是否有中断产生
* 输入: channel: 通道号
* 输出: none
* 返回: 0: 没有中断产生
       1: 有中断产生
       -EINVAL： channel值无效
       -ENODEV： 没有设备注册使用通道
       -EFAULT： 通道没有被分配
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_irq_is_pending(__in int32_t channel)
{
	int ret = 0;
	struct dma_chan *pchan;

	if (channel >= DMA_MAX_CHANNELS)
		return -EINVAL;

	pchan = dma_channels + channel;
	if(!(pchan->flags & DMA_FLAGS_VALID))
        return -ENODEV;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return -EFAULT;

	ret = dma_apbh_irq_is_pending(channel);

	return ret;
}

/********************************************************************************
* 函数: void dma_ack_irq(__in uint32_t channel)
* 描述: 清零通道中断标志
* 输入: channel: 通道号
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void dma_ack_irq(__in uint32_t channel)
{
    struct dma_chan *pchan;

    if(channel >= DMA_MAX_CHANNELS)
        return ;

    pchan = dma_channels + channel;

    if(!(pchan->flags & DMA_FLAGS_VALID))
        return ;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return ;

    dma_apbh_ack_irq(channel);
}

/********************************************************************************
* 函数: struct dma_desc *dma_alloc_desc(void)
* 描述: 动态分配描述符结构
* 输入: none
* 输出: none
* 返回: 成功: 描述符地址
       失败: NULL
* 作者:
* 版本: V1.0
**********************************************************************************/
struct dma_desc *dma_alloc_desc(void)
{
    struct dma_desc *pdesc;

#ifdef CONFIG_MMU
    /* 此处添加MMU支持 */
#else
    pdesc = (struct dma_desc *)dlmemalign(DMA_ALIGNMENT, sizeof(struct dma_desc));
#endif

    if(NULL == pdesc)
        return NULL;

    memset(pdesc, 0, sizeof(struct dma_desc));

    pdesc->address = (uint32_t)pdesc;

    return pdesc;
}

/********************************************************************************
* 函数: void dma_free_desc(struct dma_desc *pdesc)
* 描述: 释放描述符动态分配的结构体
* 输入: pdesc: 描述符指针
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void dma_free_desc(__in struct dma_desc *pdesc)
{
    if(NULL == pdesc)
        return ;

    dlfree((int8_t *)pdesc);
}

/********************************************************************************
* 函数: int32_t dma_desc_append(__in uint32_t channel,
                               __in struct dma_desc *pdesc)
* 描述: 添加一个描述符到当前通道尾部
* 输入: channel: 通道号
       pdesc: 需要添加的描述符
* 输出: none
* 返回: 0: 成功
       -EINVAL： channel参数无效
       -ENODEV： 此通道没有有效的设备
       -EFAULT： 此通道没有被分配使用
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_desc_append(__in uint32_t channel, __in struct dma_desc *pdesc)
{
    struct dma_chan *pchan;
    struct dma_desc *last;

    if(channel >= DMA_MAX_CHANNELS)
        return -EINVAL;

    pchan = dma_channels + channel;

    if(!(pchan->flags & DMA_FLAGS_VALID))
        return -ENODEV;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return -EFAULT;

    /* 初始化此描述符数据 */
    pdesc->cmd.cmd.bits.dec_sem = 1;
    pdesc->cmd.next = dma_cmd_address(pdesc);
    pdesc->flags |= DMA_DESC_FIRST | DMA_DESC_LAST;

    if(!list_empty(&pchan->active))  /* active链表不为空 */
    {
        /* 改变标志位，添加到链表尾 */
        last = list_entry(pchan->active.prev, struct dma_desc, node);
        pdesc->flags &= ~DMA_DESC_FIRST;
        last->flags &= ~DMA_DESC_LAST;

        last->cmd.next = dma_cmd_address(pdesc);
        last->cmd.cmd.bits.chain = 1;
    }

    pchan->pending_num ++;

    pdesc->flags |= DMA_DESC_READY;

    list_add_tail(&pdesc->node, &pchan->active);

    return 0;
}


/********************************************************************************
* 函数: int32_t dma_desc_add_list(__in uint32_t channel,
                                 __in struct list_head *head)
* 描述: 添加一个描述符链表到当前通道尾部
* 输入: channel: 通道号
       head: 需要添加的描述符链表
* 输出: none
* 返回: 0: 成功
       -EINVAL： channel参数无效
       -ENODEV： 此通道没有有效的设备
       -EFAULT： 此通道没有被分配使用
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_desc_add_list(__in uint32_t channel, __in struct list_head *head)
{
    int32_t size = 0;
    struct dma_chan *pchan;
    struct list_head *p;
    struct dma_desc *pcur, *prev = NULL;

    if(channel >= DMA_MAX_CHANNELS)
        return -EINVAL;

    pchan = dma_channels + channel;

    if(!(pchan->flags & DMA_FLAGS_VALID))
        return -ENODEV;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return -EFAULT;


    /* 组织好指令链表 */
    list_for_each(p, head)
    {
        pcur = list_entry(p, struct dma_desc, node);
        pcur->cmd.cmd.bits.dec_sem = 1;

        if(prev)
            prev->cmd.next = dma_cmd_address(pcur);
        else
            pcur->flags |= DMA_DESC_FIRST;

        pcur->flags |= DMA_DESC_READY;
        prev = pcur;

        size++;
    }

    pcur = list_first_entry(head, struct dma_desc, node);
    prev->cmd.next = dma_cmd_address(pcur);
    prev->flags |= DMA_DESC_LAST;


    if(!list_empty(&pchan->active)) /* active链表不为空 */
    {
        prev = list_entry(pchan->active.prev, struct dma_desc, node);
        prev->cmd.next = dma_cmd_address(pcur);
        pcur->flags &= ~DMA_DESC_FIRST;
        prev->flags &= ~DMA_DESC_LAST;

        pcur = list_first_entry(&pchan->active, struct dma_desc, node);
        prev = list_entry(head->prev, struct dma_desc, node);

        prev->cmd.next = dma_cmd_address(pcur);
    }

    list_splice_tail(head, &pchan->active);

    pchan->pending_num += size;

    return 0;

}

/********************************************************************************
* 函数: int32_t dma_get_cooked(__in int32_t channel, __out struct list_head *head)
* 描述: 取得dma已经完成的指令链表
* 输入: channel: dma通道
* 输出: head: 完成链表拷贝的位置
* 返回: 0: 成功
       -EINVAL： channel参数无效
       -ENODEV： 此通道没有有效的设备
       -EFAULT： 此通道没有被分配使用
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_get_cooked(__in int32_t channel, __out struct list_head *head)
{
    struct dma_chan *pchan;

    if(head == NULL)
        return -EINVAL;

    if(channel >= DMA_MAX_CHANNELS)
        return -EINVAL;

    pchan = dma_channels + channel;

     if(!(pchan->flags & DMA_FLAGS_VALID))
        return -ENODEV;

    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return -EFAULT;


    list_splice(&pchan->done, head);

    return 0;

}


/********************************************************************************
* 函数: int32_t dma_init(__in enum dma_channel channel)
* 描述: 初始化系统dma模块
* 输入: channel: dma通道
* 输出: none
* 返回: 0: 成功
       -EINVAL: channel参数无效
       -EBUSY: 通道已经被分配
       -ETIMEDOUT: 复位失败,超时
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_init(__in enum dma_channel channel)
{
    struct dma_chan *pchan;
    int32_t err = 0;
    int32_t i = 0;

    if(!dma_reset_flag)
    {
        /* 复位dma控制器，超时时间1s */
        REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL0, BM_APBH_CTRL0_SFTRST);
        mdelay(2);
        REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL0, BM_APBH_CTRL0_CLKGATE);
        REG_SET(REGS_APBH_BASE, HW_APBH_CTRL0, BM_APBH_CTRL0_SFTRST);
        for(i = 1000000; i > 0; --i)
        {
            if(REG_RD(REGS_APBH_BASE, HW_APBH_CTRL0) & BM_APBH_CTRL0_CLKGATE)
                break;

            udelay(1);
        }
        /* 复位超时 */
        if(i <= 0)
        {
            printl(LOG_LEVEL_ERR, "[DMA:ERR] reset dma block timeout.\n");
            return -ETIMEDOUT;
        }

        REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL0, BM_APBH_CTRL0_SFTRST);
        mdelay(2);
        REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL0, BM_APBH_CTRL0_CLKGATE);

        for(i = 1000000; i > 0; --i)
        {
            if(!(REG_RD(REGS_APBH_BASE, HW_APBH_CTRL0) & BM_APBH_CTRL0_CLKGATE))
                break;

            udelay(1);
        }
        /* 复位超时 */
        if(i <= 0)
        {
            printl(LOG_LEVEL_ERR, "[DMA:ERR] reset dma block timeout.\n");
            return -ETIMEDOUT;
        }

    #ifdef CONFIG_APBH_DMA_BURST8
        REG_SET(REGS_APBH_BASE HW_APBH_CTRL0, BM_APBH_CTRL0_AHB_BURST8_EN);
    #else
        REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL0, BM_APBH_CTRL0_AHB_BURST8_EN);
    #endif

    #ifdef CONFIG_APBH_DMA_BURST
        REG_SET(REGS_APBH_BASE, HW_APBH_CTRL0, BM_APBH_CTRL0_APB_BURST_EN);
    #else
        REG_CLR(REGS_APBH_BASE, HW_APBH_CTRL0, BM_APBH_CTRL0_APB_BURST_EN);
    #endif

        dma_reset_flag = true;
    }


    /* 初始化dma通道 */
    if(channel >= DMA_MAX_CHANNELS)
        return -EINVAL;

    pchan = dma_channels + channel;
    pchan->flags |= DMA_FLAGS_VALID;

    err = dma_request(channel);

    if(err)
    {
        printl(LOG_LEVEL_ERR, "[DMA:ERR] can't acquire dma channel %d.\n", channel);

        //释放通道
        dma_release(channel);
        return err;
    }

    dma_reset(channel);
    dma_ack_irq(channel);

    return 0;
}


/********************************************************************************
* 函数: int32_t dma_wait_complete(__in uint32_t uSecTimeout, __in uint32_t chan)
* 描述: 等待指定通道的dma执行完毕
* 输入: uSecimeout: 等待时间
       chan: 通道号
* 输出: none
* 返回: 0: 成功
       1: 失败
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_wait_complete(__in uint32_t uSecTimeout, __in uint32_t chan)
{
    struct dma_chan *pchan;
    if(chan >= DMA_MAX_CHANNELS)
        return 1;

    pchan = dma_channels + chan;
    if(!(pchan->flags & DMA_FLAGS_ALLOCATED))
        return 1;

    while(!(REG_RD(REGS_APBH_BASE, HW_APBH_CTRL1) & (1 << chan)) && --uSecTimeout);

    if(uSecTimeout <= 0)
    {
        dma_apbh_reset(chan);
        return 1;
    }

    return 0;
}


/********************************************************************************
* 函数: int32_t dma_go(__in int32_t chan)
* 描述: 开启dma执行
* 输入: chan: 通道号
* 输出: none
* 返回: 0: 执行完毕
       -ETIMEDOUT: 执行失败，超时
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t dma_go(__in int32_t chan)
{
    uint32_t timeout = 10000;
    int32_t err;

    LIST_HEAD(tmp_desc_list);

    /* 使能中断 */
    dma_enable_irq(chan, true);

    /* 开始执行 */
    dma_enable(chan);

    /* 等待执行完毕 */
    err = (dma_wait_complete(timeout, chan)) ? -ETIMEDOUT : 0;

    /* 清除运行完毕的指令 */
    dma_cooked(chan, &tmp_desc_list);

    /* 关闭通道，清中断标志位 */
    dma_ack_irq(chan);
    dma_reset(chan);
    dma_enable_irq(chan, false);
    dma_disable(chan);

    return err;
}
