#ifndef _SWAB_H_
  #define _SWAB_H_

#include "types.h"

#define __swab16(x) \
  ((uint16_t)( \
      (((uint16_t)(x) & (uint16_t)0x00ffU) << 8) | \
      (((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))


#define __swab32(x) \
  ((uint32_t)( \
      (((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) | \
      (((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) | \
      (((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) | \
      (((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))


#define __swab64(x) \
  ((uint64_t)( \
      (((uint64_t)(x) & (uint64_t)0x00000000000000ffULL) << 56) | \
      (((uint64_t)(x) & (uint64_t)0x000000000000ff00ULL) << 40) | \
      (((uint64_t)(x) & (uint64_t)0x0000000000ff0000ULL) << 24) | \
      (((uint64_t)(x) & (uint64_t)0x00000000ff000000ULL) <<  8) | \
      (((uint64_t)(x) & (uint64_t)0x000000ff00000000ULL) >>  8) | \
      (((uint64_t)(x) & (uint64_t)0x0000ff0000000000ULL) >> 24) | \
      (((uint64_t)(x) & (uint64_t)0x00ff000000000000ULL) >> 40) | \
      (((uint64_t)(x) & (uint64_t)0xff00000000000000ULL) >> 56)))



#define __swap16p(x) __swap16(*(x))
#define __swap32p(x) __swap32(*(x))
#define __swap64p(x) __swap64(*(x))


#define __swap16s(x) do{ *(x) = __swap16p((x));}while(0)
#define __swap32s(x) do{ *(x) = __swap32p((x));}while(0)
#define __swap64s(x) do{ *(x) = __swap64p((x));}while(0)



#endif

