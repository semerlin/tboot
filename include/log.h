#ifndef _LOG_H_
  #define _LOG_H_

#include "config.h"
#include "common.h"


//#define LOG_LEVEL_QUIET    -1  //不打印信息
#define LOG_LEVEL_ERR      0   //打印错误信息
#define LOG_LEVEL_INFO     0   //打印硬件信息
#define LOG_LEVEL_WARN     1   //打印警告和错误信息
#define LOG_LEVEL_MSG      2   //打印所有信息




#ifndef CONFIG_LOG_LEVEL
  #error "error: you need to config log level!"
#endif

#if (CONFIG_LOG_LEVEL == 0)
  #define LOG_LEVEL    LOG_LEVEL_ERR
#elif (CONFIG_LOG_LEVEL == 1)
  #define LOG_LEVEL    LOG_LEVEL_WARN
#elif (CONFIG_LOG_LEVEL == 2)
  #define LOG_LEVEL    LOG_LEVEL_MSG
#else
  #define LOG_LEVEL    LOG_LEVEL_MSG
#endif



#define printl(n, args...)    \
    do                        \
    {                         \
        if(n <= LOG_LEVEL)    \
            printf(args);     \
    }while(0)



#ifndef BUG
#define BUG()  \
    do         \
    {          \
        printf("BUG: failure at %s: %d/%s()!\n", __FILE__, __LINE__, __FUNCTION__);    \
    }while(0);

#define BUG_ON(condition)    \
    do                       \
    {                        \
        if(unlikely((condition) != 0))    \
            BUG();           \
    }while(0);

#endif

#endif

