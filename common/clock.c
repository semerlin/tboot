#include "clock.h"
#include "errno.h"
#include "string.h"
#include "list.h"

/********************************************************
* 静态变量
*********************************************************/
static struct clk clks;  //stdio设备的链表头



/********************************************************************************
* 函数: int32_t clk_register(__in struct clk *clk)
* 描述: 注册时钟设备
* 输入: clk: 需要注册的时钟设备的指针
* 输出: none
* 返回: 0: 成功
       -EINVAL: 参数无效
       //-ENOMEM: 内存不足
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t clk_register(__in struct clk *clk)
{
    if(!clk)
        return -EINVAL;

/*
    struct clk *_pclk = NULL;

    _pclk = dlcalloc(1, sizeof(struct clk));

    if(!_pclk)
        return -ENOMEM;

    memcpy(_pclk, clk, sizeof(struct clk));
*/
    list_add_tail(&(clk->list), &(clks.list));

    return 0;
}



/********************************************************************************
* 函数: void clk_unregister(__in const int8_t *name)
* 描述: 卸载指定名字的clk设备
* 输入: name: 卸载的clk设备的名称
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
void clk_unregister(__in const int8_t *name)
{
    struct clk *pclk = NULL;
    //struct list_head *pos = NULL;

    pclk = clk_get_by_name(name);
    if(!pclk)
        return ;

    list_del(&(pclk->list));
    //dlfree(pclk);

    //return 0;
}

/********************************************************************************
* 函数: struct clk *clk_get_by_name(__in const int8_t *name)
* 描述: 通过名字获取clk设备的指针
* 输入: name: clk设备的名称
* 输出: none
* 返回: NULL: 没有找到指定名字的设备
       成功: clk设备的指针
* 作者:
* 版本: v1.0
**********************************************************************************/
struct clk *clk_get_by_name(__in const int8_t *name)
{
    struct list_head *pos = NULL;
    struct clk *pclk = NULL;

    list_for_each(pos, &(clks.list))
    {
        pclk = list_entry(pos, struct clk, list);
        if(0 == strcmp(pclk->name, name))
            return pclk;
    }

    return NULL;
}


#ifdef CONFIG_POWER_CONTROL
/********************************************************************************
* 函数: int32_t clk_dynamic(int32_t mode);
* 描述: 动态调整时钟频率
* 输入: mode: 需要调整到的模式
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t clk_dynamic(int32_t mode)
{
    return 0;
}
#endif


/********************************************************************************
* 函数: int32_t clk_init(void)
* 描述: 初始化始终设备
* 输入: none
* 输出: none
* 返回: 0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
/* 外部实现函数 */
extern int32_t board_clk_init(void);

int32_t clk_init(void)
{
    INIT_LIST_HEAD(&(clks.list));

    return board_clk_init();
}


