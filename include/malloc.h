#ifndef _MALLOC_H_
  #define _MALLOC_H_

#include "stddef.h"
#include "string.h"
#include "assert.h"

#ifdef __cplusplus
extern "c" {
#endif


#define malloc_getpagesize    (4096)

/* 使用 do{}while结构把函数放在一个括号里面 */
/********************************************************************************
* 函数: MALLOC_ZERO(charp, nbytes)
* 描述: 把分配的内存块清零
* 输入: charp: 内存起始地址
       nbytes: 字节数
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
#define MALLOC_ZERO(charp, nbytes)                       \
do                                                       \
{                                                        \
    size_t mzsz = (nbytes);                              \
    if(mzsz <= 9*sizeof(mzsz))                           \
    {                                                    \
        size_t *mz = (size_t*)(charp);                   \
        if(mzsz >= 5*sizeof(mzsz))                       \
        {                                                \
            *mz++ = 0;                                   \
            *mz++ = 0;                                   \
            if(mzsz >= 7*sizeof(mzsz))                   \
            {                                            \
                *mz++ = 0;                               \
                *mz++ = 0;                               \
                if(mzsz >= 9*sizeof(mzsz))               \
                {                                        \
                    *mz++ = 0;                           \
                    *mz++ = 0;                           \
                }                                        \
            }                                            \
        }                                                \
        *mz++ = 0;                                       \
        *mz++ = 0;                                       \
        *mz   = 0;                                       \
    }                                                    \
    else                                                 \
        memset((charp), 0, mzsz);                        \
}while(0);


/********************************************************************************
* 函数: MALLOC_COPY(dest,src,nbytes)
* 描述: 把分配的内存块清零
* 输入: charp: 内存起始地址
       nbytes: 字节数
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
#define MALLOC_COPY(dest,src,nbytes)                     \
do                                                       \
{                                                        \
    size_t mcsz = (nbytes);                              \
    if(mcsz <= 9*sizeof(mcsz))                           \
    {                                                    \
        size_t* mcsrc = (size_t*) (src);                 \
        size_t* mcdst = (size_t*) (dest);                \
        if(mcsz >= 5*sizeof(mcsz))                       \
        {                                                \
            *mcdst++ = *mcsrc++;                         \
			*mcdst++ = *mcsrc++;                         \
            if(mcsz >= 7*sizeof(mcsz))                   \
            {                                            \
                *mcdst++ = *mcsrc++;                     \
				*mcdst++ = *mcsrc++;                     \
                if(mcsz >= 9*sizeof(mcsz))               \
                {                                        \
                    *mcdst++ = *mcsrc++;                 \
				    *mcdst++ = *mcsrc++;                 \
                }                                        \
            }                                            \
        }                                                \
		*mcdst++ = *mcsrc++;                             \
		*mcdst++ = *mcsrc++;                             \
		*mcdst   = *mcsrc  ;                             \
    }                                                    \
    else                                                 \
        memcpy(dest, src, mcsz);                         \
}while(0);


/* mallinfo结构体 */
struct mallinfo
{
    int arena;    /* 从系统分配的总内存大小(按字节计算),调用sbrk获得 */
    int ordblks;  /* 空闲块的数量 */
    int smblks;   /* 不支持--始终为0 */
    int hblks;    /* 使用mmap分配的块的数量 */
    int hblkhd;   /* 使用mmap分配的总内存大小(按字节计算) */
    int usmblks;  /* 不支持--始终为0 */
    int fsmblks;  /* 不支持--始终为0 */
    int uordblks; /* 通过malloc分配的内存大小 */
    int fordblks; /* 总空闲空间大小 */
    int keepcost; /* 最顶部可释放的空间大小 */
};

/* mallopt选项 */

/* M_TRIM_THRESHOLD表示在使用free()中的malloc_trim时顶部可以保留的最大数量的未使用空间 */
#define M_TRIM_THRESHOLD    -1

#define M_TOP_PAD           -2
#define M_MMAP_THRESHOLD    -3
#define M_MMAP_MAX          -4


/*
  在free()中调用malloc_trim释放内存之前能保持的最大未使用内存
  在调用free()时并不立即释放内存回系统，而是保留在当前程序中，为
  了是下个malloc时可以直接分配而不用从系统申请。只有当空闲的空间达
  到DEFAULT_TRIM_THRESHOLD指定的值时才会释放内存回系统中。
*/
#ifndef DEFAULT_TRIM_THRESHOLD
#define DEFAULT_TRIM_THRESHOLD (128 * 1024)
#endif

#ifndef DEFAULT_TOP_PAD
#define DEFAULT_TOP_PAD        (0)
#endif

#ifndef DEFAULT_MMAP_THRESHOLD
#define DEFAULT_MMAP_THRESHOLD (128 * 1024)
#endif

#ifndef DEFAULT_MMAP_MAX
#if HAVE_MMAP
#define DEFAULT_MMAP_MAX       (64)
#else
#define DEFAULT_MMAP_MAX       (0)
#endif
#endif

/*
  sbrk在外部board.c中实现
*/
extern void *sbrk(ptrdiff_t size);

#define MORECORE              sbrk
#define MORECORE_FAILURE      -1
#define MORECORE_CLEARS       1



/* 外部调用函数 */
#define cALLOc		dlcalloc
#define fREe		dlfree
#define mALLOc		dlmalloc
#define mEMALIGn	dlmemalign
#define rEALLOc		dlrealloc
#define vALLOc		dlvalloc
#define pvALLOc		dlpvalloc
#define mALLINFo	dlmallinfo
#define mALLOPt		dlmallopt


void* mALLOc(__in size_t bytes);
void fREe(__in char* mem);
void* rEALLOc(__in void* oldmem, __in size_t bytes);
void* mEMALIGn(__in size_t alignment, __in size_t bytes);
void* vALLOc(__in size_t bytes);
void* pvALLOc(__in size_t bytes);
void* cALLOc(__in size_t n, __in size_t elem_size);
int malloc_trim(__in size_t pad);
size_t malloc_usable_size(__in void* mem);
void malloc_stats(void);
struct mallinfo mALLINFo(void);
int mALLOPt(__in int param_number, __in int value);





#ifdef __cplusplus
}
#endif



#endif /* _MALLOC_H_ */
