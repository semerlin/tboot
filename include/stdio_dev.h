#ifndef _STDIO_DEV_H_
  #define _STDIO_DEV_H_


#include "stddef.h"
#include "list.h"
#include "common.h"
#include "types.h"

/************************************************************
* 标准输入输出设备
*************************************************************/
#define DEV_FLAGS_INPUT       0x00000001  /* 设备可以当作输入设备 */
#define DEV_FLAGS_OUTPUT      0x00000002  /* 设备可以当作输出设备 */
#define DEV_FLAGS_SYSTEM      0x80000000  /* 设备是系统设备 */
#define DEV_EXT_VIDEO        0x00000001  /* 设备支持视频显示 */


/************************************************************
* 设备信息
*************************************************************/
struct stdio_dev
{
    int32_t flags; /* 设备标记: 输入/输出/系统 */
    int32_t ext; /* 设备扩展信息 */
    int8_t name[16]; /* 设备名字 */

    /* 通用函数 */
    int32_t (*start)(void); /* 启动设备 */
    int32_t (*stop)(void); /* 停止设备 */

    /* 输出函数 */
    void (*putc)(const int8_t c); /* 发送一个字节 */
    void (*puts)(const int8_t *s); /*发送一串字符 */

    /* 输入函数 */
    int32_t (*tstc)(void); /* 检测一个字节是否接收完成 */
    int32_t (*getc)(void); /* 接收一个字节 */

    /* 其他函数 */
    void *priv;   /* 私有扩展 */
    struct list_head list;
};

/**************************************************************
* 视频标志
***************************************************************/
#define VIDEO_FORMAT_RGB_INDEXED         0x0000
#define VIDEO_FORMAT_RGB_DIRECTCOLOR     0x0001
#define VIDEO_FORMAT_YUYV_4_4_4          0x0010
#define VIDEO_FORMAT_YUYV_4_2_2          0x0011


/**************************************************************
* 视频扩展信息
***************************************************************/
typedef struct
{
    void *address;
    int16_t width;
    int16_t height;
    int8_t format;
    int8_t color;
    void (*setcolreg)(int32_t, int32_t, int32_t, int32_t);
    void (*getcolreg)(int32_t, void *);
}VIDEO_EXT, *PVIDEO_EXT;

/**************************************************************
* 变量
***************************************************************/
extern struct stdio_dev *stdio_devices[];
extern int8_t *stdio_names[MAX_FILES];


/* 外部调用函数 */
extern int32_t stdio_init(void);
extern int32_t stdio_register(__in struct stdio_dev *pdev);
#ifdef CONFIG_SYS_STDIO_DEREGISTER
extern int32_t stdio_deregister(__in const int8_t *pname);
#endif
extern struct stdio_dev *stdio_get_by_name(__in const int8_t *name);
extern struct list_head *stdio_get_list(void);
extern struct stdio_dev *stdio_clone(__in struct stdio_dev *pdev);




#endif /* _STDIO_DEV_H_ */
