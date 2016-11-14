#ifndef _MTD_PARTITIONS_H_
  #define _MTD_PARTITIONS_H_


#include "types.h"



struct mtd_partition
{
	int8_t *name;			/* 分区名字 */
	uint64_t size;			/* 分区大小 */
	uint64_t offset;		/* 分区相对于mtd主设备的偏移地址 */
	uint32_t mask_flags;		/* 分区设备屏蔽的标志位 */
	struct nand_ecclayout *ecclayout;	/* 分区的ecc分布 */
	struct mtd_info **mtdp;		/* pointer to store the MTD object */
};

#define MTDPART_OFS_NXTBLK	(-2)
#define MTDPART_OFS_APPEND	(-1)
#define MTDPART_SIZ_FULL	(0)


int add_mtd_partitions(struct mtd_info *, const struct mtd_partition *, int);
int del_mtd_partitions(struct mtd_info *);








#endif


