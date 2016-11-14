#ifndef _BEDTERMINAL_H_
  #define _BEDTERMINAL_H_


/*
* 打印输出级别
*/
#define CONFIG_LOG_LEVEL      2

#define CONFIG_LITTLE_ENDIAN   1


/*
* Serial Driver
*/
#define CONFIG_UARTDBG_CLK		      24000000
#define CONFIG_BAUDRATE			      115200		/* Default baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	  {9600, 19200, 38400, 57600, 115200}

/**/
#define CONFIG_MTD_PARTITIONS         1

#define CONFIG_SYS_HZ		          1000

#define CONFIG_NR_DRAM_BANKS          1


#define CONFIG_SYS_PROMPT			  "=> "
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE			  256
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE			  (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)


#define NAND_MAX_CHIPS		8
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_MAX_NAND_DEVICE	  1

#endif

