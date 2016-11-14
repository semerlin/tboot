#include "stddef.h"
#include "mtd/mtd.h"
#include "mtd/nand/nand.h"
#include "global_data.h"
#include "log.h"

DECLARE_GLOBAL_DATA_PTR;

/* nand设备地址列表 */
#ifndef CONFIG_SYS_NAND_BASE_LSIT
#define CONFIG_SYS_NAND_BASE_LIST {CONFIG_SYS_NAND_BASE}
#endif



/* 当前使用的nand设备 */
int32_t nand_cur_device = -1;

struct mtd_info nand_info[CONFIG_SYS_MAX_NAND_DEVICE];

/* nandflash设备信息 */
static struct nand_chip nand_chips[CONFIG_SYS_MAX_NAND_DEVICE];
static size_t nand_base_address[CONFIG_SYS_MAX_NAND_DEVICE] = CONFIG_SYS_NAND_BASE_LIST;

static const int8_t default_nand_name[] = "nand";

/* 设备名字 */
static int8_t dev_name[CONFIG_SYS_MAX_NAND_DEVICE][8];


/********************************************************************************
* 函数: static void nand_init_chip(__in struct mtd_info *mtd,
                                  __in struct nand_chip *nand,
                                  __in size_t base_addr)
* 描述: 初始化nandflash芯片
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static void nand_init_chip(__out struct mtd_info *mtd, __in struct nand_chip *nand, __in size_t base_addr)
{
	int32_t maxchips = CONFIG_SYS_NAND_MAX_CHIPS;
	static int32_t i = 0;

	if(maxchips < 1)
		maxchips = 1;

	mtd->priv = nand;

	nand->IO_ADDR_R = nand->IO_ADDR_W = (void *)base_addr;

    //初始化nand芯片，添加进mtd设备统一管理
	if(board_nand_init(nand) == 0)
	{
		if(nand_scan(mtd, maxchips) == 0)
		{
			if(!mtd->name)
				mtd->name = (char *)default_nand_name;
			else
				mtd->name += gd->reloc_off;

			sprintf(dev_name[i], "nand%d", i);
			mtd->name = dev_name[i++];
			add_mtd_device(mtd);
		}
		else
			mtd->name = NULL;
	}
	else
	{
		mtd->name = NULL;
		mtd->size = 0;
	}

}


/********************************************************************************
* 函数: void nand_init(void)
* 描述: 初始化nand设备
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void nand_init(void)
{
	uint32_t size = 0;
	int32_t i;
	for(i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
	{
		nand_init_chip(&nand_info[i], &nand_chips[i], nand_base_address[i]);
		size += nand_info[i].size / SZ_1K;
		if(nand_cur_device == -1)
			nand_cur_device = i;
	}
	printl(LOG_LEVEL_INFO, "nand device total size: %u MiB\n", size / SZ_1K);

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
	board_nand_select_device(nand_info[nand_curr_device].priv, nand_curr_device);
#endif
}

