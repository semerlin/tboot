#ifndef _NAND_DEVICE_INFO_H_
  #define _NAND_DEVICE_INFO_H_

#include "stddef.h"


#define NAND_DEVICE_ID_BYTE_COUNT    (6)


/* nandflash技术架构*/
enum nand_device_cell_technology
{
    NAND_DEVICE_CELL_TECH_SLC = 0,
    NAND_DEVICE_CELL_TECH_MLC = 1,
};

/* nandflash时序 */
struct nand_timing
{
    /* nandflash时序 */
	int8_t data_setup_in_ns; /* 数据有效建立时间,RE#或WR#低电平时间 */
	int8_t data_hold_in_ns; /* 数据保持时间,RE#或WR#高电平时间 */
	int8_t address_setup_in_ns; /* 地址有效建立时间,ALE高电平时间 */
	int8_t gpmi_sample_delay_in_ns; /* gpmi采样延时时间，大部分是计算得出 */

	/* 更具体的参数，可以用于提高带宽 */
	int8_t tREA_in_ns; /* RE#或WR#拉低之后数据准备完成时间*/
	int8_t tRLOH_in_ns;  /* RE#或WR#拉低之后数据保持时间 */
	int8_t tRHOH_in_ns;  /* RE#或WR#拉高之后数据保持时间 */
};


/* nandflash设备信息结构体 */
struct nand_device_info
{
	/* 标记nand_device_info_table数组是否到末尾，true表示到末尾 */
	bool end_of_table;

	/* 制造商和设备ID */
	uint8_t manufacturer_code;
	uint8_t device_code;

	/* nandflash芯片技术架构 */
	enum nand_device_cell_technology  cell_technology;

	/* nandflash芯片布局 */
	uint64_t chip_size_in_bytes;
	uint32_t block_size_in_bytes;
	uint16_t page_data_size_in_bytes;
	uint16_t page_oob_size_in_bytes;

    struct nand_timing timing;

    /* 操作选项 */
    uint64_t options;

	/* nandflash描述 */
	const int8_t  *description;
};




extern struct nand_device_info *nand_device_get_info(__in const uint8_t *id);
extern struct nand_device_info *nand_device_get_safenand_info(void);
extern void nand_device_print_info(__in struct nand_device_info *info);








#endif /* _NAND_DEVICE_INFO_H_*/


