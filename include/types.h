#ifndef _TYPES_H_
  #define _TYPES_H_


/* 通用定义 */
typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long long int64_t;
typedef unsigned long long uint64_t;

#if 0
typedef char *pint8_t;
typedef unsigned char *puint8_t;

typedef short *pint16_t;
typedef unsigned short *puint16_t;

typedef int *pint32_t;
typedef unsigned int *puint32_t;

typedef long long *pint64_t;
typedef unsigned long long *puint64_t;
#endif


/* 布尔量 */
//#ifndef __bool_true_false_are_defined
//#define __bool_true_false_are_defined 1

#undef bool
#undef true
#undef false

#define bool   int
#define true   1
#define false  0

#define BITS_PER_LONG   32


/* 机器相关类型 */
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef long ssize_t;
#endif


#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef int ptrdiff_t;   //机器相关类型，通常用来保存两个指针减法操作的结果
#endif

#ifndef _LOFF_T
#define _LOFF_T
typedef long long loff_t;
#endif


#ifndef _PHYS_ADDR_T
#define _PHYS_ADDR_T
typedef unsigned long phys_addr_t;
#endif

#endif /* _TYPES_H_ */


