#include "types.h"
#include "serial_def.h"
#include "string.h"
#include "arch/arch-mx28/mx28_regs.h"
#include "arch/arch-mx28/regs_uart.h"
#include "config.h"



/********************************************************************************
* 函数: static void serial_setbrg(void)
* 描述: 设置串口寄存器
* 输入: none
* 输出: none
* 返回: none
* 作者:
* 版本: v1.0
**********************************************************************************/
static void serial_setbrg(void)
{
    uint32_t cr;
    uint32_t quot;

    /* Disable everything */
    cr = REG_RD(REGS_UARTDBG_BASE, HW_UARTDBGCR);
    REG_WR(REGS_UARTDBG_BASE, HW_UARTDBGCR, 0);

    /* Calculate and set baudrate */
    quot = (CONFIG_UARTDBG_CLK * 4)	/ CONFIG_BAUDRATE;
    REG_WR(REGS_UARTDBG_BASE, HW_UARTDBGFBRD, quot & 0x3f);
    REG_WR(REGS_UARTDBG_BASE, HW_UARTDBGIBRD, quot >> 6);

    /* Set 8n1 mode, enable FIFOs */
    REG_WR(REGS_UARTDBG_BASE, HW_UARTDBGLCR_H,
           BM_UARTDBGLCR_H_WLEN | BM_UARTDBGLCR_H_FEN);

    /* Enable Debug UART */
    REG_WR(REGS_UARTDBG_BASE, HW_UARTDBGCR, cr);
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t serial_init(void)
{
    /* Disable UART */
    REG_WR(REGS_UARTDBG_BASE, HW_UARTDBGCR, 0);

    /* Mask interrupts */
    REG_WR(REGS_UARTDBG_BASE, HW_UARTDBGIMSC, 0);

    /* Set default baudrate */
    serial_setbrg();

    /* Enable UART */
    REG_WR(REGS_UARTDBG_BASE, HW_UARTDBGCR,
           BM_UARTDBGCR_TXE | BM_UARTDBGCR_RXE | BM_UARTDBGCR_UARTEN);

    return 0;
}

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t serial_deinit(void)
{


    return 0;
}



/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static void serial_putc(const int8_t c)
{
    /* Wait for room in TX FIFO */
    while (REG_RD(REGS_UARTDBG_BASE, HW_UARTDBGFR) & BM_UARTDBGFR_TXFF)
        ;

    /* Write the data byte */
    REG_WR(REGS_UARTDBG_BASE, HW_UARTDBGDR, c);

    if (c == '\n')
        serial_putc('\r');
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static void serial_puts(const int8_t *s)
{
    while (*s)
        serial_putc(*s++);
}

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t serial_tstc(void)
{
    /* Check if RX FIFO is not empty */
    return !(REG_RD(REGS_UARTDBG_BASE, HW_UARTDBGFR) & BM_UARTDBGFR_RXFE);
}

/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
static int32_t serial_getc(void)
{
    /* Wait while TX FIFO is empty */
    while (REG_RD(REGS_UARTDBG_BASE, HW_UARTDBGFR) & BM_UARTDBGFR_RXFE);

    /* Read data byte */
    return REG_RD(REGS_UARTDBG_BASE, HW_UARTDBGDR) & 0xff;
}


/********************************************************************************
* 函数:
* 描述:
* 输入:
* 输出:
* 返回:
* 作者:
* 版本: v1.0
**********************************************************************************/
void serial_bind(struct serial_device *dev)
{
    strcpy(dev->name, "imx28 serial");
    dev->init = serial_init;
    dev->deinit = serial_deinit;
    dev->setbrg = serial_setbrg;
    dev->getc = serial_getc;
    dev->tstc = serial_tstc;
    dev->putc = serial_putc;
    dev->puts = serial_puts;
}



