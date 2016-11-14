#ifndef _CONVERT_H_
  #define _CONVERT_H_


#include "stddef.h"


#ifdef __cplusplus
extern "C" {
#endif


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
unsigned long simple_strtoul(__in const char *cp, __out char **endp, __in unsigned int base);

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
long simple_strtol(__in const char * cp, __out char ** endp, __in unsigned int base);


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
unsigned long long simple_strtoull(__in const char *cp, __out char **endp, __in unsigned int base);


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
long long simple_strtoll(__in const char *cp, __in char **endp, __in unsigned int base);


/********************************************************************************
* 函数: int atoi(__in const char *buf)
* 描述: 把字符串转换为整数，只支持10进制字符串
* 输入: buf: 需要转换的字符串
* 输出: none
* 返回: 转换好的字符串
* 作者:
* 版本: v1.0
**********************************************************************************/
int atoi(__in const char *buf);

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
int vsprintf(__out char *buf, __in const char *fmt, __in va_list args);


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
int sprintf(__out char * buf, __in const char *fmt, ...);







#ifdef __cplusplus
}
#endif



#endif  /* _CONVERT_H_ */


