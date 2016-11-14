#ifndef _CLKCTRL_H_
  #define _CLKCTRL_H_

#include "clock.h"


/* 参考时钟 */
extern struct clk ref_xtal; /* 24MHz */
extern struct clk pll_clk[]; /* 480MHz, 480MHz, 50MHz */
extern struct clk ref_cpu;
extern struct clk ref_emi;
extern struct clk ref_io1;
extern struct clk ref_io0;
extern struct clk ref_gpmi;
extern struct clk ref_hsadc;
extern struct clk ref_pix;


/* 运行时钟 */
extern struct clk clk_p;
extern struct clk clk_h;
extern struct clk clk_x;
extern struct clk clk_uart;
extern struct clk clk_pwm;
extern struct clk clk_timrot;
extern struct clk clk_ssp[];
extern struct clk clk_gpmi;
extern struct clk clk_spdif;
extern struct clk clk_emi;
extern struct clk clk_saif[];
extern struct clk clk_lcdif;
extern struct clk clk_etm;
extern struct clk clk_hsadc;
extern struct clk clk_flexcan[];

#endif

