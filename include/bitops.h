#ifndef _BITOPS_H_
  #define _BITOPS_H_

#include "stddef.h"



/**/
#define BIT_MASK(nr)    (1UL << ((nr) % BITS_PER_LONG))

/**/
#define BIT_WORD(nr)    ((nr) / BITS_PER_LONG)


#ifndef __SET_BIT
#define __set_bit generic_set_bit
#endif

#ifndef __CLEAR_BIT
#define __clear_bit generic_clear_bit
#endif

#ifndef __FFS
#define ffs generic_ffs
#endif

#ifndef __FLS
#define fls generic_fls
#endif


/********************************************************************************
* 函数: static inline int32_t generic_ffs(__in int32_t x)
* 描述: 找到第一个置1的位，从低位往高位找
* 输入: x: 需要查找的数据
* 输出: none
* 返回: 第一个置1的位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline int32_t generic_ffs(__in int32_t x)
{
    int32_t r = 1;

    if(!x)
        return 0;

    if(!(x & 0xffff))
    {
        x >>= 16;
        r += 16;
    }

    if(!(x & 0xff))
    {
        x >>= 8;
        r += 8;
    }

    if(!(x & 0x0f))
    {
        x >>= 4;
        r += 4;
    }

    if(!(x & 0x03))
    {
        x >>= 2;
        r += 2;
    }

    if(!(x & 0x01))
    {
        x >>= 1;
        r += 1;
    }

    return r;
}

/********************************************************************************
* 函数: static inline int32_t generic_fls(__in int32_t x)
* 描述: 找到最后一个置1的位置
* 输入: x: 需要查找的数据
* 输出: none
* 返回: 最后一个置1的位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline int32_t generic_fls(__in int32_t x)
{
    int32_t r = 32;

    if(!x)
        return 0;

    if(!(x & 0xffff0000u))
    {
        x <<= 16;
        r -= 16;
    }

    if(!(x & 0xff000000u))
    {
        x <<= 8;
        r -= 8;
    }

    if(!(x & 0xf0000000u))
    {
        x <<= 4;
        r -= 4;
    }

    if(!(x & 0xc0000000u))
    {
        x <<= 2;
        r -= 2;
    }

    if(!(x & 0x80000000u))
    {
        x <<= 1;
        r -= 1;
    }

    return r;
}

/********************************************************************************
* 函数: static inline uint32_t generic_hweight32(__in uint32_t w)
* 描述: 计算32位汉明权重
* 输入: w: 需要计算的数据
* 输出: none
* 返回: 数据中为1的位的个数
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline uint32_t generic_hweight32(__in uint32_t w)
{
    uint32_t res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
    res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
    res = (res & 0x0f0f0f0f) + ((res >> 4) & 0x0f0f0f0f);
    res = (res & 0x00ff00ff) + ((res >> 8) & 0x00ff00ff);
    return (res & 0x0000ffff) + ((res >> 16) & 0x0000ffff);
}

/********************************************************************************
* 函数: static inline uint32_t generic_hweight16(__in uint16_t w)
* 描述: 计算16位汉明权重
* 输入: w: 需要计算的数据
* 输出: none
* 返回: 数据中为1的位的个数
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline uint32_t generic_hweight16(__in uint16_t w)
{
    uint16_t res = (w & 0x5555) + ((w >> 1) & 0x5555);
    res = (res & 0x3333) + ((res >> 2) & 0x3333);
    res = (res & 0x0f0f) + ((res >> 4) & 0x0f0f);
    return (res & 0x00ff) + ((res >> 8) & 0x00ff);

}

/********************************************************************************
* 函数: static inline uint32_t generic_hweight8(__in uint8_t w)
* 描述: 计算8位汉明权重
* 输入: w: 需要计算的数据
* 输出: none
* 返回: 数据中为1的位的个数
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline uint32_t generic_hweight8(__in uint8_t w)
{
    uint8_t res = (w & 0x55) + ((w >> 1) & 0x55);
    res = (res & 0x33) + ((res >> 2) & 0x33);
    return (res & 0x0f) + ((res >> 4) & 0x0f);
}

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void generic_set_bit(__in int32_t nr, __in volatile size_t *addr)
{
    size_t mask = BIT_MASK(nr);
    size_t *p = ((size_t *)addr) + BIT_WORD(nr);

    *p |= mask;
}

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline void generic_clear_bit(__in int32_t nr, __in volatile size_t *addr)
{
    size_t mask = BIT_MASK(nr);
    size_t *p = ((size_t *)addr) + BIT_WORD(nr);

    *p &= ~mask;
}











#endif

