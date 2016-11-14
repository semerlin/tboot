#ifndef _MTD_ABI_H__
#define _MTD_ABI_H__


/* 用户使用的擦除信息 */
struct erase_info_user {
	uint32_t start;
	uint32_t length;
};

/* oob数据缓冲区 */
struct mtd_oob_buf {
	uint32_t start;
	uint32_t length;
	unsigned char *ptr;
};

#define MTD_ABSENT		    0
#define MTD_RAM			    1
#define MTD_ROM			    2
#define MTD_NORFLASH		3
#define MTD_NANDFLASH		4
#define MTD_DATAFLASH		6
#define MTD_UBIVOLUME		7

#define MTD_WRITEABLE		0x400	/* 设备可写 */
#define MTD_BIT_WRITEABLE	0x800	/* 设备可按位操作 */
#define MTD_NO_ERASE		0x1000	/* 设备不需要擦除 */
#define MTD_STUPID_LOCK		0x2000	/* 设备复位之后就锁定 */

/* 常用设备功能集合 */
#define MTD_CAP_ROM		    0
#define MTD_CAP_RAM		    (MTD_WRITEABLE | MTD_BIT_WRITEABLE | MTD_NO_ERASE)
#define MTD_CAP_NORFLASH	(MTD_WRITEABLE | MTD_BIT_WRITEABLE)
#define MTD_CAP_NANDFLASH	(MTD_WRITEABLE)

/* ECC字节存放方式 */
#define MTD_NANDECC_OFF		    0	/* 关闭ECC(不建议) */
#define MTD_NANDECC_PLACE	    1	/* 使用结构体中的给定存放方式(YAFFS1传统模式) */
#define MTD_NANDECC_AUTOPLACE	2	/* 使用默认存放图表 */
#define MTD_NANDECC_PLACEONLY	3	/* 使用结构体中的给定存放方式(在读的时候不保存ecc结果) */
#define MTD_NANDECC_AUTOPL_USR	4	/* 使用给定的自动存放图标而不是使用默认的 */

/* OTP模式选择 */
#define MTD_OTP_OFF		    0
#define MTD_OTP_FACTORY		1
#define MTD_OTP_USER		2


/* mtd用户层信息结构体 */
struct mtd_info_user
{
	uint8_t type;
	uint32_t flags;
	uint32_t size;			/* MTD设备总大小 */
	uint32_t erasesize;
	uint32_t writesize;
	uint32_t oobsize;		/* Amount of OOB data per block (e.g. 16) */
	/* The below two fields are obsolete and broken, do not use them
	 * (TODO: remove at some point) */
	uint32_t ecctype;
	uint32_t eccsize;
};

/* 擦除区域用户层信息结构体 */
struct region_info_user
{
	uint32_t offset;		/* 此区域相对于MTD设备起始地址的偏移 */
	uint32_t erasesize;		/* 此区域可擦除大小 */
	uint32_t numblocks;		/* 此区域块的个数 */
	uint32_t regionindex;
};

/* opt信息结构体 */
struct otp_info {
	uint32_t start;
	uint32_t length;
	uint32_t locked;
};

/*
#define MEMGETINFO		_IOR('M', 1, struct mtd_info_user)
#define MEMERASE		_IOW('M', 2, struct erase_info_user)
#define MEMWRITEOOB		_IOWR('M', 3, struct mtd_oob_buf)
#define MEMREADOOB		_IOWR('M', 4, struct mtd_oob_buf)
#define MEMLOCK			_IOW('M', 5, struct erase_info_user)
#define MEMUNLOCK		_IOW('M', 6, struct erase_info_user)
#define MEMGETREGIONCOUNT	_IOR('M', 7, int)
#define MEMGETREGIONINFO	_IOWR('M', 8, struct region_info_user)
#define MEMSETOOBSEL		_IOW('M', 9, struct nand_oobinfo)
#define MEMGETOOBSEL		_IOR('M', 10, struct nand_oobinfo)
#define MEMGETBADBLOCK		_IOW('M', 11, loff_t)
#define MEMSETBADBLOCK		_IOW('M', 12, loff_t)
#define OTPSELECT		_IOR('M', 13, int)
#define OTPGETREGIONCOUNT	_IOW('M', 14, int)
#define OTPGETREGIONINFO	_IOW('M', 15, struct otp_info)
#define OTPLOCK			_IOR('M', 16, struct otp_info)
#define ECCGETLAYOUT		_IOR('M', 17, struct nand_ecclayout)
#define ECCGETSTATS		_IOR('M', 18, struct mtd_ecc_stats)
#define MTDFILEMODE		_IO('M', 19)
*/


/*
 * Obsolete legacy interface. Keep it in order not to break userspace
 * interfaces
 */
struct nand_oobinfo {
	uint32_t useecc;
	uint32_t eccbytes;
	uint32_t oobfree[8][2];
	uint32_t eccpos[48];
};

struct nand_oobfree {
	uint32_t offset;
	uint32_t length;
};

#define MTD_MAX_OOBFREE_ENTRIES	8
/*
 * ECC layout control structure. Exported to userspace for
 * diagnosis and to allow creation of raw images
 */
struct nand_ecclayout {
	uint32_t eccbytes;
	uint32_t eccpos[64];
	uint32_t oobavail;
	struct nand_oobfree oobfree[MTD_MAX_OOBFREE_ENTRIES];
};

/**
 * struct mtd_ecc_stats - error correction stats
 *
 * @corrected:	number of corrected bits
 * @failed:	number of uncorrectable errors
 * @badblocks:	number of bad blocks in this partition
 * @bbtblocks:	number of blocks reserved for bad block tables
 */
struct mtd_ecc_stats {
	uint32_t corrected;
	uint32_t failed;
	uint32_t badblocks;
	uint32_t bbtblocks;
};

/*
 * Read/write file modes for access to MTD
 */
enum mtd_file_modes {
	MTD_MODE_NORMAL = MTD_OTP_OFF,
	MTD_MODE_OTP_FACTORY = MTD_OTP_FACTORY,
	MTD_MODE_OTP_USER = MTD_OTP_USER,
	MTD_MODE_RAW,
};

#endif /* __MTD_ABI */
