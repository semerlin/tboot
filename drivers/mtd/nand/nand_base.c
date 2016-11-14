#include "stddef.h"
#include "io.h"
#include "malloc.h"
#include "log.h"
#include "errno.h"
#include "math.h"
#include "ecc.h"
#include "config.h"
#include "mtd/nand/nand.h"
#include "mtd/nand/nand_device_info.h"
#include "cpu_endian.h"
#include "bitops.h"

/* nandflash复位默认延时 */
#ifndef CONFIG_SYS_NAND_RESET_CNT
  #define CONFIG_SYS_NAND_RESET_CNT  200000
#endif


/* 外部函数 */
extern int32_t nand_default_bbt(__in struct mtd_info *mtd);


/* ecc在oob区中的布局 */

/* 一页256字节 */
static struct nand_ecclayout nand_oob_8 =
{
	.eccbytes = 3,
	.eccpos = {0, 1, 2},
	.oobfree =
	{
		{
		    .offset = 3,
            .length = 2
        },

		{
		    .offset = 6,
            .length = 2
        }
    }
};

/* 一页512字节 */
static struct nand_ecclayout nand_oob_16 =
{
	.eccbytes = 6,
	.eccpos = {0, 1, 2, 3, 6, 7},
	.oobfree =
	{
		{
		    .offset = 8,
            .length = 8
        }
    }
};

/* 一页2k字节 */
static struct nand_ecclayout nand_oob_64 =
{
	.eccbytes = 24,
	.eccpos =
	{
        40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63
    },

	.oobfree =
	{
		{
		    .offset = 2,
            .length = 38
        }
    }
};

/* 一页4k字节 */
static struct nand_ecclayout nand_oob_128 = {
	.eccbytes = 48,
	.eccpos =
	{
        80,  81,  82,  83,  84,  85,  86,  87,
		88,  89,  90,  91,  92,  93,  94,  95,
        96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127
    },

	.oobfree =
	{
		{
		    .offset = 2,
            .length = 78
        }
    }
};





/********************************************************************************
* 函数: static int32_t nand_get_device(__in struct mtd_info *mtd,
                                      __in int32_t new_state)
* 描述: 取得nandflash设备, 设置nandflash设备的状态
* 输入: mtd: nandflash设备父类
       new_state: nandflash设备新状态
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_get_device(__in struct mtd_info *mtd, __in int32_t new_state)
{
    struct nand_chip *this = mtd->priv;
    this->state = new_state;
    return 0;
}


/********************************************************************************
* 函数: static void nand_release_device(__in struct mtd_info *mtd)
* 描述: 从mtd设备中释放指定nandflash设备
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_release_device(__in struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;
    this->select_chip(mtd, -1);
}


/********************************************************************************
* 函数: static void nand_select_chip(__in struct mtd_info *mtd,
                                    __in int32_t chipnr)
* 描述: 选取指定的nandflash设备
* 输入: mtd: nandflash设备存在的mtd节点
       chipnr: 芯片号，-1为不选择芯片
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_select_chip(__in struct mtd_info *mtd, __in int32_t chipnr)
{
    struct nand_chip *this = mtd->priv;

    switch(chipnr)
    {
    case -1:
        this->cmd_ctrl(mtd, NAND_CMD_NONE, 0 | NAND_CTRL_CHANGE);
        break;
    case 0:
        break;
    default:
        //BUG();
        break;
    }
}

/*-----------------------------------------------------------------------------------------
-----------------------------------读写一个或多个字节-----------------------------------------
------------------------------------------------------------------------------------------*/


/********************************************************************************
* 函数: static uint8_t nand_read_byte(__in struct mtd_info *mtd)
* 描述: 从nandflash读取一个字节
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: 读取到的一个字节的数据
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint8_t nand_read_byte(__in struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;
    return readb(this->IO_ADDR_R);
}


/********************************************************************************
* 函数: static uint8_t nand_read_byte16(__in struct mtd_info *mtd)
* 描述: 按小端模式读取低字节
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: 读取到的一个字节的数据
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint8_t nand_read_byte16(__in struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;
    return (uint8_t)cpu_to_le16(readw(this->IO_ADDR_R));
}


/********************************************************************************
* 函数: static uint16_t nand_read_word(__in struct mtd_info *mtd)
* 描述: 从nandflash读取一个字
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: 读取到的一个字的数据
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint16_t nand_read_word(__in struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;
    return readw(this->IO_ADDR_R);
}


/********************************************************************************
* 函数: static void nand_write_buf(__in struct mtd_info *mtd,
                                  __in const uint8_t *buf,
                                  __in int32_t len)
* 描述: 写缓冲区数据进入nandflash设备(8位数据线)
* 输入: mtd: nandflash设备父类
       buf: 需要写入的数据缓冲区
       len: 数据长度
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_write_buf(__in struct mtd_info *mtd, __in const uint8_t *buf,
                            __in int32_t len)
{
    int32_t i;
    struct nand_chip *this = mtd->priv;

    for(i = 0; i < len; i++)
        writeb(buf[i], this->IO_ADDR_W);
}

/********************************************************************************
* 函数: static void nand_read_buf(__in struct mtd_info *mtd, __out uint8_t *buf,
                                 __in int32_t len)
* 描述: 从nandflash读输入到缓冲区(8位数据线)
* 输入: mtd: nandflash设备父类
       len: 需要读取的数据的长度
* 输出: buf: 数据存入的缓冲区
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_read_buf(__in struct mtd_info *mtd, __out uint8_t *buf, __in int32_t len)
{
    int32_t i;
    struct nand_chip *this = mtd->priv;

    for(i = 0; i < len; i++)
        buf[i] = readb(this->IO_ADDR_R);
}

/********************************************************************************
* 函数: static int32_t nand_verify_buf(__in struct mtd_info *mtd,
                                      __in const uint8_t *buf,
                                      __in int32_t len)
* 描述: 检验缓冲区中数据和nandflash中数据是否一致(8位数据线)
* 输入: mtd: nandflsh设备父类
       buf: 需要校验对比的缓冲区
       len: 校验对比的长度
* 输出: none
* 返回: 0: 数据完全一致
       -EFAULT: 数据存在不同
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_verify_buf(__in struct mtd_info *mtd, __in const uint8_t *buf,
                                 __in int32_t len)
{
    int32_t i;
    struct nand_chip *this = mtd->priv;

    for(i = 0; i < len; i++)
    {
        if(buf[i] != readb(this->IO_ADDR_R))
            return -EFAULT;
    }

    return 0;
}

/********************************************************************************
* 函数: static nand_write_buf16(__in struct mtd_info *mtd,
                               __in const uint8_t *buf,
                               __in int32_t len)
* 描述: 写缓冲区数据到nandflash设备(16位数据线)
* 输入: mtd: nandflash设备父类
       buf: 需要写入的缓冲区数据
       len: 需要写入的长度
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_write_buf16(__in struct mtd_info *mtd, __in const uint8_t *buf,
                              __in int32_t len)
{
    int32_t i;
    struct nand_chip *this = mtd->priv;
    uint16_t *pbuf = (uint16_t *)buf;

    for(i = 0; i < len; i++)
        writew(pbuf[i], this->IO_ADDR_W);
}

/********************************************************************************
* 函数: static nand_read_buf16(__in struct mtd_info *mtd, __out uint8_t *buf,
                              __in int32_t len)
* 描述: 从nandflash中读取指定长度的数据存入缓冲区中(16位数据线)
* 输入: mts: nandflash设备父类
       len: 需要读取的长度
* 输出: buf: 读出数据存储缓冲区
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_read_buf16(__in struct mtd_info *mtd, __out uint8_t *buf, __in int32_t len)
{
    int32_t i;
    struct nand_chip *this = mtd->priv;

    uint16_t *pbuf = (uint16_t *)buf;

    for(i = 0; i < len; i++)
        pbuf[i] = readw(this->IO_ADDR_R);
}

/********************************************************************************
* 函数: static int32_t nand_verify_buf16(__in struct mtd_info *mtd,
                                        __in const uint8_t *buf,
                                        __in int32_t len)
* 描述: 检验缓冲区中数据和nandflash中数据是否一致(16位数据线)
* 输入: mtd: nandflash设备父类
       buf: 需要校验对比的缓冲区
       len: 校验对比的长度
* 输出: none
* 返回: 0: 数据完全一致
       -EFAULT: 数据存在不同
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_verify_buf16(__in struct mtd_info *mtd, __in const uint8_t *buf, __in int32_t len)
{
    int32_t i;
    struct nand_chip *this = mtd->priv;

    uint16_t *pbuf = (uint16_t *)buf;

    for(i = 0; i < len; i++)
        if(pbuf[i] != readw(this->IO_ADDR_R))
            return -EFAULT;

    return 0;
}

/********************************************************************************
* 函数: static int32_t nand_block_checkbad(__in struct mtd_info *mtd,
                                          __in loff_t ofs,
                                          __in int32_t getchip,
                                          __in int32_t allowbbt)
* 描述: 检测块是否为坏块，读bbt或者直接读取块的原始数据
* 输入: this: nandflash设备自身指针
       ofs: 块的偏移地址
       getchip: 0: 芯片已经选中
                1: 需要选中芯片
       allowbbt: 0: 不允许访问bbt
                 1: 允许访问bbt
* 输出: none
* 返回: 0: 好块
       1: 坏块
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_block_checkbad(__in struct mtd_info *mtd, __in loff_t ofs,
                                     __in int32_t getchip, __in int32_t allowbbt)
{
    struct nand_chip *this = mtd->priv;

    /* 检测bbt是否建立和读取 */
    if(!(this->options & NAND_BBT_SCANNED))
    {
        /* bbt没有读取 */
        this->options |= NAND_BBT_SCANNED;
        /* 扫描读取bbt */
        this->scan_bbt(mtd);
    }


    /* 没有bbt */
    if(!this->bbt)
        return this->block_bad(mtd, ofs, getchip);

    return nand_isbad_bbt(mtd, ofs, allowbbt);
}

/*------------------------------------------------------------------------------------------
-----------------------------------读写一个或多个字节(end)-------------------------------------
------------------------------------------------------------------------------------------*/



/********************************************************************************
* 函数: static int32_t nand_check_wp(__in struct mtd_info *mtd)
* 描述: 检测nandflash是否处于写保护状态
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: 0: 不是写保护
       1: 写保护
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_check_wp(__in struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;

    /* 读取芯片状态 */
    this->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);

    return (this->read_byte(mtd) & NAND_STATUS_WP) ? 0 : 1;
}

/********************************************************************************
* 函数: static void single_erase_cmd(__in struct mtd_info *mtd,
                                    __in int32_t page)
* 描述: nandflash芯片擦除命令
* 输入: mtd: nandflash设备父类
       page: 擦除的起始页，擦除此页所在的块
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void single_erase_cmd(__in struct mtd_info *mtd, __in int32_t page)
{
    struct nand_chip *this = mtd->priv;
    this->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page);
    this->cmdfunc(mtd, NAND_CMD_ERASE2, -1, -1);
}


/********************************************************************************
* 函数: static int32_t nand_erase(__in struct mtd_info *mtd,
                                 __in struct erase_info *instr)
* 描述: 擦除nandflash芯片一个或多个块
* 输入: mtd: nandflash设备父类
       instr: 擦除的信息
* 输出: none
* 返回: 0: 擦除成功
       -EINVAL: 参数无效
       -EIO: 出现IO错误，1、擦除区域中有坏块，2、擦除出现错误
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_erase(__in struct mtd_info *mtd, __in struct erase_info *instr)
{
    return nand_erase_nand(mtd, instr, 0);
}

/********************************************************************************
* 函数: int32_t nand_erase_nand(__in struct mtd_info *mtd,
                               __in struct erase_info *instr,
                               __in int32_t allowbbt)
* 描述: 擦除nandflash芯片一个或多个块
* 输入: this: nandflash设备自身指针
       instr: 擦除的信息
       allowbbt: 0: 不允许擦除bbt区域
                 1: 允许擦除bbt区域
* 输出: none
* 返回: 0: 擦除成功
       -EINVAL: 参数无效
       -EIO: 出现IO错误，1、擦除区域中有坏块，2、擦除出现错误
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t nand_erase_nand(__in struct mtd_info *mtd, __in struct erase_info *instr,
                         __in int32_t allowbbt)
{
    struct nand_chip *this = mtd->priv;
    int32_t page, chipnr, pages_per_block, status, ret;
    uint64_t len;

    printl(LOG_LEVEL_INFO, "[NAND:INFO] nand_erase_nand: start = 0x%012llx, len = %llu\n",
           (uint64_t)instr->addr, (uint64_t)instr->len);


    /* 块边界对齐*/
    if(instr->addr & ((1 << this->phys_erase_shift) - 1))
    {
        printl(LOG_LEVEL_ERR, "[NAND:ERR] nand_erase_nand: unaligned address\n");
        return -EINVAL;
    }

    if(instr->len & ((1 << this->phys_erase_shift) - 1))
    {
        printl(LOG_LEVEL_ERR, "[NAND:ERR] nand_erase_nand: length not block aligned\n");
        return -EINVAL;
    }

    /* 越界检查 */
    if((instr->addr + instr->len) > mtd->size)
    {
        printl(LOG_LEVEL_ERR, "[NAND:ERR] nand_erase_nand: erase past end of device\n");
        return -EINVAL;
    }

    instr->fail_addr = 0xffffffff;

    /* 获取设备 */
    nand_get_device(mtd, FL_ERASING);

    page = (int32_t)(instr->addr >> this->page_shift);
    chipnr = (int32_t)(instr->addr >> this->chip_shift);

    pages_per_block = 1 << (this->phys_erase_shift - this->page_shift);

    /* 选择芯片 */
    this->select_chip(mtd, chipnr);

    if(nand_check_wp(mtd))
    {
        printl(LOG_LEVEL_WARN, "[NAND:WARN] nand_erase_nand: device is write protected!\n");
        instr->state = MTD_ERASE_FAILED;
        goto erase_exit;
    }

    len = instr->len;

    instr->state = MTD_ERASING;

    /* 开始擦除 */
    while(len)
    {
        if(nand_block_checkbad(mtd, ((loff_t)page) << this->page_shift, 0, allowbbt))
        {
            printl(LOG_LEVEL_WARN, "[NAND:WARN] nand_erase_nand: attempt to erase a bad block at page 0x%08x\n",
                                    page);

            instr->state = MTD_ERASE_FAILED;

            goto erase_exit;
        }

        /* 检测当前擦除的块是否包含cache的页，包含就使cache的页失效 */
        if((page <= this->pagebuf) && ((page + pages_per_block) > this->pagebuf))
            this->pagebuf = -1;

        this->erase_cmd(mtd, page & this->page_mask);

        status = this->waitfunc(mtd);

        if((status & NAND_STATUS_FAIL) && (this->errstat))
            status = this->errstat(mtd, FL_ERASING, status, page);

        if(status & NAND_STATUS_FAIL)
        {
            printl(LOG_LEVEL_ERR, "[NAND:ERR] nand_erase_nand: failed erase, page 0x%08x\n", page);
            instr->state = MTD_ERASE_FAILED;
            instr->fail_addr = ((loff_t)page << this->page_shift);
            goto erase_exit;
        }

        len -= (1 << this->phys_erase_shift);
        page += pages_per_block;

        /* 擦除区域跨芯片 */
        if(len && !(page & this->page_mask))
        {
            chipnr++;
            this->select_chip(mtd, -1);
            this->select_chip(mtd, chipnr);
        }
    }

    instr->state = MTD_ERASE_DONE;

erase_exit:

    ret = ((instr->state == MTD_ERASE_DONE) ? 0 : -EIO);

    /* 释放设备 */
    nand_release_device(mtd);

    /* 未出现错误，调用回调函数 */
    if(!ret)
        mtd_erase_callback(instr);

    return ret;
}



/********************************************************************************
* 函数: void nand_wait_ready(__in struct mtd_info *mtd)
* 描述: 等待nandflash执行命令结束, 等待R/B引脚
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void nand_wait_ready(__in struct mtd_info *mtd)
{
    uint32_t timeo = (CONFIG_SYS_HZ * 20) / 1000;   //超时时间20ms
    struct nand_chip *this = mtd->priv;
    reset_timer();

    /* 等待命令处理完毕或者超时(20ms) */
    while(get_timer(0) < timeo)
    {
        if(this->dev_ready)
            if(this->dev_ready(mtd))
                break;
    }

    if(get_timer(0) > timeo)
        printl(LOG_LEVEL_WARN, "[NAND:WARN] nand wait ready timeout.\n");
}

/********************************************************************************
* 函数: void nand_wait_ready(__in struct nand_chip *this)
* 描述: 等待nandflash执行命令结束, 仅使用在擦除和写之后的等待，此函数不会超时返回
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: nandflash芯片status寄存器的值
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_wait(__in struct mtd_info *mtd)
{
    uint64_t timeo;
    struct nand_chip *this = mtd->priv;
    int32_t state = this->state;

    if(state == FL_ERASING)
        timeo = (CONFIG_SYS_HZ * 400) / 1000;
    else
        timeo = (CONFIG_SYS_HZ * 20) / 1000;

    if(state == FL_ERASING)
        this->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);

	reset_timer();

	while(get_timer(0) < timeo)
	{
		if (this->dev_ready)
		{
			if(this->dev_ready(mtd))
				break;
		}
		else
		{
			if(this->read_byte(mtd) & NAND_STATUS_READY)
				break;
		}
	}

    if(get_timer(0) > timeo)
        printl(LOG_LEVEL_WARN, "[NAND:WARN] nand wait timeout.\n");


	return this->read_byte(mtd);
}


/********************************************************************************
* 函数: static void nand_command(__in struct mtd_info *mtd,
                                __in uint32_t command,
                                __in int32_t column, __in int32_t page_addr)
* 描述: 写指令到nandflash设备(小页设备，512字节每页)
* 输入: mtd: nandflash设备父类
       command: 具体指令
       column: 页内地址
       page_addr: 页地址
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_command(__in struct mtd_info *mtd, __in uint32_t command,
                          __in int32_t column, __in int32_t page_addr)
{
    int32_t ctrl = NAND_CTRL_CLE | NAND_CTRL_CHANGE;
    uint32_t rst_sts_cnt = CONFIG_SYS_NAND_RESET_CNT;
    struct nand_chip *this = mtd->priv;

    /* 写命令到设备 */
    if(command == NAND_CMD_SEQIN)
    {
        /* 小页设备分A，B，C区，写数据之前要先指定写哪个区 */

        /*
           512字节的每页的nandflash一页分为A，B,C三区
           每个区有不同的读指令，先发送读指令指定写哪个区
        */

        int32_t readcmd;

        if(column >= mtd->writesize)
        {
			/* OOB 区域, C区, 最后的16字节 */
			column -= mtd->writesize;
			readcmd = NAND_CMD_READOOB;
		}
		else if(column < 256)
		{
			/* A区，起始的256字节 */
			readcmd = NAND_CMD_READ0;
		}
		else
		{
		    /* B区，后256字节 */
			column -= 256;
			readcmd = NAND_CMD_READ1;
		}
		this->cmd_ctrl(mtd, readcmd, ctrl);
		ctrl &= ~NAND_CTRL_CHANGE;
    }
    this->cmd_ctrl(mtd, command, ctrl);

    /* 写地址到设备 */
    ctrl = NAND_CTRL_ALE | NAND_CTRL_CHANGE;
    /* 写列地址 */
    if(column != -1)
    {
        if(this->options & NAND_BUSWIDTH_16)
            column >>= 1;

        this->cmd_ctrl(mtd, column, ctrl);
        ctrl &= ~NAND_CTRL_CHANGE;
    }

    /* 写页地址 */
    if(page_addr != -1)
    {
        this->cmd_ctrl(mtd, page_addr, ctrl);
		ctrl &= ~NAND_CTRL_CHANGE;
		this->cmd_ctrl(mtd, page_addr >> 8, ctrl);
		/* 大于32MiB的设备多一个页地址字节 */
		if(this->chipsize > (32 << 20))
			this->cmd_ctrl(mtd, page_addr >> 16, ctrl);
    }
    this->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

    /* 指令等待 */
    switch(command)
    {
    /* 这些指令有自己的忙等待处理方式 */
	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_STATUS:
		return;

	case NAND_CMD_RESET:
		if (this->dev_ready)
			break;
		udelay(this->chip_delay);
		this->cmd_ctrl(mtd, NAND_CMD_STATUS, NAND_CTRL_CLE | NAND_CTRL_CHANGE);
		this->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		while (!(this->read_byte(mtd) & NAND_STATUS_READY) &&
			(rst_sts_cnt--));
		return;

	default:
		/*
		 * 这些用于读指令，如果我们不检测R/B引脚，就是用默认的指令延时
		 */
		if(!this->dev_ready)
		{
			udelay(this->chip_delay);
                return;
		}
	}

	/* 延时等待任何情况下的tWB，tWB最大时间默认为100ns */
	ndelay(100);

    /* 超时等待 */
	nand_wait_ready(mtd);
}

/********************************************************************************
* 函数: static void nand_command_lp(__in struct mtd_info *mtd,
                                __in uint32_t command,
                                __in int32_t column, __in int32_t page_addr)
* 描述: 写指令到nandflash设备(大页设备，2048字节每页或更大)
* 输入: mtd: nandflash设备父类
       command: 具体指令
       column: 页内地址
       page_addr: 页地址
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_command_lp(__in struct mtd_info *mtd, __in uint32_t command,
                             __in int32_t column, __in int32_t page_addr)
{
    int32_t ctrl = NAND_CTRL_CLE | NAND_CTRL_CHANGE;
    uint32_t rst_sts_cnt = CONFIG_SYS_NAND_RESET_CNT;
    struct nand_chip *this = mtd->priv;

    if(command == NAND_CMD_READOOB)
    {
        column += mtd->writesize;
        command = NAND_CMD_READ;
    }

    /* 写指令 */
    this->cmd_ctrl(mtd, command & 0xff, ctrl);

    /* 写地址 */
    ctrl = NAND_CTRL_ALE | NAND_CTRL_CHANGE;
    if(column != -1)
    {
        if (this->options & NAND_BUSWIDTH_16)
            column >>= 1;
        this->cmd_ctrl(mtd, column, ctrl);
        ctrl &= ~NAND_CTRL_CHANGE;
        this->cmd_ctrl(mtd, column >> 8, ctrl);
    }


    if (page_addr != -1)
    {
        this->cmd_ctrl(mtd, page_addr, ctrl);
        this->cmd_ctrl(mtd, page_addr >> 8, ctrl);
        /* 大于128MiB的设备多一个页地址字节 */
        if (this->chipsize > (128 << 20))
            this->cmd_ctrl(mtd, page_addr >> 16, ctrl);
    }

    this->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

    /* 指令等待 */
	switch(command)
	{
    /* 这些指令有自己的忙等待方式,可直接返回之后等待发送数据等 */
	case NAND_CMD_CACHEDPROG:
	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_RNDIN:
	case NAND_CMD_STATUS:
		return ;

    /* 芯片复位 */
	case NAND_CMD_RESET:
		if(this->dev_ready) /* 有自身等待完成函数 */
			break;

        /* 这一部分实现了自身等待完成函数的功能 */
		udelay(this->chip_delay);
		this->cmd_ctrl(mtd, NAND_CMD_STATUS,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		this->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);
		while (!(this->read_byte(mtd) & NAND_STATUS_READY) &&
			(rst_sts_cnt--));
		return;

    /* 随机读取数据，开启随机读取第二步，之后返回等待读数据 */
	case NAND_CMD_RNDOUT:
        /* 不需要等待芯片空闲 */
		this->cmd_ctrl(mtd, NAND_CMD_RNDOUTSTART,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		this->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);
		return;

    /* 读取数据，开启读取第二步，之后返回等待读取数据 */
	case NAND_CMD_READ:
		this->cmd_ctrl(mtd, NAND_CMD_READSTART,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		this->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);

	default:
		/* 读指令等待延时 */
		if(!this->dev_ready)
		{
			udelay(this->chip_delay);
                return;
		}
	}

	/* 延时等待任何情况下的tWB，tWB最大时间默认为100ns */
	ndelay(100);

    /* 等待指令执行完成 */
	nand_wait_ready(mtd);
}


/********************************************************************************
* 函数: static int32_t nand_read_page_raw(__in struct mtd_info *mtd,
                                         __out uint8_t *buf)
* 描述: 直接读一页的原始数据，不经过ecc计算，oob区会全部读取
* 输入: mtd: nandflash设备父类
* 输出: buf: 原始数据输出缓冲区，不包含oob区
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_read_page_raw(__in struct mtd_info *mtd, __out uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    this->read_buf(mtd, buf, mtd->writesize);
    this->read_buf(mtd, this->oob_poi, mtd->oobsize);

    return 0;
}


/********************************************************************************
* 函数: static int32_t nand_read_page_swecc(__in struct mtd_info *mtd,
                                           __out uint8_t *buf)
* 描述: 读取一页页数据，通过软件ecc校准, oob区会全部读取
* 输入: mtd: nandflash设备父类
* 输出: buf: 经过ecc校准检验的页数据, 不包含oob区
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_read_page_swecc(__in struct mtd_info *mtd, __out uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    int32_t i, stat, ecc_size = this->ecc_ctrl.data_size_per_step;
	int32_t ecc_bytes = this->ecc_ctrl.ecc_bytes_per_step;
	int32_t ecc_steps = this->ecc_ctrl.ecc_steps_per_page;
	uint8_t *p = buf;
	uint8_t *ecc_calc = this->ecc_ctrl.ecc_calcbuf;
	uint8_t *ecc_code = this->ecc_ctrl.ecc_codebuf;
	uint32_t *eccpos = this->ecc_ctrl.layout->eccpos;


	this->ecc_ctrl.read_page_raw(mtd, buf);

	for (i = 0; ecc_steps > 0; ecc_steps--)
	{
	    this->ecc_ctrl.calculate(p, &ecc_calc[i]);
	    i += ecc_bytes;
	    p += ecc_size;
	}


	for (i = 0; i < this->ecc_ctrl.ecc_total_bytes_per_page; i++)
		ecc_code[i] = this->oob_poi[eccpos[i]];

	ecc_steps = this->ecc_ctrl.ecc_steps_per_page;
	p = buf;

	for (i = 0; ecc_steps > 0; ecc_steps--)
	{
		stat = this->ecc_ctrl.correct(p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
            mtd->ecc_stats.failed ++;
		else
            mtd->ecc_stats.corrected += stat;

        i += ecc_bytes;
        p += ecc_size;
	}


	return 0;
}


/********************************************************************************
* 函数: static int32_t nand_read_subpage(__in struct mtd_info *mtd,
                                        __in uint32_t offs,
                                        __in uint32_t len,
                                        __out uint8_t *buf)
* 描述: 读取经过ecc校验的一段数据，地址可能不是页对齐, oob区不一定全部读取
* 输入: mtd: nandflash设备父类
       offs: 需要读取数据的地址在一页中的偏移地址
       len: 需要读取的长度
* 输出: buf: 经过ecc校准的数据输出缓冲区, 数据出现的地址页偏移offs，不包含oob区
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_read_subpage(__in struct mtd_info *mtd, __in uint32_t offs,
                                   __in uint32_t len, __out uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    int32_t start_step, end_step, num_steps;
	uint32_t *eccpos = this->ecc_ctrl.layout->eccpos;
	uint8_t *p;
	int32_t data_col_addr, i, gaps = 0;
	int32_t datafrag_len, eccfrag_len, aligned_len, aligned_pos;
	int32_t busw = (this->options & NAND_BUSWIDTH_16) ? 2 : 1;
	int32_t stat;

	/* 计算ecc处于的ecc校验块的位置 */
	start_step = offs / this->ecc_ctrl.data_size_per_step;
	end_step = (offs + len - 1) / this->ecc_ctrl.data_size_per_step;
	num_steps = end_step - start_step + 1;

	/* ecc块对齐的数据长度 */
	datafrag_len = num_steps * this->ecc_ctrl.data_size_per_step;
	eccfrag_len = num_steps * this->ecc_ctrl.ecc_bytes_per_step;

	data_col_addr = start_step * this->ecc_ctrl.data_size_per_step;

	/* 不是页对齐的地址 */
	if (data_col_addr != 0)
		this->cmdfunc(mtd, NAND_CMD_RNDOUT, data_col_addr, -1);

	p = buf + data_col_addr;
	/* 读取数据段    */
	this->read_buf(mtd, p, datafrag_len);

	/* 计算ecc */
	for (i = 0; i < eccfrag_len ; i += this->ecc_ctrl.ecc_bytes_per_step)
	{
	    this->ecc_ctrl.calculate(p, &this->ecc_ctrl.ecc_calcbuf[i]);
	    p += this->ecc_ctrl.data_size_per_step;
	}


	/* 检测ecc存储字节数否连续 */
	for (i = 0; i < eccfrag_len - 1; i++)
	{
		if (eccpos[i + start_step * this->ecc_ctrl.ecc_bytes_per_step] + 1 !=
			eccpos[i + start_step * this->ecc_ctrl.ecc_bytes_per_step + 1])
        {
			gaps = 1;
			break;
		}
	}

	if(gaps)
	{
	    /* 存储字节不连续, 读取整个oob区域 */
		this->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize, -1);
		this->read_buf(mtd, this->oob_poi, mtd->oobsize);
	}
	else
	{
		/* 存储字节连续, 读取特定的字节 */
		aligned_pos = eccpos[start_step * this->ecc_ctrl.ecc_bytes_per_step] & ~(busw - 1);
		aligned_len = eccfrag_len;
		if (eccpos[start_step * this->ecc_ctrl.ecc_bytes_per_step] & (busw - 1))
			aligned_len++;
		if (eccpos[(start_step + num_steps) * this->ecc_ctrl.ecc_bytes_per_step] & (busw - 1))
			aligned_len++;

		this->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize + aligned_pos, -1);
		this->read_buf(mtd, &this->oob_poi[aligned_pos], aligned_len);
	}

	for (i = 0; i < eccfrag_len; i++)
		this->ecc_ctrl.ecc_codebuf[i] = this->oob_poi[eccpos[i + start_step * this->ecc_ctrl.ecc_bytes_per_step]];

	p = buf + data_col_addr;
	for(i = 0; i < eccfrag_len; i += this->ecc_ctrl.ecc_bytes_per_step)
    {
        stat = this->ecc_ctrl.correct(p, &this->ecc_ctrl.ecc_codebuf[i], &this->ecc_ctrl.ecc_calcbuf[i]);
		if(stat < 0)
            mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;

        p += this->ecc_ctrl.data_size_per_step;
	}
	return 0;
}

/********************************************************************************
* 函数: static int32_t nand_read_page_hwecc(__in struct mtd_info *mtd,
                                           __in uint8_t *buf)
* 描述: 读取nandflash一页数据，硬件ecc校准
* 输入: mtd: nandflash设备父类
* 输出: buf: 经过硬件ecc校准输出的数据缓冲区
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_read_page_hwecc(__in struct mtd_info *mtd, __out uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    int32_t i, stat;
    int32_t eccsize = this->ecc_ctrl.data_size_per_step;
	int32_t eccbytes = this->ecc_ctrl.ecc_bytes_per_step;
	int32_t eccsteps = this->ecc_ctrl.ecc_steps_per_page;
	uint8_t *p = buf;
	uint8_t *ecc_calc = this->ecc_ctrl.ecc_calcbuf;
	uint8_t *ecc_code = this->ecc_ctrl.ecc_codebuf;
	uint32_t *eccpos = this->ecc_ctrl.layout->eccpos;

	for(i = 0; eccsteps > 0; eccsteps--)
	{
		this->ecc_ctrl.hwctl(mtd, NAND_ECC_READ);
		this->read_buf(mtd, p, eccsize);
		this->ecc_ctrl.calculate(p, &ecc_calc[i]);
		i += eccbytes;
		p += eccsize;
	}
	this->read_buf(mtd, this->oob_poi, mtd->oobsize);

	for(i = 0; i < this->ecc_ctrl.ecc_total_bytes_per_page; i++)
		ecc_code[i] = this->oob_poi[eccpos[i]];

	eccsteps = this->ecc_ctrl.ecc_steps_per_page;
	p = buf;

	for(i = 0; eccsteps; eccsteps--)
	{
		stat = this->ecc_ctrl.correct(p, &ecc_code[i], &ecc_calc[i]);
		if(stat < 0)
            mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;

        i += eccbytes;
        p += eccsize;
	}
	return 0;
}





/********************************************************************************
* 函数: static int32_t nand_read(__in struct mtd_info *mtd, __in loff_t from,
                                __in size_t len, __out size_t *retlen,
                                __out uint8_t *buf)
* 描述: 读取nandflash经过ecc校验的数据
* 输入: mtd: nandflash设备父类
       from: 读取的起始地址
       len: 需要读取的长度
* 输出: retlen: 成功读取的长度
       buf: 经过ecc校验的数据
* 返回: 0: 成功且数据没有错误产生
       -EINVAL: 输入参数无效
       -EBADMSG: 出现坏块
       -EUCLEAN: 数据读取成功，错误位被纠正
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_read(__in struct mtd_info *mtd, __in loff_t from, __in size_t len,
                          __out size_t *retlen, __out uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    int32_t ret = 0;
    int32_t chipnr, page, realpage, colume, bytes, aligned;
    uint32_t readlen = len;
    uint8_t *bufpoi = buf;
    int32_t sndcmd = 1;
    int32_t blkcheck = (1 << (this->phys_erase_shift - this->page_shift)) - 1;
    struct mtd_ecc_stats stats = mtd->ecc_stats;


    if((from + len) > mtd->size)
        return -EINVAL;
    if(!len)
    {
        *retlen = 0;
        return 0;
    }

    /* 获取设备 */
    nand_get_device(mtd, FL_READING);

    /* 计算读取的页和芯片 */
    chipnr = (int32_t)(from >> this->chip_shift);
    this->select_chip(mtd, chipnr);

    realpage = (int32_t)(from >> this->page_shift);
    page = realpage & this->page_mask;

    /* 页内列地址 */
    colume = (int32_t)(from & (mtd->writesize - 1));

    while(1)
    {
        bytes = min_t(uint32_t, mtd->writesize - colume, readlen);
        /* 读取地址是否页对齐 */
        aligned = (bytes == mtd->writesize);

        if(realpage != this->pagebuf)
        {
            if(likely(sndcmd))
            {
                this->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page);
                sndcmd = 0;
            }

            if(aligned)
                ret = this->ecc_ctrl.read_page(mtd, bufpoi);
            else
            {
                if(this->ecc_ctrl.mode == NAND_ECC_SOFT)
                    ret = this->ecc_ctrl.read_subpage(mtd, colume, bytes, this->page_databuf);
                else
                    ret = this->ecc_ctrl.read_page(mtd, this->page_databuf);
            }

            /* 数据读取出现错误 */
            if(ret < 0)
                break;

            /* 拷贝不对齐数据 */
            if(!aligned)
            {
                this->pagebuf = realpage;
                memcpy(bufpoi, this->page_databuf + colume, bytes);
            }

            /* 检测连续读取页是否要等待，一般不需要等待 */
            if(!(this->options & NAND_NO_READRDY))
            {
                if(!this->dev_ready)
                    udelay(this->chip_delay);
                else
                    nand_wait_ready(mtd);
            }
        }
        else
        {
            memcpy(bufpoi, this->page_databuf + colume, bytes);
        }

        bufpoi += bytes;
        readlen -= bytes;

        if(!readlen)
            break;

        colume = 0;

        realpage++;
        page = realpage & this->page_mask;

        /* 读取地址跨芯片 */
        if(!page)
        {
            chipnr++;
            this->select_chip(mtd, -1);
            this->select_chip(mtd, chipnr);
        }

        /* 检测芯片是否支持页自动增加或者跨了一个块 */
        if((this->options & NAND_NO_AUTOINCR) || !(page & blkcheck))
            sndcmd = 1;
    }

    /* 计算读取的长度和返回值 */
    *retlen = len - readlen;

    /* 即测读取的数据是否有错误产生 */
	if(mtd->ecc_stats.failed - stats.failed)
		ret = -EBADMSG;
    else if(mtd->ecc_stats.corrected - stats.corrected)
        ret = -EUCLEAN;
    else
        ret  = 0;

    /* 释放设备 */
    nand_release_device(mtd);

    return ret;
}



/********************************************************************************
* 函数: static uint8_t *nand_transfer_oob(__in struct nand_chip *this,
                                         __in uint8_t *oob,
                                         __in struct mtd_oob_ops *ops,
                                         __in size_t len)
* 描述: 传送oob段数据区数据到指定位置缓冲区
* 输入: this: nandflash设备自身指针
       ops: oob区数据布局
       len: 读取的长度
* 输出: oob: oob段数据区数据输出缓冲区
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint8_t *nand_transfer_oob(__in struct nand_chip *this, __out uint8_t *oob,
                                    __in struct mtd_oob_ops *ops, __in size_t len)
{
    switch(ops->mode)
    {
    case MTD_OOB_PLACE:
    case MTD_OOB_RAW:
        memcpy(oob, this->oob_poi + ops->ooboffs, len);
        return oob + len;
    case MTD_OOB_AUTO:
        {
            struct nand_oobfree *free = this->ecc_ctrl.layout->oobfree;
            uint32_t boffs = 0;
            size_t bytes = 0;

            for(; free->length && len; free++)
            {
                bytes = min_t(size_t, len, free->length);
                boffs = free->offset;

                memcpy(oob, this->oob_poi + boffs, bytes);
                oob += bytes;
                len -= bytes;
            }

            return oob;
        }
    default:
        break;
    }

    printf(LOG_LEVEL_ERR, "[NAND:ERR] invalid oob mode(%d)\n", ops->mode);
    return NULL;
}


/********************************************************************************
* 函数: static int32_t nand_read_oob_std(__in struct mtd_info *mtd,
                                        __in int32_t page,
                                        __in int32_t sndcmd)
* 描述: 读取oob区全部数据
* 输入: mtd: nandflash设备父类
       page: 读取的页
       sndcmd: 1:需要先发送读取命令之后再读取数据 0:可以直接读取数据
* 输出: none
* 返回: 0: 没有先发送命令
       1: 先发送了读取命令
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_read_oob_std(__in struct mtd_info *mtd, __in int32_t page,
                                   __in int32_t sndcmd)
{
    struct nand_chip *this = mtd->priv;
    if(sndcmd)
    {
        this->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
        sndcmd = 0;
    }

    this->read_buf(mtd, this->oob_poi, mtd->oobsize);

    return sndcmd;
}



/********************************************************************************
* 函数: static int32_t nand_read_oob(__in struct mtd_info *mtd,
                                    __in loff_t from,
                                    __inout struct mtd_oob_ops *ops)
* 描述: 根据ops选项读取oob或者oob+data在一起的数据
* 输入: mtd: nandflash设备父类
       from: 数据起始地址
       ops: 数据读取参数
* 输出: ops: 读取完毕的数据和参数
* 返回: 0: 成功且数据没有错误产生
       -EINVAL: 输入参数无效
       -EBADMSG: 出现坏块
       -EUCLEAN: 数据读取成功，错误位被纠正
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_read_oob(__in struct mtd_info *mtd, __in loff_t from,
                               __in struct mtd_oob_ops *ops)
{
    struct nand_chip *this = mtd->priv;
    int32_t ret = 0;
    int32_t page, realpage, chipnr, aligned, sndcmd = 1;
	int32_t blkcheck = (1 << (this->phys_erase_shift - this->page_shift)) - 1;
	int32_t oobreadlen = ops->ooblen;
	uint8_t *oobbuf = ops->oobbuf;
    int32_t oob_per_pagelen;

    if(ops->databuf && ((from + ops->len) > mtd->size))
    {
        printl(LOG_LEVEL_ERR, "[NAND:ERR] attempt read beyond end of device.\n");
        return -EINVAL;
    }


    nand_get_device(mtd, FL_READING);

    switch(ops->mode)
    {
    case MTD_OOB_PLACE:
    case MTD_OOB_AUTO:
    case MTD_OOB_RAW:
        break;

    default:
        goto out;
    }

    chipnr = (int32_t)(from >> this->chip_shift);
    this->select_chip(mtd, chipnr);

    realpage = (int32_t)(from >> this->page_shift);
    page = realpage & this->page_mask;

    if(ops->mode == MTD_OOB_AUTO)
        oob_per_pagelen = this->ecc_ctrl.layout->oobavail;
    else
        oob_per_pagelen = mtd->oobsize;

    if(!ops->databuf)
    {
        /* 仅读取oob */
        while(1)
        {
            sndcmd = this->ecc_ctrl.read_oob(mtd, page, sndcmd);

            oob_per_pagelen = min_t(int32_t, oob_per_pagelen, oobreadlen);
            oobbuf = nand_transfer_oob(this, oobbuf, ops, oob_per_pagelen);

            if(!(this->options & NAND_NO_READRDY))
            {
                if(!this->dev_ready)
                    udelay(this->chip_delay);
                else
                    nand_wait_ready(mtd);
            }

            oobreadlen -= oob_per_pagelen;
            if(!oobreadlen)
                break;

            realpage ++;

            page = realpage & this->page_mask;
            if(!page)
            {
                chipnr++;
                this->select_chip(mtd, -1);
                this->select_chip(mtd, chipnr);
            }

            if((this->options & NAND_NO_AUTOINCR) || (page & blkcheck))
                sndcmd = 1;
        }


    }
    else
    {
        /* 读取oob+data*/
        int32_t readlen = ops->len;
        uint8_t *buf = ops->databuf;
        int32_t colume;
        int32_t bytes;
        struct mtd_ecc_stats stats = mtd->ecc_stats;


        /* 页内列地址 */
        colume = (int32_t)(from & (mtd->writesize - 1));

        while(1)
        {
            bytes = min_t(uint32_t, mtd->writesize - colume, readlen);
            /* 读取地址是否页对齐 */
            aligned = (bytes == mtd->writesize);

            if(realpage != this->pagebuf)
            {
                if(likely(sndcmd))
                {
                    this->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page);
                    sndcmd = 0;
                }

                if(aligned)
                    if(ops->mode == MTD_MODE_RAW)
                        this->ecc_ctrl.read_page_raw(mtd, buf);
                    else
                        this->ecc_ctrl.read_page(mtd, buf);
                else
                    if(ops->mode == MTD_MODE_RAW)
                        this->ecc_ctrl.read_page_raw(mtd, this->page_databuf);
                    else
                        this->ecc_ctrl.read_page(mtd, this->page_databuf);

                /* 拷贝不对齐数据 */
                if(!aligned)
                {
                    this->pagebuf = realpage;
                    memcpy(buf, this->page_databuf + colume, bytes);
                }

                /* 检测连续读取页是否要等待，一般不需要等待 */
                if(!(this->options & NAND_NO_READRDY))
                {
                    if(!this->dev_ready)
                        udelay(this->chip_delay);
                    else
                        nand_wait_ready(mtd);
                }
            }
            else
            {
                memcpy(buf, this->page_databuf + colume, bytes);
            }

            /* 拷贝oob段数据 */
            if(oobreadlen != 0)
            {
                oob_per_pagelen = min_t(int32_t, oob_per_pagelen, oobreadlen);
                oobbuf = nand_transfer_oob(this, oobbuf, ops, oob_per_pagelen);
                oobreadlen -= oob_per_pagelen;
            }


            buf += bytes;
            readlen -= bytes;

            if(!readlen)
                break;

            colume = 0;

            realpage++;
            page = realpage & this->page_mask;

            /* 读取地址跨芯片 */
            if(!page)
            {
                chipnr++;
                this->select_chip(mtd, -1);
                this->select_chip(mtd, chipnr);
            }

            /* 检测芯片是否支持页自动增加或者跨了一个块 */
            if((this->options & NAND_NO_AUTOINCR) || !(page & blkcheck))
                sndcmd = 1;
        }

        /* 计算读取的长度和返回值 */
        ops->retlen = ops->len - readlen;

        /* 即测读取的数据是否有错误产生 */
        if(mtd->ecc_stats.failed - stats.failed)
            ret = -EBADMSG;
        else if(mtd->ecc_stats.corrected - stats.corrected)
            ret = -EUCLEAN;
        else
            ret  = 0;
    }

    ops->oobretlen = ops->ooblen - oobreadlen;

out:
    nand_release_device(mtd);

    return ret;
}



/********************************************************************************
* 函数: static int32_t nand_write_page_raw(__in struct mtd_info *mtd,
                                          __in const uint8_t *buf)
* 描述: 直接写一页的原始数据，不经过ecc计算，oob区会全部写入
* 输入: mtd: nandflash设备父类
       buf: 原始数据写入缓冲区，不包含oob区
* 输出: none
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_write_page_raw(__in struct mtd_info *mtd, __in const uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    this->write_buf(mtd, buf, mtd->writesize);
    this->write_buf(mtd, this->oob_poi, mtd->oobsize);

    return 0;
}


/********************************************************************************
* 函数: static int32_t nand_write_page_swecc(__in struct mtd_info *mtd,
                                            __in const uint8_t *buf)
* 描述: 写一页数据，通过软件ecc校准, oob区会全部写入
* 输入: mtd: nandflash设备父类
       buf: 需要写入的原始数据缓冲区, 不包含oob区
* 输出: none
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_write_page_swecc(__in struct mtd_info *mtd, __in const uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    int32_t i, eccsize = this->ecc_ctrl.data_size_per_step;
	int32_t eccbytes = this->ecc_ctrl.ecc_bytes_per_step;
	int32_t eccsteps = this->ecc_ctrl.ecc_steps_per_page;
	uint8_t *ecc_calc = this->ecc_ctrl.ecc_calcbuf;
	const uint8_t *p = buf;
	uint32_t *eccpos = this->ecc_ctrl.layout->eccpos;

	/* 计算ecc校验值 */
	for (i = 0; eccsteps; eccsteps--)
	{
	    this->ecc_ctrl.calculate(p, &ecc_calc[i]);
	    i += eccbytes;
	    p += eccsize;
	}

    /* 插入ecc校验值 */
	for (i = 0; i < this->ecc_ctrl.ecc_total_bytes_per_page; i++)
		this->oob_poi[eccpos[i]] = ecc_calc[i];

	this->ecc_ctrl.write_page_raw(mtd, buf);

	return 0;
}



/********************************************************************************
* 函数: static int32_t nand_read_page_hwecc(__in struct mtd_info *mtd,
                                           __in const uint8_t *buf)
* 描述: 写nandflash一页数据，硬件ecc校准
* 输入: mtd: nandflash设备父类
       buf: 需要写入的数据缓冲区，不包含oob区
* 输出: none
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_write_page_hwecc(__in struct mtd_info *mtd, __in const uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    int i, eccsize = this->ecc_ctrl.data_size_per_step;
	int eccbytes = this->ecc_ctrl.ecc_bytes_per_step;
	int eccsteps = this->ecc_ctrl.ecc_steps_per_page;
	uint8_t *ecc_calc = this->ecc_ctrl.ecc_calcbuf;
	const uint8_t *p = buf;
	uint32_t *eccpos = this->ecc_ctrl.layout->eccpos;

	for (i = 0; eccsteps; eccsteps--)
	{
		this->ecc_ctrl.hwctl(mtd, NAND_ECC_WRITE);
		this->write_buf(mtd, p, eccsize);
		this->ecc_ctrl.calculate(p, &ecc_calc[i]);

		i += eccbytes;
		p += eccsize;
	}

	for (i = 0; i < this->ecc_ctrl.ecc_total_bytes_per_page; i++)
		this->oob_poi[eccpos[i]] = ecc_calc[i];

	this->write_buf(mtd, this->oob_poi, mtd->oobsize);

	return 0;
}


/********************************************************************************
* 函数: static int32_t nand_write_page(__in struct mtd_info *mtd,
                                      __in const uint8_t *buf,
                                      __in int32_t page, __in int32_t cached,
                                      __in int32_t raw)
* 描述: 写入nandflash一页数据
* 输入: mtd: nandflash设备的父类
       buf: 需要写入的缓冲区数据
       page: 写入的页
       cached: 是否允许使用cache编程
       raw: 是否写原始数据
* 输出: none
* 返回: 0: 写入成功
       -EIO: 写入失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_write_page(__in struct mtd_info *mtd, __in const uint8_t *buf,
                                 __in int32_t page, __in int32_t cached,
                                 __in int32_t raw)
{
    struct nand_chip *this = mtd->priv;
    int32_t status;

	this->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);

	if (unlikely(raw))
		this->ecc_ctrl.write_page_raw(mtd, buf);
	else
		this->ecc_ctrl.write_page(mtd, buf);

	/*
	   目前暂不支持cached写入，应为不确定速度提升(2.3->2.6Mib/s, 速度提升并不是很明显)
	   带来的风险是否值得
    */
	cached = 0;

	if (!cached || !(this->options & NAND_CACHEPRG))
	{

		this->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
		status = this->waitfunc(mtd);

		/* 检测写入结果 */
		if ((status & NAND_STATUS_FAIL) && (this->errstat))
			status = this->errstat(mtd, FL_WRITING, status, page);

		if (status & NAND_STATUS_FAIL)
			return -EIO;
	}
	else
	{
		this->cmdfunc(mtd, NAND_CMD_CACHEDPROG, -1, -1);
		status = this->waitfunc(mtd);
	}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/* 校验写入的数据 */
	this->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

	if (this->verify_buf(mtd, buf, this->page_writesize))
		return -EIO;
#endif
	return 0;
}


/********************************************************************************
* 函数: static uint8_t *nand_fill_oob(__in struct nand_chip *this,
                                     __out uint8_t *oob,
                                     __in struct mtd_oob_ops *ops)
* 描述: 填充oob数据段数据
* 输入: this: nandflash设备自身指针
       ops: oob区操作方式
* 输出: oob: oob数据输出缓冲区
* 返回: oob变为指到当前空闲位置的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
static uint8_t *nand_fill_oob(__in struct nand_chip *this, __in uint8_t *oob,
                                __in struct mtd_oob_ops *ops)
{
    size_t len = ops->ooblen;

    switch(ops->mode)
    {
    case MTD_OOB_PLACE:
    case MTD_OOB_RAW:
        memcpy(this->oob_poi+ops->ooboffs, oob, len);
        return oob + len;

    case MTD_OOB_AUTO:
    {
        struct nand_oobfree *free = this->ecc_ctrl.layout->oobfree;
        uint32_t boffs = 0;
        size_t bytes = 0;

        for(; free->length && len; free++, len -= bytes)
        {
            bytes = min_t(size_t, len, free->length);
            boffs = free->offset;

            memcpy(this->oob_poi+boffs, oob, bytes);

            oob += bytes;
        }

        return oob;
    }

    default:
        break;
    }

    return NULL;
}

/********************************************************************************
* 函数: static int32_t nand_write(__in struct mtd_info *mtd, __in loff_t to,
                                 __in size_t len, __out size_t *retlen,
                                 __in const uint8_t *buf)
* 描述: 读取nandflash经过ecc校验的数据
* 输入: mtd: nandflash设备父类
       to: 写入的起始地址
       len: 需要写入的长度
       buf: 需要写入的数据，不包含oob区
* 输出: retlen: 成功写入的长度
* 返回: 0: 写入成功
       -EIO: 写入失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_write(__in struct mtd_info *mtd, __in loff_t to, __in size_t len,
                            __out size_t *retlen, __in const uint8_t *buf)
{
    struct nand_chip *this = mtd->priv;
    int32_t chipnr, page, realpage, column;
    int32_t blkcheck = (1 << (this->chip_shift - this->page_shift)) - 1;
    int32_t bytes;
    int32_t writelen = len;
    const uint8_t *writebuf = buf;
    int32_t cached;
	int ret;

	/* 写入范围越界 */
	if ((to + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	nand_get_device(mtd, FL_WRITING);

    /* 写入地址不对齐 */
    if((to & (this->subpage_size - 1)) || ((to + len) & (this->subpage_size - 1)))
    {
        printl(LOG_LEVEL_ERR, "[NAND:ERR] attemp to write not page aligned data.\n");
        return -EINVAL;
    }

    column = to & (mtd->writesize - 1);

    chipnr = (int32_t)(to >> this->chip_shift);
    this->select_chip(mtd, chipnr);

    realpage = (int32_t)(to >> this->page_shift);
    page = realpage & this->page_mask;

    /* 当要写的页的范围包含cache的页时，失效cache的页 */
    if((to <= (this->pagebuf << this->page_shift)) &&
        ((this->pagebuf << this->page_shift) < (to + len)))
        this->pagebuf = -1;

    /* oob区全部填充0xff */
    memset(this->oob_poi, 0xff, mtd->oobsize);


    while(1)
    {
        bytes = mtd->writesize;
        cached = (writelen > bytes && page != blkcheck);

        /* 写一页的部分 */
        if(unlikely(column || (writelen < (mtd->writesize - 1))))
        {
            bytes = min_t(int32_t, bytes - column, writelen);

            /* data区不写的区域填充0xff*/
            memset(this->page_databuf, 0xff, mtd->writesize);
            memcpy(&this->page_databuf[column], writebuf, bytes);

            /* 不写oob区，使用RAW模式写data区 */
            ret = nand_write_page(mtd, this->page_databuf, page, cached, MTD_OOB_RAW);
        }
        else
        {
            ret = nand_write_page(mtd, writebuf, page, cached, MTD_OOB_RAW);
        }

        writelen -= bytes;
        if(!writelen)
            break;

        column = 0;
        writebuf += bytes;

        realpage++;
        page = realpage & this->page_mask;

        if(!page)
        {
            chipnr++;
            this->select_chip(mtd, -1);
            this->select_chip(mtd, chipnr);
        }
    }

    *retlen = len - writelen;

	nand_release_device(mtd);

	return ret;
}



/********************************************************************************
* 函数: static int32_t nand_write_oob_std(__in struct mtd_info *mtd,
                                         __in int32_t page)
* 描述: 仅写入oob区全部数据
* 输入: this: nandflash设备自身指针
       page: 写入的页
* 输出: none
* 返回: 0: 写入成功
       -EIO: 写入失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_write_oob_std(__in struct mtd_info *mtd, __in int32_t page)
{
    int32_t ret = 0;
    struct nand_chip *this = mtd->priv;
	const uint8_t *buf = this->oob_poi;
	int len = mtd->oobsize;

	this->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);
	this->write_buf(mtd, buf, len);
	/* 开始写入 */
	this->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	ret = this->waitfunc(mtd);

	return ret & NAND_STATUS_FAIL ? -EIO : 0;
}



/********************************************************************************
* 函数: static int32_t nand_read_oob(__in struct mtd_info *mtd,
                                    __in loff_t to,
                                    __inout struct mtd_oob_ops *ops)
* 描述: 根据ops选项写入oob或者oob+data在一起的数据
* 输入: mtd: nandflash设备父类
       to: 数据起始地址
       ops: 数据写入参数
* 输出: ops: 写入完毕后但会的数据和参数
* 返回: 0: 写入成功
       -EIO: 写入失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_write_oob(__in struct mtd_info *mtd, __in loff_t to,
                               __in struct mtd_oob_ops *ops)
{
    struct nand_chip *this = mtd->priv;
    int32_t page, realpage, chipnr;
	int32_t blkcheck = (1 << (this->phys_erase_shift - this->page_shift)) - 1;
    int32_t ret = 0;
    int32_t ooblen_per_page;
    size_t writelen = ops->len;
    uint8_t *oobwritebuf = ops->oobbuf;
    uint8_t *writebuf = ops->databuf;
    int32_t bytes;

    nand_get_device(mtd, FL_WRITING);

    switch(ops->mode)
    {
    case MTD_OOB_PLACE:
    case MTD_OOB_AUTO:
    case MTD_OOB_RAW:
        break;

    default:
        goto out;
    }

    if(ops->mode == MTD_OOB_AUTO)
        ooblen_per_page = this->ecc_ctrl.layout->oobavail;
    else
        ooblen_per_page = mtd->oobsize;

    /* 检验写入地址和范围 */
    if(ops->databuf && (to + ops->len) > mtd->size)
    {
        printl(LOG_LEVEL_ERR, "[NAND:ERR] attempt to write beyond end of device.\n");
        return -EINVAL;
    }



    chipnr = (int32_t)(to >> this->chip_shift);
    this->select_chip(mtd, chipnr);

    realpage = (int32_t)(to >> this->page_shift);
    page = realpage & this->page_mask;

    //this->cmdfunc(this, NAND_CMD_RESET, -1, -1);
    /* 检测wp位 */
    if(nand_check_wp(mtd))
        return -EROFS;

    if(!ops->databuf)
    {
        /* 仅写入oob区, 只能写一页 */
        if((ops->ooboffs + ops->ooblen) > ooblen_per_page)
        {
            printl(LOG_LEVEL_ERR, "[NAND:ERR] attempt to write beyond end of device.\n");
            return -EINVAL;
        }

        if(unlikely((to >= mtd->size) ||
                    ((ops->ooboffs + ops->ooblen) >
                     ((mtd->size >> this->page_shift) -
                      (to >> this->page_shift) * ooblen_per_page))))
        {
            printl(LOG_LEVEL_ERR, "[NAND:ERR] attempt to write beyond end of device.\n");
            return -EINVAL;
        }

        /* 检测当前cache的页 */
        if(realpage == this->pagebuf)
            this->pagebuf = -1;

        memset(this->oob_poi, 0xff, mtd->oobsize);
        nand_fill_oob(this, oobwritebuf, ops);
        ret = this->ecc_ctrl.write_oob(mtd, page);
        memset(this->oob_poi, 0xff, mtd->oobsize);

        ops->oobretlen = oobwritebuf - ops->oobbuf;
        ops->retlen = 0;
    }
    else
    {
        /* 写入oob+data区 */
        int32_t cached;
        int32_t column;


        /* 写入地址和长度不对齐 */
        if((to & (this->subpage_size - 1)) || ((to + writelen) & (this->subpage_size - 1)))
        {
            printl(LOG_LEVEL_ERR, "[NAND:ERR] attemp to write not page aligned data.\n");
            return -EINVAL;
        }

        column = to & (mtd->writesize - 1);

        /* 当要写的页的范围包含cache的页时，失效cache的页 */
        if((to <= (this->pagebuf << this->page_shift)) &&
            ((this->pagebuf << this->page_shift) < (to + writelen)))
            this->pagebuf = -1;


        while(1)
        {
            bytes = mtd->writesize;
            cached = (writelen > bytes && page != blkcheck);
            /* 填充oob区 */
            oobwritebuf = nand_fill_oob(this, oobwritebuf, ops);

            /* 写一页的部分 */
            if(unlikely(column || (writelen < (mtd->writesize - 1))))
            {
                bytes = min_t(int32_t, bytes - column, writelen);

                /* data区不写的区域填充0xff*/
                memset(this->page_databuf, 0xff, mtd->writesize);
                memcpy(&this->page_databuf[column], writebuf, bytes);

                /* 使用RAW模式写oob+data区 */
                ret = nand_write_page(mtd, this->page_databuf, page, cached, MTD_OOB_RAW);
            }
            else
            {
                ret = nand_write_page(mtd, writebuf, page, cached, MTD_OOB_RAW);
            }
            if(ret)
                break;

            writelen -= bytes;
            if(!writelen)
                break;

            column = 0;
            writebuf += bytes;

            realpage++;
            page = realpage & this->page_mask;

            if(!page)
            {
                chipnr++;
                this->select_chip(mtd, -1);
                this->select_chip(mtd, chipnr);
            }
        }

        ops->retlen = ops->len - writelen;
        ops->oobretlen = ops->ooblen;
    }

out:
    nand_release_device(mtd);

    return ret;
}


/********************************************************************************
* 函数: static void nand_sync(__in struct mtd_info *mtd)
* 描述: 同步nandflash设备
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_sync(__in struct mtd_info *mtd)
{
    nand_get_device(mtd, FL_SYNCING);

    nand_release_device(mtd);
}


/*-------------------------------------------------------------------------------------------
-----------------------------------------坏块判断处理------------------------------------------
--------------------------------------------------------------------------------------------*/

/********************************************************************************
* 函数: static int32_t nand_block_bad(__in struct mtd_info *mtd,
                                     __in loff_t ofs,
                                     __in int32_t getchip)
* 描述: 检测块是否是坏块
* 输入: mtd: nandflash设备父类
       ofs: 块地址
       getchip: 0: 芯片已经被选中
                1: 需要选取芯片
* 输出: none
* 返回: 0: 块正常
       1: 块是坏块
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_block_bad(__in struct mtd_info *mtd, __in loff_t ofs,
                                __in int32_t getchip)
{
    int32_t page = 0, chipnr = 0, res = 0;
    uint16_t bad = 0;
    struct nand_chip *this = mtd->priv;

    page = (int32_t)(ofs >> this->page_shift) & (this->page_mask);

    /* 选取nand设备 */
    if(getchip)
    {
        /* 需要选取芯片 */
        chipnr = (int32_t)(ofs >> this->chip_shift);

        /* 设置芯片状态 */
        nand_get_device(mtd, FL_READING);

        /* 选择芯片 */
        this->select_chip(mtd, chipnr);
    }

    if(this->options & NAND_BUSWIDTH_16)
    {
        /* 坏块标记2字节对齐 */
        this->cmdfunc(mtd, NAND_CMD_READOOB, this->badblockpos & 0xfe, page);
        bad = cpu_to_le16(this->read_word(mtd));

        /* 不对齐处理 */
        if(this->badblockpos & 0x01)
            bad >>= 8;

        /* 出现坏块 */
        if((bad & 0xff) != 0xff)
            res = 1;
    }
    else
    {
        this->cmdfunc(mtd, NAND_CMD_READOOB, this->badblockpos, page);
        if(this->read_byte(mtd) != 0xff)
            res = 1;
    }

    /* 释放nand设备 */
    if(getchip)
        nand_release_device(mtd);


    return res;

}


/********************************************************************************
* 函数: int32_t nand_isbad_bbt(__in struct mtd_info *mtd, __in loff_t offs,
                              __in int32_t allowbbt)
* 描述: 通过bbt检测块是否是坏块
* 输入: mtd: nandflash设备父类
       offs: 块的偏移地址
       allowbbt: 是否允许访问bbt，0: 不允许 1: 允许
* 输出: none
* 返回: 0: 好块
       1: 坏块
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t nand_isbad_bbt(__in struct mtd_info *mtd, __in loff_t offs, __in int32_t allowbbt)
{
    int32_t block;
    uint8_t res;
    struct nand_chip *this = mtd->priv;

    /* 计算block号 */
    block = (int32_t)(offs >> this->phys_erase_shift);

    /* 取bbt中的值 */
    res = (this->bbt[block >> 2] >> ((block & 0x03) << 1)) & 0x03;

    printl(LOG_LEVEL_MSG, "[NAND:MSG] nand_isbad_bbt: bbt info for offset 0x%08x: (block %d) 0x%02x\n",
                           (uint32_t)offs, block, res);

    switch(res)
    {
    /* 好块 */
    case 0:
        return 0;
    /* 使用过程中写坏 */
    case 1:
        return 1;
    /* 保留 */
    case 2:
        return allowbbt ? 0 : 1;
    /* 出厂就是坏块 */
    default:
        return 1;
    }

}

/********************************************************************************
* 函数: static int32_t nand_default_block_markbad(__in struct mtd_info *mtd,
                                                 __in loff_t ofs)
* 描述: 标记块为坏块(默认函数)
* 输入: mtd: nandflash设备存在的mtd节点
       ofs: 坏块的偏移地址
* 输出: none
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_default_block_markbad(__in struct mtd_info *mtd, __in loff_t ofs)
{
    struct nand_chip *this = mtd->priv;
    int32_t block = 0, ret = 0;


    //标记坏块
    block = (int32_t)(ofs >> this->bbt_erase_shift);
    if(this->bbt)
        this->bbt[block >> 2] |= 1 << ((block & 0x03) << 1);

    if(this->options & NAND_USE_FLASH_BBT)
        ret = nand_update_bbt(mtd, ofs);
    else
    {
        struct mtd_oob_ops ops;
        uint8_t buf[2] = { 0, 0 };

        nand_get_device(mtd, FL_WRITING);
        ofs += mtd->oobsize;
		ops.len = ops.ooblen = 2;
		ops.databuf = NULL;
		ops.oobbuf = buf;
		ops.ooboffs = this->badblockpos & ~0x01;

		ret = nand_write_oob(mtd, ofs, &ops);
		nand_release_device(mtd);
    }

    if(!ret)
        mtd->ecc_stats.badblocks ++;

    return ret;
}



/********************************************************************************
* 函数: static int32_t nand_block_isbad(__in struct mtd_info *mtd,
                                       __in loff_t offs)
* 描述: 检测指定偏移地址的block是否时坏块
* 输入: mtd: nandflash设备父类
       offs: 需要检测的块所在地址
* 输出: none
* 返回: 0： 好块
       1： 坏块
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_block_isbad(__in struct mtd_info *mtd, __in loff_t offs)
{
    if(offs > mtd->size)
        return -EINVAL;

    return nand_block_checkbad(mtd, offs, 1, 0);
}


/********************************************************************************
* 函数: static int32_t nand_block_markbad(__in struct mtd_info *mtd,
                                         __in loff_t offs)
* 描述: 标记nandflash块为坏块
* 输入: mtd: nandflash设备父类
       offs: 块的地址
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_block_markbad(__in struct mtd_info *mtd, __in loff_t offs)
{
    struct nand_chip *this = mtd->priv;
    int32_t ret;

    if((ret = nand_block_isbad(mtd, offs)))
    {
        if(ret > 0)
            return 0;

        return 0;
    }

    return this->block_markbad(mtd, offs);
}
/*-------------------------------------------------------------------------------------------
--------------------------------------坏块判断处理(end)----------------------------------------
--------------------------------------------------------------------------------------------*/


/********************************************************************************
* 函数: static int32_t nand_suspend(__in struct mtd_info *mtd)
* 描述: 挂起nandflash设备
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t nand_suspend(__in struct mtd_info *mtd)
{
	return nand_get_device(mtd, FL_PM_SUSPENDED);
}


/********************************************************************************
* 函数: static void nand_resume(__in struct mtd_info *mtd)
* 描述: 恢复nandflash设备
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_resume(__in struct mtd_info *mtd)
{
	struct nand_chip *this = mtd->priv;

	if(this->state == FL_PM_SUSPENDED)
		nand_release_device(mtd);
	else
		printl(LOG_LEVEL_WARN, "[NAND:WARN] called for a chip which is not in suspended state\n");
}


/********************************************************************************
* 函数: static void nand_set_defaults(__in struct nand_chip *this,
                                     __in int32_t busw)
* 描述: 设置nandflash默认操作基本函数
* 输入: this: nandflash设备自身指针
       busw: 总线宽度
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_set_defaults(__in struct nand_chip *this, __in int32_t busw)
{
    /* 检测芯片延时是否有效，无效就设置为20us */
	if(!this->chip_delay)
        this->chip_delay = 20;

	if(this->cmdfunc == NULL)
        this->cmdfunc = nand_command;

	if(this->waitfunc == NULL)
        this->waitfunc = nand_wait;

	if(!this->select_chip)
		this->select_chip = nand_select_chip;
	if(!this->read_byte)
		this->read_byte = busw ? nand_read_byte16 : nand_read_byte;
	if(!this->read_word)
		this->read_word = nand_read_word;
	if(!this->block_bad)
		this->block_bad = nand_block_bad;
	if(!this->block_markbad)
		this->block_markbad = nand_default_block_markbad;
	if(!this->write_buf)
		this->write_buf = busw ? nand_write_buf16 : nand_write_buf;
	if(!this->read_buf)
		this->read_buf = busw ? nand_read_buf16 : nand_read_buf;
	if(!this->verify_buf)
		this->verify_buf = busw ? nand_verify_buf16 : nand_verify_buf;
	if(!this->scan_bbt)
		this->scan_bbt = nand_default_bbt;
}

/********************************************************************************
* 函数: static struct nand_device_info *nand_get_flash_type(__in struct
                                                           mtd_info *mtd)
* 描述: 获取板级芯片的各项具体参数
* 输入: chip: nandflash芯片自身指针
* 输出: none
* 返回: 成功: nandflash芯片参数结构体
       失败: -ENODEV, 未知芯片
* 作者:
* 版本: v1.0
**********************************************************************************/
static struct nand_device_info *nand_get_flash_type(__in struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;
	struct nand_device_info *type = NULL;
	uint8_t id_bytes[2];
	uint8_t tmp_id_bytes[2];

	/* 选中设备 */
	this->select_chip(mtd, 0);

	/* 线复位芯片，有些芯片需要上电先复位 */
	this->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	/* 读取芯片ID */
	this->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	/* 第一次读取ID */
	this->read_buf(mtd, id_bytes, 2);

	/* 读取芯片ID */
	this->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	/* 第二次读取ID，防止出错 */
	this->read_buf(mtd, tmp_id_bytes, 2);

	if((id_bytes[0] != tmp_id_bytes[0]) || (id_bytes[1] != tmp_id_bytes[1]))
	{
		printl(LOG_LEVEL_ERR, "[NAND:ERR] second ID read did not match "
		                       "%02x,%02x against %02x,%02x\n",
		                       id_bytes[0], id_bytes[1], tmp_id_bytes[0], tmp_id_bytes[1]);
		return ERR_PTR(-ENODEV);
	}

	/* 从内建的表中找到设备的具体信息 */
	type = nand_device_get_info(id_bytes);

	if(!type)
	{
	    printl(LOG_LEVEL_ERR, "[NAND:ERR] unkonwn nandflash device %d(%d)\n",
		                       id_bytes[0], id_bytes[1]);
        return ERR_PTR(-ENODEV);
	}


    /* 直接从内建的表中读取所有数据 */
    this->chipsize = type->chip_size_in_bytes;
    mtd->erasesize = type->block_size_in_bytes;
    mtd->writesize = type->page_data_size_in_bytes;
    mtd->oobsize = type->page_oob_size_in_bytes;


	/* 计算page_shift */
	this->page_shift = ffs(mtd->writesize) - 1;
	/* 计算page_mask. */
	this->page_mask = (this->chipsize >> this->page_shift) - 1;

	this->bbt_erase_shift = this->phys_erase_shift = ffs(mtd->erasesize) - 1;

	if (this->chipsize & 0xffffffff)
		this->chip_shift = ffs(this->chipsize) - 1;
	else
		this->chip_shift = ffs((uint32_t)(this->chipsize >> 32)) + 31;

	/* 计算坏块标记的位置 */
	this->badblockpos = mtd->writesize > 512 ?
		NAND_LARGE_BADBLOCK_POS : NAND_SMALL_BADBLOCK_POS;

	/* 芯片选项 */
	this->options &= ~NAND_CHIPOPTIONS_MSK;
	this->options |= type->options & NAND_CHIPOPTIONS_MSK;

	/* 默认不支持NAND_NO_AUTOINCR，板级可以修改 */
	this->options |= NAND_NO_AUTOINCR;

	/* 擦除命令 */
    this->erase_cmd = single_erase_cmd;

	/* 检测页大小，重新设置命令函数 */
	if(mtd->writesize > 512 && this->cmdfunc == nand_command)
		this->cmdfunc = nand_command_lp;

	/* 打印nandflash信息 */
    nand_device_print_info(type);

	return type;
}

/********************************************************************************
* 函数: int32_t nand_scan_ident(__in struct mtd_info *mtd, __in int32_t maxchips)
* 描述: 校准芯片数量和mtd大小
* 输入: mtd: nandflash设备父类
       maxchips: 可能有的最多的芯片数量
* 输出: none
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t nand_scan_ident(__in struct mtd_info *mtd, __in int32_t maxchips)
{
	int32_t i, busw;
	struct nand_chip *this = mtd->priv;
    struct nand_device_info *type = NULL;

	/* 取得总线宽度s */
	busw = this->options & NAND_BUSWIDTH_16;
	/* 设置默认函数和选项 */
	nand_set_defaults(this, busw);

	/* 取得芯片型号和具体参数信息 */
	type = nand_get_flash_type(mtd);

	if(IS_ERR(type))
	{
		this->select_chip(mtd, -1);
		return PTR_ERR(type);
	}

	/* 检测是不是有多个芯片 */
	for(i = 1; i < maxchips; i++)
	{
		this->select_chip(mtd, i);
		/* 先复位芯片 */
		this->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
		/* 读取芯片的ID */
		this->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);
		/* 比较ID是否一样 */
		if((type->manufacturer_code != this->read_byte(mtd)) ||
		    (type->device_code != this->read_byte(mtd)))
			break;
	}

	this->timing = &(type->timing);

	if(i > 1)
		printl(LOG_LEVEL_INFO, "[NAND:INFO] %d NAND chips detected\n", i);


	/* 校准芯片和mtd大小 */
	this->numchips = i;
	mtd->size = i * this->chipsize;

	return 0;
}

/********************************************************************************
* 函数: int32_t nand_scan_tail(__in struct mtd_info *mtd)
* 描述: 扫描具体芯片，初始化默认函数
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t nand_scan_tail(__in struct mtd_info *mtd)
{
	int32_t i;
	struct nand_chip *this = mtd->priv;

	/* 设置oob缓冲区位置 */
	this->oob_poi = this->page_databuf + mtd->writesize;

	/* 设置ecc布局 */
	if(!this->ecc_ctrl.layout)
	{
		switch(mtd->oobsize)
		{
		case 8:
			this->ecc_ctrl.layout = &nand_oob_8;
			break;
		case 16:
			this->ecc_ctrl.layout = &nand_oob_16;
			break;
		case 64:
			this->ecc_ctrl.layout = &nand_oob_64;
			break;
		case 128:
			this->ecc_ctrl.layout = &nand_oob_128;
			break;
		default:
			printl(LOG_LEVEL_WARN, "[NAND:WARN] No oob scheme defined for "
			       "oobsize %d\n", mtd->oobsize);
			BUG();
		}
	}

	/* 设置ecc参数 */
	if(!this->ecc_ctrl.read_page_raw)
		this->ecc_ctrl.read_page_raw = nand_read_page_raw;
	if(!this->ecc_ctrl.write_page_raw)
		this->ecc_ctrl.write_page_raw = nand_write_page_raw;

	switch (this->ecc_ctrl.mode)
	{
	case NAND_ECC_HW:
		/* 是否使用默认的硬件读写函数? */
		if(!this->ecc_ctrl.read_page)
			this->ecc_ctrl.read_page = nand_read_page_hwecc;
		if(!this->ecc_ctrl.write_page)
			this->ecc_ctrl.write_page = nand_write_page_hwecc;
		if(!this->ecc_ctrl.read_oob)
			this->ecc_ctrl.read_oob = nand_read_oob_std;
		if(!this->ecc_ctrl.write_oob)
			this->ecc_ctrl.write_oob = nand_write_oob_std;

	case NAND_ECC_SOFT:
		this->ecc_ctrl.calculate = ecc_calculate;
		this->ecc_ctrl.correct = ecc_correct_data;
		this->ecc_ctrl.read_page = nand_read_page_swecc;
		this->ecc_ctrl.read_subpage = nand_read_subpage;
		this->ecc_ctrl.write_page = nand_write_page_swecc;
		this->ecc_ctrl.read_oob = nand_read_oob_std;
		this->ecc_ctrl.write_oob = nand_write_oob_std;
		this->ecc_ctrl.data_size_per_step = 256;
		this->ecc_ctrl.ecc_bytes_per_step = 3;
		break;

	case NAND_ECC_NONE:
		printl(LOG_LEVEL_WARN, "[NAND:WARN] NAND_ECC_NONE selected by board driver. "
		       "This is not recommended !!!\n");
		this->ecc_ctrl.read_page = nand_read_page_raw;
		this->ecc_ctrl.write_page = nand_write_page_raw;
		this->ecc_ctrl.read_oob = nand_read_oob_std;
		this->ecc_ctrl.write_oob = nand_write_oob_std;
		this->ecc_ctrl.data_size_per_step = mtd->writesize;
		this->ecc_ctrl.ecc_bytes_per_step = 0;
		break;

	default:
		printl(LOG_LEVEL_WARN, "[NAND:WARN] Invalid NAND_ECC_MODE %d\n",
		       this->ecc_ctrl.mode);
		BUG();
	}

	/* 计算oob区域中允许用户放数据的空闲空间大小 */
	this->ecc_ctrl.layout->oobavail = 0;
	for(i = 0; this->ecc_ctrl.layout->oobfree[i].length; i++)
		this->ecc_ctrl.layout->oobavail += this->ecc_ctrl.layout->oobfree[i].length;
	mtd->oobavail = this->ecc_ctrl.layout->oobavail;

	/* 计算oob区域中ecc剩余参数 */
	this->ecc_ctrl.ecc_steps_per_page = mtd->writesize / this->ecc_ctrl.data_size_per_step;
	if(this->ecc_ctrl.ecc_steps_per_page * this->ecc_ctrl.data_size_per_step != mtd->writesize)
	{
		printl(LOG_LEVEL_WARN, "[NAND:WARN] Invalid ecc parameters\n");
		BUG();
	}
	this->ecc_ctrl.ecc_total_bytes_per_page = this->ecc_ctrl.ecc_steps_per_page * this->ecc_ctrl.ecc_bytes_per_step;

	/* 计算subpage大小，MLC没有subpage，SLC一般都有subpage */
	if(!(this->options & NAND_NO_SUBPAGE_WRITE) &&
	    !(this->cellinfo & NAND_CI_CELLTYPE_MSK))
    {
		switch(this->ecc_ctrl.ecc_steps_per_page)
		{
		case 2:
			mtd->subpage_sft = 1;
			break;
		case 4:
		case 8:
			mtd->subpage_sft = 2;
			break;
		}
	}
	this->subpage_size = mtd->writesize >> mtd->subpage_sft;

	/* 初始化芯片状态 */
	this->state = FL_READY;

	/* 不选中任何芯片 */
	this->select_chip(mtd, -1);

	/* 失效数据缓冲区*/
	this->pagebuf = -1;

	/* 初始化mtd默认函数 */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->erase = nand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = nand_read;
	mtd->write = nand_write;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->sync = nand_sync;
	mtd->lock = NULL;
	mtd->unlock = NULL;
	mtd->suspend = nand_suspend;
	mtd->resume = nand_resume;
	mtd->block_isbad = nand_block_isbad;
	mtd->block_markbad = nand_block_markbad;

	/* 初始化mtd ecc布局 */
	mtd->ecclayout = this->ecc_ctrl.layout;

	/* 扫描具体芯片 */
	this->options |= NAND_BBT_SCANNED;

	return this->scan_bbt(mtd);
}

/********************************************************************************
* 函数: int32_t nand_scan(__in struct mtd_info *mtd, __in int32_t maxchips)
* 描述: 扫描初始化nandflash设备
* 输入: mtd: nandflash设备父类
       maxchips: 最多的芯片数量
* 输出: none
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t nand_scan(__in struct mtd_info *mtd, __in int32_t maxchips)
{
	int32_t ret;

	ret = nand_scan_ident(mtd, maxchips);
	if (!ret)
		ret = nand_scan_tail(mtd);
	return ret;
}




