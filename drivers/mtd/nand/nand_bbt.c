/********************************************************************************
* BBT在nandflash芯片上表示
* 11b: 好块
* 00b: 厂商标记坏块
* 01b,10b: 使用过程中出现的坏块
*
* BBT在内存中表示方法
* 00b: 好块
* 01b: 使用过程中出现的坏块
* 10b: 保留
* 11b: 厂商标记的坏块
*
**********************************************************************************/
#include "stddef.h"
#include "string.h"
#include "malloc.h"
#include "log.h"
#include "errno.h"
#include "mtd/bbm.h"
#include "mtd/nand/nand.h"



/* 好/坏块标记 */
static uint8_t scan_ff_pattern[] = {0xff, 0xff};

/* 内存bbt */
static struct nand_bbt_desc smallpage_memorybased =
{
    .options = NAND_BBT_SCAN2NDPAGE,
    .offs = 5,
    .len = 1,
    .pattern = scan_ff_pattern
};

static struct nand_bbt_desc largepage_memorybased =
{
    .options = 0,
    .offs = 0,
    .len = 2,
    .pattern = scan_ff_pattern
};

/* flash bbt */
static struct nand_bbt_desc smallpage_flashbased =
{
    .options = NAND_BBT_SCAN2NDPAGE,
    .offs = 5,
    .len = 1,
    .pattern = scan_ff_pattern
};

static struct nand_bbt_desc largepage_flashbased =
{
    .options = NAND_BBT_SCAN2NDPAGE,
    .offs = 0,
    .len = 2,
    .pattern = scan_ff_pattern
};


/* 基于flash的bbt描述符 */
static uint8_t bbt_pattern[] = {'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_desc bbt_main_desc =
{
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	8,
	.len = 4,
	.veroffs = 12,
	.maxblocks = 4,
	.pattern = bbt_pattern
};

static struct nand_bbt_desc bbt_mirror_desc =
{
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	8,
	.len = 4,
	.veroffs = 12,
	.maxblocks = 4,
	.pattern = mirror_pattern
};

/********************************************************************************
* 函数: static int32_t check_short_pattern(__in uint8_t *buf,
                                          __in struct nand_bbt_desc *td)
* 描述: 检测好/坏块或者bbt标记
* 输入: buf: 需要检测的缓冲区
       td: bbt区域描述符
* 输出: none
* 返回: 0: 是好块
       -1: 是坏块
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t check_short_pattern(__in uint8_t *buf, __in struct nand_bbt_desc *td)
{
    int32_t i;
    const uint8_t *p = buf;

    for(i = 0; i < td->len; i++)
    {
        if(p[td->offs + i] != td->pattern[i])
            return -1;
    }

    return 0;
}

/********************************************************************************
* 函数: static int32_t read_bbt(__in struct mtd_info *mtd, __out uint8_t *buf,
                               __in int32_t page, __in int32_t num,
                               __in int32_t bits, __in int32_t offs,
                               __in int32_t reserved_block_code)
* 描述: 读取指定位置的bbt数据到内存(bbt已经建立)
* 输入: mtd: nandflash设备父类
       page: bbt所在的起始页
       numblocks: 块的数量
       bits: 一块的好/坏标记在nandflash中存储的位数
       offs: bbt数据存在内存bbt缓冲区的偏移地址，当存在多个bbt时有效(NAND_BBT_PERCHIP)
       reserved_block_code: 保留块标记
* 输出: buf: 临时数据缓冲区
* 返回: 0: 读取成功
       !0: 读取失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t read_bbt(__in struct mtd_info *mtd, __out uint8_t *buf, __in int32_t page,
                          __in int32_t numblocks, __in int32_t bits, __in int32_t offs,
                          __in int32_t reserved_block_code)
{
    struct nand_chip *this = mtd->priv;
    size_t readlen, len, retlen, act = 0;
    int32_t res, i, j;
    loff_t from = (loff_t)(page << this->page_shift);
    uint8_t mask = (uint8_t)((1 << bits) - 1);
    uint8_t blkflag;

    /* 需要读取的字节数 */
    readlen = (numblocks * bits) >> 3;

    while(1)
    {
        /* 读取bbt数据 */
        len = min_t(size_t, readlen, (size_t)(1 << this->bbt_erase_shift));
        res = mtd->read(mtd, from, len, &retlen, buf);

        if(res < 0)
        {
            /* bbt块读取错误 */
            if(retlen != len)
            {
                printl(LOG_LEVEL_ERR, "[NANDBBT:ERR] error reading bad block table.\n");
                return res;
            }

            /* 出现错误，ecc成功校正 */
            printl(LOG_LEVEL_WARN, "[NANDBBT:WARN] ecc error while reading bad block table.\n");
        }

        /* 分析读到的数据 */
        for(i = 0; i < len; i++)
        {
            for(j = 0; j < 8; j += bits)
            {
                blkflag = ((buf[i] >> j) & mask);

                /* 是好块 */
                if(blkflag == mask)
                    continue;

                /* 是保留的块 */
                if(reserved_block_code && (reserved_block_code == blkflag))
                {
                    printl(LOG_LEVEL_MSG, "[NANDBBT:MSG] reserved block at 0x%012llx\n",
                           (loff_t)((offs << 2) + (act >> 1)) << this->bbt_erase_shift);

                    this->bbt[offs + (act >> 3)] |= (0x02 <<  (act & 0x06));
                    mtd->ecc_stats.bbtblocks ++;

                    continue;
                }

                printl(LOG_LEVEL_MSG, "[NANDBBT:MSG]: bad block at 0x%012llx\n",
					(loff_t)((offs << 2) + (act >> 1)) << this->bbt_erase_shift);

                if(blkflag == 0)
                    /* 厂商标记的坏块 */
                    this->bbt[offs + (act >> 3)] |= (0x03 <<  (act & 0x06));
                else
                    /* 使用过程中出现的坏块 */
                    this->bbt[offs + (act >> 3)] |= (0x01 <<  (act & 0x06));

                act += 2;

                mtd->ecc_stats.badblocks ++;
            }
        }

        readlen -= len;
        if(!readlen)
            break;

        from += len;
    }

    return 0;
}



/********************************************************************************
* 函数: static int32_t read_abs_bbt(__in struct mtd_info *mtd, __out uint8_t *buf,
                                   __in struct nand_bbt_desc *td,
                                   __in int32_t chip)
* 描述: 读取指定页开始的bbt到内存(bbt已经建立)
* 输入: mtd: nandflash设备的父类
       td: bbt描述符
       chip: 在有NAND_BBT_PERCHIP标志位时才有效，
             -1: 读取所有的物理芯片保存的各自的bbt
             其他值: 读取指定的物理芯片
* 输出: buf: nandflash读取出的原始数据输出缓冲区
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t read_abs_bbt(__in struct mtd_info *mtd, __out uint8_t *buf,
                              __in struct nand_bbt_desc *td, __in int32_t chip)
{
    struct nand_chip *this = mtd->priv;
    int32_t bits, res = 0, i;

    bits = td->options & NAND_BBT_NRBITS_MSK;
	if(td->options & NAND_BBT_PERCHIP)
	{
		int offs = 0;
		for(i = 0; i < this->numchips; i++)
		{
			if(chip == -1 || chip == i)
                res = read_bbt(mtd, buf, td->pages[i], this->chipsize >> this->bbt_erase_shift, bits, offs, td->reserved_block_code);
			if(res)
                return res;

            /* 计算内存bbt的偏移地址 */
			offs += (this->chipsize) >> (this->bbt_erase_shift + 2);
		}
	}
	else
	{
		res = read_bbt(mtd, buf, td->pages[0], mtd->size >> this->bbt_erase_shift, bits, 0, td->reserved_block_code);
		if(res)
			return res;
	}

	return 0;
}



/********************************************************************************
* 函数: static int32_t read_abs_bbts(__in struct mtd_info *mtd, __out uint8_t *buf,
                                    __in nand_bbt_descr *td,
                                    __in nand_bbt_descr *md)
* 描述: 读取原始和镜像bbt
* 输入: mtd: nandflash设备父类
       td: 原始bbt描述符
       md: 镜像bbt描述符
* 输出: buf: bbt原始数据输出缓冲区
* 返回: 1
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t read_abs_bbts(__in struct mtd_info *mtd, __out uint8_t *buf,
                               __in struct nand_bbt_desc *td,
                               __in struct nand_bbt_desc *md)
{
    struct nand_chip *this = mtd->priv;
    struct mtd_oob_ops ops;
    int32_t res;

    ops.mode = MTD_OOB_RAW;
    ops.ooboffs = 0;
    ops.ooblen = mtd->oobsize;
    ops.oobbuf = buf + mtd->writesize;
    ops.databuf = buf;
    ops.len = mtd->writesize;

    if(td->options & NAND_BBT_VERSION)
    {
        res = mtd->read_oob(mtd, (loff_t)(td->pages[0] << this->page_shift), &ops);

        if(res)
        {
            printl(LOG_LEVEL_MSG, "[NANDBBT:MSG] bad block table at page %d, version 0x%02x\n",
                               td->pages[0], td->version[0]);
            return res;
        }


        td->version[0] = buf[mtd->writesize + td->veroffs];

        printl(LOG_LEVEL_MSG, "[NANDBBT:MSG] bad block table at page %d, version 0x%02x\n",
                               td->pages[0], td->version[0]);
    }

    if(md && (md->options & NAND_BBT_VERSION))
    {
         res = mtd->read_oob(mtd, (loff_t)(md->pages[0] << this->page_shift), &ops);

        if(res)
        {
            printl(LOG_LEVEL_MSG, "[NANDBBT:MSG] bad block table at page %d, version 0x%02x\n",
                               td->pages[0], td->version[0]);
            return res;
        }


        md->version[0] = buf[mtd->writesize + md->veroffs];

        printl(LOG_LEVEL_MSG, "[NANDBBT:MSG] mirror bad block table at page %d, version 0x%02x\n",
                               md->pages[0], md->version[0]);
    }

    return 1;
}

/********************************************************************************
* 函数: static int32_t scan_block_fast(__in struct mtd_info *mtd,
                                      __in nand_bbt_desc *bd,
                                      __in loff_t offs,
                                      __out uint8_t *buf, __in int32_t numpages)
* 描述: 快速扫描好/坏块标记
* 输入: mtd: nandflash设备父类
       bd: bbt区域描述符
       offs: 扫描的起始地址
       numpages: 扫描的页的个数
* 输出: buf: oob原始数据输出缓冲区
* 返回: 0: 是好块
       1: 是坏块
       <0: 出现坏块
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t scan_block_fast(__in struct mtd_info *mtd, __in struct nand_bbt_desc *bd,
                                 __in loff_t offs, __out uint8_t *buf, __in int32_t numpages)
{
    struct mtd_oob_ops ops;
    int32_t i, ret;

    ops.mode = MTD_OOB_PLACE;
    ops.ooblen = mtd->oobsize;
    ops.oobbuf = buf;
    ops.ooboffs = 0;
    ops.databuf = NULL;

    for(i = 0; i < numpages; i++)
    {
        ret = mtd->read_oob(mtd, offs, &ops);
        if(ret)
            return ret;

        /* 检测好/坏块标记 */
        if(check_short_pattern(buf, bd))
            return 1; /* 是坏块 */

        offs += mtd->writesize;
    }

    return 0;
}



/********************************************************************************
* 函数: static int32_t create_bbt(__in struct mtd_info *mtd, __out uint8_t *buf,
                                 __in struct nand_bbt_desc *bd,
                                 __in int32_t chipnum)
* 描述: 扫描设备每块的好/坏标志，在内存中建立bbt
* 输入: mtd: nandflash设备父类
       bd: bbt区域描述符
       chipnum: 芯片号
* 输出: buf: 原始数据输出缓冲区
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t create_bbt(__in struct mtd_info *mtd, __in uint8_t *buf,
                            __in struct nand_bbt_desc *bd, __in int32_t chipnum)
{
    struct nand_chip *this = mtd->priv;
    int32_t len, numblocks, startblock, i;

    loff_t from;


    printl(LOG_LEVEL_MSG, "[NANDBBT:MSG] scanning device for bad blocks\n");

    if(bd->options & NAND_BBT_SCAN2NDPAGE)
        len = 2;
    else
        len = 1;



    if(chipnum == -1)
    {
        /* 扫描mtd大小的芯片，numblocks时真实块数目的两倍，因为下面使用到i+=2 */
        numblocks = mtd->size >> (this->bbt_erase_shift - 1);
        startblock = 0;
        from = 0;
    }
    else
    {
        if(chipnum >= this->numchips)
        {
            return -EINVAL;
        }

        /* 扫描指定芯片，numblocks时真实块数目的两倍，因为下面使用到i+=2 */
        numblocks = this->chipsize >> (this->bbt_erase_shift - 1);
        startblock = chipnum * numblocks;
        numblocks += startblock;

        /* 计算真实起始偏移 */
        from = (loff_t)(startblock << (this->bbt_erase_shift - 1));
    }

    for(i = startblock; i < numblocks; i++)
    {
        int32_t ret;

        ret = scan_block_fast(mtd, bd, from, buf, len);

        /* 读取好/坏块标记错误，一般一块的第1/2页都保证是好页，可以存放此块的好/坏标志 */
        if(ret < 0)
            return ret;

        if(ret)
        {
            /* 出现坏块，标记坏块 */
            this->bbt[i >> 3] |= 0x03 << (i & 0x06);

            printl(LOG_LEVEL_INFO, "[NANDBBT:INFO] bad eraseblock %d at 0x%012llx\n",
                                   i >> 1, (uint64_t)from);

            mtd->ecc_stats.badblocks++;
        }

        i += 2;
        from += (1 << this->bbt_erase_shift);
    }

    return 0;
}

/********************************************************************************
* 函数: static int32_t search_bbt(__in struct mtd_info *mtd, __out uint8_t *buf,
                                 __in struct nand_bbt_descr *td)
* 描述: 寻找存储在nandflash中的bbt
* 输入: mtd: nandflash设备父类
       td: bbt描述符
* 输出: buf: 读取的bbt原始数据
* 返回: 0
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t search_bbt(__in struct mtd_info *mtd, __out uint8_t *buf,
                            __in struct nand_bbt_desc *td)
{
    struct nand_chip *this = mtd->priv;
    int32_t startblock, dir, block;
    int32_t blocktopage = this->bbt_erase_shift - this->page_shift;
    int32_t i, numchips;
    struct mtd_oob_ops ops;

    ops.mode = MTD_OOB_RAW;
    ops.ooblen = mtd->oobsize;
    ops.oobbuf = buf;
    ops.ooboffs = 0;
    ops.databuf = NULL;


    /* bbt存放位置 */
    if(td->options & NAND_BBT_LASTBLOCK)
    {
        /* bbt存放在芯片最后一个好块, 从后往前寻找 */
        startblock = (mtd->size >> this->bbt_erase_shift) - 1;
        dir = -1;
    }
    else
    {
        /* 从前到后寻找bbt存放位置 */
        startblock = 0;
        dir = 1;
    }


    /* bbt按芯片存放还是都存放在一起 */
    if(td->options & NAND_BBT_PERCHIP)
    {
        numchips = this->numchips;
    }
    else
    {
        numchips = 1;
    }

    /* 寻找bbt */
    for(i = 0; i < numchips; i++)
    {
        td->version[i] = 0;
        td->pages[i] = 0;

        for(block = 0; block < td->maxblocks; block++)
        {
            int curblock = startblock + dir * block;
			loff_t offs = (loff_t)curblock << this->bbt_erase_shift;

            /* 读取block的第一页，bbt都存放在block的第一页 */
            mtd->read_oob(mtd, offs, &ops);
            if(!check_short_pattern(buf, td))
            {
                /* 找到bbt */
				td->pages[i] = curblock << blocktopage;
				if(td->options & NAND_BBT_VERSION)
                    td->version[i] = buf[td->veroffs];

				break;
            }
        }

        /* 到下一芯片 */
        startblock += this->chipsize >> this->bbt_erase_shift;
    }

    for (i = 0; i < numchips; i++)
    {
		if (td->pages[i] == -1)
			printl(LOG_LEVEL_WARN, "[NANDBBT:WARN] Bad block table not found for chip %d\n", i);
		else
			printl(LOG_LEVEL_MSG, "[NANDBBT:MSG] Bad block table found at page %d for chip %d, version 0x%02X\n",
                                   td->pages[i], i, td->version[i]);
	}
	return 0;
}


/********************************************************************************
* 函数: static int32_t search_read_bbts(__in struct mtd_info *mtd,
                                       __out uint8_t *buf,
                                       __in struct nand_bbt_descr *td,
                                       __in struct nand_bbt_descr *md)
* 描述: 寻找bbt
* 输入: mtd: nandflash设备父类
       td: 原始bbt
       md: 镜像bbt
* 输出: buf: bbt数据输出缓冲区
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t search_read_bbts(__in struct mtd_info *mtd, __out uint8_t *buf,
                                  __in struct nand_bbt_desc *td,
                                  __in struct nand_bbt_desc *md)
{
    search_bbt(mtd, buf, td);

    if(md)
        search_bbt(mtd, buf, md);

    return 1;
}


/********************************************************************************
* 函数: static int write_bbt(__in struct mtd_info *mtd, __out uint8_t *buf,
                            __in struct nand_bbt_descr *td,
                            __in struct nand_bbt_descr *md,
                            __in int32_t chipsel)
* 描述: 写bbt数据到芯片,此时内存中已经建立好bbt表
* 输入: mtd: nandflash设备父类
       td: 原始bbt, 是主要需要写入的bbt
       md: 镜像bbt, 在此用来防止td写入到md区域, 不需要写入
       chipsel: 需要写入的芯片号，-1表示每个芯片都写入
       buf: bbt或块数据输入缓冲区
* 输出: buf: 数据输出临时缓冲区
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t write_bbt(__in struct mtd_info *mtd, __inout uint8_t *buf,
		                   __in struct nand_bbt_desc *td, __in struct nand_bbt_desc *md,
		                   __in int32_t chipsel)
{
    struct nand_chip *this = mtd->priv;
    struct erase_info einfo;
    int32_t i, j, res, chip = 0;
    int32_t bits, startblock, dir, page, offs, numblocks, sft, sftmsk;
    int32_t numchips, bbtoffs, pageoffs, ooboffs;
    uint8_t msk[4];
    uint8_t rcode = td->reserved_block_code;
    size_t retlen, len = 0;
    loff_t to;
    struct mtd_oob_ops ops;


	if(!rcode)
		rcode = 0xff;

	/* 判断bbt时按芯片数量还是设备数量写入 */
	if(td->options & NAND_BBT_PERCHIP)
	{
		numblocks = (int32_t)(this->chipsize >> this->bbt_erase_shift);
		/* 所有芯片都写入各自的bbt还是写在指定的芯片 */
		if(chipsel == -1)
		{
			numchips = this->numchips;
		}
		else
		{
			numchips = chipsel + 1;
			chip = chipsel;
		}
	}
	else
	{
		numblocks = (int)(mtd->size >> this->bbt_erase_shift);
		numchips = 1;
	}

	/* 循环写入bbt数据 */
	for(; chip < numchips; chip++)
	{

		/* 检测原始bbt是否有指定写入的起始页 */
		if(td->pages[chip] != -1)
		{
		    /* 有指定写入的起始页，直接写入 */
			page = td->pages[chip];
			goto write;
		}

		/* 没有指定写入页，寻找合适的写入页写入 */
		if(td->options & NAND_BBT_LASTBLOCK)
		{
			startblock = numblocks * (chip + 1) - 1;
			dir = -1;
		}
		else
		{
			startblock = chip * numblocks;
			dir = 1;
		}

		for(i = 0; i < td->maxblocks; i++)
		{
		    /* 寻找一个好块写入 */
			int block = startblock + dir * i;
			/* Check, if the block is bad */
			switch ((this->bbt[block >> 2] >>
				 (2 * (block & 0x03))) & 0x03)
            {
            /* 此块时坏块，跳过 */
			case 0x01:
			case 0x03:
				continue;
			}

			/* 好块的起始页 */
			page = block << (this->bbt_erase_shift - this->page_shift);
			/* 检测这个好块有没有被镜像bbt使用 */
			if (!md || md->pages[chip] != page)
				goto write;
		}

		/* 没有可以写入的块 */
		printl(LOG_LEVEL_ERR, "[NANDBBT:ERR] No space left to write bad block table\n");
		return -ENOSPC;

write:
		/* 好/坏标示在nandflash中表示的位数 */
		bits = td->options & NAND_BBT_NRBITS_MSK;
		msk[2] = ~rcode;
		switch(bits)
		{
		case 1:
		    sft = 3;
		    sftmsk = 0x07;
		    msk[0] = 0x00;
		    msk[1] = 0x01;
			msk[3] = 0x01;
			break;
		case 2:
            sft = 2;
            sftmsk = 0x06;
            msk[0] = 0x00;
            msk[1] = 0x01;
			msk[3] = 0x03;
			break;
		case 4:
            sft = 1;
            sftmsk = 0x04;
            msk[0] = 0x00;
            msk[1] = 0x0C;
			msk[3] = 0x0f;
			break;
		case 8:
            sft = 0;
            sftmsk = 0x00;
            msk[0] = 0x00;
            msk[1] = 0x0F;
			msk[3] = 0xff;
			break;
		default:
            return -EINVAL;
		}

        /* 现有bbt数据在内存bbt中的偏移地址 */
		bbtoffs = chip * (numblocks >> 2);

        /* 写入的起始地址 */
		to = ((loff_t) page) << this->page_shift;

		/* 必须保存块里面的原始数据? */
		if (td->options & NAND_BBT_SAVECONTENT)
		{
		    /* 应为擦除是按块擦除，所以要块对齐，之后读取整个块的数据 */
			/* 写入/读取地址块对齐 */
			to &= ~((loff_t) ((1 << this->bbt_erase_shift) - 1));

			/* 读取一块的整个数据 */
			len = 1 << this->bbt_erase_shift;
			res = mtd->read(mtd, to, len, &retlen, buf);
			if (res < 0)
			{
			    /* 读取数据出现错误 */
				if(retlen != len)
				{
					printl(LOG_LEVEL_ERR, "[NANDBBT:ERR] err reading block data for writing the bad block table\n");
					return res;
				}
				printl(LOG_LEVEL_WARN, "[NANDBBT_WARN] ecc error while reading block for writing bad block table\n");
			}

			/* 读取oob数据 */
			ops.mode = MTD_OOB_PLACE;
			ops.ooblen = (len >> this->page_shift) * mtd->oobsize;
            ops.ooboffs = 0;
            ops.databuf = NULL;
			ops.oobbuf = &buf[len];
			res = mtd->read_oob(mtd, to + mtd->writesize, &ops);
			/* oob数据读取失败 */
			if (res < 0 || ops.oobretlen != ops.ooblen)
            {
                /* 读取数据出现错误 */
				printl(LOG_LEVEL_ERR, "[NANDBBT:ERR] err reading block oob for writing the bad block table\n");
                return res;
			}


			/* 计算bbt区域在buf中的偏移地址 */
			pageoffs = page - (int)(to >> this->page_shift);
			offs = pageoffs << this->page_shift;
			/* 把bbt区域读出来的数据全部先填充为0xff */
			memset(&buf[offs], 0xff, (size_t) (numblocks >> sft));
			/* bbt所在页的oob在整个块oob数据中的偏移地址*/
			ooboffs = len + (pageoffs * mtd->oobsize);

		}
		else
		{
			/* 计算bbt长度 */
			len = (size_t) (numblocks >> sft);
			/* 长度页对齐 */
			len = (len + (mtd->writesize - 1)) &
				~(mtd->writesize - 1);
			/* data+oob区域全部填充为0xff */
			memset(buf, 0xff, len + (len >> this->page_shift) * mtd->oobsize);
			offs = 0;
			ooboffs = len;
			/* oob区域填充bbt指示标示 */
			memcpy(&buf[ooboffs + td->offs], td->pattern, td->len);
		}

        /* bbt有版本控制 */
		if(td->options & NAND_BBT_VERSION)
			buf[ooboffs + td->veroffs] = td->version[chip];

		/* 把内存bbt中的数据转换填充到写入缓冲区 */
		for(i = 0; i < numblocks;)
		{
			uint8_t dat;
			dat = this->bbt[bbtoffs + (i >> 2)];

			/* 一次处理4个块 */
			for(j = 0; j < 4; j++, i++)
			{
			    /* 计算移位数 */
				int sftcnt = (i << (3 - sft)) & sftmsk;
				/* 保留保留块的表示方法，其他进行转换 */
				buf[offs + (i >> sft)] &= ~(msk[dat & 0x03] << sftcnt);
				dat >>= 2;
			}
		}

		memset(&einfo, 0, sizeof(einfo));
		einfo.mtd = mtd;
		einfo.addr = to;
		einfo.len = 1 << this->bbt_erase_shift;
		res = nand_erase_nand(mtd, &einfo, 1);
		if (res < 0)
		{
		    printl(LOG_LEVEL_ERR, "[NANDBBT:ERR] Error while writing bad block table %d\n", res);
		}
			goto outerr;

        /* 写入bbt数据 */
        ops.mode = MTD_OOB_PLACE;
        ops.ooboffs = 0;
        ops.ooblen = mtd->oobsize;
        ops.databuf = buf;
        ops.oobbuf = &buf[len];
        ops.len = len;

        res = mtd->write_oob(mtd, to, &ops);
		if (res < 0)
			goto outerr;

		printl(LOG_LEVEL_MSG, "[NANDBBT:MSG] Bad block table written to 0x%012llx, "
		       "version 0x%02X\n", (unsigned long long)to,
		       td->version[chip]);

		/* 标记bbt存在的页 */
		td->pages[chip] = page;
	}
	return 0;

 outerr:
	printl(LOG_LEVEL_ERR, "[NANDBBT:ERR] Error while writing bad block table %d\n", res);

	return res;
}



/********************************************************************************
* 函数: static int check_create(__in struct mtd_info *mtd, __in uint8_t *buf,
                               __in struct nand_bbt_desc *bd)
* 描述: 检测和创建bbt
* 输入: mtd: nandflash设备父类
       buf: bbt数据输入输出缓冲区
       bd: 需要创建的bbt的样式
* 输出: buf: bbt数据输入输出缓冲区
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
static int check_create(__in struct mtd_info *mtd, __inout uint8_t *buf,
                         __in struct nand_bbt_desc *bd)
{
	int i, numchips, writeops, chipsel, res;
	struct nand_chip *this = mtd->priv;
	/* td必须存在，md可以不存在 */
	struct nand_bbt_desc *td = this->bbt_td;
	struct nand_bbt_desc *md = this->bbt_md;
	struct nand_bbt_desc *rd;

	/* 每个芯片存储自身bbt? */
	if(td->options & NAND_BBT_PERCHIP)
		numchips = this->numchips;
	else
		numchips = 1;

	for(i = 0; i < numchips; i++)
	{
		writeops = 0;
		rd = NULL;

		/* 每个芯片存储自身bbt? */
		chipsel = (td->options & NAND_BBT_PERCHIP) ? i : -1;
		/* 存在镜像bbt? */
		if(md)
		{
		    /* 原始和镜像bbt都不存在 */
			if(td->pages[i] == -1 && md->pages[i] == -1)
			{
				writeops = 0x03;
				goto create;
			}

            /* 只存在镜像bbt */
			if(td->pages[i] == -1)
			{
				rd = md;
				td->version[i] = md->version[i];
				writeops = 1;
				goto writecheck;
			}

            /* 只存在原始bbt */
			if(md->pages[i] == -1)
			{
				rd = td;
				md->version[i] = td->version[i];
				writeops = 2;
				goto writecheck;
			}

            /* 都存在且版本都一样 */
			if(td->version[i] == md->version[i])
			{
				rd = td;
				goto writecheck;
			}


			if(((int8_t) (td->version[i] - md->version[i])) > 0)
			{
			    /* 都存在，且原始bbt版本较新 */
				rd = td;
				md->version[i] = td->version[i];
				writeops = 2;
			}
			else
			{
			    /* 都存在，且镜像bbt版本较新 */
				rd = md;
				td->version[i] = md->version[i];
				writeops = 1;
			}

			goto writecheck;

		}
		else
		{
		    /* 原始bbt不存在 */
			if(td->pages[i] == -1)
			{
				writeops = 0x01;
				goto create;
			}
			rd = td;
			goto writecheck;
		}

create:
		/* 扫描整个设备创建基于内存的bbt? */
		if(!(td->options & NAND_BBT_CREATE))
			continue;

		/* 扫描整个设备创建基于内存的bbt */
		create_bbt(mtd, buf, bd, chipsel);

		td->version[i] = 1;
		if(md)
            md->version[i] = 1;

writecheck:
		/* 读取现有的bbt? */
		if(rd)
			read_abs_bbt(mtd, buf, rd, chipsel);

		/* 写入原始bbt */
		if((writeops & 0x01) && (td->options & NAND_BBT_WRITE))
		{
			res = write_bbt(mtd, buf, td, md, chipsel);
			if(res < 0)
				return res;
		}

		/* 写入镜像bbt */
		if((writeops & 0x02) && md && (md->options & NAND_BBT_WRITE))
		{
			res = write_bbt(mtd, buf, md, td, chipsel);
			if(res < 0)
				return res;
		}
	}
	return 0;
}

/********************************************************************************
* 函数: static void mark_bbt_region(__in struct mtd_info *mtd,
                                   __in struct nand_bbt_desc *td)
* 描述: 把bbt所在的块标记为保留块0x02, 防止无意的擦写
* 输入: mtd: nandflash设备父类
       td: bbt区域描述符
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void mark_bbt_region(__in struct mtd_info *mtd, __in struct nand_bbt_desc *td)
{
	struct nand_chip *this = mtd->priv;
	int i, j, numchips, block, nrblocks, update;
	uint8_t oldval, newval;

	/* 每个芯片保存自己的bbt? */
	if(td->options & NAND_BBT_PERCHIP)
	{
		numchips = this->numchips;
		nrblocks = (int)(this->chipsize >> this->bbt_erase_shift);
	}
	else
	{
		numchips = 1;
		nrblocks = (int)(mtd->size >> this->bbt_erase_shift);
	}

	for(i = 0; i < numchips; i++)
	{
		if((td->options & NAND_BBT_ABSPAGE) || !(td->options & NAND_BBT_WRITE))
        {
			if(td->pages[i] == -1)
				continue;

            /* 计算bbt所在的块 */
			block = td->pages[i] >> (this->bbt_erase_shift - this->page_shift);
			block <<= 1;
			oldval = this->bbt[(block >> 3)];
			newval = oldval | (0x2 << (block & 0x06));
			this->bbt[(block >> 3)] = newval;
			if((oldval != newval) && td->reserved_block_code)
				nand_update_bbt(mtd, (loff_t)block << (this->bbt_erase_shift - 1));
			continue;
		}
		update = 0;

        /* 寻找bbt所在的块 */
		if (td->options & NAND_BBT_LASTBLOCK)
			block = ((i + 1) * nrblocks) - td->maxblocks;
		else
			block = i * nrblocks;
		block <<= 1;

		for(j = 0; j < td->maxblocks; j++)
		{
		    /* 把最多寻找的bbt块全部标记为保留 */
			oldval = this->bbt[(block >> 3)];
			newval = oldval | (0x2 << (block & 0x06));
			this->bbt[(block >> 3)] = newval;
			if(oldval != newval)
				update = 1;
			block += 2;
		}
		/* 更新bbt块标记 */
		if(update && td->reserved_block_code)
			nand_update_bbt(mtd, (loff_t)(block - 2) << (this->bbt_erase_shift - 1));
	}
}

/********************************************************************************
* 函数: int32_t nand_scan_bbt(__in struct mtd_info *mtd,
                             __in struct nand_bbt_desc *bd)
* 描述: 扫描设备并创建bbt
* 输入: mtd: nandflash设备父类
       bd: bbt区域描述符
* 输出: none
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t nand_scan_bbt(__in struct mtd_info *mtd, __in struct nand_bbt_desc *bd)
{
	struct nand_chip *this = mtd->priv;
	int len, res = 0;
	uint8_t *buf;
	struct nand_bbt_desc *td = this->bbt_td;
	struct nand_bbt_desc *md = this->bbt_md;

    /* 计算bbt最长的长度 */
	len = mtd->size >> (this->bbt_erase_shift + 2);
	/* 分配空间 */
	this->bbt = dlmalloc(len);
	if(!this->bbt)
	{
		printl(LOG_LEVEL_ERR, "[NANDBBT:ERR] Out of memory\n");
		return -ENOMEM;
	}

	/* 分配可以保存一块数据大小的临时空间 */
	len = (1 << this->bbt_erase_shift);
	len += (len >> this->page_shift) * mtd->oobsize;
	buf = dlmalloc(len);
	if(!buf)
	{
		printl(LOG_LEVEL_ERR, "[NANDBBT:ERR] Out of memory\n");
		dlfree((int8_t *)(this->bbt));
		this->bbt = NULL;
		return -ENOMEM;
	}

	/* bbt存放在指定位置? */
	if (td->options & NAND_BBT_ABSPAGE)
	{
		res = read_abs_bbts(mtd, buf, td, md);
	}
	else
	{
		/* 使用标记符号寻找bbt */
		res = search_read_bbts(mtd, buf, td, md);
	}

    /* 创建bbt */
	if(res)
		res = check_create(mtd, buf, bd);

	/* 防止bbt区域被擦除或者写入其他数据 */
	mark_bbt_region(mtd, td);
	if(md)
		mark_bbt_region(mtd, md);

	dlfree((int8_t *)buf);
	return res;
}


/********************************************************************************
* 函数: int32_t nand_update_bbt(__in struct mtd_info *mtd, __in loff_t offs)
* 描述: 更新bbt,此时内存中已经建立好bbt表
* 输入: mtd: nandflash设备父类
       offs: bbt数据存储在nandflash中的起始地址, 在存在NAND_BBT_PERCHIP时有效
* 输出: none
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t nand_update_bbt(__in struct mtd_info *mtd, __in loff_t offs)
{
	struct nand_chip *this = mtd->priv;
	int32_t len, res = 0, writeops = 0;
	int32_t chip, chipsel;
	uint8_t *buf;
	struct nand_bbt_desc *td = this->bbt_td;
	struct nand_bbt_desc *md = this->bbt_md;

	if(!this->bbt || !td)
        return -EINVAL;

	/* 计算一块的数据长度,分配内存 */
	len = (1 << this->bbt_erase_shift);
	len += (len >> this->page_shift) * mtd->oobsize;
	buf = dlmalloc(len);

	if(!buf)
	{
		printl(LOG_LEVEL_ERR, "[NANDBBT:ERR] Out of memory\n");
		return -ENOMEM;
	}

	writeops = md != NULL ? 0x03 : 0x01;

	/* 每个芯片存储自身的BBT? */
	if (td->options & NAND_BBT_PERCHIP)
	{
		chip = (int32_t)(offs >> this->chip_shift);
		chipsel = chip;
	}
	else
	{
		chip = 0;
		chipsel = -1;
	}

	td->version[chip]++;
	if(md)
		md->version[chip]++;

	/* 写原始bbt? */
	if((writeops & 0x01) && (td->options & NAND_BBT_WRITE))
	{
		res = write_bbt(mtd, buf, td, md, chipsel);
		if(res < 0)
			goto out;
	}

	/* 写镜像bbt? */
	if((writeops & 0x02) && md && (md->options & NAND_BBT_WRITE))
	{
		res = write_bbt(mtd, buf, md, td, chipsel);
	}

out:
	dlfree((int8_t *)buf);
	return res;
}




/********************************************************************************
* 函数: int32_t nand_default_bbt(__in struct mtd_info *mtd)
* 描述: 扫描nandflash设备建立bbt
* 输入: mtd: nandflash设备父类
* 输出: none
* 返回: 0: 成功
       !0: 失败
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t nand_default_bbt(__in struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;

    if(this->options & NAND_USE_FLASH_BBT)
    {
        /* 基于flash的bbt */
		if(!this->bbt_td)
		{
			this->bbt_td = &bbt_main_desc;
			this->bbt_md = &bbt_mirror_desc;
		}

		if(!this->badblock_pattern)
		{
			this->badblock_pattern = (mtd->writesize > 512) ? &largepage_flashbased : &smallpage_flashbased;
		}
    }
    else
    {
        /* 基于内存的bbt */
        this->bbt_td = NULL;
		this->bbt_md = NULL;
		if(!this->badblock_pattern)
		{
			this->badblock_pattern = (mtd->writesize > 512) ? &largepage_memorybased : &smallpage_memorybased;
		}
    }

    return nand_scan_bbt(mtd, this->badblock_pattern);
}








