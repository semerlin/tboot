#include "mtd/nand/nand.h"
#include "mtd/nand/nand_device_info.h"
#include "log.h"

static struct nand_device_info nand_device_info_table[] =
{
    /* 安全的时间，用于扫描检测是否有表中出现的nandflash设备 */
    {
        .end_of_table             = false,
        .manufacturer_code        = 0xff,
        .device_code              = 0xff,
        .cell_technology          = NAND_DEVICE_CELL_TECH_SLC,
        .chip_size_in_bytes       = 0,
        .block_size_in_bytes      = 0,
        .page_data_size_in_bytes  = 0,
        .page_oob_size_in_bytes   = 0,

        {
            .data_setup_in_ns         = 80,
            .data_hold_in_ns          = 60,
            .address_setup_in_ns      = 25,
            .gpmi_sample_delay_in_ns  = 6,
            .tREA_in_ns               = -1,
            .tRLOH_in_ns              = -1,
            .tRHOH_in_ns              = -1,
        },

        .options                  = 0,

        "safenand",
    },

    /* 其他的正式芯片 */

    {
        .end_of_table             = false,
        .manufacturer_code        = 0xec,
        .device_code              = 0xf1,
        .cell_technology          = NAND_DEVICE_CELL_TECH_SLC,
        .chip_size_in_bytes       = 128LL*SZ_1M,
        .block_size_in_bytes      = 64*SZ_1K,
        .page_data_size_in_bytes  = 2*SZ_1K,
        .page_oob_size_in_bytes   = 64,

        {
            .data_setup_in_ns         = 35,
            .data_hold_in_ns          = 25,
            .address_setup_in_ns      = 0,
            .gpmi_sample_delay_in_ns  = 6,
            .tREA_in_ns               = -1,
            .tRLOH_in_ns              = -1,
            .tRHOH_in_ns              = -1,
        },

        .options = NAND_NO_PADDING | NAND_CACHEPRG | NAND_NO_READRDY | NAND_NO_AUTOINCR,
        "K9F1F08",
    },

    //表末尾
    {true}
};





/*
 * nandflash制造商id代码
 */
#define NAND_MFR_TOSHIBA	0x98
#define NAND_MFR_SAMSUNG	0xec
#define NAND_MFR_FUJITSU	0x04
#define NAND_MFR_NATIONAL	0x8f
#define NAND_MFR_RENESAS	0x07
#define NAND_MFR_STMICRO	0x20
#define NAND_MFR_HYNIX		0xad
#define NAND_MFR_MICRON		0x2c
#define NAND_MFR_AMD		0x01
#define NAND_MFR_SANDISK	0x45
#define NAND_MFR_INTEL		0x89
#define NAND_MFR_MACRONIX   0xc2

/*
  nandflash设备制造商ID代码
*/
struct nand_manufacturers
{
	int32_t id; /* 制造商ID */
	int8_t *name; /* 制造商名字 */
};

struct nand_manufacturers nand_manuf_ids[] =
{
	{NAND_MFR_TOSHIBA,  "Toshiba"},
	{NAND_MFR_SAMSUNG,  "Samsung"},
	{NAND_MFR_FUJITSU,  "Fujitsu"},
	{NAND_MFR_NATIONAL, "National"},
	{NAND_MFR_RENESAS,  "Renesas"},
	{NAND_MFR_STMICRO,  "ST Micro"},
	{NAND_MFR_HYNIX,    "Hynix"},
	{NAND_MFR_MICRON,   "Micron"},
	{NAND_MFR_AMD,      "AMD"},
	{NAND_MFR_SANDISK,  "SanDisk"} ,
	{NAND_MFR_INTEL,    "Intel"},
    {NAND_MFR_MACRONIX, "Macronix"},

	{0x0, "Unknown"}
};




/* 取得ID信息 */
/* 第一个字节--------------------------------------------------------------*/
#define ID_GET_BYTE_1(id)               ((id)[0])
#define ID_GET_MFR_CORE(id)             ID_GET_BYTE_1(id)


/* 第二个字节--------------------------------------------------------------*/
#define ID_GET_BYTE_2(id)               ((id)[1])
#define ID_GET_DEVICE_CODE(id)          ID_GET_BYTE_2(id)


/* 第三个字节--------------------------------------------------------------*/
#define ID_GET_BYTE_3(id)               ((id)[2])

#define ID_GET_DIE_COUNT_CODE(id)       ((ID_GET_BYTE_3(id) >> 0) & 0x3)

#define ID_GET_CELL_TYPE_CODE(id)       ((ID_GET_BYTE_3(id) >> 2) & 0x3)
#define ID_CELL_TYPE_CODE_SLC           (0x0) /* 其他值都代表MLC架构 */

#define ID_GET_SIMUL_PROG(id)           ((ID_GET_BYTE_3(id) >> 4) & 0x3)

#define ID_GET_CACHE_PROGRAM(id)        ((ID_GET_BYTE_3(id) >> 7) & 0x1)


/* 第四个字节--------------------------------------------------------------*/
#define ID_GET_BYTE_4(id)                ((id)[3])

#define ID_GET_PAGE_SIZE_CODE(id)        ((ID_GET_BYTE_4(id) >> 0) & 0x3)
#define ID_PAGE_SIZE_CODE_1K         (0x0)
#define ID_PAGE_SIZE_CODE_2K         (0x1)
#define ID_PAGE_SIZE_CODE_4K         (0x2)
#define ID_PAGE_SIZE_CODE_8K         (0x3)


#define ID_GET_OOB_SIZE_CODE(id)         ((ID_GET_BYTE_4(id) >> 2) & 0x1)

#define ID_GET_BLOCK_SIZE_CODE(id)       ((ID_GET_BYTE_4(id) >> 4) & 0x3)


/* 第五个字节--------------------------------------------------------------*/
#define ID_GET_BYTE_5(id)                ((id)[4])


/* 第六个字节--------------------------------------------------------------*/
#define ID_GET_BYTE_6(id)                ((id)[5])





/********************************************************************************
* 函数: void nand_device_print_info(__in struct nand_device_info *info)
* 描述: 打印输出nandflash信息
* 输入: info: nandflash信息结构体
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void nand_device_print_info(__in struct nand_device_info *info)
{
    const int8_t *mfr_name;
    const int8_t *cell_technology_name;
    uint64_t chip_size = 0;
    const char *chip_size_units;
    int32_t i = 0;

    if(!info)
        return ;

    mfr_name = "Unknown";

    /* 取得设备制造商 */
    for(i = 0; nand_manuf_ids[i].id; i++)
    {
        if(info->manufacturer_code == nand_manuf_ids[i].id)
        {
            mfr_name = nand_manuf_ids[i].name;
            break;
        }
    }

    /* 取得nandflash技术架构 */
    switch(info->cell_technology)
    {
    case NAND_DEVICE_CELL_TECH_SLC:
        cell_technology_name = "SLC";
        break;
    case NAND_DEVICE_CELL_TECH_MLC:
        cell_technology_name = "MLC";
        break;
    default:
        cell_technology_name = "Unknown";
        break;
    }

    /* 取得nandflash大小 */
    if((info->chip_size_in_bytes >= SZ_1G) && !(info->chip_size_in_bytes % SZ_1G))
    {
        chip_size = info->chip_size_in_bytes / ((uint64_t)SZ_1G);
        chip_size_units = "GiB";
    }
    else if((info->chip_size_in_bytes >= SZ_1M) && !(info->chip_size_in_bytes % SZ_1M))
    {
        chip_size = info->chip_size_in_bytes / ((uint64_t)SZ_1M);
        chip_size_units = "MiB";
    }
    else
    {
        chip_size = info->chip_size_in_bytes;
        chip_size_units = "B";
    }


    //打印nandflash信息
    printl(LOG_LEVEL_INFO, "Manufacturer      : %s(0x%02x)", mfr_name, info->manufacturer_code);
    printl(LOG_LEVEL_INFO, "Device Code       : 0x%02x\n", info->device_code);
    printl(LOG_LEVEL_INFO, "Cell Technology   : %s\n", cell_technology_name);
    printl(LOG_LEVEL_INFO, "Chip Size         : %u %s\n", (uint32_t)chip_size, chip_size_units);
    printl(LOG_LEVEL_INFO, "Pages per Block   : %u\n", info->block_size_in_bytes);
    printl(LOG_LEVEL_INFO, "Page Geometry     : %u+%u\n", info->page_data_size_in_bytes, info->page_oob_size_in_bytes);
    printl(LOG_LEVEL_INFO, "Data Setup Time   : %u ns\n", info->timing.data_setup_in_ns);
    printl(LOG_LEVEL_INFO, "Data Hold Time    : %u ns\n", info->timing.data_hold_in_ns);
    printl(LOG_LEVEL_INFO, "Address Setup Time: %u ns\n", info->timing.address_setup_in_ns);
    printl(LOG_LEVEL_INFO, "GPMI Sample Delay : %u ns\n", info->timing.gpmi_sample_delay_in_ns);

    if(info->timing.tREA_in_ns >= 0)
        printl(LOG_LEVEL_INFO, "tREA              : %u ns\n", info->timing.tREA_in_ns);
    else
        printl(LOG_LEVEL_INFO, "tREA              : Unknown\n");

    if(info->timing.tRLOH_in_ns >= 0)
        printl(LOG_LEVEL_INFO, "tRLOH             : %u ns\n", info->timing.tRLOH_in_ns);
    else
        printl(LOG_LEVEL_INFO, "tRLOH             : Unknown\n");

    if(info->timing.tRHOH_in_ns >= 0)
        printl(LOG_LEVEL_INFO, "tRHOH             : %u ns\n", info->timing.tRHOH_in_ns);
    else
        printl(LOG_LEVEL_INFO, "tRHOH             : Unknown\n");

    if(info->description)
        printl(LOG_LEVEL_INFO, "Description       : %s\n", info->description);
    else
        printl(LOG_LEVEL_INFO, "Description       : <None>\n");

}


/********************************************************************************
* 函数: static struct nand_device_info *nand_device_info_search(
                                                        __in uint8_t mfr_code,
                                                        __in uint8_t device_code)
* 描述: 从指定的nandflash编号寻找nandflash数据
* 输入: mfr_code: 设备制造商代码
       device_code: 设备编码
* 输出: none
* 返回: 成功: 指向nandflash设备信息的结构体指针
       失败: NULL
* 作者:
* 版本: V1.0
**********************************************************************************/
static struct nand_device_info *nand_device_info_search(__in uint8_t mfr_code,
                                                          __in uint8_t device_code)
{
    struct nand_device_info *table = nand_device_info_table;
    for(; !table->end_of_table; table++)
    {
        if(table->manufacturer_code == mfr_code)
        {
            if(table->device_code == device_code)
                return table;
        }

    }

    return NULL;
}


/********************************************************************************
* 函数: static void nand_device_info_test_table(void)
* 描述: 检测nandflash设备列表是否正确，可能有重复设备
* 输入: none
* 输出: none
* 返回: none, 检测出错误之后可能永远不返回
* 作者:
* 版本: V1.0
**********************************************************************************/
static void nand_device_info_test_table(void)
{
    struct nand_device_info *table_cur = nand_device_info_table;
    struct nand_device_info *table_left = nand_device_info_table;
    uint8_t mfr_code = 0;
    uint8_t device_code = 0;

    for(; !table_cur->end_of_table; table_cur++)
    {
        mfr_code = table_cur->manufacturer_code;
        device_code = table_cur->device_code;

        for(table_left = table_cur + 1; !table_left->end_of_table; table_left++)
        {
            if((mfr_code == table_left->manufacturer_code) &&
                    (device_code == table_left->device_code))
            {
                goto error;
            }
        }
    }

    return ;

error:
    printl(LOG_LEVEL_ERR, "\n== NAND Flash device info table failed validity check ==\n");

    printl(LOG_LEVEL_ERR, "\nTable Index %u\n", (table_cur - nand_device_info_table) / sizeof(struct nand_device_info));
    nand_device_print_info(table_cur);
    printl(LOG_LEVEL_ERR, "\nTable Index %u\n", (table_left - nand_device_info_table) / sizeof(struct nand_device_info));
    nand_device_print_info(table_left);
    printl(LOG_LEVEL_ERR, "\n");

    BUG();
}

/********************************************************************************
* 函数: struct nand_device_info *nand_device_get_info(__in const uint8_t *id)
* 描述: 取得nandflash设备信息
* 输入: id: 设备制造商和id的数组
* 输出: none
* 返回: 成功: 指向nandflash设备的结构体指针
       失败: NULL
* 作者:
* 版本: V1.0
**********************************************************************************/
struct nand_device_info *nand_device_get_info(__in const uint8_t *id)
{
    uint8_t mfr_code = ID_GET_MFR_CORE(id);
    uint8_t device_code = ID_GET_DEVICE_CODE(id);

    //检测nandflash列表
    nand_device_info_test_table();

    return nand_device_info_search(mfr_code, device_code);
}


/********************************************************************************
* 函数: struct nand_device_info *nand_device_get_safenand_info(void)
* 描述: 取得nandflash安全设备信息
* 输入: none
* 输出: none
* 返回: nandflash安全设备信息
* 作者:
* 版本: V1.0
**********************************************************************************/
struct nand_device_info *nand_device_get_safenand_info(void)
{
    return &nand_device_info_table[0];
}
























