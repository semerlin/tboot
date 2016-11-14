#ifndef _COMMON_H_
  #define _COMMON_H_


#include <stdarg.h>
#include "stddef.h"
#include "types.h"

/********************************************************
* 输入输出基础函数
*********************************************************/
#define stdin         0
#define stdout        1
#define stderr        2
#define MAX_FILES     3

extern int32_t getc(void);
extern int32_t tstc(void);
extern void putc(__in const int8_t c);
extern void puts(__in const int8_t *s);
extern void printf(__in const int8_t *fmt, ...);
extern void vprintf(__in const int8_t *fmt, va_list args);
extern int32_t ctrlc(void);


/********************************************************
* 延时基础函数
*********************************************************/
void reset_timer(void);
uint32_t get_timer(__in uint32_t base);
void set_timer(__in uint32_t t);
extern void udelay(__in uint32_t usec);
extern void mdelay(__in uint32_t msec);
#define ndelay(x)    udelay(1)

/********************************************************
* 格式化函数
*********************************************************/
extern int vsprintf(__out char *buf, __in const char *fmt, __in va_list args);
extern int sprintf(__out char * buf, __in const char *fmt, ...);








#endif /* _COMMON_H_ */
