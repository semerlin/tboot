#ifndef _SERIAL_DEF_H_
  #define _SERIAL_DEF_H_

#include "types.h"


#define NAMESIZE    16
#define CTRLSIZE    8

struct serial_device
{
    int8_t name[NAMESIZE];   //串口设备名称
    int8_t ctrl[CTRLSIZE];

    int32_t (*init)(void);   //初始化串口设备
    int32_t (*deinit)(void);  //停止串口设备
    void (*setbrg)(void);  //设置串口寄存器
    int32_t (*getc)(void);  //取得一个字节
    int32_t (*tstc)(void); //测试一个字节是否接受完毕
    void (*putc)(const int8_t c); //发送一个字节
    void (*puts)(const int8_t *s); //发送一串字符串

    struct serial_device *next;  //serial设备单向链表
};











#endif

