#ifndef _MTD_H_
  #define _MTD_H_


#include "types.h"
#include "math.h"
#include "mtd_abi.h"


/* mtd设备最大数量 */
#define MAX_MTD_DEVICES     32

/* nand/onenand设备状态 */
enum
{
    /* 设备准备就绪，现在空闲 */
	FL_READY,
	/* 等待设备就绪 */
	FL_READING,
	/* 正在写设备 */
	FL_WRITING,
	/* 正在擦除设备 */
	FL_ERASING,
	/* 设备正在同步 */
	FL_SYNCING,
	/* 设备通过cache在编程 */
	FL_CACHEDPRG,
	/* 设备正在复位 */
	FL_RESETING,
	/* 设备正在解锁 */
	FL_UNLOCKING,
	/* 设备正在上锁 */
	FL_LOCKING,
	/* 设备挂起(电源管理) */
	FL_PM_SUSPENDED,
};

/* MTD擦除信息结构体 */
#define MTD_ERASE_PENDING	     0x01
#define MTD_ERASING		         0x02
#define MTD_ERASE_SUSPEND	     0x04
#define MTD_ERASE_DONE           0x08
#define MTD_ERASE_FAILED         0x10

#define MTD_FAIL_ADDR_UNKNOWN	 -1LL

struct erase_info
{
	struct mtd_info *mtd;  /* 指向需要擦除的mtd设备的指针*/
	uint64_t addr;  /* 需要擦除的地址*/
	uint64_t len;  /* 可擦除的大小，此值必须和mtd->erasesize大小相等*/
	uint64_t fail_addr;  /* 记录擦除失败的地址，如果此值为MTD_FAIL_ADDR_UNKNOWN, 那么此错误不是设备问题或者和任何特殊块绑定 */

	size_t time;  /* 擦除次数 */
	size_t retries;  /* 重试的次数 */

	void (*callback) (struct erase_info *self);  /* 供内部驱动使用 */
	size_t priv;   /* 擦除期间供用户使用的私有数据 */
	uint8_t state;  /* 当前的擦除状态，驱动层才擦除时应该把它设备为MTD_ERASING，当擦除完成时应该设置为MTD_ERASE_DONE或者MTD_ERASE_FAILED */
	struct erase_info *next;  /* 指向下一要擦除的区域 */
};


//擦除的区域的信息
struct mtd_erase_region_info
{
	uint64_t offset;		/* 擦除区域相对于MTD设备起始地址的偏移 */
	uint32_t erasesize;	/* 此区域可擦除的大小 */
	uint32_t numblocks;	/* 此区域可擦除的块的个数 */
};



/* oob数据操作模式 */
typedef enum
{
    //oob数据放在指定位置
	MTD_OOB_PLACE,
    //oob数据放在ecclayout定义中的空闲区域
	MTD_OOB_AUTO,
    //在一块中读取原始的data+obb数据。oob数据插入在data数据中。这是原始的flash数据镜像。
	MTD_OOB_RAW,
} mtd_oob_mode_t;

/* oob和data操作结构 */
struct mtd_oob_ops
{
    mtd_oob_mode_t mode;  /* oob操作模式 */
	size_t len;  /* 需要写入/读取的data数据的大小 */
	size_t	retlen;  /* 已经写入/读取的data数据大小 */
	size_t	ooblen;  /* 需要写入/读取的obb数据大小 */
	size_t	oobretlen;  /* 已经写入读取的obb数据大小 */
	uint32_t ooboffs;  /* oob数据在区域中的偏移地址(只有mode=MTD_OOB_PLACE时有效) */
	uint8_t *databuf;  /* data数据缓冲区，为NULL时只有oob数据会被读写 */
	uint8_t *oobbuf;  /* oob数据缓冲区，为null时只会读写data数据 */
};


/* mtd信息结构体 */
struct mtd_info
{
	uint8_t type;  /* MTD设备类型 */
	uint32_t flags;  /* MTD设备可操作标志 */
	uint64_t size;	 /* MTD设备的总大小 */


	uint32_t erasesize; /* 可擦除的块的大小 */
	uint32_t writesize; /* 最小的可写单元大小，NOR flash为1(即使可按位操作)，nandflash是一页(或者半页，1/4页)。此值为0是非法的 */

	uint32_t oobsize;   /* 每个块中oob区域的总大小 */
	uint32_t oobavail;  /* 每个块中可用的oob的大小 */

	/* Kernel-only stuff starts here. */
	const int8_t *name;  /* mtd设备名称 */
	int32_t index; /* mtd设备索引编号 */

	/* ecc分布结构体指针-只读! */
	struct nand_ecclayout *ecclayout;

    /* 可变擦除数据区域的数据。如果numeraseregions为0表明整个设备可擦除空间和上面的erasesize一样大 */
	int32_t numeraseregions;
	struct mtd_erase_region_info *eraseregions;

	/* 擦除是异步操作。当擦除完成时设备驱动会调用instr->callback()函数。 */
	int32_t (*erase)(struct mtd_info *mtd, struct erase_info *instr);

	/* 片上执行 */
	int32_t (*point)(struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, void **virt, phys_addr_t *phys);

	/* We probably shouldn't allow XIP if the unpoint isn't a NULL */
	void (*unpoint) (struct mtd_info *mtd, loff_t from, size_t len);


	int32_t (*read) (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uint8_t *buf);
	int32_t (*write) (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const uint8_t *buf);

	/* In blackbox flight recorder like scenarios we want to make successful
	   writes in interrupt context. panic_write() is only intended to be
	   called when its known the kernel is about to panic and we need the
	   write to succeed. Since the kernel is not going to be running for much
	   longer, this function can break locks and delay to ensure the write
	   succeeds (but not sleep). */

	int32_t (*panic_write) (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const uint8_t *buf);

	int32_t (*read_oob) (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
	int32_t (*write_oob) (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);

	/*
	 * Methods to access the protection register area, present in some
	 * flash devices. The user data is one time programmable but the
	 * factory data is read only.
	 */
	int32_t (*get_fact_prot_info) (struct mtd_info *mtd, struct otp_info *buf, size_t len);
	int32_t (*read_fact_prot_reg) (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uint8_t *buf);
	int32_t (*get_user_prot_info) (struct mtd_info *mtd, struct otp_info *buf, size_t len);
	int32_t (*read_user_prot_reg) (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uint8_t *buf);
	int32_t (*write_user_prot_reg) (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uint8_t *buf);
	int32_t (*lock_user_prot_reg) (struct mtd_info *mtd, loff_t from, size_t len);

/* XXX U-BOOT XXX */
#if 0
	/* kvec-based read/write methods.
	   NB: The 'count' parameter is the number of _vectors_, each of
	   which contains an (ofs, len) tuple.
	*/
	int (*writev) (struct mtd_info *mtd, const struct kvec *vecs, unsigned long count, loff_t to, size_t *retlen);
#endif

	/* Sync */
	void (*sync) (struct mtd_info *mtd);

	/* Chip-supported device locking */
	int32_t (*lock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
	int32_t (*unlock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);

	/* Power Management functions */
	int32_t (*suspend) (struct mtd_info *mtd);
	void (*resume) (struct mtd_info *mtd);

	/* Bad block management functions */
	int32_t (*block_isbad) (struct mtd_info *mtd, loff_t ofs);
	int32_t (*block_markbad) (struct mtd_info *mtd, loff_t ofs);


	/* ECC status information */
	struct mtd_ecc_stats ecc_stats;
	/* Subpage shift (NAND) */
	int32_t subpage_sft;

	void *priv;

	int32_t usecount; /* 被引用次数 */

	/* If the driver is something smart, like UBI, it may need to maintain
	 * its own reference counting. The below functions are only for driver.
	 * The driver may register its callbacks. These callbacks are not
	 * supposed to be called by MTD users */
	int32_t (*get_device) (struct mtd_info *mtd);
	void (*put_device) (struct mtd_info *mtd);
};

static inline uint32_t mtd_div_by_eb(uint64_t sz, struct mtd_info *mtd)
{
	do_div(sz, mtd->erasesize);
	return sz;
}

static inline uint32_t mtd_mod_by_eb(uint64_t sz, struct mtd_info *mtd)
{
	return do_div(sz, mtd->erasesize);
}

	/* Kernel-side ioctl definitions */

extern int add_mtd_device(struct mtd_info *mtd);
extern int del_mtd_device (struct mtd_info *mtd);

extern struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num);
extern struct mtd_info *get_mtd_device_nm(const char *name);

extern void put_mtd_device(struct mtd_info *mtd);



#ifdef CONFIG_MTD_PARTITIONS
void mtd_erase_callback(struct erase_info *instr);
#else
/*
static inline void mtd_erase_callback(struct erase_info *instr)
{
	if (instr->callback)
		instr->callback(instr);
}
*/
#endif






#endif /* _MTD_H_ */

