#ifndef _BIG_ENDIAN_H_
  #define _BIG_ENDIAN_H_



/* 大端模式 */

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN  4321
#endif

#define __BYTE_ORDER  __BIG_ENDIAN

#include "types.h"
#include "swab.h"


#define __cpu_to_le64(x)    __swab64((x))
#define __le64_to_cpu(x)    __swab64((x))
#define __cpu_to_le32(x)    __swab32((x))
#define __le32_to_cpu(x)    __swab32((x))
#define __cpu_to_le16(x)    __swab16((x))
#define __le16_to_cpu(x)    __swab16((x))
#define __cpu_to_be64(x)    ((uint64_t)(x))
#define __be64_to_cpu(x)    ((uint64_t)(x))
#define __cpu_to_be32(x)    ((uint32_t)(x))
#define __be32_to_cpu(x)    ((uint32_t)(x))
#define __cpu_to_be16(x)    ((uint16_t)(x))
#define __be16_to_cpu(x)    ((uint16_t)(x))
#define __cpu_to_le64p(x)   __swab64p((x))
#define __le64_to_cpup(x)   __swab64p((x))
#define __cpu_to_le32p(x)   __swab32p((x))
#define __le32_to_cpup(x)   __swab32p((x))
#define __cpu_to_le16p(x)   __swab16p((x))
#define __le16_to_cpup(x)   __swab16p((x))
#define __cpu_to_be64p(x)   (*(uint64_t*)(x))
#define __be64_to_cpup(x)   (*(uint64_t*)(x))
#define __cpu_to_be32p(x)   (*(uint32_t*)(x))
#define __be32_to_cpup(x)   (*(uint32_t*)(x))
#define __cpu_to_be16p(x)   (*(uint16_t*)(x))
#define __be16_to_cpup(x)   (*(uint16_t*)(x))
#define __cpu_to_le64s(x)   __swab64s((x))
#define __le64_to_cpus(x)   __swab64s((x))
#define __cpu_to_le32s(x)   __swab32s((x))
#define __le32_to_cpus(x)   __swab32s((x))
#define __cpu_to_le16s(x)   __swab16s((x))
#define __le16_to_cpus(x)   __swab16s((x))
#define __cpu_to_be64s(x)   do {} while (0)
#define __be64_to_cpus(x)   do {} while (0)
#define __cpu_to_be32s(x)   do {} while (0)
#define __be32_to_cpus(x)   do {} while (0)
#define __cpu_to_be16s(x)   do {} while (0)
#define __be16_to_cpus(x)   do {} while (0)
















#endif

