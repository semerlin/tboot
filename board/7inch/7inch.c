#include "stddef.h"
#include "clock.h"
#include "log.h"
#include "arch/arch-mx28/clkctrl.h"



/********************************************************************************
* 函数: int32_t board_clk_init(void)
* 描述: 初始化电路板时钟
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
int32_t board_clk_init(void)
{
    int32_t val = 0;

    clk_register(&pll_clk[0]);
    clk_register(&ref_cpu);
    clk_register(&ref_emi);
    //clk_register(&ref_gpmi);
    //clk_register(&ref_pix);
    clk_register(&clk_p);
    clk_register(&clk_h);
    clk_register(&clk_x);
    clk_register(&clk_uart);
    clk_register(&clk_timrot);
    clk_register(&clk_gpmi);
    clk_register(&clk_emi);
    //clk_register(&clk_lcdif);

    /* 设置pll时钟 */
    val = pll_clk[0].enable(&pll_clk[0]);
    if(val)
    {
        printl(LOG_LEVEL_ERR, "[CLK:ERR] init %s clock failed, errcode = %d.\n", pll_clk[0].name, val);
        return val;
    }

    /* 设置cpu参考时钟 */
    val = ref_cpu.set_rate(&ref_cpu, 454000000);
    if(val)
    {
        printl(LOG_LEVEL_ERR, "[CLK:ERR] set %s rate failed, errcode = %d!\n", ref_cpu.name, val);
        return val;
    }
    ref_cpu.enable(&ref_cpu);

    /* 设置emi参考时钟 */
    val = ref_emi.set_rate(&ref_emi, 410000000);
    if(val)
    {
        printl(LOG_LEVEL_ERR, "[CLK:ERR] set %s rate failed, errcode = %d!\n", ref_emi.name, val);
        return val;
    }
    ref_emi.enable(&ref_emi);

    /* 设置clk_p */
    clk_p.set_parent(&clk_p, &ref_cpu);
    val = clk_p.set_rate(&clk_p, ref_cpu.get_rate(&ref_cpu));
    if(val)
    {
        printl(LOG_LEVEL_ERR, "[CLK:ERR] set %s rate failed, errcode = %d!\n", clk_p.name, val);
        return val;
    }
    clk_p.enable(&clk_p);


    /* 设置clk_h */
    val = clk_h.set_rate(&clk_h, 151000000);
    if(val)
    {
        printl(LOG_LEVEL_ERR, "[CLK:ERR] set %s rate failed, errcode = %d!\n", clk_h.name, val);
        return val;
    }

    /* 设置clk_x */
    val = clk_x.set_rate(&clk_x, 24000000);
    if(val)
    {
        printl(LOG_LEVEL_ERR, "[CLK:ERR] set %s rate failed, errcode = %d!\n", clk_x.name, val);
        return val;
    }


    /* 设置clk_uart */
    clk_uart.enable(&clk_uart);

    /* 设置clk_timrot */
    clk_timrot.enable(&clk_uart);

    /* 设置clk_gpmi */
    clk_gpmi.set_parent(&clk_gpmi, &ref_xtal);
    val = clk_gpmi.set_rate(&clk_gpmi, ref_xtal.get_rate(&ref_xtal));
    if(val)
    {
        printl(LOG_LEVEL_ERR, "[CLK:ERR] set %s rate failed, errcode = %d!\n", clk_gpmi.name, val);
        return val;
    }
    clk_gpmi.enable(&clk_gpmi);

    /* 设置clk_emi */
    clk_emi.set_parent(&clk_emi, &ref_emi);
    val = clk_emi.set_rate(&clk_emi, 205000000);
    if(val)
    {
        printl(LOG_LEVEL_ERR, "[CLK:ERR] set %s rate failed, errcode = %d!\n", clk_emi.name, val);
        return val;
    }
    clk_emi.enable(&clk_emi);

    /* 设置clk_lcdif */

    return 0;
}
