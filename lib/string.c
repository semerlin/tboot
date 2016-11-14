#include "stddef.h"
#include "string.h"
#include "malloc.h"

/********************************************************************************
* 函数: size_t strspn(__in const int8_t *s, __in const int8_t *accept)
* 描述: 返回字符串s中第一个不在指定字符串accept中出现的字符下标
* 输入: s: 需要检验的字符串
		accept: 检验字符串
* 输出: none
* 返回: 成功: 第一个不匹配的字符的位置
       失败: 超出s或者accept长度的下一位置
* 作者:
* 版本: V1.0
**********************************************************************************/
size_t strspn(__in const int8_t *s, __in const int8_t *accept)
{
	size_t count = 0;

	for(; *s != '\0'; s++)
	{
		for(; *accept != '\0'; accept++)
		{
			if(*s == *accept)
				break;
		}

		if(*accept == '\0')
			return count;

		count ++;
	}

	return count;
}



/********************************************************************************
* 函数: int8_t * strpbrk(__in const int8_t *cs, __in const int8_t *ct)
* 描述: 依次检验字符串cs中的字符，当被检验字符在字符串ct中也包含时，则停止检验，
       并返回该字符位置，空字符NULL不包括在内
* 输入: cs: 检验的字符
       ct: 检验的字符串
* 输出: none
* 返回: NULL: 没有匹配的字符
       成功: 返回匹配的位置
* 作者:
* 版本: V1.0
**********************************************************************************/
int8_t *strpbrk(__in const int8_t *cs, __in const int8_t *ct)
{
	for(; *cs != '\0'; cs++)
	{
		for(; *ct != '\0'; ct++)
		{
			if(*cs == *ct)
				return (int8_t *)cs;
		}
	}

	return NULL;
}



/********************************************************************************
* 函数: int8_t *strtok(__in int8_t *s, __in const int8_t *delim)
* 描述: 分解字符串为一组字符串。s为要分解的字符串，delim为分隔符字符串。首次调用时，
       s指向要分解的字符串，之后再次调用要把s设成NULL。
* 输入: s: 需要分隔的字符串
       delim: 分隔符
* 输出: none
* 返回: NULL: 分隔结束
		成功: 指向每个分隔后的字符
* 作者:
* 版本: V1.0
**********************************************************************************/
static int8_t *___strtok = NULL;  /* 用作记录每次分隔的位置 */
int8_t *strtok(__in int8_t *s, __in const int8_t *delim)
{
	int8_t *sbegin, *send;
	sbegin = s ? s : ___strtok;
	if(!sbegin)
        return NULL;

 	sbegin += strspn(sbegin, delim);
    if(*sbegin == '\0')   /* 没有出现分隔符 */
	{
		___strtok = NULL;
		return NULL;
	}

	send = strpbrk(sbegin, delim);
	if(!send)
	{
		___strtok = NULL;
		return NULL;
	}
	else
	{
		*send++ = '\0';
		___strtok = send;
	}

	return sbegin;
}


/********************************************************************************
* 函数: int8_t *strsep(__in int8_t **s, __in const int8_t *delim)
* 描述: 分解字符串为一组字符串。从s指向的位置起向后扫描，遇到delim指向的字符串
       中的字符后，将此字符替换为\0，返回s指向的地址。它适用于分割“关键字”在
       两个字符串之间只“严格出现一次”的情况。
* 输入: s: 需要分隔的字符串
		delim: 分隔符号
* 输出: none
* 返回: 成功: 分隔出来的字符串首地址
       失败: NULL
* 作者:
* 版本: V1.0
**********************************************************************************/
int8_t *strsep(__in int8_t **s, __in const int8_t *delim)
{
	int8_t *sbegin = *s, *send;
	if(!sbegin)
		return NULL;

	send = strpbrk(sbegin, delim);
	if(send)
		*send++ = '\0';

	*s = send;

	return sbegin;
}



/********************************************************************************
* 函数: int8_t *strcpy(__out int8_t *dest, __in const int8_t *src)
* 描述: 把从src地址开始且含有NULL结束符的字符串复制到以dest开始的地址空间
* 输入: src: 源地址
* 输出: dest: 目的地地址
* 返回: 指向dest的指针
* 作者:
* 版本: V1.0
**********************************************************************************/
int8_t *strcpy(__out int8_t *dest, __in const int8_t *src)
{
	int8_t *r = dest;

	while((*dest++ = *src++) != '\0');

	return r;
}

/********************************************************************************
* 函数: int8_t *strncpy(__out int8_t *dest, __in const int8_t *src, __in size_t num)
* 描述: 把src所指字符串的前n个字节复制到dest所指的数组中，并返回指向dest的指针
* 输入: src: 源地址
		num: 需要复制的数量
* 输出: dest: 目的地地址
* 返回: 指向dest的指针
* 作者:
* 版本:
**********************************************************************************/
int8_t *strncpy(__out int8_t *dest, __in const int8_t *src, __in size_t num)
{
	int8_t *r = dest;

	while(num-- && ((*dest++ = *src++) != '\0'));

	return r;
}

/********************************************************************************
* 函数: int8_t *strcat(__in int8_t *dest, __in const int8_t *src)
* 描述: 把src所指字符串添加到dest结尾处(覆盖dest结尾处的'\0')并添加'\0'
* 输入: src: 源地址
* 输出: dest: 目的地地址
* 返回: 指向dest的指针
* 作者: hy
* 版本: V1.0
**********************************************************************************/
int8_t *strcat(__in int8_t *dest, __in const int8_t *src)
{
	int8_t *r = dest;
	while(*dest)
		dest++;

	while((*dest++ = *src++) != '\0');

	return r;

}

/********************************************************************************
* 函数: int8_t *strncat(__in int8_t *dest, __in const int8_t *src, size_t num)
* 描述: 把src所指字符串中的前num个添加到dest结尾处(覆盖dest结尾处的'\0')并添加'\0'
* 输入: src: 源地址
		num: 复制的数量
* 输出: dest: 目的地地址
* 返回: 指向dest的指针
* 作者: hy
* 版本: v1.0
**********************************************************************************/
int8_t *strncat(__in int8_t *dest, __in const int8_t *src, size_t num)
{
	int8_t *r = dest;
	while(*dest)
		dest++;

	while((*dest++ = *src++) != '\0')
	{
        if(--num == 0)
        {
            *dest = '\0';
            break;
        }
	}

	return r;
}


/********************************************************************************
* 函数: int32_t strcmp(__in const int8_t *cs, __in const int8_t *ct)
* 描述: 比较两个字符串大小
* 输入: cs: 字符串1
       ct: 字符串2
* 输出: none
* 返回: 0: cs == ct
       >0: cs > ct
       <0: cs < ct
* 作者: hy
* 版本: v1.0
**********************************************************************************/
int32_t strcmp(__in const int8_t *cs, __in const int8_t *ct)
{
    register int8_t _res;

    while(1)
    {
        if(((_res = *cs - *ct++) !=0) || !*cs++)
            break;
    }

    return _res;
}


/********************************************************************************
* 函数: int8_t *strchr(__in const int8_t *s, __in int8_t val)
* 描述: 返回src中首次出现val的位置的指针，返回的地址是被查找字符串指针开始的第一个与Val
       相同字符的指针，如果src中不存在val则返回NULL。
* 输入: src: 被查找的字符串
		val: 需要查找的值
* 输出: none
* 返回: 失败: NULL
		成功: 指向查找到的位置
* 作者: hy
* 版本: V1.0
**********************************************************************************/
int8_t *strchr(__in const int8_t *src, __in int8_t val)
{
	while(*src != val)
	{
		if(*src == '\0')
			return NULL;

        src++;
	}

	return (int8_t *)src;
}

/********************************************************************************
* 函数: int8_t *strrchr(__in const int8_t *src, __in int8_t val)
* 描述: 返回src中首次出现c的位置的指针，返回的地址是被查找字符串指针开始的最后一个与Val
       相同字符的指针，如果src中不存在val则返回NULL。
* 输入: src: 被查找的字符串
		val: 需要查找的值
* 输出: none
* 返回: 失败: NULL
		成功: 指向查找到的位置
* 作者: hy
* 版本: V1.0
**********************************************************************************/
int8_t *strrchr(__in const int8_t *src, __in int8_t val)
{
	const int8_t *psrc = src;
	while(*psrc++ != '\0');

	for(; psrc != src; psrc--)
	{
		if(*psrc == val)
			return (int8_t *)psrc;
	}

	return NULL;
}

/********************************************************************************
* 函数: size_t strlen(__in const int8_t *src)
* 描述: 计算src的长度，不包括'\0'
* 输入: src: 源字符串
* 输出: none
* 返回: 字符串长度
* 作者: hy
* 版本: V1.0
**********************************************************************************/
size_t strlen(__in const int8_t *src)
{
	size_t len = 0;

	while(*src++ != '\0')
		len++;

	return len;
}

/********************************************************************************
* 函数: size_t strnlen(__in const int8_t *src, __in size_t maxlen)
* 描述: 计算字符串src的长度，不包括结束符NULL，该长度最大为maxlen。
* 输入: src: 源字符串
		maxlen: 计数的最大长度
* 输出: none
* 返回: 字符串的长度
* 作者:
* 版本: v1.0
**********************************************************************************/
size_t strnlen(__in const int8_t *src, __in size_t maxlen)
{
	size_t len = 0;
	while(maxlen-- && (*src++ != '\0'))
		len++;

	return len;
}

/********************************************************************************
* 函数: int8_t *strdup(__in const int8_t *src)
* 描述: 将串拷贝到新建的位置处
* 输入: src: 需要拷贝的字符串
* 输出: none
* 返回: 失败: NULL
		成功: 指向为复制字符串分配的空间
* 作者: hy
* 版本: v1.0
**********************************************************************************/
int8_t *strdup(__in const int8_t *src)
{
	int8_t *new;
	if(!src || ((new = dlmalloc(strlen(src)+1)) == NULL))
	{
		return NULL;
	}

	strcpy(new, src);

	return new;
}



/********************************************************************************
* 函数: void *memset(__in void *src, __in int32_t ch, __in size_t size)
* 描述: 将src中前size字节用ch替换并返回s
* 输入: src: 需要替换的字符串
		ch: 替换的字符
		size: 替换块的大小
* 输出: none
* 返回: 指向src的指针
* 作者: hy
* 版本: v1.0
**********************************************************************************/
void *memset(__in void *src, __in int32_t ch, __in size_t size)
{
    int8_t *psrc = (int8_t *)src;
	if(psrc)
	{
		while(size--)
			*psrc++ = ch;
	}

	return src;
}

/********************************************************************************
* 函数: void *memcpy(__in void *dest, __in const void *src, __in size_t num)
* 描述: 从源src所指的内存地址的起始位置开始拷贝num个字节到目标dest所指的内存地址的
       起始位置中
* 输入: src: 源地址
		num: 拷贝的字节数
* 输出: dest: 目的地址
* 返回: 指向dest的指针
* 作者: hy
* 版本: v1.0
**********************************************************************************/
void *memcpy(__out void *dest, __in const void *src, __in size_t num)
{
	int8_t *pdest = dest;
	const int8_t *psrc = (int8_t *)src;

	while(num--)
		*pdest++ = *psrc++;

	return dest;
}

/********************************************************************************
* 函数: void *memmove(__in void *dest, __in const void *src, __in size_t count)
* 描述: 用于从src拷贝count个字符到dest
* 输入: src: 源地址
		count: 拷贝的数量
* 输出: dest: 目的地地址
* 返回: 指向dest的指针
* 作者: hy
* 版本: v1.0
**********************************************************************************/
void *memmove(__in void *dest, __in const void *src, __in size_t count)
{
	int8_t *pdest, *psrc;
	if(dest <= src)
	{
		pdest = (int8_t *)dest;
		psrc = (int8_t *)src;
		while(count--)
			*pdest++ = *psrc++;
	}
	else
	{
		pdest = (int8_t *)dest + count;
		psrc = (int8_t *)src + count;
		while(count--)
			*pdest-- = *psrc--;
	}

	return dest;
}

/********************************************************************************
* 函数: void *memscan(__in void *src, __in int8_t ch, size_t size)
* 描述: 在内存的size大小的区域中找ch字符
* 输入: src: 源地址
		ch: 需要寻找的字符
		size: 寻找的区域大小
* 输出: none
* 返回: 成功: 指向找到字符的位置
		失败: 指向寻找区域的下一位置
* 作者: hy
* 版本: v1.0
**********************************************************************************/
void *memscan(__in void *src, __in int8_t ch, size_t size)
{
	int8_t *psrc = (int8_t *)src;

	while(size--)
	{
		if(*psrc == ch)
			break;

		psrc++;
	}

	return (void *)psrc;
}

/********************************************************************************
* 函数: int memcmp(__in const void *buf1, __in const void *buf2, __in size_t count)
* 描述: 比较内存区域buf1和buf2的前count个字节
* 输入: buf1: 比较字符串
		buf2: 比较字符串
		count: 比较的个数
* 输出: none
* 返回: 当buf1<buf2时, 返回值<0
		当buf1=buf2时, 返回值=0
		当buf1>buf2时, 返回值>0
* 作者: hy
* 版本: v1.0
**********************************************************************************/
int32_t memcmp(__in const void *buf1, __in const void *buf2, __in size_t count)
{
	const uint8_t *pbuf1 = (uint8_t *)buf1, *pbuf2 = (uint8_t *)buf2;
	int32_t rval = 0;

	while(count--)
	{
		rval = *pbuf1 - *pbuf2;
		if(rval != 0)
			break;
	}

	return rval;
}

/********************************************************************************
* 函数: void *memchr(__in const void *src, __in int8_t ch, __in size_t size)
* 描述: 在内存的size大小的区域中找ch字符
* 输入: src: 源地址
		ch: 需要寻找的字符
		size: 寻找的区域大小
* 输出: none
* 返回: 成功: 指向找到字符的位置
		失败: NULL
* 作者: hy
* 版本: v1.0
**********************************************************************************/
void *memchr(__in const void *src, __in int8_t ch, __in size_t size)
{
	const int8_t *psrc = (int8_t *)src;

	while(size--)
	{
		if(*psrc == ch)
			return (int8_t *)psrc;

		psrc++;
	}

	return NULL;
}


/********************************************************************************
* 函数: int8_t *bcopy(__in const int8_t *src, __in int8_t *dest, __in int32_t count)
* 描述: 把内存src中的前count字节拷贝到dest区域
* 输入: src: 源地址
		count: 拷贝的字节数
* 输出: dest: 目的地址
* 返回: 指向dest的指针
* 作者: hy
* 版本: v1.0
**********************************************************************************/
int8_t *bcopy(__in const int8_t *src, __in int8_t *dest, __in int32_t count)
{
	int8_t *pdest = dest;

	while(count--)
		*pdest++ = *src++;

	return dest;
}


/********************************************************************************
* 函数: int8_t *strstr(__in const int8_t *str, __in const int8_t *search)
* 描述: 搜索search字符串在str字符串中的第一次出现。找到所搜索的字符串，则该函数返回
       第一次匹配的字符串的地址；如果未找到所搜索的字符串，则返回NULL。
* 输入: str: 搜索的字符串
		search: 需要搜素的字符串
* 输出: none
* 返回: 成功: search是str的子串，则先确定search在str的第一次出现的位置，
            并返回此search在str首位置的地址
		失败: NULL
* 作者:
* 版本: v1.0
**********************************************************************************/
int8_t *strstr(__in const int8_t *str, __in const int8_t *search)
{
	int32_t str_len, search_len;
	search_len = strlen(search);

	if(!search_len)
		return (char *)str;

	str_len = strlen(str);

	while(str_len >= search_len)
	{
		str_len--;
		if(!memcmp(str, search, search_len))
			return (char *)str;

		str++;
	}

	return NULL;

}





