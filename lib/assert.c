#include "stddef.h"
#include "assert.h"
#include "common.h"
#include "exception_handle.h"



/********************************************************************************
* 函数: void _Assert(__in int8_t *exp, __in int8_t *file, __in int8_t *line)
* 描述: 断言异常处理
* 输入: exp: 断言失败的表达式
       file: 断言失败的文件
       line: 断言失败的行号
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void _Assert(__in int8_t *exp, __in int8_t *file, __in int8_t *line)
{
    printf("[ASSERT:ERR] exp(%s), file(%s), line(%s)\n", exp, file, line);
    panic("[PANIC:ERR] assert failed!");
}
