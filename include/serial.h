#ifndef _SERIAL_H_
  #define _SERIAL_H_

#include "types.h"
#include "serial_def.h"

extern int32_t serial_register(struct serial_device *pDev);
extern void serial_initialize(void);
extern void serial_stdio_init(void);
extern int32_t serial_assign(const int8_t *name);
extern void serial_reinit_all(void);
extern void serial_deinit(void);
extern void serial_setbrg(void);
extern int32_t serial_getc(void);
extern int32_t serial_tstc(void);
extern void serial_putc(const int8_t c);
extern void serial_puts(const int8_t *s);

/* 需要板级实现，绑定串口板级函数 */
extern void serial_bind(struct serial_device *dev);



#endif


