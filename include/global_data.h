#ifndef _GLOBAL_DATA_H_
  #define _GLOBAL_DATA_H_

#include "types.h"
#include "environment.h"
#include "config.h"

/* 此部分变量存储在nandflash中 */
typedef struct bd_info
{
    int32_t bi_baudrate;  //串口波特率
    uint32_t bi_ip_addr;  //IP地址
    struct environment_s *bi_env; //环境变量指针
    uint32_t bi_arch_number; //板子ID号
    uint32_t bi_boot_params; //启动参数

    struct  //DRAM BANKS配置，起始地址和长度
    {
        uint32_t start;
        uint32_t size;
    }bi_dram[CONFIG_NR_DRAM_BANKS];
}bd_t;


typedef struct global_data
{
    bd_t *bd;  //板子信息结构体
    uint32_t flags;  //指示标志，如设备已经初始化标志等
    uint32_t baudrate;  //串口波特率
    uint32_t have_console; //串口初始化标志
    uint32_t reloc_off;  //内存映射地址
    uint32_t env_addr;  //环境参数地址
    uint32_t env_valid;  //环境参数CRC检验有效标志
    uint32_t fb_base;  //frame buffer的基址

    void **jt; //跳转地址
}gd_t;




/* 全局变量标志 */
#define GD_FLG_RELOC                    0x0001
#define GD_FLG_DEVINIT                  0x0002
#define GD_FLG_SILENT                   0x0004
#define GD_FLG_LOGINIT                  0x0008
#define GD_FLG_DISABLE_CONSOLE          0x0010


#define DECLARE_GLOBAL_DATA_PTR   register volatile gd_t *gd asm("r8")















#endif


