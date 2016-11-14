#include <stdarg.h>
#include "stddef.h"
#include "ctype.h"
#include "string.h"

//16进制ASC表
const char hex_asc[] = "0123456789abcdef";
#define hex_asc_lo(x)  hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)  hex_asc[((x) & 0xf0) >> 4]


#define ZEROPAD    0x01   //输出前补0填充满宽度
#define SIGN       0x02   //有符号数，默认为无符号数
#define PLUS       0x04   //在数字前面显示+-号表示正负
#define SPACE      0x08   //输出为正时用空格，为负时用负号
#define LEFT       0x10   //左对齐, 宽度不够时右边为空, 默认右对齐
#define SMALL      0x20   //字母为小写(必须为 32 == 0x20)
#define SPECIAL    0x40   //特殊字符, 即在数据为8,16进制时前面加上0,0x或0X


#define is_digit(c) (((c) >= '0') && ((c) <= '9'))

/********************************************************************************
* 函数: static inline char *pack_hex_byte(__out char *buf, __in unsigned char byte)
* 描述: 把一个字节的数据转换为16进制的字符串
* 输入: byte: 需要转换的字节
* 输出: buf: 转换完成的16进制字节
* 返回: 指向转换完成的下一字节位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static inline char *pack_hex_byte(__out char *buf, __in unsigned char byte)
{
	*buf++ = hex_asc_hi(byte);
	*buf++ = hex_asc_lo(byte);

	return buf;
}

/********************************************************************************
* 函数: unsigned long simple_strtoul(__in const char *cp, __in char **endp,
                                    __in unsigned int base)
* 描述: 把一个字符串转换为一个无符号长整数
* 输入: cp: 指向字符串的开始
		base: 要用到的基数(进制数),base为0表示通过cp来自动判断基数,函数可自动识别的
              基数：'0x'表示16进制,'0'表示8进制，其它都认为10进制。函数可转换成数字的
              有效字符为: [0-f]
              举例：cp='0x12str',base=0, 则返回 unsigned long为18, *endp="str"。
* 输出: endp: 指向分析字符串的末尾的位置
* 返回: 转换后的数值
* 作者:
* 版本: v1.0
**********************************************************************************/
unsigned long simple_strtoul(__in const char *cp, __out char **endp,
                             __in unsigned int base)
{
	unsigned long rval = 0, value;

	if(*cp == '0')
	{
		cp++;
		if(((*cp == 'x') || (*cp == 'X')) && (isxdigit(cp[1])))
		{
			cp++;
			base = 16;
		}

		if(!base)
		{
			base = 8;
		}
	}

	if(!base)
	{
		base = 10;
	}

	while(isxdigit(*cp) && ((value = (isdigit(*cp) ? *cp - '0' : (islower(*cp) ? toupper(*cp) : *cp) - 'A' + 10)) < base))
	{
		rval = rval * base + value;
		cp++;
	}

	if(endp)
		*endp = (char *)cp;


	return rval;
}


/********************************************************************************
* 函数: long simple_strtol(__in const char * cp, __out char ** endp,
                           __in unsigned int base)
* 描述: 把一个字符串转换为一个有符号长整数
* 输入: cp: 指向字符串的开始
		base: 要用到的基数(进制数),base为0表示通过cp来自动判断基数,函数可自动识别的
              基数：'0x'表示16进制,'0'表示8进制，其它都认为10进制。函数可转换成数字的
              有效字符为: [0-f]
              举例：cp='0x12str',base=0, 则返回 unsigned long为18, *endp="str"。
* 输出: endp: 指向分析字符串的末尾的位置
* 返回: 转换后的数值
* 作者:
* 版本: v1.0
**********************************************************************************/
long simple_strtol(__in const char * cp, __out char ** endp,
                   __in unsigned int base)
{
	if(*cp == '-')
		return -simple_strtoul(cp+1, endp, base);

	return simple_strtoul(cp, endp, base);
}



/********************************************************************************
* 函数: unsigned long long simple_strtoull(__in const char *cp, __out char **endp,
                                           __in unsigned int base)
* 描述: 把一个字符串转换为一个无符号长长整数
* 输入: cp: 指向字符串的开始
		base: 要用到的基数(进制数),base为0表示通过cp来自动判断基数,函数可自动识别的
              基数：'0x'表示16进制,'0'表示8进制，其它都认为10进制。函数可转换成数字的
              有效字符为: [0-f]
              举例：cp='0x12str',base=0, 则返回 unsigned long为18, *endp="str"。
* 输出: endp: 指向分析字符串的末尾的位置
* 返回: 转换后的数值
* 作者:
* 版本: v1.0
**********************************************************************************/
unsigned long long simple_strtoull(__in const char *cp, __out char **endp,
                                   __in unsigned int base)
{
	unsigned long long rval = 0, value;

	if(*cp == '0')
	{
		cp++;
		if(((*cp == 'x') || (*cp == 'X')) && (isxdigit(cp[1])))
		{
			cp++;
			base = 16;
		}

		if(!base)
		{
			base = 8;
		}
	}

	if(!base)
	{
		base = 10;
	}

	while(isxdigit(*cp) && ((value = (isdigit(*cp) ? *cp - '0' : (islower(*cp) ? toupper(*cp) : *cp) - 'A' + 10)) < base))
	{
		rval = rval * base + value;
		cp++;
	}

	if(endp)
		*endp = (char *)cp;


	return rval;
}


/********************************************************************************
* 函数: long long simple_strtoll(__in const char *cp, __in char **endp,
                                 __in unsigned int base)
* 描述: 把一个字符串转换为一个有符号长长整数
* 输入: cp: 指向字符串的开始
		base: 要用到的基数(进制数),base为0表示通过cp来自动判断基数,函数可自动识别的
              基数：'0x'表示16进制,'0'表示8进制，其它都认为10进制。函数可转换成数字的
              有效字符为: [0-f]
              举例：cp='0x12str',base=0, 则返回 unsigned long为18, *endp="str"。
* 输出: endp: 指向分析字符串的末尾的位置
* 返回: 转换后的数值
* 作者:
* 版本: v1.0
**********************************************************************************/
long long simple_strtoll(__in const char *cp, __in char **endp,
                         __in unsigned int base)
{
	if(*cp == '-')
		return -simple_strtoull(cp+1, endp, base);

	return simple_strtoull(cp, endp, base);
}


/********************************************************************************
* 函数: int atoi(__in const char *buf)
* 描述: 把字符串转换为整数，只支持10进制字符串
* 输入: buf: 需要转换的字符串
* 输出: none
* 返回: 转换好的字符串
* 作者:
* 版本: v1.0
**********************************************************************************/
int atoi(__in const char *buf)
{
	int rval = 0;
	char sign = 0;  //0为正， 1为负

	if(*buf == '-')
	{
 		sign = 1;
		buf++;
	}

	if(*buf == '+')
		buf++;


	while((*buf >= '0') && (*buf <= '9'))
	{
		rval = rval * 10 + *buf - '0';
		buf++;
	}

	if(sign)
		return -rval;

	return rval;
}


/********************************************************************************
* 函数: static char *put_dec_full(__out char *buf, __in unsigned int q)
* 描述: 把[0,99999]之间的10进制数转换为字符串, 以大端模式存储
* 输入: q: 需要转换的数字，范围是[0,99999]
* 输出: buf: 转换完成的字符串, 大端模式
* 返回: 指向转换完成的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *put_dec_full(__out char *buf, __in unsigned int q)
{
    unsigned int d3, d2, d1, d0;

    d1 = (q>>4) & 0xf;
    d2 = (q>>8) & 0xf;
    d3 = (q>>12);


    d0 = 6*(d3 + d2 + d1) + (q & 0xf);
    q = (d0 * 0xcd) >> 11;
    d0 = d0 - 10*q;

    d1 = q + 9*d3 + 5*d2 + d1;
    q = (d1 * 0xcd) >> 11;
    d1 = d1 - 10*q;


    d2 = q + 2*d2;
    q = (d2 * 0xd) >> 7;
    d2 = d2 - 10*q;


    d3 = q + 4*d3;
    q = (d3 * 0xcd) >> 11;
    d3 = d3 - 10*q;

    if(q != 0)
        *buf++ = q + '0';
    if(d3 != 0)
        *buf++ = d3 + '0';
    if(d2 != 0)
        *buf++ = d2 + '0';
    if(d1 != 0)
        *buf++ = d1 + '0';

    *buf++ = d0 + '0';



	return buf;
}

/********************************************************************************
* 函数: static char *put_dec_full_sign(__out char *buf, __in int q)
* 描述: 把[-99999,99999]之间的10进制数转换为字符串, 以大端模式存储
* 输入: q: 需要转换的数字，范围是[-99999,99999]
* 输出: buf: 转换完成的字符串, 大端模式
* 返回: 指向转换完成的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *put_dec_full_sign(__out char *buf, __in int q)
{
    unsigned int d3, d2, d1, d0;

    if(q < 0)
    {
        *buf++ = '-';
        q = -q;
    }

    d1 = (q>>4) & 0xf;
    d2 = (q>>8) & 0xf;
    d3 = (q>>12);

    d0 = 6*(d3 + d2 + d1) + (q & 0xf);
    q = (d0 * 0xcd) >> 11;
    d0 = d0 - 10*q;

    d1 = q + 9*d3 + 5*d2 + d1;
    q = (d1 * 0xcd) >> 11;
    d1 = d1 - 10*q;


    d2 = q + 2*d2;
    q = (d2 * 0xd) >> 7;
    d2 = d2 - 10*q;


    d3 = q + 4*d3;
    q = (d3 * 0xcd) >> 11; /* - shorter code */
    /* q = (d3 * 0x67) >> 10; - would also work */
    d3 = d3 - 10*q;

    if(q != 0)
        *buf++ = q + '0';
    if(d3 != 0)
        *buf++ = d3 + '0';
    if(d2 != 0)
        *buf++ = d2 + '0';
    if(d1 != 0)
        *buf++ = d1 + '0';

    *buf++ = d0 + '0';



	return buf;
}



/********************************************************************************
* 函数: static char *put_dec_sign(__out char *buf, __in int q)
* 描述: 把[-999999999, 999999999]之间的10进制数转换为字符串, 以大端模式存储
* 输入: q: 需要转换的数字，范围是[-999999999, 999999999]
* 输出: buf: 转换完成的字符串, 大端模式
* 返回: 指向转换完成的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *put_dec_sign(__out char *buf, __in int q)
{
	unsigned int rem = 0;
    if(q < 0)
    {
        *buf ++ = '-';
        q = -q;
    }

    while(1)
    {
        if(q < 100000)
            return put_dec_full(buf, q);

        rem = q / 100000;
        q %= 100000;
        buf = put_dec_full(buf, rem);
    }
}


/********************************************************************************
* 函数: static char *put_dec(__out char *buf, __in int q)
* 描述: 把[0, 999999999]之间的10进制数转换为字符串, 以大端模式存储
* 输入: q: 需要转换的数字，范围是[0, 999999999]
* 输出: buf: 转换完成的字符串, 大端模式
* 返回: 指向转换完成的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *put_dec(__out char *buf, __in int q)
{
	unsigned int rem = 0;

    while(1)
    {
        if(q < 100000)
            return put_dec_full(buf, q);

        rem = q / 100000;
        q %= 100000;
        buf = put_dec_full(buf, rem);
    }
}

/********************************************************************************
* 函数: static char *number(__ouy char *buf, __in unsigned long num, __in int base,
                            __in int size, __in int precision, __in int type)
* 描述: 将指定数字转换成不同格式的数字字符串
* 输入: num: 需要转换的数字
		base: 进制数, 8,10,16进制. 8进制前缀0, 16进制前缀0x或0X
		size: 输出字符串的宽度
		precision: 转换进度(保留多少位有效数字)
		type: 转换的附加格式
* 输出: buf: 转换后的字符串存储位置
* 返回: 指向转换后字符串的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *number(__out char *buf, __in unsigned long num, __in int base,
                    __in int size, __in int precision, __in int type)
{
	//16进制表
	static const char digitsMap[] = "0123456789ABCDEF";

	char temp[66];  //数据转换缓冲区
	char locase = (type & SMALL);  //大小写标志位, 0为大写, 非0为小写
	char sign = 0;   //符号标志位, 0为不包含符号位, ' ', '+', '-'为符号位
    //数字前缀, 0为不需要前缀, 8进制前缀为0, 16进制前缀为0x或0X
	int need_pfx = ((type & SPECIAL) && (base != 10));
	int pos = 0;
	int mask = base - 1;
	int shift = 3;   //默认8进制

	if(type & LEFT)
	{
		type &= ~ZEROPAD;
	}

	//数字正负转换
	if(type & SIGN)
	{
		if((signed long)num < 0)
		{
			sign = '-';
			num = -(signed long)num;
			size--;
		}
		else if(type & PLUS)
		{
			sign = '+';
			size--;
		}
		else if(type & SPACE)
		{
			sign = ' ';
			size--;
		}
	}

	if(need_pfx)
	{
		size--;
		if(base == 16)
			size--;
	}

	//转换数字存入缓冲区
	if(num == 0)
		temp[pos++] = '0';
	else if(base != 10) /* 8或16进制 */
	{
		if(base == 16)
			shift = 4;

		do
		{
			temp[pos++] = digitsMap[((unsigned char)num) & mask] | locase;
			num >>= shift;
		}while(num);
	}
	else /* 10进制 */
	{
		pos = put_dec(temp, num) - temp;
	}

	if(pos > precision)
		precision = pos;


	size -= precision;
	if(!(type & (ZEROPAD | LEFT)))
	{
		//缓冲区大于精度时，同时不是左对齐和用0填充，就使用' '填充
		while(--size >= 0)
			*buf++ = ' ';
	}

	/* 符号位 */
	if(sign)
		*buf++ = sign;

	/* 进制前缀 */
	if(need_pfx)
	{
		*buf++ = '0';
		if(base == 16)
			*buf++ = ('X' | locase);
	}

	/* 0或空格填充 */
	if(!(type & LEFT))
	{
		char c = (type & ZEROPAD) ? '0' : ' ';
		while(--size >= 0)
			*buf++ = c;
	}

	/* 还有更多的填充区域 */
	while(pos <= --precision)
		*buf++ = '0';

	/* 填充数字 */
	while(--pos >= 0)
		*buf++ = temp[pos];

	/* 多余宽度填充空格 */
	while(--size >= 0)
		*buf++ = ' ';

	return buf;
}

/********************************************************************************
* 函数: static char *string(__out char *buf, __in char *s, __in int field_width,
					__in int precision, __in int flags)
* 描述: 把字符串s按照flags指定的格式转换到buf缓冲区中
* 输入: s: 需要格式化的字符串
		filed_width: 缓冲区的长度
		precision: 转换精度, s的有效长度
		flags: 转换的格式
* 输出: buf: 字符串转换输出缓冲区
* 返回: 指向转换好的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *string(__out char *buf, __in char *s, __in int field_width,
					__in int precision, __in int flags)
{
	int len = 0, i = 0;
	if(s == 0)
		s = "<NULL>";

	len = strnlen(s, precision);

	if(!(flags & LEFT))
		while(len < field_width--)
			*buf++ = ' ';

	for(i = 0; i < len; i++)
		*buf++ = *s++;

	while(len < field_width--)
		*buf++ = ' ';

	return buf;
}


/********************************************************************************
* 函数: static char *mac_address_string(__out char *buf, __in unsigned char *addr,
								        __in int filed_width, __in int precision,
								        __in int flags)
* 描述: 把addr数组中的mac地址转换为16进制表示的字符串
* 输入: addr: 需要转换的mac地址数组
		field_width: 输出缓冲区的大小
		precision: 转换的精度, 需要转换的位数
		flags: 转换的附加格式
* 输出: buf: 转换完成的输出缓冲区
* 返回: 指向转换完成后buf的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *mac_address_string(__out char *buf, __in unsigned char *addr,
								__in int field_width, __in int precision,
								__in int flags)
{
	char mac_addr[6 * 3];
	char *p = mac_addr;
	int i = 0;

	for(i = 0; i < 6; i++)
	{
		p = pack_hex_byte(mac_addr, addr[i]);
		if((flags & SPECIAL) && (i != 5))
			*p++ = ':';
	}
	*p = '\0';

	return string(buf, mac_addr, field_width, precision, flags & ~SPECIAL);
}


/********************************************************************************
* 函数: static char *ip6_addr_string(__out char *buf, __in unsigned char *addr,
							         __in int field_width, __in int precision,
							         __in int flags)
* 描述: 把addr数组中的ipv6地址转换为16进制表示的字符串
* 输入: addr: 需要转换的ipv6地址数组
		field_width: 输出缓冲区的大小
		precision: 转换的精度, 需要转换的位数
		flags: 转换的附加格式
* 输出: buf: 转换完成的输出缓冲区
* 返回: 指向转换完成后buf的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *ip6_addr_string(__out char *buf, __in unsigned char *addr,
							 __in int field_width, __in int precision,
							 __in int flags)
{
	char ip6_addr[8 * 5];
	char *p = ip6_addr;
	int i = 0;

	for(i = 0; i < 8; i++)
	{
		p = pack_hex_byte(p, addr[2 * i]);
		p = pack_hex_byte(p, addr[2 * i + 1]);
		if((flags & SPECIAL) && (i != 7))
			*p++ = ':';
	}
	*p = '\0';

	return string(buf, ip6_addr, field_width, precision, flags & ~SPECIAL);
}


/********************************************************************************
* 函数: static char *ip4_addr_string(__out char *buf, __in unsigned char *addr,
							         __in int field_width, __in int precision,
							         __in int flags)
* 描述: 把addr数组中的ipv4地址转换为16进制表示的字符串
* 输入: addr: 需要转换的ipv4地址数组
		field_width: 输出缓冲区的大小
		precision: 转换的精度, 需要转换的位数
		flags: 转换的附加格式
* 输出: buf: 转换完成的输出缓冲区
* 返回: 指向转换完成后buf的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *ip4_addr_string(__out char *buf, __in unsigned char *addr,
							 __in int field_width, __in int precision,
							 __in int flags)
{
	char ip4_addr[4 * 4];
	char *p = ip4_addr;
	int i = 0;

	for(i = 0; i < 4; i++)
	{
		p = put_dec_full(p, addr[i]);
		if((flags & SPECIAL) && (i != 3))
			*p++ = '.';
	}
	*p = '\0';

	return string(buf, ip4_addr, field_width, precision, flags & ~SPECIAL);
}

/********************************************************************************
* 函数: static char *pointer(__in const char *fmt, __out char *buf,
							 __in void *ptr, __in int field_width,
                             __in int precision, __in int flags)
* 描述: 转换mac地址或ip地址到指定的缓冲区中，不特别指定转换为16进制格式
* 输入: fmt: m: 忽略:分隔
			 M: 使用:分隔
			 i6: IPv6忽略:分隔
			 i4: IPv4保持不变
			 I6: IPv6使用:分隔
			 I4: IPv4保持不变
		ptr: mac或者ip数据存放数组
		field_width: 输出缓冲区长度
		precision: 输入缓冲区长度
		flags: 转换的附加格式
* 输出: buf: 转换完成的数据存放缓冲区
* 返回: 指向转换完成后buf的下一位置
* 作者:
* 版本: v1.0
**********************************************************************************/
static char *pointer(__in const char *fmt, __out char *buf, __in void *ptr,
					 __in int field_width, __in int precision, __in int flags)
{
	if(!ptr)
		return string(buf, "(null)", field_width, precision, flags);

	switch(*fmt)
	{
	case 'M':
		flags |= SPECIAL;
 	case 'm':
		return mac_address_string(buf, ptr, field_width, precision, flags);
		break;
	case 'I':
		flags |= SPECIAL;
	case 'i':
		if(fmt[1] == '6')
			return ip6_addr_string(buf, ptr, field_width, precision, flags);
		if(fmt[1] == '4')
			return ip4_addr_string(buf, ptr, field_width, precision, flags);
		break;
	default:
		break;
	}

	flags |= SMALL;
	if(field_width == -1)
	{
		field_width = 2 * sizeof(void *);
		flags |= ZEROPAD;
	}
	return number(buf, (unsigned long)ptr, 16, field_width, precision, flags);
}


/********************************************************************************
* 函数: int vsprintf(__out char *buf, __in const char *fmt, __in va_list args)
* 描述: 把args中的参数按照fmt中指定的格式转换到buf缓冲区中
* 输入: fmt: 转换的格式
		args: 需要转换的参数
* 输出: buf: 准换完成后的输出缓冲区
* 返回: 字符串的有效长度
* 作者:
* 版本: v1.0
**********************************************************************************/
int vsprintf(__out char *buf, __in const char *fmt, __in va_list args)
{
	char *str = buf;
	int flags = 0;
	int field_width = -1;
	int precision = -1;
	int base = 10;
	int qualifier = -1;
	unsigned long num = 0;

	for(; *fmt != '\0'; fmt++)
	{
		if(*fmt != '%')   //等待%格式标记
		{
			*str++ = *fmt;
			continue;
		}

		/* 处理标志位 */
	repeat:
		fmt++;
		switch(*fmt)
		{
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		default:
			break;
		}

		/* 取得宽度 */
		if(is_digit(*fmt))
			field_width = atoi(fmt);
		else if(*fmt == '*')
		{
			fmt++;
			field_width = va_arg(args, int);
			if(field_width < 0)
			{
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* 取得精度 */
		if(*fmt == '.')
		{
			fmt++;
			if(is_digit(*fmt))
				precision = atoi(fmt);
			else if(*fmt == '*')
			{
				fmt++;
				precision = va_arg(args, int);
			}
			if(precision < 0)
				precision = 0;
		}

		/* 取得转换修饰符 */
		if((*fmt == 'h') || (*fmt == 'l') || (*fmt == 'L') ||
		   (*fmt == 'Z') || (*fmt == 'z') || (*fmt == 't'))
		{
			qualifier = *fmt;
			++fmt;
			if((qualifier == 'l') && (*fmt == 'l'))
			{
				qualifier = 'L';
				++fmt;
			}
		}

		/* 默认基数 */
		base = 10;

		switch(*fmt)
		{
		case 'c':
			if(!(flags & LEFT))
				while(--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char)va_arg(args, int);
			while(--field_width > 0)
				*str++ = ' ';
			continue;

		case 's':
			str = string(str, va_arg(args, char*), field_width, precision, flags);
			continue;

		case 'p':
			str = pointer(fmt+1, str, va_arg(args, void*), field_width, precision, flags);
			while(isalnum(fmt[1]))
				fmt++;
			continue;

		case 'n':
			if (qualifier == 'l') {
				long * ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			*str++ = '%';
			continue;

		case 'o':
			base = 8;
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}

		if (qualifier == 'l') {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long) num;
		} else if (qualifier == 'Z' || qualifier == 'z') {
			num = va_arg(args, size_t);
		} else if (qualifier == 't') {
			num = va_arg(args, ptrdiff_t);
		} else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (signed short) num;
		} else {
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int) num;
		}
		str = number(str, num, base, field_width, precision, flags);
	}

	*str = '\0';

	return str-buf;
}



/********************************************************************************
* 函数: int sprintf(__out char * buf, __in const char *fmt, ...)
* 描述: 把...中的参数按照fmt中指定的格式转换到buf缓冲区中
* 输入: fmt: 转换的格式
		...: 需要转换的参数
* 输出: buf: 准换完成后的输出缓冲区
* 返回: 字符串的有效长度
* 作者:
* 版本: v1.0
**********************************************************************************/
int sprintf(__out char * buf, __in const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);
	return i;
}





