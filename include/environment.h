#ifndef _ENVIRONMENT_H_
  #define _ENVIRONMENT_H_



#if defined(CONFIG_ENV_IS_IN_NAND)
# ifndef CONFIG_ENV_OFFSET
#  error "Need to define CONFIG_ENV_OFFSET when using CONFIG_ENV_IS_IN_NAND"
# endif
# ifndef CONFIG_ENV_SIZE
#  error "Need to define CONFIG_ENV_SIZE when using CONFIG_ENV_IS_IN_NAND"
# endif
# ifdef CONFIG_ENV_OFFSET_REDUND
#  define CONFIG_SYS_REDUNDAND_ENVIRONMENT
# endif
# ifdef CONFIG_ENV_IS_EMBEDDED
#  define ENV_IS_EMBEDDED	1
# endif
#endif /* CONFIG_ENV_IS_IN_NAND */




#if defined(CONFIG_ENV_IS_IN_MMC)
# ifndef CONFIG_ENV_OFFSET
#  error "Need to define CONFIG_ENV_OFFSET when using CONFIG_ENV_IS_IN_MMC"
# endif
# ifndef CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR	(CONFIG_ENV_OFFSET)
# endif
# ifndef CONFIG_ENV_OFFSET
#  define CONFIG_ENV_OFFSET (CONFIG_ENV_ADDR)
# endif
# ifdef CONFIG_ENV_OFFSET_REDUND
#  define CONFIG_SYS_REDUNDAND_ENVIRONMENT
# endif
# ifdef CONFIG_ENV_IS_EMBEDDED
#  define ENV_IS_EMBEDDED	1
# endif
#endif /* CONFIG_ENV_IS_IN_MMC */


#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
# define ENV_HEADER_SIZE	(sizeof(uint32_t) + 1)
#else
# define ENV_HEADER_SIZE	(sizeof(uint32_t))
#endif


#define ENV_SIZE (CONFIG_ENV_SIZE - ENV_HEADER_SIZE)

#if 0
typedef struct environment_s
{
	uint32_t crc;		/* 环境变量CRC校验结果	*/
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
	uint8_t flags;		/* 有效/无效标志位	*/
#endif
	uint8_t data[ENV_SIZE]; /* 环境变量数据 */
}env_t;
#endif








#endif

