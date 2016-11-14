#ifndef _DMA_APBH_H_
  #define _DMA_APBH_H_


#include "list.h"

/* DMA通道定义 */
enum dma_channel
{
    DMA_CHANNEL_AHB_APBH_BASE = 0,
    DMA_CHANNEL_AHB_APBH_SSP0 = 0,
    DMA_CHANNEL_AHB_APBH_SSP1,
    DMA_CHANNEL_AHB_APBH_SSP2,
    DMA_CHANNEL_AHB_APBH_SSP3,
    DMA_CHANNEL_AHB_APBH_GPMI0,
    DMA_CHANNEL_AHB_APBH_GPMI1,
    DMA_CHANNEL_AHB_APBH_GPMI2,
    DMA_CHANNEL_AHB_APBH_GPMI3,
    DMA_CHANNEL_AHB_APBH_GPMI4,
    DMA_CHANNEL_AHB_APBH_GPMI5,
    DMA_CHANNEL_AHB_APBH_GPMI6,
    DMA_CHANNEL_AHB_APBH_GPMI7,
    DMA_CHANNEL_AHB_APBH_HSADC,
    DMA_CHANNEL_AHB_APBH_LCDIF,
    DMA_CHANNEL_AHB_APBH_EMPTY0,
    DMA_CHANNEL_AHB_APBH_EMPTY1,
    DMA_MAX_CHANNELS,
};



/* DMA指令结构体 */
#define NO_DMA_XFER	   0x00
#define DMA_WRITE	   0x01
#define DMA_READ	   0x02
#define DMA_SENSE	   0x03

struct dma_cmd_bits
{
	uint32_t command:2;
	uint32_t chain:1;
	uint32_t irq_complete:1;
	uint32_t nand_lock:1;
	uint32_t nand_wait4ready:1;
	uint32_t dec_sem:1;  //内部强制设置为1，为了操作方便和统计
	uint32_t cmd_wait4end:1;
	uint32_t halt_on_terminate:1;
	uint32_t resv:3;
	uint32_t num_pio_words:4;
	uint32_t num_trans_bytes:16;
};

/* DMA支持0-15个PIO字节 */
#ifndef CONFIG_ARCH_DMA_PIO_WORDS
  #define DMA_PIO_WORDS   15
#else
  #define DMA_PIO_WORDS   CONFIG_ARCH_DMA_PIO_WORDS
#endif

struct dma_cmd
{
	uint32_t next;
	union
	{
        uint32_t data;
        struct dma_cmd_bits bits;
	}cmd;
	uint32_t bufaddr;
	uint32_t pio_words[DMA_PIO_WORDS];
};


#define DMA_ALIGNMENT	8  //描述符结构体8字节对齐
/* DMA指令描述 */

/* flag位 */
#define DMA_DESC_READY    0x80000000  //指令可执行
#define DMA_DESC_FIRST    0x00000001  //第一条指令
#define DMA_DESC_LAST     0x00000002  //最后一条指令
struct dma_desc
{
    /* DMA指令 */
	struct dma_cmd cmd;
	/* 这条指令的标记 */
	uint32_t flags;
	/* 此描述器的物理地址,MMU使能时有用 */
	uint32_t address;
	/* 描述器链表 */
	struct list_head node;
};

/* DMA状态信息 */
#define DMA_INFO_ERR       0x00000001
#define DMA_INFO_ERR_STAT  0x00010000
struct dma_info
{
	uint32_t status;   //错误信息
    uint32_t buf_addr;  //buffer字段的地址
};



/* dma通道链表结构体 */
#define DMA_FLAGS_BUSY	     0x00000001   //通道正在使用
#define DMA_FLAGS_ALLOCATED	 0x00000002   //通道是否被分配
#define DMA_FLAGS_VALID	     0x80000000   //通道是否有设备存在
struct dma_chan
{
	const int8_t *name;  //用此通道的名字
	size_t dev;  //指向struct device设备的指针，指明操作此通道的设备
	uint32_t flags;  //通道标志
	uint32_t active_num;  //通道空闲时，此值为0，当通道正在运行时，此值包含需要执行的指令的个数
	uint32_t pending_num;  //active链表尾部等待执行的命令个数
	struct list_head active;  //正在执行的指令的链表
	struct list_head done;  //已经完成指令的链表或调用dma_disable()忽略的指令
};


/*******************************************************************************/
/************************************ 外部接口 ***********************************/
/*******************************************************************************/

/********************************************************************************
* 函数: static inline uint32_t dma_cmd_address(__in struct dma_desc *desc)
* 描述: 获取dma命令的地址
* 输入: dma_desc: dma描述器地址
* 输出: none
* 返回: dma命令的地址
* 作者:
* 版本: V1.0
**********************************************************************************/
static inline uint32_t dma_cmd_address(__in struct dma_desc *desc)
{
	return desc->address += offsetof(struct dma_desc, cmd);
}

/********************************************************************************
* 函数: static uint32_t dma_desc_is_pending(__in struct dma_desc *pdesc)
* 描述: 检测指定的描述器是否准备完成，等待执行
* 输入: pdesc: 需要检测的描述器
* 输出: none
* 返回: 0: 没有准备完成
       1: 准备完成
* 作者:
* 版本: V1.0
**********************************************************************************/
static inline uint32_t dma_desc_is_pending(__in struct dma_desc *pdesc)
{
	return pdesc->flags & DMA_DESC_READY;
}


int32_t dma_request(__in uint32_t channel);
void dma_release(__in uint32_t channel);
int32_t dma_enable(__in uint32_t channel);
void dma_disable(__in uint32_t channel);
int32_t dma_get_info(__in uint32_t channel, __out struct dma_info *info);
int32_t dma_cooked(__in int32_t channel, __in struct list_head *head);
void dma_reset(__in uint32_t channel);
void dma_freeze(__in uint32_t channel);
void dma_unfreeze(__in uint32_t channel);
int32_t dma_read_semaphore(__in uint32_t channel);
void dma_enable_irq(__in uint32_t channel, __in bool enable);
int32_t dma_irq_is_pending(__in int32_t channel);
void dma_ack_irq(__in uint32_t channel);
struct dma_desc *dma_alloc_desc(void);
void dma_free_desc(__in struct dma_desc *pdesc);
int32_t dma_desc_append(__in uint32_t channel, __in struct dma_desc *pdesc);
int32_t dma_desc_add_list(__in uint32_t channel, __in struct list_head *head);
int32_t dma_get_cooked(__in int32_t channel, __out struct list_head *head);
int32_t dma_init(__in enum dma_channel channel);
int32_t dma_wait_complete(__in uint32_t uSecTimeout, __in uint32_t chan);
int32_t dma_go(__in int32_t chan);



#endif


