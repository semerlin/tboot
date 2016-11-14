#include "config.h"
#include "string.h"
#include "malloc.h"

#if defined(CONFIG_CONSOLE_MUX)

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
void iomux_printdevs(__in const int32_t console)
{
    PSTDIO_DEV pDev;

    for(int i = 0; i < cd_count[console]; i++)
    {
        pdev = console_devices[console][i];
        printf("%s ", pDev->name);
    }
    printf("\n");
}



/********************************************************************************
* 函数: int32_t iomux_doenv(const int32_t console, const int8_t *arg)
* 描述: 通过名字注册console设备, arg可以包含多个名字,使用‘,’隔开
* 输入: console: 设备类型
       arg: 环境变量值
* 输出: none
* 返回: 1: 失败
       0: 成功
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t iomux_doenv(__in const int32_t console, const int8_t *arg)
{
    int8_t *console_args, *temp, **start;
    int32_t cnt = 0, io_flags, idx;
    PSTDIO_DEV *cons_set, pDev, repeat;

    console_args = strdup(arg);
    if(!console_args)
        return 1;

    //取得','分隔的参数个数
    temp = console_args;
    for(;;)
    {
        temp = strchr(temp, ',');
        if(temp)
        {
            cnt++;
            continue;
        }

        cnt++;
        break;
    }

    start = (int8_t **)malloc(cnt * sizeof(int8_t *));
    if(!start)
    {
        free(console_args);
        return 1;
    }

    cnt = 0;

    //分隔出具体参数
    start[0] = console_args;
    for(;;)
    {
        temp = strchr(start[cnt++], ',');
        if(temp == NULL)
            break;

        *temp = '\0';

        start[cnt] = temp + 1;
    }

    cons_set = (PSTDIO_DEV *)calloc(cnt, sizeof(PSTDIO_DEV));

    if(!cons_set)
    {
        free(start);
        free(console_args);
        return 1;
    }

    //取得设备类别
    switch(console)
    {
    case stdin:
        io_flags = DEV_FLAGS_INPUT;
        break;
    case stdout:
    case stderr;
        io_flags = DEV_FLAGS_OUTPUT;
        break;
    default:
        free(start);
        free(console_args);
        free(cons_set);
        return 1;
    }

    //重新绑定设备
    for(int32_t i = 0; i < cnt; i++)
    {
        pDev = search_device(console, start[i]);
        if(pDev == NULL)
            continue;

        //防止重复
        repeat = 0;
        for(int32_t j = 0; j < idx; j++)
        {
            if(pDev == cons_set[j])
            {
                repeat ++;
                break;
            }
        }

        if(repeat)
            continue;


        //绑定concole设备，这可能打乱之前的concole设备设置
        if(console_assign(console, start[j]) < 0)
            continue;

#ifdef CONFIG_SERIAL_MULTI
        /* 多串口绑定 */
        if(serial_assign(start[j]) < 0)
            continue;
#endif

        cons_set[idx++] = pdev;
    }

    free(console_args);
    free(start);

    if(idx == 0) /* 绑定失败 */
    {
        free(cons_set);
        return 1;
    }
    else
    {
        console_devices[console] = realloc(console_devices[console], idx * sizeof(PSTDIO_DEV));
        if(console_devices[console] == NULL)
        {
            free(cons_set);
            return 1;
        }

        memcpy(console_devices[console], cons_set, idx * sizeof(PSTDIO_DEV);

        cd_count[console] = idx;
    }

    free(cons_set);

    return 0;
}




#endif
