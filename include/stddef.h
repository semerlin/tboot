#ifndef _STDDEF_H_
  #define _STDDEF_H_


#include "types.h"
#include "sizes.h"

#undef NULL
#define NULL ((void *)0)



#undef offsetof
#define offsetof(type, member) ((size_t) &((type *)0)->member)

/* 根据一个结构体变量中的一个域成员变量的指针来获取整个结构体变量的指针 */
#define container_of(ptr, type, member) (                   \
{                                                           \
    const typeof(((type *)0)->member) *__mptr = (ptr);      \
    (type *)((char *)__mptr - offsetof(type,member));       \
})


/* inline定义 */
#define inline __inline

/* 描述符 */
#define __in
#define __out
#define __inout







#endif  /* _STDDEF_H_ */



