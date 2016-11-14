#ifndef _NAND_H
  #define _NAND_H


#include "stddef.h"
#include "config.h"

#include "mtd/mtd.h"
#include "mtd/bbm.h"
#include "mtd/nand/nand_device_info.h"

/* 让gcc不警告 */
struct mtd_info;
struct nand_chip;

/* nandflash外部接口 */
extern int32_t nand_erase_nand(__in struct mtd_info *mtd, __in struct erase_info *instr, __in int32_t allowbbt);
extern int32_t nand_scan(__in struct mtd_info *mtd, __in int32_t maxchips);
extern int32_t nand_scan_ident(__in struct mtd_info *mtd, __in int32_t maxchips);
extern int32_t nand_scan_tail(__in struct mtd_info *mtd);
extern int32_t nand_scan_bbt(__in struct mtd_info *mtd, __in struct nand_bbt_desc *bd);
extern int32_t nand_update_bbt(__in struct mtd_info *mtd, __in loff_t offs);
extern int32_t nand_default_bbt(__in struct mtd_info *mtd);
extern int32_t nand_isbad_bbt(__in struct mtd_info *mtd, __in loff_t offs, __in int32_t allowbbt);
extern int32_t board_nand_init(__in struct nand_chip *chip);


/* nandflash一页数据缓存最大值 */
#define NAND_MAX_OOBSIZE	(218 * CONFIG_SYS_NAND_MAX_CHIPS)
#define NAND_MAX_PAGESIZE	(4096 * CONFIG_SYS_NAND_MAX_CHIPS)

/*
* 出场坏块标记位置
*/
#define NAND_SMALL_BADBLOCK_POS		5
#define NAND_LARGE_BADBLOCK_POS		0


/* 片选(低电平有效) */
#define NAND_NCE		    0x01
/* 指令锁存(高电平有效) */
#define NAND_CLE		    0x02
/* 地址锁存(低电平有效) */
#define NAND_ALE		    0x04

#define NAND_CTRL_CLE		(NAND_NCE | NAND_CLE)
#define NAND_CTRL_ALE		(NAND_NCE | NAND_ALE)
/* 控制是否改变标志 */
#define NAND_CTRL_CHANGE	0x80

/*
 * nandflash命令
 */
#define NAND_CMD_READ                0x00
#define NAND_CMD_READ0		         0x00
#define NAND_CMD_READ1		         0x01
#define NAND_CMD_READOOB	         0x50
#define NAND_CMD_READSTART	         0x30
#define NAND_CMD_CACHEREADSTART      0x31

#define NAND_CMD_RNDIN		         0x85
#define NAND_CMD_RNDOUT		         0x05
#define NAND_CMD_RNDOUTSTART	     0xe0

#define NAND_CMD_SEQIN		         0x80
#define NAND_CMD_PAGEPROG	         0x10
#define NAND_CMD_CACHEDPROG          0x15

#define NAND_CMD_ERASE1		         0x60
#define NAND_CMD_ERASE2		         0xd0

#define NAND_CMD_STATUS		         0x70
#define NAND_CMD_STATUS_MULTI	     0x71

#define NAND_CMD_READID		         0x90

#define NAND_CMD_RESET		         0xff

/* 空指令 */
#define NAND_CMD_NONE		         -1

/* nandflash状态标志 */
#define NAND_STATUS_FAIL	         0x01
#define NAND_STATUS_FAIL_N1	         0x02
#define NAND_STATUS_TRUE_READY	     0x20
#define NAND_STATUS_READY	         0x40
#define NAND_STATUS_WP		         0x80



/*
 * Constants for Hardware ECC
 */
/* Reset Hardware ECC for read */
#define NAND_ECC_READ		0
/* Reset Hardware ECC for write */
#define NAND_ECC_WRITE		1
/* Enable Hardware ECC before syndrom is read back from flash */
#define NAND_ECC_READSYN	2

/* Bit mask for flags passed to do_nand_read_ecc */
#define NAND_GET_DEVICE		0x80


/* Option constants for bizarre disfunctionality and real
*  features
*/
/* Chip can not auto increment pages */
#define NAND_NO_AUTOINCR	0x00000001
/* Buswitdh is 16 bit */
#define NAND_BUSWIDTH_16	0x00000002
/* Device supports partial programming without padding */
#define NAND_NO_PADDING		0x00000004
/* Chip has cache program function */
#define NAND_CACHEPRG		0x00000008
/* Chip has copy back function */
#define NAND_COPYBACK		0x00000010
/* Chip does not require ready check on read. True
 * for all large page devices, as they do not support
 * autoincrement.*/
#define NAND_NO_READRDY		0x00000100
/* Chip does not allow subpage writes */
#define NAND_NO_SUBPAGE_WRITE	0x00000200



/* Macros to identify the above */
#define NAND_MUST_PAD(chip) (!(chip->options & NAND_NO_PADDING))
#define NAND_HAS_CACHEPROG(chip) ((chip->options & NAND_CACHEPRG))
#define NAND_HAS_COPYBACK(chip) ((chip->options & NAND_COPYBACK))
/* Large page NAND with SOFT_ECC should support subpage reads */
#define NAND_SUBPAGE_READ(chip) ((chip->ecc.mode == NAND_ECC_SOFT) \
					&& (chip->page_shift > 9))

/* Mask to zero out the chip options, which come from the id table */
#define NAND_CHIPOPTIONS_MSK	(0x0000ffff & ~NAND_NO_AUTOINCR)

/* Non chip related options */
/* Use a flash based bad block table. This option is passed to the
 * default bad block table function. */
#define NAND_USE_FLASH_BBT	0x00010000
/* This option skips the bbt scan during initialization. */
#define NAND_SKIP_BBTSCAN	0x00020000
/* Options set by nand scan */
/* bbt has already been read */
#define NAND_BBT_SCANNED	0x40000000
/* Nand scan has allocated controller struct */
#define NAND_CONTROLLER_ALLOC	0x80000000

/* Cell info constants */
#define NAND_CI_CHIPNR_MSK	0x03
#define NAND_CI_CELLTYPE_MSK	0x0C



/**
 * struct nand_ecc_ctrl - Control structure for ecc
 * @mode:	ecc mode
 * @steps:	number of ecc steps per page
 * @size:	data bytes per ecc step
 * @bytes:	ecc bytes per step
 * @total:	total number of ecc bytes per page
 * @prepad:	padding information for syndrome based ecc generators
 * @postpad:	padding information for syndrome based ecc generators
 * @layout:	ECC layout control struct pointer
 * @hwctl:	function to control hardware ecc generator. Must only
 *		be provided if an hardware ECC is available
 * @calculate:	function for ecc calculation or readback from ecc hardware
 * @correct:	function for ecc correction, matching to ecc generator (sw/hw)
 * @read_page_raw:	function to read a raw page without ECC
 * @write_page_raw:	function to write a raw page without ECC
 * @read_page:	function to read a page according to the ecc generator requirements
 * @write_page:	function to write a page according to the ecc generator requirements
 * @read_oob:	function to read chip OOB data
 * @write_oob:	function to write chip OOB data
 */

 /* ECC模式 */
typedef enum
{
	NAND_ECC_NONE,
	NAND_ECC_SOFT,
	NAND_ECC_HW,
	NAND_ECC_HW_SYNDROME,
} nand_ecc_modes_t;



/* ecc控制 */
struct nand_ecc_ctrl
{
	nand_ecc_modes_t mode; /* ecc模式 */
	int32_t ecc_steps_per_page; /* 一页有多少块ecc */
	int32_t data_size_per_step; /* 每块ecc管理多少字节数据 */
	int32_t ecc_bytes_per_step; /* 每块ecc字节数 */
	int32_t ecc_total_bytes_per_page; /* 一页ecc总字节数 */
	uint8_t ecc_calcbuf[NAND_MAX_OOBSIZE]; /* 计算出的ecc数据保存缓冲区 */
	uint8_t ecc_codebuf[NAND_MAX_OOBSIZE]; /* 从nandflash读出的ecc数据保存缓冲区*/
	struct nand_ecclayout *layout; /* ecc布局 */
	void (*hwctl)(struct mtd_info *mtd, int32_t mode);
	int32_t (*calculate)(const uint8_t *data, uint8_t *ecc_code);
	int32_t (*correct)(uint8_t *data, uint8_t *read_ecc, uint8_t *calc_ecc);
	int32_t (*read_page_raw)(struct mtd_info *mtd, uint8_t *buf);
	int32_t (*write_page_raw)(struct mtd_info *mtd, const uint8_t *buf);
	int32_t (*read_page)(struct mtd_info *mtd, uint8_t *buf);
	int32_t (*read_subpage)(struct mtd_info *mtd, uint32_t offs, uint32_t len, uint8_t *buf);
	int32_t (*write_page)(struct mtd_info *mtd, const uint8_t *buf);
	int32_t (*read_oob)(struct mtd_info *mtd, int32_t page, int32_t sndcmd);
	int32_t (*write_oob)(struct mtd_info *mtd, int32_t page);
};

/* nanflash芯片控制结构体 */
struct nand_chip
{
	void *IO_ADDR_R;  /* 读数据地址/寄存器 */
	void *IO_ADDR_W;  /* 写数据地址/寄存器 */

    /**********************************************************
    --------------------------接口函数--------------------------
    **********************************************************/
    /* MTD适配层 */

    /*----------------------需要外部驱动/板级实现------------------------*/
    void (*cmd_ctrl)(__in struct mtd_info *mtd, __in int data, __in uint32_t ctrl);
    int32_t (*dev_ready)(__in struct mtd_info *mtd);
    int32_t chip_delay; /* 芯片执行指令之后等待时间 */
    uint32_t options;

    /*------------------------nandflash私有层函数----------------------*/
    /*----------------------------可替换(根函数)------------------------------*/
	uint8_t (*read_byte)(__in struct mtd_info *mtd);
	uint16_t (*read_word)(__in struct mtd_info *mtd);
	void (*write_buf)(__in struct mtd_info *mtd, __in const uint8_t *buf, __in int32_t len);
	void (*read_buf)(__in struct mtd_info *mtd, __in uint8_t *buf, __in int32_t len);
	int32_t (*verify_buf)(__in struct mtd_info *mtd, __in const uint8_t *buf, __in int32_t len);
	void (*select_chip)(__in struct mtd_info *mtd, __in int32_t chipnr);
	int (*block_bad)(__in struct mtd_info *mtd, __in loff_t ofs, __in  int32_t getchip);
	int (*block_markbad)(struct mtd_info *mtd, loff_t ofs);

	void (*cmdfunc)(__in struct mtd_info *mtd, __in uint32_t command, __in int32_t column, __in int32_t page_addr);
	int (*waitfunc)(struct mtd_info *mtd);
	void (*erase_cmd)(struct mtd_info *mtd, int page);
	int (*scan_bbt)(struct mtd_info *mtd);

    /*--------------------内部私有变量，外部不需要直接操作-----------------*/

	int32_t page_shift;  /* 一页的大小，以移位数表示(例如2048表示为11)*/
	int32_t page_mask;  /* 一块芯片一共有多少页-1 */
	uint8_t page_databuf[NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE]; /* 芯片一页数据缓冲区(data + oob) */
	int32_t subpage_size; /* 子页大小，按ecc块分 */

	int32_t phys_erase_shift; /* 擦除块的大小，以移位表示 */
	int32_t bbt_erase_shift;  /* bbt擦除块的大小，以移位表示 */
	int32_t chip_shift; /* 一片nandflash组的大小, 以移位表示(目前仅支持单片(一个CE#选择)大小4G以下的flash) */

	int32_t numchips; /* nandflash物理芯片的数量, 按CE#脚数量计算 */
	uint64_t chipsize; /* 一片物理nandflash的总大小, 内部可能包含多个plane, 所以大小可能超过4g */

	int32_t pagebuf; /* 当前缓冲区中保存的数据属于的页数 */
	uint8_t cellinfo;

	int32_t badblockpos; /* 坏块标记位置 */

	int32_t state; /* 芯片当前状态 */

	uint8_t *oob_poi; /* oob数据指针 */

	struct nand_ecc_ctrl ecc_ctrl; /* ecc控制 */

	uint8_t *bbt; /* bbt数据缓冲区 */
	struct nand_bbt_desc *bbt_td; /* 原始bbt描述符,必须存在 */
	struct nand_bbt_desc *bbt_md; /* 镜像bbt描述符,可以不存在 */

	struct nand_bbt_desc *badblock_pattern;

	struct nand_timing *timing; /* 物理芯片的时序 */

    /*--------------------------可选支持------------------------*/
	void *priv;
	int (*errstat)(__in struct mtd_info *mtd, __in int32_t state, __in int32_t status, __in int32_t page);
};







#endif /* _NAND_H_ */
