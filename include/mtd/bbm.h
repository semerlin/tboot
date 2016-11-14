#ifndef _BBM_H_
  #define _BBM_H_

#include "types.h"


#ifndef CONFIG_SYS_NAND_MAX_CHIPS
  #define CONFIG_SYS_NAND_MAX_CHIPS    1
#endif



/* nandflash bbt描述符 */
struct nand_bbt_desc
{
	int32_t options; /* bbt选项和标志*/
	int32_t pages[CONFIG_SYS_NAND_MAX_CHIPS]; /* bbt存放的起始页 */
	int32_t offs; /* bbt标示在oob区域的偏移 */
	int32_t len; /* bbt标识的长度 */
	uint8_t *pattern; /* 标识字符串,可以是标示好/坏块，也可以标示bbt */
	int32_t veroffs; /* 版本号的偏移地址 */
	uint8_t version[CONFIG_SYS_NAND_MAX_CHIPS]; /* 版本号存放位置 */
	int32_t maxblocks; /* 最多扫描的块数量 */
	int32_t reserved_block_code; /* 保留块的代码 */
};


/* bbt选项和标志 */

/* 物理芯片上块状态描述的位数 */
#define NAND_BBT_NRBITS_MSK	    0x0000000F
#define NAND_BBT_1BIT		    0x00000001
#define NAND_BBT_2BIT		    0x00000002
#define NAND_BBT_4BIT		    0x00000004
#define NAND_BBT_8BIT		    0x00000008

/* bbt在物理芯片的最后一个好块上 */
#define NAND_BBT_LASTBLOCK	    0x00000010
/* bbt存放在指定的页 */
#define NAND_BBT_ABSPAGE	    0x00000020
/* 自动寻找bbt存放位置 */
#define NAND_BBT_SEARCH		    0x00000040
/* 多物理芯片情况下，每个芯片的bbt存放在各自的芯片上 */
#define NAND_BBT_PERCHIP	    0x00000080
/* bbt版本 */
#define NAND_BBT_VERSION	    0x00000100
/* 没有找到bbt时创建一个 */
#define NAND_BBT_CREATE		    0x00000200
/* 需要的话就写bbt数据 */
#define NAND_BBT_WRITE		    0x00001000
/* 当写bbt时，读和写回块中的原有内容 */
#define NAND_BBT_SAVECONTENT	0x00002000
/* bbt存放在块的第一或者第二页 */
#define NAND_BBT_SCAN2NDPAGE	0x00004000

/* 寻找bbt时最多寻找的块 */
#define NAND_BBT_SCAN_MAXBLOCKS	4













#endif

