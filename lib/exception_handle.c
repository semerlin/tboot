#include <stdarg.h>
#include "exception_handle.h"
#include "common.h"



/********************************************************************************
* 函数: void panic(__in const int8_t *fmt, ...)
* 描述: 致命错误出现的处理方式
* 输入: fmt: 错误描述
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void panic(__in const int8_t *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    while(1);
}
