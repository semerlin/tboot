#ifndef _MATH_H_
  #define _MATH_H_

#include "types.h"


#define do_div(n, base)                 \
({                                      \
    int32_t __res;                      \
    __res = ((uint32_t)n) % base;       \
    n = ((uint32_t) n) / base;          \
    __res;                              \
})

#define min_t(type, x, y)           \
({                                  \
      type __x = (x);               \
      type __y = (y);               \
      __x < __y ? __x : __y;        \
})



#define max_t(type, x, y)           \
({                                  \
      type __x = (x);               \
      type __y = (y);               \
      __x > __y ? __x : __y;        \
})


#define abs(x)                  \
({                              \
    int64_t __x = (x);          \
    (__x < 0) ? -__x : __x;     \
})




#endif

