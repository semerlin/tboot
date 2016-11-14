#ifndef _COMPILER_H_
#define _COMPILER_H_


/* 编译器指令优化 */

/* 认为x通常为1 */
#define likely(x)     __builtin_expect(!!(x), 1)

/* 认为x通常为0 */
#define unlikely(x)   __builtin_expect(!!(x), 0)














#endif

