#include "malloc.h"
#include "common.h"
#include "global_data.h"


DECLARE_GLOBAL_DATA_PTR;

/*
  内存块的结构体，保存在分配好的内存块之前

  块是8字节对齐

  有两种类型的chunk，已分配的chunk和未分配的chunk，两者交错排列，
  可能出现相邻的两个chunk都是使用，不会出现相邻的两个chunk都是空闲，
  占据了整个heap空间。注意，没有相邻的两个未分配chunk，因为在调用
  free()释放被使用过的chunk时，dlmalloc将合并任何相邻的空闲chunk。
*/
struct malloc_chunk
{
    size_t prev_size;           /* 前一个块的大小，必须前一个块空闲时才有用 */
    size_t size;                /* 本内存块的大小，包括结构体数据和后面可用空间的大小，size对齐，最低3位始终为0 */

    /*
      双向链表(用来链接空闲的内存块)
      块在使用时，此两指针是无效的
    */
    struct malloc_chunk *fd; /* 指向下一个 */
    struct malloc_chunk *bk; /* 指向上一个 */
};

typedef struct malloc_chunk* mchunkptr;


/* 大小，对齐 */
#define SIZE_SZ                (sizeof(size_t))
#define MALLOC_ALIGNMENT       (SIZE_SZ + SIZE_SZ)
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#define MINSIZE                (sizeof(struct malloc_chunk))


/* 内存地址和块地址转换 */
#define chunk2mem(p) ((char*)(p) + 2*SIZE_SZ)
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))


/* 把需求的字节转换为合适大小和对齐的字节 */
#define request2size(req) \
(((long)((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) < \
(long)(MINSIZE + MALLOC_ALIGN_MASK)) ? MINSIZE : \
(((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) & ~(MALLOC_ALIGN_MASK)))


/* 检测数据是否对齐 */
#define aligned_OK(m)    (((unsigned long)((m)) & (MALLOC_ALIGN_MASK)) == 0)


/*
  块操作选项
*/

/* 标记前一块是否已经被使用，存储在size的最低一个字节 */
#define PREV_INUSE   0x01

/* 标记此内存块是否为映射空间 */
#define IS_MMAPPED   0x02

/* 取得真实大小时需要屏蔽的位 */
#define SIZE_BITS (PREV_INUSE|IS_MMAPPED)
//#define SIZE_BITS    0x03


/* 计算下一块地址，size因为是8字节对齐所以低3位始终为0，不影响计算大小*/
//#define next_chunk(p) ((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))
 #define next_chunk(p) ((mchunkptr)(((char*)(p)) + ((p)->size & ~SIZE_BITS)))

/* 计算上一块地址 */
#define prev_chunk(p) ((mchunkptr)(((char*)(p)) - ((p)->prev_size)))


/* 把一个地址指针加上偏移量之后的地址当作是一个块 */
#define chunk_at_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))



/*
  处理PREV_INUSE位
*/

/* 计算本内存块是否被使用 */
#define inuse(p)\
((((mchunkptr)(((char*)(p))+((p)->size & ~SIZE_BITS)))->size) & PREV_INUSE)

/* 计算上一块是否被使用 */
#define prev_inuse(p)  ((p)->size & PREV_INUSE)

/* 计算本块是否是映射空间 */
#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)

/* 设置本内存块在使用 */
#define set_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~SIZE_BITS)))->size |= PREV_INUSE

/* 设置本内存块不在使用 */
#define clear_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~SIZE_BITS)))->size &= ~PREV_INUSE

/* 检测/设置/清除 指定地址的PREV_INUSE位 */
#define inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size & PREV_INUSE)

#define set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size |= PREV_INUSE)

#define clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size &= ~PREV_INUSE)



/*
  处理size字段
*/

/* 计算本块的大小 */
#define chunksize(p)          ((p)->size & ~(SIZE_BITS))

/* 设置当前块大小，不影响标记位 */
#define set_head_size(p, s)   ((p)->size = (((p)->size & SIZE_BITS) | (s)))

/* 设置当前块大小(忽略标记位) */
#define set_head(p, s)        ((p)->size = (s))

/* 设置下一块中当前块的大小(仅当当前块不在使用时有效) */
#define set_foot(p, s)   (((mchunkptr)((char*)(p) + (s)))->prev_size = (s))




/*
   箱体 每个箱体管理固定大小的内存块，内存块之间以链表链接

    分布图:

    64 箱体间隔字节数       8     //16 24 32 ...
    32 箱体间隔字节数      64
    16 箱体间隔字节数     512
     8 箱体间隔字节数    4096
     4 箱体间隔字节数   32768
     2 箱体间隔字节数  262144
     1 箱体间隔字节数 what's left

     "top"和"last_remainder"有自己的箱体
*/

/* 箱子的个数 */
#define NAV             128

/* 箱子结构体指针 */
typedef struct malloc_chunk* mbinptr;

/* 箱体获取 */
#define bin_at(i)      ((mbinptr)((char*)&(av_[2*(i) + 2]) - 2*SIZE_SZ))
#define next_bin(b)    ((mbinptr)((char*)(b) + 2 * sizeof(mbinptr)))
#define prev_bin(b)    ((mbinptr)((char*)(b) - 2 * sizeof(mbinptr)))

/*
  一开始的两个箱体永远不会被索引到
*/

#define top            (av_[2])          /* 最顶端的块 */
#define last_remainder (bin_at(1))       /* 指向最近一次分隔后剩下的块 */


/*
   指向第一个箱体的指针
*/
#define initial_top    ((mchunkptr)(bin_at(0)))

/* 初始化av_数组 */
#define IAV(i)  bin_at(i), bin_at(i)

/*
   IAV(0)是top箱体
   第一个箱体从IAV(1)位置开始
   av_[1]留给块的bitmap使用
*/
static mbinptr av_[NAV * 2 + 2] =
{
    0, 0,
    IAV(0),   IAV(1),   IAV(2),   IAV(3),   IAV(4),   IAV(5),   IAV(6),   IAV(7),
    IAV(8),   IAV(9),   IAV(10),  IAV(11),  IAV(12),  IAV(13),  IAV(14),  IAV(15),
    IAV(16),  IAV(17),  IAV(18),  IAV(19),  IAV(20),  IAV(21),  IAV(22),  IAV(23),
    IAV(24),  IAV(25),  IAV(26),  IAV(27),  IAV(28),  IAV(29),  IAV(30),  IAV(31),
    IAV(32),  IAV(33),  IAV(34),  IAV(35),  IAV(36),  IAV(37),  IAV(38),  IAV(39),
    IAV(40),  IAV(41),  IAV(42),  IAV(43),  IAV(44),  IAV(45),  IAV(46),  IAV(47),
    IAV(48),  IAV(49),  IAV(50),  IAV(51),  IAV(52),  IAV(53),  IAV(54),  IAV(55),
    IAV(56),  IAV(57),  IAV(58),  IAV(59),  IAV(60),  IAV(61),  IAV(62),  IAV(63),
    IAV(64),  IAV(65),  IAV(66),  IAV(67),  IAV(68),  IAV(69),  IAV(70),  IAV(71),
    IAV(72),  IAV(73),  IAV(74),  IAV(75),  IAV(76),  IAV(77),  IAV(78),  IAV(79),
    IAV(80),  IAV(81),  IAV(82),  IAV(83),  IAV(84),  IAV(85),  IAV(86),  IAV(87),
    IAV(88),  IAV(89),  IAV(90),  IAV(91),  IAV(92),  IAV(93),  IAV(94),  IAV(95),
    IAV(96),  IAV(97),  IAV(98),  IAV(99),  IAV(100), IAV(101), IAV(102), IAV(103),
    IAV(104), IAV(105), IAV(106), IAV(107), IAV(108), IAV(109), IAV(110), IAV(111),
    IAV(112), IAV(113), IAV(114), IAV(115), IAV(116), IAV(117), IAV(118), IAV(119),
    IAV(120), IAV(121), IAV(122), IAV(123), IAV(124), IAV(125), IAV(126), IAV(127)
};

/* 地址重映射 */
void malloc_bin_reloc(void)
{
	uint32_t *p = (uint32_t *)(&av_[2]);
    int32_t i;
	for(i = 2; i < (sizeof(av_) / sizeof(mbinptr)); ++i)
	{
		*p++ += gd->reloc_off;
	}
}


/* 遍历指针 */
#define first(b) ((b)->fd) /* 指向前一块 */
#define last(b)  ((b)->bk) /* 指向下一块 */

/*
  通过大小计算箱体的索引
*/
#define bin_index(sz)                                                          \
(((((unsigned long)(sz)) >> 9) ==    0) ?       (((unsigned long)(sz)) >>  3): \
 ((((unsigned long)(sz)) >> 9) <=    4) ?  56 + (((unsigned long)(sz)) >>  6): \
 ((((unsigned long)(sz)) >> 9) <=   20) ?  91 + (((unsigned long)(sz)) >>  9): \
 ((((unsigned long)(sz)) >> 9) <=   84) ? 110 + (((unsigned long)(sz)) >> 12): \
 ((((unsigned long)(sz)) >> 9) <=  340) ? 119 + (((unsigned long)(sz)) >> 15): \
 ((((unsigned long)(sz)) >> 9) <= 1364) ? 124 + (((unsigned long)(sz)) >> 18): \
					  126)
/*
  大小小于512字节的箱体都是以8个字节间隔，大小从16-512，分布在63个箱体中
*/
#define MAX_SMALLBIN          63
#define MAX_SMALLBIN_SIZE     512
#define SMALLBIN_WIDTH        8

#define smallbin_index(sz)  (((unsigned long)(sz)) >> 3)

/*
   检测需要分配的空间大小属于smallbin范畴
*/

#define is_small_request(nb) (nb < (MAX_SMALLBIN_SIZE - SMALLBIN_WIDTH))



/*
    即用binblocks建立了所有分箱的一个bitmap，binblocks的bit来表示
    连续的4个相邻的bin是否为空，只要有一个不为空，其对应的bit置1。
    binblocks实际上是av_[1]，一个32位数据类型，32×4=128，正好对应
    128个bins。扫描时先判断binblocks的相应位，只有当bit不为0时才会
    真正的去扫描对应的bin。
*/

#define BINBLOCKWIDTH     4   /* 每一位代表4个箱体 */

#define binblocks_r     ((size_t)av_[1]) /* 存放bitmap的位置 */
#define binblocks_w     (av_[1])

/* 箱体和标记位之间的转换 */
#define idx2binblock(ix)    ((unsigned)1 << (ix / BINBLOCKWIDTH))
#define mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#define clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))




/*  其他静态变量 */

/* 可调变量 */
static unsigned long trim_threshold   = DEFAULT_TRIM_THRESHOLD;
static unsigned long top_pad          = DEFAULT_TOP_PAD;
static unsigned int  n_mmaps_max      = DEFAULT_MMAP_MAX;
static unsigned long mmap_threshold   = DEFAULT_MMAP_THRESHOLD;

/* 调用sbrk返回的第一个值, 没有调用sbrk分配内存之前，sbrk_base的值为-1 */
static char *sbrk_base = (char*)(-1);

/* 通过sbrk从系统得到的最大内存 */
static unsigned long max_sbrked_mem = 0;

/* 通过sbrk或者mmap从系统得到的最大内存 */
static unsigned long max_total_mem = 0;

/* 内部使用的mallinfo */
static struct mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* 通过sbrk从系统获得的总内存 */
#define sbrked_mem  (current_mallinfo.arena)

/* 跟踪mmaps */
static unsigned long mmapped_mem = 0;
#if HAVE_MMAP
static unsigned int max_n_mmaps = 0;
static unsigned long max_mmapped_mem = 0;
#endif


/*
  调试支持
*/

#ifndef NDEBUG
/*
  这些函数对一些任何时刻都应该为真的数据结构做了assert判断。如果任何一个
  不为真，那么就有可能是用户的程序搞跨的内存。(也有可能是此dlmalloc.c中函数
  的bug)。
*/


/********************************************************************************
* 函数: static void do_check_chunk(__in mchunkptr p)
* 描述: 检测块地址是否合法
* 输入: mchunkptr: 块指针
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void do_check_chunk(__in mchunkptr p)
{
    size_t sz = (p->size & ~SIZE_BITS);

    /* mmapped块不可以检测 */
    assert(!chunk_is_mmapped(p));

    /* 检测地址是否合法 */
    assert((char*)p >= sbrk_base);
    if (p != top)
        assert((char*)p + sz <= (char*)top);
    else
        assert((char*)p + sz <= sbrk_base + sbrked_mem);
}


/********************************************************************************
* 函数: static void do_check_free_chunk(__in mchunkptr p)
* 描述: 检测内存块是否空闲
* 输入: p: 内存块地址
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void do_check_free_chunk(__in mchunkptr p)
{
    size_t sz = (p->size & ~SIZE_BITS);

    mchunkptr next = chunk_at_offset(p, sz);


    do_check_chunk(p);

    /* 确定当前块空闲*/
    assert(!inuse(p));

    /* 除非是特殊标记域，否则sz的大小必须大于或等于MINSIZE */
    if ((long)sz >= (long)MINSIZE)
    {
        assert((sz & MALLOC_ALIGN_MASK) == 0);
        assert(aligned_OK(chunk2mem(p)));
        /* 确认大小 */
        assert(next->prev_size == sz);
        /* 确认完全合并 */
        assert(prev_inuse(p));
        assert (next == top || inuse(next));

        /* 确认空闲链表 */
        assert(p->fd->bk == p);
        assert(p->bk->fd == p);
    }
    else /* p指向的是一个特殊标记域而不是一个箱体，特殊标记域始终只有4个字节 */
        assert(sz == SIZE_SZ);
}

/********************************************************************************
* 函数: static void do_check_inuse_chunk(__in mchunkptr p)
* 描述: 检测内存块是否在使用
* 输入: p:指向内存块的指针
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void do_check_inuse_chunk(__in mchunkptr p)
{
    mchunkptr next = next_chunk(p);
    do_check_chunk(p);

    /* 确认当前块正在使用 */
    assert(inuse(p));

    /*
      检测周围块是否合法
    */
    if (!prev_inuse(p))
    {
        mchunkptr prv = prev_chunk(p);
        assert(next_chunk(prv) == p);
        do_check_free_chunk(prv);
    }
    if (next == top)
    {
        assert(prev_inuse(next));
        assert(chunksize(next) >= MINSIZE);
    }
    else if (!inuse(next))
        do_check_free_chunk(next);

}

/********************************************************************************
* 函数: static void do_check_malloced_chunk(__in mchunkptr p, __in size_t s)
* 描述: 检测动态分配的内存块
* 输入: p: 指向内存块的指针
       s:
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void do_check_malloced_chunk(__in mchunkptr p, __in size_t s)
{
  size_t sz = (p->size & ~PREV_INUSE);
  long room = sz - s;

  do_check_inuse_chunk(p);

  /* 大小合法 */
  assert((long)sz >= (long)MINSIZE);
  assert((sz & MALLOC_ALIGN_MASK) == 0);
  assert(room >= 0);
  assert(room < (long)MINSIZE);

  /* 对齐检测 */
  assert(aligned_OK(chunk2mem(p)));


  /* ... and was allocated at front of an available chunk */
  assert(prev_inuse(p));

}


#define check_free_chunk(P)           do_check_free_chunk(P)
#define check_inuse_chunk(P)          do_check_inuse_chunk(P)
#define check_chunk(P)                do_check_chunk(P)
#define check_malloced_chunk(P,N)     do_check_malloced_chunk(P,N)
#else
#define check_free_chunk(P)
#define check_inuse_chunk(P)
#define check_chunk(P)
#define check_malloced_chunk(P,N)
#endif





/********************************************************************************
* 函数: frontlink(P, S, IDX, BK, FD)
* 描述: 把大小为S的内存放到属于自己的箱体中,大小小于MAX_SMALLBIN_SIZE的块直接插入到链表头，
       大于MAX_SMALLBIN_SIZE的块按降序插入到链表中合适位置
* 输入: P: 指向内存块的指针
       S: 内存块的大小
       IDX: 箱体块的位置
       BK: 指向上一空闲块的指针，临时中间变量
       FD: 指向下一空闲块的指针，临时中间变量
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
#define frontlink(P, S, IDX, BK, FD)                                              \
{                                                                                 \
    if(S < MAX_SMALLBIN_SIZE)                                                     \
    {                                                                             \
        IDX = smallbin_index(S);                                                  \
        mark_binblock(IDX);                                                       \
        BK = bin_at(IDX);                                                         \
        FD = BK->fd;                                                              \
        P->bk = BK;                                                               \
        P->fd = FD;                                                               \
        FD->bk = BK->fd = P;                                                      \
    }                                                                             \
    else                                                                          \
    {                                                                             \
        IDX = bin_index(S);                                                       \
        BK = bin_at(IDX);                                                         \
        FD = BK->fd;                                                              \
        if (FD == BK)                                                             \
            mark_binblock(IDX);                                                   \
        else                                                                      \
        {                                                                         \
            while ((FD != BK) && (S < chunksize(FD)))                             \
                FD = FD->fd;                                                      \
            BK = FD->bk;                                                          \
        }                                                                         \
        P->bk = BK;                                                               \
        P->fd = FD;                                                               \
        FD->bk = BK->fd = P;                                                      \
    }                                                                             \
}



/********************************************************************************
* 函数: unlink(P, BK, FD)
* 描述: 把内存块从空闲内存块链表中断开
* 输入: P: 指向空闲内存块的指针
       BK: 指向上一空闲块的指针，临时中间变量
       FD: 指向下一空闲块的指针，临时中间变量
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
#define unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/********************************************************************************
* 函数: link_last_remainder(P)
* 描述: 把p指向的块设置为lastremainder
* 输入: P: 指向内存块的指针
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
#define link_last_remainder(P)                                                \
{                                                                             \
  last_remainder->fd = last_remainder->bk =  P;                               \
  P->fd = P->bk = last_remainder;                                             \
}

/********************************************************************************
* 函数: clear_last_remainder
* 描述: 清空lastremainder
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
#define clear_last_remainder \
  (last_remainder->fd = last_remainder->bk = last_remainder)






/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: V1.0
**********************************************************************************/
#if HAVE_MMAP
static mchunkptr mmap_chunk(size_t size)
{
    size_t page_mask = malloc_getpagesize - 1;
    mchunkptr p;

    #ifndef MAP_ANONYMOUS
    static int fd = -1;
    #endif

    if(n_mmaps >= n_mmaps_max)
        return 0; /* too many regions */

    /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
    * there is no following chunk whose prev_size field could be used.
    */
    size = (size + SIZE_SZ + page_mask) & ~page_mask;

    #ifdef MAP_ANONYMOUS
    p = (mchunkptr)mmap(0, size, PROT_READ|PROT_WRITE,
    MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    #else /* !MAP_ANONYMOUS */
    if (fd < 0)
    {
        fd = open("/dev/zero", O_RDWR);
        if(fd < 0)
            return 0;
    }
    p = (mchunkptr)mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    #endif

    if(p == (mchunkptr)-1) return 0;

    n_mmaps++;
    if (n_mmaps > max_n_mmaps)
        max_n_mmaps = n_mmaps;

    /* We demand that eight bytes into a page must be 8-byte aligned. */
    assert(aligned_OK(chunk2mem(p)));

    /* The offset to the start of the mmapped region is stored
    * in the prev_size field of the chunk; normally it is zero,
    * but that can be changed in memalign().
    */
    p->prev_size = 0;
    set_head(p, size|IS_MMAPPED);

    mmapped_mem += size;
    if ((unsigned long)mmapped_mem > (unsigned long)max_mmapped_mem)
        max_mmapped_mem = mmapped_mem;
    if ((unsigned long)(mmapped_mem + sbrked_mem) > (unsigned long)max_total_mem)
        max_total_mem = mmapped_mem + sbrked_mem;
    return p;
}

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: V1.0
**********************************************************************************/
static void munmap_chunk(mchunkptr p)
{
    size_t size = chunksize(p);
    int ret;

    assert (chunk_is_mmapped(p));
    assert(! ((char*)p >= sbrk_base && (char*)p < sbrk_base + sbrked_mem));
    assert((n_mmaps > 0));
    assert(((p->prev_size + size) & (malloc_getpagesize-1)) == 0);

    n_mmaps--;
    mmapped_mem -= (size + p->prev_size);

    ret = munmap((char *)p - p->prev_size, size + p->prev_size);

    /* munmap returns non-zero on failure */
    assert(ret == 0);
}

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: V1.0
**********************************************************************************/
#if HAVE_MREMAP
static mchunkptr mremap_chunk(mchunkptr p, size_t new_size)
{
    size_t page_mask = malloc_getpagesize - 1;
    size_t offset = p->prev_size;
    size_t size = chunksize(p);
    char *cp;

    assert (chunk_is_mmapped(p));
    assert(! ((char*)p >= sbrk_base && (char*)p < sbrk_base + sbrked_mem));
    assert((n_mmaps > 0));
    assert(((size + offset) & (malloc_getpagesize-1)) == 0);

    /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
    new_size = (new_size + offset + SIZE_SZ + page_mask) & ~page_mask;

    cp = (char *)mremap((char *)p - offset, size + offset, new_size, 1);

    if (cp == (char *)-1)
        return 0;

    p = (mchunkptr)(cp + offset);

    assert(aligned_OK(chunk2mem(p)));

    assert((p->prev_size == offset));
    set_head(p, (new_size - offset)|IS_MMAPPED);

    mmapped_mem -= size + offset;
    mmapped_mem += new_size;
    if ((unsigned long)mmapped_mem > (unsigned long)max_mmapped_mem)
        max_mmapped_mem = mmapped_mem;
    if ((unsigned long)(mmapped_mem + sbrked_mem) > (unsigned long)max_total_mem)
        max_total_mem = mmapped_mem + sbrked_mem;
    return p;
}

#endif /* HAVE_MREMAP */

#endif /* HAVE_MMAP */




/********************************************************************************
* 函数: static void malloc_extend_top(__in size_t nb)
* 描述: 扩展内存，当malloc分配内存不够时，扩展top的大小
* 输入: nb: 需要扩展的大小
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void malloc_extend_top(__in size_t nb)
{
    char *brk;                  /* 调用sbrk返回值 */
    size_t front_misalign; /* sbrk申请的内存顶部不可使用的字节数 */
    size_t correction;     /* bytes for 2nd sbrk call */
    char *new_brk;              /* return of 2nd sbrk call */
    size_t top_size;       /* new size of top chunk */

    mchunkptr old_top = top;  /* Record state of old top */
    size_t old_top_size = chunksize(old_top);
    char *old_end = (char*)(chunk_at_offset(old_top, old_top_size));

    /* Pad request with top_pad plus minimal overhead */
    size_t sbrk_size = nb + top_pad + MINSIZE;
    unsigned long pagesz = malloc_getpagesize;

    /* 从系统分配的内存按页对齐，不足一页的按一页分配 */
    if (sbrk_base != (char*)(-1))
        sbrk_size = (sbrk_size + (pagesz - 1)) & ~(pagesz - 1);

    brk = (char*)(MORECORE (sbrk_size));

    /* sbrk返回失败，或者其他的sbrk调用导致空间不一样 */
    if ((brk == (char*)(MORECORE_FAILURE)) || ((brk < old_end) && (old_top != initial_top)))
        return;

    /* 更新sbrk分配的内存总大小 */
    sbrked_mem += sbrk_size;

    if (brk == old_end) /* can just add bytes to current top */
    {
        top_size = sbrk_size + old_top_size;
        set_head(top, top_size | PREV_INUSE);
    }
    else
    {
        if (sbrk_base == (char*)(-1))  /* First time through. Record base */
            sbrk_base = brk;
        else  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
            sbrked_mem += brk - (char*)old_end;

        /* Guarantee alignment of first new chunk made from this space */
        front_misalign = (unsigned long)chunk2mem(brk) & MALLOC_ALIGN_MASK;
        if (front_misalign > 0)
        {
            correction = (MALLOC_ALIGNMENT) - front_misalign;
            brk += correction;
        }
        else
            correction = 0;

        /* Guarantee the next brk will be at a page boundary */

        correction += ((((unsigned long)(brk + sbrk_size))+(pagesz-1)) &
        ~(pagesz - 1)) - ((unsigned long)(brk + sbrk_size));

        /* Allocate correction */
        new_brk = (char*)(MORECORE (correction));
        if (new_brk == (char*)(MORECORE_FAILURE))
            return;

        sbrked_mem += correction;

        top = (mchunkptr)brk;
        top_size = new_brk - brk + correction;
        set_head(top, top_size | PREV_INUSE);

        if (old_top != initial_top)
        {

            /* There must have been an intervening foreign sbrk call. */
            /* A double fencepost is necessary to prevent consolidation */

            /* If not enough space to do this, then user did something very wrong */
            if (old_top_size < MINSIZE)
            {
                set_head(top, PREV_INUSE); /* will force null return from malloc */
                return;
            }

            /* Also keep size a multiple of MALLOC_ALIGNMENT */
            old_top_size = (old_top_size - 3*SIZE_SZ) & ~MALLOC_ALIGN_MASK;
            set_head_size(old_top, old_top_size);
            chunk_at_offset(old_top, old_top_size)->size = (SIZE_SZ | PREV_INUSE);
            chunk_at_offset(old_top, old_top_size + SIZE_SZ)->size = (SIZE_SZ | PREV_INUSE);
            /* If possible, release the rest. */
            if (old_top_size >= MINSIZE)
                fREe(chunk2mem(old_top));
        }
    }

    if ((unsigned long)sbrked_mem > (unsigned long)max_sbrked_mem)
        max_sbrked_mem = sbrked_mem;
    if ((unsigned long)(mmapped_mem + sbrked_mem) > (unsigned long)max_total_mem)
        max_total_mem = mmapped_mem + sbrked_mem;

    /* We always land on a page boundary */
    assert(((unsigned long)((char*)top + top_size) & (pagesz - 1)) == 0);
}



/********************************************************************************
* 函数: void* mALLOc(__in size_t bytes)
* 描述: 申请内存
* 输入: bytes: 需要申请的字节
* 输出: none
* 返回: 指向申请到内存的首地址
* 作者:
* 版本: V1.0
**********************************************************************************/
void* mALLOc(__in size_t bytes)
{
    mchunkptr victim;                  /* inspected/selected chunk */
    size_t victim_size;       /* its size */
    int       idx;                     /* index for bin traversal */
    mbinptr   bin;                     /* associated bin */
    mchunkptr remainder;               /* remainder from a split */
    long      remainder_size;          /* its size */
    int       remainder_index;         /* its bin index */
    unsigned long block;               /* block traverser bit */
    int       startidx;                /* first bin of a traversed block */
    mchunkptr fwd;                     /* misc temp for linking */
    mchunkptr bck;                     /* misc temp for linking */
    mbinptr q;                         /* misc temp */

    size_t nb;

    if ((long)bytes < 0)
        return 0;

    nb = request2size(bytes);  /* padded request size; */

    /* Check for exact match in a bin */

    if (is_small_request(nb))  /* Faster version for small requests */
    {
        idx = smallbin_index(nb);

        /* No traversal or size check necessary for small bins.  */

        q = bin_at(idx);
        victim = last(q);

        /* Also scan the next one, since it would have a remainder < MINSIZE */
        if (victim == q)
        {
            q = next_bin(q);
            victim = last(q);
        }
        if (victim != q)
        {
            victim_size = chunksize(victim);
            unlink(victim, bck, fwd);
            set_inuse_bit_at_offset(victim, victim_size);
            check_malloced_chunk(victim, nb);
            return chunk2mem(victim);
        }

        idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

    }
    else
    {
        idx = bin_index(nb);
        bin = bin_at(idx);

        for (victim = last(bin); victim != bin; victim = victim->bk)
        {
            victim_size = chunksize(victim);
            remainder_size = victim_size - nb;

            if (remainder_size >= (long)MINSIZE) /* too big */
            {
                --idx; /* adjust to rescan below after checking last remainder */
                break;
            }

            else if (remainder_size >= 0) /* exact fit */
            {
                unlink(victim, bck, fwd);
                set_inuse_bit_at_offset(victim, victim_size);
                check_malloced_chunk(victim, nb);
                return chunk2mem(victim);
            }
        }

        ++idx;

    }

    /* Try to use the last split-off remainder */

    if ( (victim = last_remainder->fd) != last_remainder)
    {
        victim_size = chunksize(victim);
        remainder_size = victim_size - nb;

        if (remainder_size >= (long)MINSIZE) /* re-split */
        {
            remainder = chunk_at_offset(victim, nb);
            set_head(victim, nb | PREV_INUSE);
            link_last_remainder(remainder);
            set_head(remainder, remainder_size | PREV_INUSE);
            set_foot(remainder, remainder_size);
            check_malloced_chunk(victim, nb);
            return chunk2mem(victim);
        }

        clear_last_remainder;

        if (remainder_size >= 0)  /* exhaust */
        {
            set_inuse_bit_at_offset(victim, victim_size);
            check_malloced_chunk(victim, nb);
            return chunk2mem(victim);
        }

        /* Else place in bin */

        frontlink(victim, victim_size, remainder_index, bck, fwd);
    }

    /*
    If there are any possibly nonempty big-enough blocks,
    search for best fitting chunk by scanning bins in blockwidth units.
    */

    if ( (block = idx2binblock(idx)) <= binblocks_r)
    {

    /* Get to the first marked block */

    if ( (block & binblocks_r) == 0)
    {
        /* force to an even block boundary */
        idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
        block <<= 1;
        while ((block & binblocks_r) == 0)
        {
            idx += BINBLOCKWIDTH;
            block <<= 1;
        }
    }

    /* For each possibly nonempty block ... */
    for (;;)
    {
        startidx = idx;          /* (track incomplete blocks) */
        q = bin = bin_at(idx);

        /* For each bin in this block ... */
        do
        {
            /* Find and use first big enough chunk ... */

            for (victim = last(bin); victim != bin; victim = victim->bk)
            {
                victim_size = chunksize(victim);
                remainder_size = victim_size - nb;

                if (remainder_size >= (long)MINSIZE) /* split */
                {
                    remainder = chunk_at_offset(victim, nb);
                    set_head(victim, nb | PREV_INUSE);
                    unlink(victim, bck, fwd);
                    link_last_remainder(remainder);
                    set_head(remainder, remainder_size | PREV_INUSE);
                    set_foot(remainder, remainder_size);
                    check_malloced_chunk(victim, nb);
                    return chunk2mem(victim);
                }

                else if (remainder_size >= 0)  /* take */
                {
                    set_inuse_bit_at_offset(victim, victim_size);
                    unlink(victim, bck, fwd);
                    check_malloced_chunk(victim, nb);
                    return chunk2mem(victim);
                }

            }

            bin = next_bin(bin);

        } while ((++idx & (BINBLOCKWIDTH - 1)) != 0);

        /* Clear out the block bit. */

        do   /* Possibly backtrack to try to clear a partial block */
        {
            if ((startidx & (BINBLOCKWIDTH - 1)) == 0)
            {
                av_[1] = (mbinptr)(binblocks_r & ~block);
                break;
            }
            --startidx;
            q = prev_bin(q);
        } while (first(q) == q);

        /* Get to the next possibly nonempty block */

        if ( (block <<= 1) <= binblocks_r && (block != 0) )
        {
            while ((block & binblocks_r) == 0)
            {
                idx += BINBLOCKWIDTH;
                block <<= 1;
            }
        }
        else
        break;
        }
    }


    /* Try to use top chunk */

    /* Require that there be a remainder, ensuring top always exists  */
    if ( (remainder_size = chunksize(top) - nb) < (long)MINSIZE)
    {

        #if HAVE_MMAP
        /* If big and would otherwise need to extend, try to use mmap instead */
        if ((unsigned long)nb >= (unsigned long)mmap_threshold &&
            (victim = mmap_chunk(nb)) != 0)
            return chunk2mem(victim);
        #endif

        /* Try to extend */
        malloc_extend_top(nb);
        if ( (remainder_size = chunksize(top) - nb) < (long)MINSIZE)
            return 0; /* propagate failure */
    }

    victim = top;
    set_head(victim, nb | PREV_INUSE);
    top = chunk_at_offset(victim, nb);
    set_head(top, remainder_size | PREV_INUSE);
    check_malloced_chunk(victim, nb);
    return chunk2mem(victim);

}



/********************************************************************************
* 函数: void fREe(char* mem)
* 描述: 释放内存
* 输入: men: 需要释放的内存的地址
* 输出: none
* 返回: 指向申请到内存的首地址
* 作者:
* 版本: V1.0
**********************************************************************************/
void fREe(char* mem)
{
    mchunkptr p;         /* chunk corresponding to mem */
    size_t hd;  /* its head field */
    size_t sz;  /* its size */
    int       idx;       /* its bin index */
    mchunkptr next;      /* next contiguous chunk */
    size_t nextsz; /* its size */
    size_t prevsz; /* size of previous contiguous chunk */
    mchunkptr bck;       /* misc temp for linking */
    mchunkptr fwd;       /* misc temp for linking */
    int       islr;      /* track whether merging with last_remainder */

    if (mem == 0)                              /* free(0) has no effect */
        return;

    p = mem2chunk(mem);
    hd = p->size;

#if HAVE_MMAP
    if (hd & IS_MMAPPED)                       /* release mmapped memory. */
    {
        munmap_chunk(p);
        return;
    }
#endif

    check_inuse_chunk(p);

    sz = hd & ~PREV_INUSE;
    next = chunk_at_offset(p, sz);
    nextsz = chunksize(next);

    if (next == top)                            /* merge with top */
    {
        sz += nextsz;

    if (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
        prevsz = p->prev_size;
        p = chunk_at_offset(p, -((long) prevsz));
        sz += prevsz;
        unlink(p, bck, fwd);
    }

        set_head(p, sz | PREV_INUSE);
        top = p;
        if ((unsigned long)(sz) >= (unsigned long)trim_threshold)
            malloc_trim(top_pad);
        return;
    }

    set_head(next, nextsz);                    /* clear inuse bit */

    islr = 0;

    if (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
        prevsz = p->prev_size;
        p = chunk_at_offset(p, -((long) prevsz));
        sz += prevsz;

        if (p->fd == last_remainder)             /* keep as last_remainder */
            islr = 1;
        else
            unlink(p, bck, fwd);
    }

    if (!(inuse_bit_at_offset(next, nextsz)))   /* consolidate forward */
    {
        sz += nextsz;

        if (!islr && next->fd == last_remainder)  /* re-insert last_remainder */
        {
            islr = 1;
            link_last_remainder(p);
        }
        else
            unlink(next, bck, fwd);
    }


    set_head(p, sz | PREV_INUSE);
    set_foot(p, sz);
    if (!islr)
        frontlink(p, sz, idx, bck, fwd);
}




/********************************************************************************
* 函数: char* rEALLOc(void* oldmem, size_t bytes)
* 描述: 重新分配内存大小
* 输入: oldmem: 旧内存地址
       bytes:新字节大小，可以大于或小于之前的内存，小于可能造成原先的数据丢失
* 输出: none
* 返回: 指向申请到内存的首地址
* 作者:
* 版本: V1.0
**********************************************************************************/
void* rEALLOc(void* oldmem, size_t bytes)
{
    size_t    nb;      /* padded request size */

    mchunkptr oldp;             /* chunk corresponding to oldmem */
    size_t    oldsize; /* its size */

    mchunkptr newp;             /* chunk to return */
    size_t    newsize; /* its size */
    void*   newmem;           /* corresponding user mem */

    mchunkptr next;             /* next contiguous chunk after oldp */
    size_t  nextsize;  /* its size */

    mchunkptr prev;             /* previous contiguous chunk before oldp */
    size_t  prevsize;  /* its size */

    mchunkptr remainder;        /* holds split off extra space from newp */
    size_t  remainder_size;   /* its size */

    mchunkptr bck;              /* misc temp for linking */
    mchunkptr fwd;              /* misc temp for linking */

#ifdef REALLOC_ZERO_BYTES_FREES
    if (bytes == 0) { fREe(oldmem); return 0; }
#endif

    if ((long)bytes < 0) return 0;

    /* realloc of null is supposed to be same as malloc */
    if (oldmem == 0) return mALLOc(bytes);

    newp    = oldp    = mem2chunk(oldmem);
    newsize = oldsize = chunksize(oldp);


    nb = request2size(bytes);

#if HAVE_MMAP
    if (chunk_is_mmapped(oldp))
    {
#if HAVE_MREMAP
        newp = mremap_chunk(oldp, nb);
        if(newp) return chunk2mem(newp);
#endif
        /* Note the extra SIZE_SZ overhead. */
        if(oldsize - SIZE_SZ >= nb) return oldmem; /* do nothing */
        /* Must alloc, copy, free. */
        newmem = mALLOc(bytes);
        if (newmem == 0) return 0; /* propagate failure */
        MALLOC_COPY(newmem, oldmem, oldsize - 2*SIZE_SZ);
        munmap_chunk(oldp);
        return newmem;
    }
#endif

    check_inuse_chunk(oldp);

    if ((long)(oldsize) < (long)(nb))
    {

        /* Try expanding forward */

        next = chunk_at_offset(oldp, oldsize);
        if (next == top || !inuse(next))
        {
            nextsize = chunksize(next);

            /* Forward into top only if a remainder */
            if (next == top)
            {
                if ((long)(nextsize + newsize) >= (long)(nb + MINSIZE))
                {
                    newsize += nextsize;
                    top = chunk_at_offset(oldp, nb);
                    set_head(top, (newsize - nb) | PREV_INUSE);
                    set_head_size(oldp, nb);
                    return chunk2mem(oldp);
                }
            }

            /* Forward into next chunk */
            else if (((long)(nextsize + newsize) >= (long)(nb)))
            {
                unlink(next, bck, fwd);
                newsize  += nextsize;
                goto split;
            }
        }
        else
        {
            next = 0;
            nextsize = 0;
        }

        /* Try shifting backwards. */

        if (!prev_inuse(oldp))
        {
            prev = prev_chunk(oldp);
            prevsize = chunksize(prev);

            /* try forward + backward first to save a later consolidation */

            if (next != 0)
            {
                /* into top */
                if (next == top)
                {
                    if ((long)(nextsize + prevsize + newsize) >= (long)(nb + MINSIZE))
                    {
                        unlink(prev, bck, fwd);
                        newp = prev;
                        newsize += prevsize + nextsize;
                        newmem = chunk2mem(newp);
                        MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
                        top = chunk_at_offset(newp, nb);
                        set_head(top, (newsize - nb) | PREV_INUSE);
                        set_head_size(newp, nb);
                        return newmem;
                    }
                }

                /* into next chunk */
                else if (((long)(nextsize + prevsize + newsize) >= (long)(nb)))
                {
                        unlink(next, bck, fwd);
                    unlink(prev, bck, fwd);
                    newp = prev;
                    newsize += nextsize + prevsize;
                    newmem = chunk2mem(newp);
                    MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
                    goto split;
                }
            }

            /* backward only */
            if (prev != 0 && (long)(prevsize + newsize) >= (long)nb)
            {
                unlink(prev, bck, fwd);
                newp = prev;
                newsize += prevsize;
                newmem = chunk2mem(newp);
                MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
                goto split;
            }
        }

        /* Must allocate */

        newmem = mALLOc (bytes);

        if (newmem == 0)  /* propagate failure */
            return 0;

        /* Avoid copy if newp is next chunk after oldp. */
        /* (This can only happen when new chunk is sbrk'ed.) */

        if ( (newp = mem2chunk(newmem)) == next_chunk(oldp))
        {
            newsize += chunksize(newp);
            newp = oldp;
            goto split;
        }

        /* Otherwise copy, free, and exit */
        MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
        fREe(oldmem);
        return newmem;
    }


 split:  /* split off extra room in old or expanded chunk */

    if (newsize - nb >= MINSIZE) /* split off remainder */
    {
        remainder = chunk_at_offset(newp, nb);
        remainder_size = newsize - nb;
        set_head_size(newp, nb);
        set_head(remainder, remainder_size | PREV_INUSE);
        set_inuse_bit_at_offset(remainder, remainder_size);
        fREe(chunk2mem(remainder)); /* let free() deal with it */
    }
    else
    {
        set_head_size(newp, newsize);
        set_inuse_bit_at_offset(newp, newsize);
    }

    check_inuse_chunk(newp);
    return chunk2mem(newp);
}



/*

  memalign algorithm:

    memalign requests more than enough space from malloc, finds a spot
    within that chunk that meets the alignment request, and then
    possibly frees the leading and trailing space.

    The alignment argument must be a power of two. This property is not
    checked by memalign, so misuse may result in random runtime errors.

    8-byte alignment is guaranteed by normal malloc calls, so don't
    bother calling memalign with an argument of 8 or less.

    Overreliance on memalign is a sure way to fragment space.

*/
/********************************************************************************
* 函数: void* mEMALIGn(size_t alignment, size_t bytes)
* 描述: 指定对齐大小分配内存
* 输入: alignment: 内存对齐字节数，必须为2的指数幂，小于8时请使用malloc函数
       bytes: 需要分配的字节数
* 输出: none
* 返回: 指向申请到内存的首地址
* 作者:
* 版本: V1.0
**********************************************************************************/
void* mEMALIGn(size_t alignment, size_t bytes)
{
    size_t    nb;      /* padded  request size */
    char*     m;                /* memory returned by malloc call */
    mchunkptr p;                /* corresponding chunk */
    char*     brk;              /* alignment point within p */
    mchunkptr newp;             /* chunk to return */
    size_t  newsize;   /* its size */
    size_t  leadsize;  /* leading space befor alignment point */
    mchunkptr remainder;        /* spare room at end to split off */
    long      remainder_size;   /* its size */

    if ((long)bytes < 0) return 0;

    /* If need less alignment than we give anyway, just relay to malloc */

    if (alignment <= MALLOC_ALIGNMENT) return mALLOc(bytes);

    /* Otherwise, ensure that it is at least a minimum chunk size */

    if (alignment <  MINSIZE) alignment = MINSIZE;

    /* Call malloc with worst case padding to hit alignment. */

    nb = request2size(bytes);
    m  = (char*)(mALLOc(nb + alignment + MINSIZE));

    if (m == 0) return 0; /* propagate failure */

    p = mem2chunk(m);

    if ((((unsigned long)(m)) % alignment) == 0) /* aligned */
    {
#if HAVE_MMAP
        if(chunk_is_mmapped(p))
            return chunk2mem(p); /* nothing more to do */
#endif
    }
    else /* misaligned */
    {
        /*
          Find an aligned spot inside chunk.
          Since we need to give back leading space in a chunk of at
          least MINSIZE, if the first calculation places us at
          a spot with less than MINSIZE leader, we can move to the
          next aligned spot -- we've allocated enough total room so that
          this is always possible.
        */

        brk = (char*)mem2chunk(((unsigned long)(m + alignment - 1)) & -((signed) alignment));
        if ((long)(brk - (char*)(p)) < MINSIZE) brk = brk + alignment;

        newp = (mchunkptr)brk;
        leadsize = brk - (char*)(p);
        newsize = chunksize(p) - leadsize;

#if HAVE_MMAP
        if(chunk_is_mmapped(p))
        {
            newp->prev_size = p->prev_size + leadsize;
            set_head(newp, newsize|IS_MMAPPED);
            return chunk2mem(newp);
        }
#endif

        /* give back leader, use the rest */

        set_head(newp, newsize | PREV_INUSE);
        set_inuse_bit_at_offset(newp, newsize);
        set_head_size(p, leadsize);
        fREe(chunk2mem(p));
        p = newp;

        assert (newsize >= nb && (((unsigned long)(chunk2mem(p))) % alignment) == 0);
    }

    /* Also give back spare room at the end */

    remainder_size = chunksize(p) - nb;

    if (remainder_size >= (long)MINSIZE)
    {
        remainder = chunk_at_offset(p, nb);
        set_head(remainder, remainder_size | PREV_INUSE);
        set_head_size(p, nb);
        fREe(chunk2mem(remainder));
    }

    check_inuse_chunk(p);
    return chunk2mem(p);

}


/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/
/********************************************************************************
* 函数: void* vALLOc(size_t bytes)
* 描述: 使用页的大小作为对齐长度分配内存
* 输入: bytes: 需要分配的内存字节数
* 输出: none
* 返回: 指向申请到内存的首地址
* 作者:
* 版本: V1.0
**********************************************************************************/
void* vALLOc(size_t bytes)
{
    return mEMALIGn (malloc_getpagesize, bytes);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/
/********************************************************************************
* 函数: void* pvALLOc(size_t bytes)
* 描述: 按页对齐分配整数页倍数的字节数
* 输入: bytes: 需要分配的字节数
* 输出: none
* 返回: 指向申请到内存的首地址
* 作者:
* 版本: V1.0
**********************************************************************************/
void* pvALLOc(size_t bytes)
{
  size_t pagesize = malloc_getpagesize;
  return mEMALIGn (pagesize, (bytes + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/
/********************************************************************************
* 函数: void* cALLOc(size_t n, size_t elem_size)
* 描述: 分配n个固定大小的内存
* 输入: n: 需要分配的个数
       size: 每个内存的大小
* 输出: none
* 返回: 指向申请到内存的首地址
* 作者:
* 版本: V1.0
**********************************************************************************/
void* cALLOc(size_t n, size_t elem_size)
{
    mchunkptr p;
    size_t csz;

    size_t sz = n * elem_size;


    /* check if expand_top called, in which case don't need to clear */
#if MORECORE_CLEARS
    mchunkptr oldtop = top;
    size_t oldtopsize = chunksize(top);
#endif
    char* mem = mALLOc (sz);

    if ((long)n < 0) return 0;

    if (mem == 0)
        return 0;
    else
    {
        p = mem2chunk(mem);

        /* Two optional cases in which clearing not necessary */


#if HAVE_MMAP
        if (chunk_is_mmapped(p)) return mem;
#endif

        csz = chunksize(p);

#if MORECORE_CLEARS
        if (p == oldtop && csz > oldtopsize)
        {
            /* clear only the bytes from non-freshly-sbrked memory */
            csz = oldtopsize;
        }
#endif

        MALLOC_ZERO(mem, csz - SIZE_SZ);
        return mem;
    }
}




/*

    Malloc_trim gives memory back to the system (via negative
    arguments to sbrk) if there is unused memory at the `high' end of
    the malloc pool. You can call this after freeing large blocks of
    memory to potentially reduce the system-level memory requirements
    of a program. However, it cannot guarantee to reduce memory. Under
    some allocation patterns, some large free blocks of memory will be
    locked between two used chunks, so they cannot be given back to
    the system.

    The `pad' argument to malloc_trim represents the amount of free
    trailing space to leave untrimmed. If this argument is zero,
    only the minimum amount of memory to maintain internal data
    structures will be left (one page or less). Non-zero arguments
    can be supplied to maintain enough trailing space to service
    future expected allocations without having to re-obtain memory
    from the system.

    Malloc_trim returns 1 if it actually released any memory, else 0.

*/
/********************************************************************************
* 函数: int malloc_trim(size_t pad)
* 描述: 将多余的内存空间释放回系统
* 输入: pad: 默认top保留的不释放的空间
* 输出: none
* 返回: 0: 释放失败
       1: 释放成功
* 作者:
* 版本: V1.0
**********************************************************************************/
int malloc_trim(size_t pad)
{
    long  top_size;        /* Amount of top-most memory */
    long  extra;           /* Amount to release */
    char* current_brk;     /* address returned by pre-check sbrk call */
    char* new_brk;         /* address returned by negative sbrk call */

    unsigned long pagesz = malloc_getpagesize;

    top_size = chunksize(top);
    extra = ((top_size - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

    if (extra < (long)pagesz)  /* Not enough memory to release */
        return 0;

    else
    {
        /* Test to make sure no one else called sbrk */
        current_brk = (char*)(MORECORE (0));
        if (current_brk != (char*)(top) + top_size)
            return 0;     /* Apparently we don't own memory; must fail */

        else
        {
            new_brk = (char*)(MORECORE (-extra));

            if (new_brk == (char*)(MORECORE_FAILURE)) /* sbrk failed? */
            {
                /* Try to figure out what we have */
                current_brk = (char*)(MORECORE (0));
                top_size = current_brk - (char*)top;
                if (top_size >= (long)MINSIZE) /* if not, we are very very dead! */
                {
                    sbrked_mem = current_brk - sbrk_base;
                    set_head(top, top_size | PREV_INUSE);
                }
                check_chunk(top);
                return 0;
            }

            else
            {
                /* Success. Adjust top accordingly. */
                set_head(top, (top_size - extra) | PREV_INUSE);
                sbrked_mem -= extra;
                check_chunk(top);
                return 1;
            }
        }
    }
}


/*
  malloc_usable_size:

    This routine tells you how many bytes you can actually use in an
    allocated chunk, which may be more than you requested (although
    often not). You can use this many bytes without worrying about
    overwriting other allocated objects. Not a particularly great
    programming practice, but still sometimes useful.

*/
/********************************************************************************
* 函数: size_t malloc_usable_size(void* mem)
* 描述: 内存可使用的空间大小，可能比指定分配的空间大
* 输入: mem: 内存的首地址
* 输出: none
* 返回: 内存可用空间大小
* 作者:
* 版本: V1.0
**********************************************************************************/
size_t malloc_usable_size(void* mem)
{
    mchunkptr p;
    if (mem == 0)
        return 0;
    else
    {
        p = mem2chunk(mem);
        if(!chunk_is_mmapped(p))
        {
            if (!inuse(p)) return 0;
            check_inuse_chunk(p);
            return chunksize(p) - SIZE_SZ;
        }
        return chunksize(p) - 2*SIZE_SZ;
    }
}


/* Utility to update current_mallinfo for malloc_stats and mallinfo() */
/********************************************************************************
* 函数: static void malloc_update_mallinfo(void)
* 描述: 更新malloc内存信息
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
static void malloc_update_mallinfo(void)
{
    int i;
    mbinptr b;
    mchunkptr p;
#ifndef NDEBUG
    mchunkptr q;
#endif

    size_t avail = chunksize(top);
    int   navail = ((long)(avail) >= (long)MINSIZE)? 1 : 0;

    for (i = 1; i < NAV; ++i)
    {
        b = bin_at(i);
        for (p = last(b); p != b; p = p->bk)
        {
#ifndef NDEBUG
            check_free_chunk(p);
            for (q = next_chunk(p);
                 q < top && inuse(q) && (long)(chunksize(q)) >= (long)MINSIZE;
                 q = next_chunk(q))
                check_inuse_chunk(q);
#endif
            avail += chunksize(p);
            navail++;
        }
    }

    current_mallinfo.ordblks = navail;
    current_mallinfo.uordblks = sbrked_mem - avail;
    current_mallinfo.fordblks = avail;
    //current_mallinfo.hblks = n_mmaps;
    current_mallinfo.hblkhd = mmapped_mem;
    current_mallinfo.keepcost = chunksize(top);

}


/*

  malloc_stats:

    Prints on the amount of space obtain from the system (both
    via sbrk and mmap), the maximum amount (which may be more than
    current if malloc_trim and/or munmap got called), the maximum
    number of simultaneous mmap regions used, and the current number
    of bytes allocated via malloc (or realloc, etc) but not yet
    freed. (Note that this is the number of bytes allocated, not the
    number requested. It will be larger than the number requested
    because of alignment and bookkeeping overhead.)

*/
/********************************************************************************
* 函数: void malloc_stats(void)
* 描述: 打印输出malloc的信息
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: V1.0
**********************************************************************************/
void malloc_stats(void)
{
    malloc_update_mallinfo();
    printf("max system bytes = %10u\n", (unsigned int)(max_total_mem));
    printf("system bytes     = %10u\n", (unsigned int)(sbrked_mem + mmapped_mem));
    printf("in use bytes     = %10u\n", (unsigned int)(current_mallinfo.uordblks + mmapped_mem));
#if HAVE_MMAP
    printf("max mmap regions = %10u\n", (unsigned int)max_n_mmaps);
#endif
}


/*
  mallinfo returns a copy of updated current mallinfo.
*/
/********************************************************************************
* 函数: struct mallinfo mALLINFo(void)
* 描述: 取得malloc的信息
* 输入: none
* 输出: none
* 返回: malloc信息结构体
* 作者:
* 版本: V1.0
**********************************************************************************/
struct mallinfo mALLINFo(void)
{
    malloc_update_mallinfo();
    return current_mallinfo;
}




/*
  mallopt:

    mallopt is the general SVID/XPG interface to tunable parameters.
    The format is to provide a (parameter-number, parameter-value) pair.
    mallopt then sets the corresponding parameter to the argument
    value if it can (i.e., so long as the value is meaningful),
    and returns 1 if successful else 0.

    See descriptions of tunable parameters above.

*/
/********************************************************************************
* 函数: int mALLOPt(int param_number, int value)
* 描述: 设置malloc的各个阀值
* 输入: param_number: M_TRIM_THRESHOLD
                     M_TOP_PAD
                     M_MMAP_THRESHOLD
                     M_MMAP_MAX
       value: 参数值
* 输出: none
* 返回: 0: 失败
       1: 成功
* 作者:
* 版本: V1.0
**********************************************************************************/
int mALLOPt(int param_number, int value)
{
    switch(param_number)
    {
    case M_TRIM_THRESHOLD:
        trim_threshold = value;
        return 1;
    case M_TOP_PAD:
        top_pad = value;
        return 1;
    case M_MMAP_THRESHOLD:
        mmap_threshold = value;
        return 1;
    case M_MMAP_MAX:
#if HAVE_MMAP
        n_mmaps_max = value;
        return 1;
#else
        if (value != 0)
            return 0;
        else
            n_mmaps_max = value;
        return 1;
#endif

    default:
        return 0;
    }
}
