#include "stddef.h"
#include "list.h"
#include "malloc.h"
#include "mtd/mtd.h"
#include "mtd/mtd_partitions.h"
#include "errno.h"
#include "compiler.h"
#include "log.h"


struct list_head mtd_partitions;

/* 单独的每个分区结构体 */
/* 分区的所有操作都是继承主分区的操作*/
struct mtd_part
{
    struct mtd_info mtd;          /* 分区信息，大部分由master决定 */
    struct mtd_info *master;      /* 分区的主分区，就是整个mtd设备 */
    uint64_t offset;              /* 分区的偏移地址 */
    int32_t index;                /* 分区号 */
    struct list_head list;        /* 分区链表 */
    int32_t registered;           /* 分区注册标记，分区注册后才有效 */
};


#define PART(x)   ((struct mtd_part *)(x))

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_read(__in struct mtd_info *mtd, __in loff_t from, __in size_t len,
                           __out size_t *retlen, __out uint8_t *buf)
{
    struct mtd_part *part = PART(mtd);
	struct mtd_ecc_stats stats;
	int32_t res;

	stats = part->master->ecc_stats;

    /* 检验参数是否有效 */
	if(from >= mtd->size)
        len = 0;
	else if(from + len > mtd->size)
		len = mtd->size - from;

	res = part->master->read(part->master, from + part->offset, len, retlen, buf);
	if(unlikely(res))
	{
        /* 可能读好几个页，所以可能校准和失败的值不止1 */
		if(res == -EUCLEAN)
			mtd->ecc_stats.corrected += part->master->ecc_stats.corrected - stats.corrected;

		if(res == -EBADMSG)
			mtd->ecc_stats.failed += part->master->ecc_stats.failed - stats.failed;
	}

	return res;
}



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_read_oob(__in struct mtd_info *mtd, __in loff_t from,
                               __inout struct mtd_oob_ops *ops)
{
	struct mtd_part *part = PART(mtd);
	int32_t res;

    /* 检验参数是否有效 */
	if(from >= mtd->size)
		return -EINVAL;

	if(ops->databuf && (from + ops->len > mtd->size))
		return -EINVAL;

	res = part->master->read_oob(part->master, from + part->offset, ops);

	if(unlikely(res))
	{
	    /* 只会读一个oob区，所以校准或失败可以直接只加1 */
		if(res == -EUCLEAN)
			mtd->ecc_stats.corrected++;

		if(res == -EBADMSG)
			mtd->ecc_stats.failed++;
	}
	return res;
}





/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_read_user_prot_reg(__in struct mtd_info *mtd, __in loff_t from,
                                         __in size_t len, __out size_t *retlen,
                                         __out uint8_t *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->read_user_prot_reg(part->master, from, len, retlen, buf);
}



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_get_user_prot_info(__in struct mtd_info *mtd, __out struct otp_info *buf,
                                         __in size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->get_user_prot_info(part->master, buf, len);
}



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_read_fact_prot_reg(__in struct mtd_info *mtd, __in loff_t from,
                                         __in size_t len, __out size_t *retlen,
                                         __out uint8_t *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->read_fact_prot_reg(part->master, from, len, retlen, buf);
}

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_get_fact_prot_info(__in struct mtd_info *mtd,
                                         __out struct otp_info *buf, __in size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->get_fact_prot_info(part->master, buf, len);
}



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_write(__in struct mtd_info *mtd, __in loff_t to, __in size_t len,
                            __out size_t *retlen, __in const uint8_t *buf)
{
	struct mtd_part *part = PART(mtd);

	/* 检测设备是否只读 */
	if(!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;

    /* 检测参数是否有效 */
	if(to >= mtd->size)
		len = 0;
	else if((to + len) > mtd->size)
		len = mtd->size - to;

	return part->master->write(part->master, to + part->offset, len, retlen, buf);
}



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_panic_write(__in struct mtd_info *mtd, __in loff_t to, __in size_t len,
                                  __out size_t *retlen, __in const uint8_t *buf)
{
	struct mtd_part *part = PART(mtd);

    /* 检测设备是否只读 */
	if(!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;

    /* 检测参数是否有效 */
	if(to >= mtd->size)
		len = 0;
	else if((to + len) > mtd->size)
		len = mtd->size - to;

	return part->master->panic_write(part->master, to + part->offset, len, retlen, buf);
}





/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_write_oob(__in struct mtd_info *mtd, __in loff_t to,
                                __in struct mtd_oob_ops *ops)
{
	struct mtd_part *part = PART(mtd);

    /* 检测设备是否只读 */
	if(!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;

    /* 检测参数是否有效 */
	if(to >= mtd->size)
		return -EINVAL;
	if(ops->databuf && (to + ops->len) > mtd->size)
		return -EINVAL;

	return part->master->write_oob(part->master, to + part->offset, ops);
}



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_write_user_prot_reg(__in struct mtd_info *mtd, __in loff_t from,
		                                  __in size_t len, __in size_t *retlen,
		                                  __in uint8_t *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->write_user_prot_reg(part->master, from, len, retlen, buf);
}




/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_lock_user_prot_reg(__in struct mtd_info *mtd, __in loff_t from,
		                                 __in size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->lock_user_prot_reg(part->master, from, len);
}



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_erase(__in struct mtd_info *mtd, __in struct erase_info *instr)
{
	struct mtd_part *part = PART(mtd);
	int32_t ret;

    /* 检测参数是否有效 */
	if(!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	if(instr->addr >= mtd->size)
		return -EINVAL;

	instr->addr += part->offset;
	ret = part->master->erase(part->master, instr);
	if(ret)
	{
		if(instr->fail_addr != MTD_FAIL_ADDR_UNKNOWN)
            instr->fail_addr -= part->offset;

		instr->addr -= part->offset;
	}
	return ret;
}





/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
void mtd_erase_callback(__in struct erase_info *instr)
{
	if (instr->mtd->erase == part_erase)
	{
		struct mtd_part *part = PART(instr->mtd);

		if (instr->fail_addr != MTD_FAIL_ADDR_UNKNOWN)
			instr->fail_addr -= part->offset;
		instr->addr -= part->offset;
	}

	if (instr->callback)
		instr->callback(instr);
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_lock(__in struct mtd_info *mtd, loff_t ofs, __in uint64_t len)
{
	struct mtd_part *part = PART(mtd);

	/* 检测参数是否有效 */
	if ((len + ofs) > mtd->size)
		return -EINVAL;

	return part->master->lock(part->master, ofs + part->offset, len);
}




/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_unlock(__in struct mtd_info *mtd, __in loff_t ofs, __in uint64_t len)
{
	struct mtd_part *part = PART(mtd);

	/* 检测参数是否有效 */
	if ((len + ofs) > mtd->size)
		return -EINVAL;

	return part->master->unlock(part->master, ofs + part->offset, len);
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static void part_sync(__in struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	part->master->sync(part->master);
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_suspend(__in struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	return part->master->suspend(part->master);
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static void part_resume(__in struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	part->master->resume(part->master);
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_block_isbad(__in struct mtd_info *mtd, __in loff_t ofs)
{
	struct mtd_part *part = PART(mtd);

	/* 检测参数是否有效 */
	if(ofs >= mtd->size)
		return -EINVAL;
	ofs += part->offset;
	return part->master->block_isbad(part->master, ofs);
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t part_block_markbad(__in struct mtd_info *mtd, __in loff_t ofs)
{
	struct mtd_part *part = PART(mtd);
	int32_t res;

    /* 检测参数是否有效 */
	if(!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	if(ofs >= mtd->size)
		return -EINVAL;

	ofs += part->offset;
	res = part->master->block_markbad(part->master, ofs);
	if(!res)
		mtd->ecc_stats.badblocks++;
	return res;
}



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t del_mtd_partitions(__in struct mtd_info *master)
{
	struct mtd_part *slave, *next;

	list_for_each_entry_safe(slave, next, &mtd_partitions, list)
	{
        if (slave->master == master)
        {
			list_del(&slave->list);
			if(slave->registered)
				del_mtd_device(&slave->mtd);

			dlfree((int8_t *)slave);
		}
	}

	return 0;
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static struct mtd_part *add_one_partition(__in struct mtd_info *master,
                                            __in const struct mtd_partition *part,
                                            __in int32_t partno,
                                            __in uint64_t cur_offset)
{
	struct mtd_part *slave;

	/* 分配分区空间 */
	slave = dlcalloc(sizeof(struct mtd_part), 1);
	if(!slave)
	{
		printl(LOG_LEVEL_ERR, "[MTD:ERR] memory allocation error while creating partitions for \"%s\"\n", master->name);
		del_mtd_partitions(master);
		return NULL;
	}
	list_add(&slave->list, &mtd_partitions);

	/* 设置分区参数 */
	slave->mtd.type = master->type;
	slave->mtd.flags = master->flags & (~(part->mask_flags));
	slave->mtd.size = part->size;
	slave->mtd.writesize = master->writesize;
	slave->mtd.oobsize = master->oobsize;
	slave->mtd.oobavail = master->oobavail;
	slave->mtd.subpage_sft = master->subpage_sft;

	slave->mtd.name = part->name;

	slave->mtd.read = part_read;
	slave->mtd.write = part_write;

	if(master->panic_write)
		slave->mtd.panic_write = part_panic_write;

	if(master->read_oob)
		slave->mtd.read_oob = part_read_oob;
	if(master->write_oob)
		slave->mtd.write_oob = part_write_oob;
	if(master->read_user_prot_reg)
		slave->mtd.read_user_prot_reg = part_read_user_prot_reg;
	if(master->read_fact_prot_reg)
		slave->mtd.read_fact_prot_reg = part_read_fact_prot_reg;
	if(master->write_user_prot_reg)
		slave->mtd.write_user_prot_reg = part_write_user_prot_reg;
	if(master->lock_user_prot_reg)
		slave->mtd.lock_user_prot_reg = part_lock_user_prot_reg;
	if(master->get_user_prot_info)
		slave->mtd.get_user_prot_info = part_get_user_prot_info;
	if(master->get_fact_prot_info)
		slave->mtd.get_fact_prot_info = part_get_fact_prot_info;
	if(master->sync)
		slave->mtd.sync = part_sync;

	if(!partno && master->suspend && master->resume)
	{
        slave->mtd.suspend = part_suspend;
        slave->mtd.resume = part_resume;
	}
	if(master->lock)
		slave->mtd.lock = part_lock;
	if(master->unlock)
		slave->mtd.unlock = part_unlock;
	if(master->block_isbad)
		slave->mtd.block_isbad = part_block_isbad;
	if(master->block_markbad)
		slave->mtd.block_markbad = part_block_markbad;
	slave->mtd.erase = part_erase;
	slave->master = master;
	slave->offset = part->offset;
	slave->index = partno;

	if(slave->offset == MTDPART_OFS_APPEND)
		slave->offset = cur_offset;
	if(slave->offset == MTDPART_OFS_NXTBLK)
	{
		slave->offset = cur_offset;
		if(mtd_mod_by_eb(cur_offset, master) != 0)
		{
			/* 按擦除块对齐 */
			slave->offset = (mtd_div_by_eb(cur_offset, master) + 1) * master->erasesize;
			printl(LOG_LEVEL_INFO, "[MTD:INFO] Moving partition %d: 0x%012llx -> 0x%012llx\n", partno,
			                        (uint64_t)cur_offset, (uint64_t)slave->offset);
		}
	}

	if (slave->mtd.size == MTDPART_SIZ_FULL)
		slave->mtd.size = master->size - slave->offset;

	printl(LOG_LEVEL_INFO, "[MTD:INFO] 0x%012llx-0x%012llx: \"%s\"\n", (uint64_t)slave->offset,
		                    (uint64_t)(slave->offset + slave->mtd.size), slave->mtd.name);

	/* 检测参数 */
	if (slave->offset >= master->size)
	{
		/* 设备起始地址越界，注册设备保持分区顺序 */
		slave->offset = 0;
		slave->mtd.size = 0;
		printl(LOG_LEVEL_ERR, "[MTD:ERR] partition \"%s\" is out of reach -- disabled\n", part->name);
		goto out_register;
	}

	if(slave->offset + slave->mtd.size > master->size)
	{
        /* 设备结束地址越界，截断设备 */
		slave->mtd.size = master->size - slave->offset;
		printl(LOG_LEVEL_WARN, "[MTD:WARN] partition \"%s\" extends beyond the end of device \"%s\" -- size truncated to %#llx\n",
			                    part->name, master->name, (uint64_t)slave->mtd.size);
	}

    /* 有不同擦除大小区域 */
	if(master->numeraseregions > 1)
	{
		int32_t i, max = master->numeraseregions;
		uint64_t end = slave->offset + slave->mtd.size;
		struct mtd_erase_region_info *regions = master->eraseregions;

		/* 找到此设备的第一块擦除区域 */
		for(i = 0; i < max && regions[i].offset <= slave->offset; i++);
		/* 循环找到的是第一块擦除区域的下一位置 */
		i--;

		/* 选取最大的擦除大小 */
		for(; i < max && regions[i].offset < end; i++)
		{
			if(slave->mtd.erasesize < regions[i].erasesize)
			{
				slave->mtd.erasesize = regions[i].erasesize;
			}
		}
		BUG_ON(slave->mtd.erasesize == 0);
	}
	else
	{
		/* 擦除大小相同 */
		slave->mtd.erasesize = master->erasesize;
	}

	if((slave->mtd.flags & MTD_WRITEABLE) && mtd_mod_by_eb(slave->offset, &slave->mtd))
	{
        /* 子设备起始地址不是擦除边界对齐，强制设置为只读 */
		slave->mtd.flags &= ~MTD_WRITEABLE;
		printl(LOG_LEVEL_WARN, "[MTD:WARN] partition \"%s\" doesn't start on an erase block boundary -- force read-only\n",
			                    part->name);
	}

	if((slave->mtd.flags & MTD_WRITEABLE) && mtd_mod_by_eb(slave->mtd.size, &slave->mtd))
	{
	    /* 子设备大小不是擦除边界对齐，强制设置为只读 */
		slave->mtd.flags &= ~MTD_WRITEABLE;
		printl(LOG_LEVEL_WARN, "[MTD:WARN] partition \"%s\" doesn't end on an erase block -- force read-only\n",
			                    part->name);
	}

    /* 坏块统计 */
	slave->mtd.ecclayout = master->ecclayout;
	if(master->block_isbad)
	{
		uint64_t offs = 0;

		while (offs < slave->mtd.size)
		{
			if (master->block_isbad(master, offs + slave->offset))
				slave->mtd.ecc_stats.badblocks++;
			offs += slave->mtd.erasesize;
		}
	}

out_register:

	/* 注册分区 */
    add_mtd_device(&slave->mtd);
    slave->registered = 1;
	return slave;
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t add_mtd_partitions(__in struct mtd_info *master,
                            __in const struct mtd_partition *parts,
		                    __in int32_t nbparts)
{
	struct mtd_part *slave;
	uint64_t cur_offset = 0;
	int32_t i;

    /* 检测链表现在是否空闲 */
	if (mtd_partitions.next == NULL)
		INIT_LIST_HEAD(&mtd_partitions);  /* 初始化链表头 */

	printl(LOG_LEVEL_INFO, "Creating %d MTD partitions on \"%s\":\n", nbparts, master->name);

	for (i = 0; i < nbparts; i++)
	{
		slave = add_one_partition(master, parts + i, i, cur_offset);
		if(!slave)
			return -ENOMEM;
		cur_offset = slave->offset + slave->mtd.size;
	}

	return 0;
}
