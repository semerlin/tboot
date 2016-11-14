#ifndef _ERRNO_H_
  #define _ERRNO_H_

#include "types.h"
#include "compiler.h"

#define	EPERM			1	/* 操作不允许 */
#define	ENOENT			2	/* 不存在此文件或文件夹 */
#define	ESRCH			3	/* No such process */
#define	EINTR			4	/* Interrupted system call */
#define	EIO				5	/* I/O错误 */
#define	ENXIO			6	/* No such device or address */
#define	EBADF			7	/* Bad file number */
#define	ECHILD			8	/* No child processes */
#define	EAGAIN			9	/* 再试一次 */
#define	ENOMEM			10	/* 内存不足 */
#define	EACCES			11	/* Permission denied */
#define	EFAULT			12	/* Bad address */
#define	EBUSY			13	/* 设备或资源被占用 */
#define	EEXIST			14	/* 文件存在 */
#define	EXDEV			15	/* Cross-device link */
#define	ENODEV			16	/* 无此设备 */
#define	ENOTDIR			17	/* 不是文件夹 */
#define	EISDIR			18	/* 是文件夹 */
#define	EINVAL			19	/* 无效参数 */
#define	ENFILE			20	/* File table overflow */
#define	ENOTTY			21	/* Not a typewriter */
#define	EFBIG			22	/* 文件太大 */
#define	ENOSPC			23	/* 设备无剩余空间 */
#define	ESPIPE			24	/* Illegal seek */
#define	EROFS			25	/* 只读文件系统 */
#define	ENAMETOOLONG	26	/* File name too long */
#define	ENOSYS			27	/* Function not implemented */
#define	ENOTEMPTY		28	/* 文件夹不为空 */
#define	ETIME			29	/* 超时 */
#define	EBADMSG			30	/* Not a data message */
#define	EILSEQ			31	/* Illegal byte sequence */
#define	EOPNOTSUPP		32	/* Operation not supported on transport endpoint */
#define	ECONNRESET		33	/* Connection reset by peer */
#define	ETIMEDOUT		34	/* Connection timed out */
#define	EUCLEAN			35	/* Structure needs cleaning */


/* 返回指针情况此下错误代码识别和处理 */
#define MAX_ERRNO	4095
#define IS_ERR_VALUE(x) unlikely((x) >= (size_t)-MAX_ERRNO)

static inline void *ERR_PTR(ssize_t error)
{
	return (void *)error;
}

static inline ssize_t PTR_ERR(const void *ptr)
{
	return (ssize_t)ptr;
}

static inline ssize_t IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((size_t)ptr);
}

#endif

