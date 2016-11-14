#include "stddef.h"
#include "mtd/mtd.h"
#include "mtd/mtd_abi.h"
#include "errno.h"
#include "malloc.h"
#include "compiler.h"
#include "log.h"



/* mtd设备链接结构体 */
struct mtd_concat
{
    /* 头设备，保存所有子设备集合起来的总信息 */
    struct mtd_info mtd;
    /* 子设备数量 */
    int32_t num_subdev;
    /* 子设备指针数组 */
    struct mtd_info **subdev;
};

/* concat结构体大小 */
#define SIZEOF_STRUCT_MTD_CONCAT(num_subdev) \
    (sizeof(struct mtd_concat) + (num_subdev) * sizeof(struct mtd_info *))

/* 获取concat设备的结构体 */
#define CONCAT(x) ((struct mtd_concat *)(x))


/********************************************************************************
* 函数: static int32_t concat_read(__in struct mtd_info *mtd,
                                  __in loff_t from,
                                  __in size_t len,
                                  __out size_t *retlen,
                                  __out uint8_t *buf)
* 描述: 从链接的mtd设备中读取数据
* 输入: mtd: mtd链中的头设备
       from: 读取的起始地址
       len: 读取的长度
* 输出: retlen: 读取到的字节
       buf: 读取到的字节存放缓冲区
* 返回: 0: 读取成功
       -EBADMSG:
       -EINVAL: from参数无效
       -EUCLEAN:
       其他: 未知错误
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t concat_read(__in struct mtd_info *mtd, __in loff_t from, __in size_t len,
                             __out size_t *retlen, __out uint8_t *buf)
{
    struct mtd_concat *concat = CONCAT(mtd);
    struct mtd_info *subdev;
    size_t size = 0, retsize;
    int32_t i = 0, err = 0, ret = 0;

    *retlen = 0;

    for(i = 0; i < concat->num_subdev; i++)
    {
        subdev = concat->subdev[i];

        //找到from地址所在的起始mtd设备
        if(from >= subdev->size)
        {
            from -= subdev->size;
            continue;
        }

        if((from + len) > subdev->size)
            size = subdev->size - from;
        else
            size = len;

        err = subdev->read(subdev, from, size, &retsize, buf);

        if(unlikely(err))
        {
            if(err == -EBADMSG)
            {
                mtd->ecc_stats.failed++;
                ret = err;
            }
            else if(err == -EUCLEAN)
            {
                mtd->ecc_stats.corrected++;
                if(!ret)
                    ret = err;
            }
            else
                return err;
        }

        *retlen += retsize;
        len -= size;
        if(len == 0)
            return ret;

        buf += size;
        from = 0;
    }

    return -EINVAL;
}

/********************************************************************************
* 函数: static int32_t concat_write(__in struct mtd_info *mtd,
                                   __in loff_t to,
                                   __in size_t len,
                                   __out size_t *retlen,
                                   __in const uint8_t *buf)
* 描述: 往链接的mtd设备中写数据
* 输入: mtd: mtd设备链中的头设备
       to: 写入的地址
       len: 写入的长度
       buf: 需要写入的数据缓冲区
* 输出: retlen: 成功写入的数据长度
* 返回: 0: 读取成功
       -EBADMSG:
       -EINVAL: to参数无效
       -EUCLEAN:
       其他: 未知错误
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t concat_write(__in struct mtd_info *mtd, __in loff_t to, __in size_t len,
                              __out size_t *retlen, __in const uint8_t *buf)

{
    struct mtd_concat *concat = CONCAT(mtd);
    struct mtd_info *subdev = NULL;
    int32_t err = -EINVAL, size = 0;
    size_t retsize = 0;
    int32_t i = 0;

    //只读设备
    if(!(mtd->flags & MTD_WRITEABLE))
        return -EROFS;

    *retlen = 0;

    for(i = 0; i < concat->num_subdev; i++)
    {
        subdev = concat->subdev[i];

        if(to >= subdev->size)
        {
            to -= subdev->size;
            continue;
        }

        if((to + len) > subdev->size)
            size = subdev->size - to;
        else
            size = len;

        /* 只读设备 */
        if(!(subdev->flags & MTD_WRITEABLE))
            err = -EROFS;
        else
            err = subdev->write(subdev, to, size, &retsize, buf);

        if(err)
            break;

        *retlen += retsize;
        len -= size;
        if(len == 0)
            return err;

        err = -EINVAL;
        buf += size;
        to = 0;
    }

    return err;
}

/********************************************************************************
* 函数: static int32_t concat_read_oob(__in mtd_info *mtd,
                                      __in loff_t from,
                                      __inout struct mtd_oob_ops *ops)
* 描述: 读取mtd链的oob数据
* 输入: mtd: mtd设备链中的头设备
       from: 读取的起始地址
       ops: 需要读取的oob数据
* 输出: ops: 读取到的oob数据
* 返回: 0: 读取成功
       -EBADMSG:
       -EINVAL: from参数无效
       -EUCLEAN:
       其他: 未知错误
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t concat_read_oob(__in struct mtd_info *mtd, __in loff_t from,
                                 __in struct mtd_oob_ops *ops)
{
    struct mtd_concat *concat = CONCAT(mtd);
    struct mtd_info *subdev = NULL;
    struct mtd_oob_ops devops = *ops;
    int32_t i = 0, err = 0, ret = 0;

    ops->retlen = ops->oobretlen = 0;

    for(i = 0; i < concat->num_subdev; i++)
    {
        subdev = concat->subdev[i];

        if(from >= subdev->size)
        {
            from -= subdev->size;
            continue;
        }

        if((from + ops->len) > subdev->size)
            devops.len = subdev->size - from;

        err = subdev->read_oob(subdev, from, &devops);
        ops->retlen += devops.retlen;
        ops->oobretlen += devops.oobretlen;

        if(unlikely(err))
        {
            if(err == -EBADMSG)
            {
                mtd->ecc_stats.failed++;
                ret = err;
            }
            else if(err == -EUCLEAN)
            {
                mtd->ecc_stats.corrected++;
                if(!ret)
                    ret = err;
            }
            else
                return err;
        }

        if(devops.databuf)
        {
            devops.len = ops->len - ops->retlen;
            if(!devops.len)
                return ret;
            devops.databuf += devops.retlen;
        }
        if(devops.oobbuf)
        {
            devops.ooblen = ops->ooblen - ops->oobretlen;
            if(!devops.ooblen)
                return ret;
            devops.oobbuf += devops.oobretlen;
        }

        from = 0;
    }

    return -EINVAL;
}

/********************************************************************************
* 函数: static int32_t concat_write_oob(__in struct mtd_info *mtd,
                                       __in loff_t to,
                                       __in struct mtd_oob_ops *ops)
* 描述: 写oob数据到mtd设备链中
* 输入: mtd: mtd设备链中的头设备
       to: 需要写入的地址
       ops: oob数据结构体
* 输出: none
* 返回: 0: 读取成功
       -EBADMSG:
       -EINVAL: to参数无效
       -EUCLEAN:
       其他: 未知错误
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t concat_write_oob(__in struct mtd_info *mtd, __in loff_t to,
                                  __in struct mtd_oob_ops *ops)
{
    struct mtd_concat *concat = CONCAT(mtd);
    struct mtd_info *subdev = NULL;
    struct mtd_oob_ops devops = *ops;
    int32_t i = 0, err = 0;

    /* 只读文件系统 */
    if(!(mtd->flags & MTD_WRITEABLE))
        return -EROFS;

        ops->retlen = 0;

    for(i = 0; i < concat->num_subdev; i++)
    {
        subdev = concat->subdev[i];

        if(to >= subdev->size)
        {
            to -= subdev->size;
            continue;
        }

        if((to + devops.len) > subdev->size)
            devops.len = subdev->size - to;

        err = subdev->write_oob(subdev, to, &devops);
        ops->retlen += devops.retlen;

        if(err)
            return err;


		if(devops.databuf)
		{
			devops.len = ops->len - ops->retlen;
			if (!devops.len)
				return 0;
			devops.databuf += devops.retlen;
		}
		if(devops.oobbuf)
		{
			devops.ooblen = ops->ooblen - ops->oobretlen;
			if (!devops.ooblen)
				return 0;
			devops.oobbuf += devops.oobretlen;
		}
		to = 0;
    }

    return -EINVAL;
}

/********************************************************************************
* 函数: static void concat_erase_callback(__in struct erase_info *instr)
* 描述: mtd设备链擦除回调函数
* 输入: instr: 擦除信息
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void concat_erase_callback(__in struct erase_info *instr)
{

}

/********************************************************************************
* 函数: static int32_t concat_dev_erase(__in struct mtd_info *mtd,
                                       __inout struct erase_info *erase)
* 描述: 擦除mtd设备
* 输入: mtd: 需要擦除的mtd设备
       erase: 擦除方法
* 输出: erase: 擦除的结果
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t concat_dev_erase(__in struct mtd_info *mtd, __inout struct erase_info *erase)
{
    int32_t err;

    erase->mtd = mtd;
    erase->callback = concat_erase_callback;
    erase->priv = 0;

    err = mtd->erase(mtd, erase);
    if(!err)
        err = (erase->state == MTD_ERASE_FAILED) ? -EIO : 0;

    return err;
}

/********************************************************************************
* 函数: static int32_t concat_erase(__in struct mtd_info *mtd,
                                   __inout struct erase_info *instr)
* 描述: 擦除mtd设备链
* 输入: mtd: mtd设备链中的头设备
       instr: 擦除的信息结构体
* 输出: instr: 擦除的信息结构体
* 返回: 0: 读取成功
       -EBADMSG:
       -EINVAL: to参数无效
       -EUCLEAN:
       其他: 未知错误
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t concat_erase(__in struct mtd_info *mtd, __inout struct erase_info *instr)
{
    struct mtd_concat *concat = CONCAT(mtd);
    struct mtd_info *subdev;
    struct erase_info *erase;
    struct mtd_erase_region_info *erase_regions = NULL;
    int32_t i = 0;
    uint64_t length = 0, offset = 0;

    if(!(mtd->flags & MTD_WRITEABLE))
        return -EROFS;

    if(instr->addr > concat->mtd.size)
        return -EINVAL;

    if((instr->addr + instr->len) > concat->mtd.size)
        return -EINVAL;

    if(!mtd->numeraseregions)
    {
        /* 所有子设备拥有同样的擦除大小 */
        /* 检测地址和长度是否对齐 */
        if (instr->addr & (concat->mtd.erasesize - 1))
			return -EINVAL;
		if (instr->len & (concat->mtd.erasesize - 1))
			return -EINVAL;
    }
    else
    {
        /* 检测分块擦除区域是否对齐 */
        erase_regions = concat->mtd.eraseregions;

		for(i = 0; (i < concat->mtd.numeraseregions) && (instr->addr >= erase_regions[i].offset); i++);
		--i;

		if(instr->addr & (erase_regions[i].erasesize - 1))
			return -EINVAL;

		for(; (i < concat->mtd.numeraseregions) && ((instr->addr + instr->len) >= erase_regions[i].offset); ++i);
		--i;

		if((instr->addr + instr->len) & (erase_regions[i].erasesize - 1))
			return -EINVAL;
    }

    instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;

    erase = dlmalloc(sizeof(struct erase_info));

    if(!erase)
        return -ENOMEM;

    *erase = *instr;
    length = instr->len;  //需要擦除的总长度

    /* 找到起始擦除的区域 */
	for(i = 0; i < concat->num_subdev; i++)
	{
		subdev = concat->subdev[i];
		if(erase->addr >= subdev->size)
		{
			erase->addr -= subdev->size;
			offset += subdev->size;
		}
		else
			break;
	}

	/* 出现bug */
	BUG_ON(i >= concat->num_subdev);

	/* 开始正式擦除 */
	int32_t err = 0;
	for (; length > 0; i++)
	{
		/* 获取需要擦除的子设备 */
		subdev = concat->subdev[i];

		/* 计算擦除的大小 */
		if((erase->addr + length) > subdev->size)
			erase->len = subdev->size - erase->addr;
		else
			erase->len = length;

		if(!(subdev->flags & MTD_WRITEABLE))
		{
			err = -EROFS;
			break;
		}

		length -= erase->len;
		if((err = concat_dev_erase(subdev, erase)))
		{
			if (erase->fail_addr != MTD_FAIL_ADDR_UNKNOWN)
				instr->fail_addr = erase->fail_addr + offset;
			break;
		}

		erase->addr = 0;
		offset += subdev->size;
	}

	instr->state = erase->state;
	dlfree((int8_t *)erase);
	if(err)
		return err;

	if (instr->callback)
		instr->callback(instr);
	return 0;
}

/********************************************************************************
* 函数: static int32_t concat_lock(__in struct mtd_info *mtd,
                                  __in loff_t ofs,
                                  __in uint64_t len)
* 描述: 锁住mtd链中的设备
* 输入: mtd: mtd设备链中的头设备
       ofs: 需要锁定的地址
       len: 需要锁定的长度
* 输出: none
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t concat_lock(__in struct mtd_info *mtd, __in loff_t ofs, __in uint64_t len)
{
    struct mtd_concat *concat = CONCAT(mtd);
    int32_t i = 0, err = -EINVAL;
    struct mtd_info *subdev = NULL;
    uint64_t size = 0;

	if ((len + ofs) > mtd->size)
		return -EINVAL;

	for (i = 0; i < concat->num_subdev; i++)
	{
        subdev = concat->subdev[i];

		if (ofs >= subdev->size)
		{
			ofs -= subdev->size;
			continue;
		}

		if (ofs + len > subdev->size)
			size = subdev->size - ofs;
		else
			size = len;

		err = subdev->lock(subdev, ofs, size);

		if(err)
			break;

		len -= size;
		if (len == 0)
			break;

		err = -EINVAL;
		ofs = 0;
	}

	return err;
}

/********************************************************************************
* 函数: static int concat_unlock(__in struct mtd_info *mtd,
                                __in loff_t ofs,
                                __in uint64_t len)
* 描述: 解锁mtd设备链设备
* 输入: mtd: mtd设备链中的头设备
       ofs: 解锁的地址
       len: 解锁的长度
* 输出: none
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int concat_unlock(__in struct mtd_info *mtd, __in loff_t ofs, __in uint64_t len)
{
	struct mtd_concat *concat = CONCAT(mtd);
	struct mtd_info *subdev = NULL;
	uint64_t size = 0;
	int i, err = 0;

	if((len + ofs) > mtd->size)
		return -EINVAL;

	for(i = 0; i < concat->num_subdev; i++)
	{
		subdev = concat->subdev[i];

		if(ofs >= subdev->size)
		{
			size = 0;
			ofs -= subdev->size;
			continue;
		}
		if((ofs + len) > subdev->size)
			size = subdev->size - ofs;
		else
			size = len;

		err = subdev->unlock(subdev, ofs, size);

		if(err)
			break;

		len -= size;
		if(len == 0)
			break;

		err = -EINVAL;
		ofs = 0;
	}

	return err;
}

/********************************************************************************
* 函数: static void concat_sync(__in struct mtd_info *mtd)
* 描述: 同步mtd设备链的设备
* 输入: mtd: mtd设备链中的头设备
* 输出: none
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static void concat_sync(__in struct mtd_info *mtd)
{
	struct mtd_concat *concat = CONCAT(mtd);
	struct mtd_info *subdev = NULL;
	int i;

	for (i = 0; i < concat->num_subdev; i++)
	{
		subdev = concat->subdev[i];
		subdev->sync(subdev);
	}
}

/********************************************************************************
* 函数: static int32_t concat_block_isbad(__in struct mtd_info *mtd,
                                         __in loff_t ofs)
* 描述: 检测mtd设备链中指定地址是否是坏块
* 输入: mtd: mtd设备链中的头设备
       ofs: 检测的编译地址
* 输出: none
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t concat_block_isbad(__in struct mtd_info *mtd, __in loff_t ofs)
{
	struct mtd_concat *concat = CONCAT(mtd);
	struct mtd_info *subdev = NULL;
	int i, res = -EINVAL;

    //检测函数指针是否有效
	if (!concat->subdev[0]->block_isbad)
		return -EINVAL;

	if (ofs > mtd->size)
		return -EINVAL;

	for (i = 0; i < concat->num_subdev; i++)
	{
		subdev = concat->subdev[i];

		if (ofs >= subdev->size)
		{
			ofs -= subdev->size;
			continue;
		}

		res = subdev->block_isbad(subdev, ofs);
		break;
	}

	return res;
}


/********************************************************************************
* 函数: static int concat_block_markbad(__in struct mtd_info *mtd,
                                       __in loff_t ofs)
* 描述: 标记mtd设备链中的设备的坏块
* 输入: mtd: mtd设备链中的头设备
       ofs: 需要标记坏块的地址
* 输出: none
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int concat_block_markbad(__in struct mtd_info *mtd, __in loff_t ofs)
{
	struct mtd_concat *concat = CONCAT(mtd);
	struct mtd_info *subdev = NULL;
	int i, err = -EINVAL;

	if(!concat->subdev[0]->block_markbad)
		return -EINVAL;

	if(ofs > mtd->size)
		return -EINVAL;

	for(i = 0; i < concat->num_subdev; i++)
	{
		subdev = concat->subdev[i];

		if(ofs >= subdev->size)
		{
			ofs -= subdev->size;
			continue;
		}

		err = subdev->block_markbad(subdev, ofs);
		if (!err)
			mtd->ecc_stats.badblocks++;
		break;
	}

	return err;
}


/********************************************************************************
* 函数: struct mtd_info *mtd_concat_create(__in struct mtd_info *subdev[],
                                          __in int32_t num_devs,
                                          __in const int8_t *name)
* 描述: 创建mtd设备链
* 输入: subdev: 需要链接的设备数组
       num_devs: 设备的数量
       name: 链接好的设备的名字
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
struct mtd_info *mtd_concat_create(__in struct mtd_info *subdev[], __in int32_t num_devs,
                                    __in const int8_t *name)
{
    int32_t i, j;
    size_t size;
    struct mtd_concat *concat;
    uint32_t max_erasesize, curr_erasesize;
    int32_t num_erase_region;

    printl(LOG_LEVEL_MSG, "[MTD:MSG] Concatenating MTD devices:\n");

	for (i = 0; i < num_devs; i++)
        printl(LOG_LEVEL_MSG, "[MTD:MSG] (%d): \"%s\"\n", i, subdev[i]->name);

    printl(LOG_LEVEL_MSG, "[MTD:MSG] into device \"%s\"\n", name);

	/* 分配设备内存 */
	size = SIZEOF_STRUCT_MTD_CONCAT(num_devs);
	concat = dlmalloc(size);
	if(!concat)
	{
		printl(LOG_LEVEL_ERR, "[MTD:ERR] memory allocation error while creating concatenated device \"%s\"\n", name);
		return NULL;
	}

	//链接subdev指针
	concat->subdev = (struct mtd_info **)(concat + 1);

	/* 设置总设备参数 */
	concat->mtd.type = subdev[0]->type;
	concat->mtd.flags = subdev[0]->flags;
	concat->mtd.size = subdev[0]->size;
	concat->mtd.erasesize = subdev[0]->erasesize;
	concat->mtd.writesize = subdev[0]->writesize;
	concat->mtd.subpage_sft = subdev[0]->subpage_sft;
	concat->mtd.oobsize = subdev[0]->oobsize;
	concat->mtd.oobavail = subdev[0]->oobavail;
	if (subdev[0]->read_oob)
		concat->mtd.read_oob = concat_read_oob;
	if (subdev[0]->write_oob)
		concat->mtd.write_oob = concat_write_oob;
	if (subdev[0]->block_isbad)
		concat->mtd.block_isbad = concat_block_isbad;
	if (subdev[0]->block_markbad)
		concat->mtd.block_markbad = concat_block_markbad;

	concat->mtd.ecc_stats.badblocks = subdev[0]->ecc_stats.badblocks;

	concat->subdev[0] = subdev[0];

	for (i = 1; i < num_devs; i++)
	{
	    /* 链接的设备的类型必须一致 */
		if(concat->mtd.type != subdev[i]->type)
		{
			dlfree((int8_t *)concat);
			printl(LOG_LEVEL_ERR, "[MTD:ERR] Incompatible device type on \"%s\"\n", subdev[i]->name);
			return NULL;
		}

        /* 检测子设备标志位，除了MTD_WRITEABLE外其他必须一置 */
		if(concat->mtd.flags != subdev[i]->flags)
		{
			if((concat->mtd.flags ^ subdev[i]->flags) & ~MTD_WRITEABLE)
			{
				dlfree((int8_t *)concat);
				printl(LOG_LEVEL_ERR, "[MTD:ERR] Incompatible device flags on \"%s\"\n", subdev[i]->name);
				return NULL;
			}
			else
				/* 标记MTD_WRITEABLE位 */
				concat->mtd.flags |= (subdev[i]->flags & MTD_WRITEABLE);
		}

		concat->mtd.size += subdev[i]->size;
		concat->mtd.ecc_stats.badblocks += subdev[i]->ecc_stats.badblocks;

		if((concat->mtd.writesize != subdev[i]->writesize) ||
           (concat->mtd.subpage_sft != subdev[i]->subpage_sft) ||
           (concat->mtd.oobsize != subdev[i]->oobsize) ||
           (!concat->mtd.read_oob != !subdev[i]->read_oob) ||
           (!concat->mtd.write_oob != !subdev[i]->write_oob))
        {
			dlfree((int8_t *)concat);
			printl(LOG_LEVEL_ERR, "[MTD:ERR] Incompatible OOB or ECC data on \"%s\"\n", subdev[i]->name);
			return NULL;
		}
		concat->subdev[i] = subdev[i];

	}

	concat->mtd.ecclayout = subdev[0]->ecclayout;

	concat->num_subdev = num_devs;
	concat->mtd.name = name;

	concat->mtd.erase = concat_erase;
	concat->mtd.read = concat_read;
	concat->mtd.write = concat_write;
	concat->mtd.sync = concat_sync;
	concat->mtd.lock = concat_lock;
	concat->mtd.unlock = concat_unlock;

	/* 统计可擦除的区域 */
	max_erasesize = curr_erasesize = subdev[0]->erasesize;
	num_erase_region = 1;
	for(i = 0; i < num_devs; i++)
	{
		if(subdev[i]->numeraseregions == 0)
		{
			/* 所有子设备本身都有统一的擦除大小 */
			if(subdev[i]->erasesize != curr_erasesize)
			{
				/* 子设备擦除大小不一样，增加计数 */
				++num_erase_region;
				curr_erasesize = subdev[i]->erasesize;
				if(curr_erasesize > max_erasesize)
					max_erasesize = curr_erasesize;
			}
		}
		else
		{
			/* 所有的子设备本身擦除大小也不同，分区域 */
			for(j = 0; j < subdev[i]->numeraseregions; j++)
			{

				/* 统计不同 */
				if(subdev[i]->eraseregions[j].erasesize != curr_erasesize)
				{
					++num_erase_region;
					curr_erasesize = subdev[i]->eraseregions[j].erasesize;
					if(curr_erasesize > max_erasesize)
						max_erasesize = curr_erasesize;
				}
			}
		}
	}

	if(num_erase_region == 1)
	{
		/* 所有子设备都有同样的擦除大小 */
		concat->mtd.erasesize = curr_erasesize;
		concat->mtd.numeraseregions = 0;
	}
	else
	{
		uint64_t tmp64;
		/* 擦除区域分配信息 */
		struct mtd_erase_region_info *erase_region_p;
		uint64_t begin, position;

		concat->mtd.erasesize = max_erasesize;
		concat->mtd.numeraseregions = num_erase_region;
		concat->mtd.eraseregions = erase_region_p = dlmalloc(num_erase_region * sizeof(struct mtd_erase_region_info));
		if(!erase_region_p)
		{
			dlfree((int8_t *)concat);
			printl(LOG_LEVEL_ERR, "[MTD:ERR] memory allocation error while creating erase region list for device \"%s\"\n", name);
			return NULL;
		}

		/* 填充擦除区域信息 */
		curr_erasesize = subdev[0]->erasesize;
		begin = position = 0;
		for(i = 0; i < num_devs; i++)
		{
			if(subdev[i]->numeraseregions == 0)
			{
				/* 子设备自身有统一擦除大小 */
				if(subdev[i]->erasesize != curr_erasesize)
				{
					/* 填充擦除区域信息 */
					erase_region_p->offset = begin;
					erase_region_p->erasesize = curr_erasesize;
					tmp64 = position - begin;
					do_div(tmp64, curr_erasesize);
					erase_region_p->numblocks = tmp64;
					begin = position;
					curr_erasesize = subdev[i]->erasesize;
					++erase_region_p;
				}
				position += subdev[i]->size;
			}
			else
			{
				/* 子设备自身也有不同的擦除区域大小 */
				for(j = 0; j < subdev[i]->numeraseregions; j++)
				{
					/* 统计不同 */
					if(subdev[i]->eraseregions[j].erasesize != curr_erasesize)
					{
						erase_region_p->offset = begin;
						erase_region_p->erasesize = curr_erasesize;
						tmp64 = position - begin;
						do_div(tmp64, curr_erasesize);
						erase_region_p->numblocks = tmp64;
						begin = position;
						curr_erasesize = subdev[i]->eraseregions[j].erasesize;
						++erase_region_p;
					}
					position += subdev[i]->eraseregions[j].numblocks * (uint64_t)curr_erasesize;
				}
			}
		}

		/* 写最后一个块 */
		erase_region_p->offset = begin;
		erase_region_p->erasesize = curr_erasesize;
		tmp64 = position - begin;
		do_div(tmp64, curr_erasesize);
		erase_region_p->numblocks = tmp64;
	}

	return &concat->mtd;
}



