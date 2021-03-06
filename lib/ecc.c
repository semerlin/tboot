#include "ecc.h"
#include "errno.h"

/* ECC预处理表 */
static const uint8_t ecc_precalc_table[] =
{
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00
};


/********************************************************************************
* 函数: static inline int32_t countbits(__in uint32_t byte)
* 描述: 统计数据中为1的位的个数
* 输入: 需要统计的数据
* 输出: none
* 返回: 数据中为1的位的个数
* 作者:
* 版本: V1.0
**********************************************************************************/
static inline int32_t countbits(__in uint32_t byte)
{
    uint32_t res = (byte & 0x55555555) + ((byte >> 1) & 0x55555555);
    res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
    res = (res & 0x0f0f0f0f) + ((res >> 4) & 0x0f0f0f0f);
    res = (res & 0x00ff00ff) + ((res >> 8) & 0x00ff00ff);
    return (res & 0x0000ffff) + ((res >> 16) & 0x0000ffff);
}

/********************************************************************************
* 函数: int32_t calculate_ecc(__in const uint8_t *data,
                                  __out uint8_t *ecc_code)
* 描述: 计算ecc
* 输入: data: 需要计算的数据
* 输出: ecc_code: 计算出来的ecc数据
* 返回: 0: 正常
       -1: 失败
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t ecc_calculate(__in const uint8_t *data, __out uint8_t *ecc_code)
{
    int32_t idx = 0, reg1 = 0, reg2 = 0, reg3 = 0;
    int32_t tmp1 = 0, tmp2 = 0;
    int32_t i = 0;

    if(!data || !ecc_code)
        return -1;

    for(i = 0; i < 256; i++)
    {
        idx = ecc_precalc_table[*data++];
        reg1 ^= (idx & 0x3f);

        //行极性为1
        if(idx & 0x40)
        {
            reg2 ^= (uint8_t)i;
            reg3 ^= ~((uint8_t)i);
        }
    }

    tmp1  = (reg3 & 0x80) >> 0;
	tmp1 |= (reg2 & 0x80) >> 1;
	tmp1 |= (reg3 & 0x40) >> 1;
	tmp1 |= (reg2 & 0x40) >> 2;
	tmp1 |= (reg3 & 0x20) >> 2;
	tmp1 |= (reg2 & 0x20) >> 3;
	tmp1 |= (reg3 & 0x10) >> 3;
	tmp1 |= (reg2 & 0x10) >> 4;

	tmp2  = (reg3 & 0x08) << 4;
	tmp2 |= (reg2 & 0x08) << 3;
	tmp2 |= (reg3 & 0x04) << 3;
	tmp2 |= (reg2 & 0x04) << 2;
	tmp2 |= (reg3 & 0x02) << 2;
	tmp2 |= (reg2 & 0x02) << 1;
	tmp2 |= (reg3 & 0x01) << 1;
	tmp2 |= (reg2 & 0x01) << 0;

	ecc_code[0] = ~tmp1;
	ecc_code[1] = ~tmp2;
	ecc_code[2] = ((~reg1 << 2)) | 0x03;

	return 0;

}


/********************************************************************************
* 函数: int32_t ecc_correct_data(__in uint8_t *data, __in uint8_t *read_ecc,
                                __in uint8_t *calc_ecc)
* 描述: 根据ecc值校准数据
* 输入: data: 数据校准和检验的数据
       read_ecc: 读取到的ecc数据
       calc_ecc: 计算出的ecc数据
* 输出: none
* 返回: 0: 数据正确不存在错误(或出现了ecc无法检验的错误)
       1: 纠正了1个位的数据
       2: oob区出现错误
       -EBADMSG: 未知错误
* 作者:
* 版本: V1.0
**********************************************************************************/
int32_t ecc_correct_data(__in uint8_t *data, __in uint8_t *read_ecc,
                          __in uint8_t *calc_ecc)
{
    uint8_t s0, s1, s2;

    s0 = calc_ecc[0] ^ read_ecc[0];
    s1 = calc_ecc[1] ^ read_ecc[1];
    s2 = calc_ecc[2] ^ read_ecc[2];

    if((s0 | s1 | s2) == 0)
        return 0;

    if(((s0 ^ (s0 >> 1)) == 0x55) &&
       ((s1 ^ (s1 >> 1)) == 0x55) &&
       ((s2 ^ (s2 >> 1)) == 0x54))
    {
        uint32_t byteoffs, bitnum;

        byteoffs = (s1 << 0) & 0x80;
        byteoffs |= (s1 << 1) & 0x40;
        byteoffs |= (s1 << 2) & 0x20;
        byteoffs |= (s1 << 3) & 0x10;
        byteoffs |= (s0 >> 4) & 0x08;
        byteoffs |= (s0 >> 3) & 0x04;
        byteoffs |= (s0 >> 2) & 0x02;
        byteoffs |= (s0 >> 1) & 0x01;

        bitnum = (s2 >> 5) & 0x04;
        bitnum |= (s2 >> 4) & 0x02;
        bitnum |= (s2 >> 3) & 0x01;

        data[byteoffs] ^= (1 << bitnum);

        return 1;
    }

    if(countbits(s0 | ((uint32_t)s1 << 8) | ((uint32_t)s2 << 16)) == 1)
        return 2;

    return -EBADMSG;
}


