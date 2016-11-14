#ifndef _IOMUX_H_
  #define _IOMUX_H_


#include "types.h"
#include "stdio_dev.h"


extern PSTDIO_DEV *console_devices[MAX_FILES];
extern int32_t cd_count[MAX_FILES];

#if defined(CONFIG_CONSOLE_MUX)

void iomux_printdevs(__in const int32_t console);
int32_t iomux_doenv(__in const int32_t console, const int8_t *arg);

#endif








#endif


