#ifndef _REGS_DMA_APBH_H_
#define _REGS_DMA_APBH_H_


#define HW_APBH_CTRL0	(0x00000000)
#define HW_APBH_CTRL0_SET	(0x00000004)
#define HW_APBH_CTRL0_CLR	(0x00000008)
#define HW_APBH_CTRL0_TOG	(0x0000000c)

#define BM_APBH_CTRL0_SFTRST 0x80000000
#define BM_APBH_CTRL0_CLKGATE 0x40000000
#define BM_APBH_CTRL0_AHB_BURST8_EN 0x20000000
#define BM_APBH_CTRL0_APB_BURST_EN 0x10000000
#define BP_APBH_CTRL0_RSVD0      16
#define BM_APBH_CTRL0_RSVD0 0x0FFF0000
#define BF_APBH_CTRL0_RSVD0(v)  \
	(((v) << 16) & BM_APBH_CTRL0_RSVD0)
#define BP_APBH_CTRL0_CLKGATE_CHANNEL      0
#define BM_APBH_CTRL0_CLKGATE_CHANNEL 0x0000FFFF
#define BF_APBH_CTRL0_CLKGATE_CHANNEL(v)  \
	(((v) << 0) & BM_APBH_CTRL0_CLKGATE_CHANNEL)
#if defined(CONFIG_APBH_DMA_V1)
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__SSP0  0x0001
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__SSP1  0x0002
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__SSP2  0x0004
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__SSP3  0x0008
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND0 0x0010
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND1 0x0020
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND2 0x0040
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND3 0x0080
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND4 0x0100
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND5 0x0200
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND6 0x0400
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND7 0x0800
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__HSADC 0x1000
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__LCDIF 0x2000
#elif defined(CONFIG_APBH_DMA_V2)
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND0 0x0001
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND1 0x0002
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND2 0x0004
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND3 0x0008
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND4 0x0010
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND5 0x0020
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND6 0x0040
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__NAND7 0x0080
#define BV_APBH_CTRL0_CLKGATE_CHANNEL__SSP   0x0100
#endif

#define HW_APBH_CTRL1	(0x00000010)
#define HW_APBH_CTRL1_SET	(0x00000014)
#define HW_APBH_CTRL1_CLR	(0x00000018)
#define HW_APBH_CTRL1_TOG	(0x0000001c)

#define BM_APBH_CTRL1_CH15_CMDCMPLT_IRQ_EN 0x80000000
#define BM_APBH_CTRL1_CH14_CMDCMPLT_IRQ_EN 0x40000000
#define BM_APBH_CTRL1_CH13_CMDCMPLT_IRQ_EN 0x20000000
#define BM_APBH_CTRL1_CH12_CMDCMPLT_IRQ_EN 0x10000000
#define BM_APBH_CTRL1_CH11_CMDCMPLT_IRQ_EN 0x08000000
#define BM_APBH_CTRL1_CH10_CMDCMPLT_IRQ_EN 0x04000000
#define BM_APBH_CTRL1_CH9_CMDCMPLT_IRQ_EN 0x02000000
#define BM_APBH_CTRL1_CH8_CMDCMPLT_IRQ_EN 0x01000000
#define BM_APBH_CTRL1_CH7_CMDCMPLT_IRQ_EN 0x00800000
#define BM_APBH_CTRL1_CH6_CMDCMPLT_IRQ_EN 0x00400000
#define BM_APBH_CTRL1_CH5_CMDCMPLT_IRQ_EN 0x00200000
#define BM_APBH_CTRL1_CH4_CMDCMPLT_IRQ_EN 0x00100000
#define BM_APBH_CTRL1_CH3_CMDCMPLT_IRQ_EN 0x00080000
#define BM_APBH_CTRL1_CH2_CMDCMPLT_IRQ_EN 0x00040000
#define BM_APBH_CTRL1_CH1_CMDCMPLT_IRQ_EN 0x00020000
#define BM_APBH_CTRL1_CH0_CMDCMPLT_IRQ_EN 0x00010000
#define BM_APBH_CTRL1_CH15_CMDCMPLT_IRQ 0x00008000
#define BM_APBH_CTRL1_CH14_CMDCMPLT_IRQ 0x00004000
#define BM_APBH_CTRL1_CH13_CMDCMPLT_IRQ 0x00002000
#define BM_APBH_CTRL1_CH12_CMDCMPLT_IRQ 0x00001000
#define BM_APBH_CTRL1_CH11_CMDCMPLT_IRQ 0x00000800
#define BM_APBH_CTRL1_CH10_CMDCMPLT_IRQ 0x00000400
#define BM_APBH_CTRL1_CH9_CMDCMPLT_IRQ 0x00000200
#define BM_APBH_CTRL1_CH8_CMDCMPLT_IRQ 0x00000100
#define BM_APBH_CTRL1_CH7_CMDCMPLT_IRQ 0x00000080
#define BM_APBH_CTRL1_CH6_CMDCMPLT_IRQ 0x00000040
#define BM_APBH_CTRL1_CH5_CMDCMPLT_IRQ 0x00000020
#define BM_APBH_CTRL1_CH4_CMDCMPLT_IRQ 0x00000010
#define BM_APBH_CTRL1_CH3_CMDCMPLT_IRQ 0x00000008
#define BM_APBH_CTRL1_CH2_CMDCMPLT_IRQ 0x00000004
#define BM_APBH_CTRL1_CH1_CMDCMPLT_IRQ 0x00000002
#define BM_APBH_CTRL1_CH0_CMDCMPLT_IRQ 0x00000001

#define HW_APBH_CTRL2	(0x00000020)
#define HW_APBH_CTRL2_SET	(0x00000024)
#define HW_APBH_CTRL2_CLR	(0x00000028)
#define HW_APBH_CTRL2_TOG	(0x0000002c)

#define BM_APBH_CTRL2_CH15_ERROR_STATUS 0x80000000
#define BV_APBH_CTRL2_CH15_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH15_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH14_ERROR_STATUS 0x40000000
#define BV_APBH_CTRL2_CH14_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH14_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH13_ERROR_STATUS 0x20000000
#define BV_APBH_CTRL2_CH13_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH13_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH12_ERROR_STATUS 0x10000000
#define BV_APBH_CTRL2_CH12_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH12_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH11_ERROR_STATUS 0x08000000
#define BV_APBH_CTRL2_CH11_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH11_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH10_ERROR_STATUS 0x04000000
#define BV_APBH_CTRL2_CH10_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH10_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH9_ERROR_STATUS 0x02000000
#define BV_APBH_CTRL2_CH9_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH9_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH8_ERROR_STATUS 0x01000000
#define BV_APBH_CTRL2_CH8_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH8_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH7_ERROR_STATUS 0x00800000
#define BV_APBH_CTRL2_CH7_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH7_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH6_ERROR_STATUS 0x00400000
#define BV_APBH_CTRL2_CH6_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH6_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH5_ERROR_STATUS 0x00200000
#define BV_APBH_CTRL2_CH5_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH5_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH4_ERROR_STATUS 0x00100000
#define BV_APBH_CTRL2_CH4_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH4_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH3_ERROR_STATUS 0x00080000
#define BV_APBH_CTRL2_CH3_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH3_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH2_ERROR_STATUS 0x00040000
#define BV_APBH_CTRL2_CH2_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH2_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH1_ERROR_STATUS 0x00020000
#define BV_APBH_CTRL2_CH1_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH1_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH0_ERROR_STATUS 0x00010000
#define BV_APBH_CTRL2_CH0_ERROR_STATUS__TERMINATION 0x0
#define BV_APBH_CTRL2_CH0_ERROR_STATUS__BUS_ERROR   0x1
#define BM_APBH_CTRL2_CH15_ERROR_IRQ 0x00008000
#define BM_APBH_CTRL2_CH14_ERROR_IRQ 0x00004000
#define BM_APBH_CTRL2_CH13_ERROR_IRQ 0x00002000
#define BM_APBH_CTRL2_CH12_ERROR_IRQ 0x00001000
#define BM_APBH_CTRL2_CH11_ERROR_IRQ 0x00000800
#define BM_APBH_CTRL2_CH10_ERROR_IRQ 0x00000400
#define BM_APBH_CTRL2_CH9_ERROR_IRQ 0x00000200
#define BM_APBH_CTRL2_CH8_ERROR_IRQ 0x00000100
#define BM_APBH_CTRL2_CH7_ERROR_IRQ 0x00000080
#define BM_APBH_CTRL2_CH6_ERROR_IRQ 0x00000040
#define BM_APBH_CTRL2_CH5_ERROR_IRQ 0x00000020
#define BM_APBH_CTRL2_CH4_ERROR_IRQ 0x00000010
#define BM_APBH_CTRL2_CH3_ERROR_IRQ 0x00000008
#define BM_APBH_CTRL2_CH2_ERROR_IRQ 0x00000004
#define BM_APBH_CTRL2_CH1_ERROR_IRQ 0x00000002
#define BM_APBH_CTRL2_CH0_ERROR_IRQ 0x00000001

#define HW_APBH_CHANNEL_CTRL	(0x00000030)
#define HW_APBH_CHANNEL_CTRL_SET	(0x00000034)
#define HW_APBH_CHANNEL_CTRL_CLR	(0x00000038)
#define HW_APBH_CHANNEL_CTRL_TOG	(0x0000003c)

#define BP_APBH_CHANNEL_CTRL_RESET_CHANNEL      16
#define BM_APBH_CHANNEL_CTRL_RESET_CHANNEL 0xFFFF0000
#define BF_APBH_CHANNEL_CTRL_RESET_CHANNEL(v) \
	(((v) << 16) & BM_APBH_CHANNEL_CTRL_RESET_CHANNEL)

#if defined(CONFIG_APBH_DMA_V1)
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__SSP0  0x0001
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__SSP1  0x0002
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__SSP2  0x0004
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__SSP3  0x0008
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND0 0x0010
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND1 0x0020
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND2 0x0040
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND3 0x0080
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND4 0x0100
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND5 0x0200
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND6 0x0400
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND7 0x0800
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__HSADC 0x1000
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__LCDIF 0x2000

#define BP_APBH_CHANNEL_CTRL_FREEZE_CHANNEL      0
#define BM_APBH_CHANNEL_CTRL_FREEZE_CHANNEL 0x0000FFFF
#define BF_APBH_CHANNEL_CTRL_FREEZE_CHANNEL(v)  \
	(((v) << 0) & BM_APBH_CHANNEL_CTRL_FREEZE_CHANNEL)

#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__SSP0  0x0001
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__SSP1  0x0002
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__SSP2  0x0004
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__SSP3  0x0008
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND0 0x0010
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND1 0x0020
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND2 0x0040
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND3 0x0080
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND4 0x0100
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND5 0x0200
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND6 0x0400
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND7 0x0800
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__HSADC 0x1000
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__LCDIF 0x2000
#elif defined(CONFIG_APBH_DMA_V2)
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND0 0x0001
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND1 0x0002
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND2 0x0004
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND3 0x0008
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND4 0x0010
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND5 0x0020
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND6 0x0040
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__NAND7 0x0080
#define BV_APBH_CHANNEL_CTRL_RESET_CHANNEL__SSP   0x0100
#define BP_APBH_CHANNEL_CTRL_FREEZE_CHANNEL      0
#define BM_APBH_CHANNEL_CTRL_FREEZE_CHANNEL 0x0000FFFF
#define BF_APBH_CHANNEL_CTRL_FREEZE_CHANNEL(v)  \
	(((v) << 0) & BM_APBH_CHANNEL_CTRL_FREEZE_CHANNEL)
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND0 0x0001
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND1 0x0002
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND2 0x0004
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND3 0x0008
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND4 0x0010
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND5 0x0020
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND6 0x0040
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__NAND7 0x0080
#define BV_APBH_CHANNEL_CTRL_FREEZE_CHANNEL__SSP   0x0100
#endif

#define HW_APBH_DEVSEL	(0x00000040)

#define BP_APBH_DEVSEL_CH15      30
#define BM_APBH_DEVSEL_CH15 0xC0000000
#define BF_APBH_DEVSEL_CH15(v) \
	(((v) << 30) & BM_APBH_DEVSEL_CH15)
#define BP_APBH_DEVSEL_CH14      28
#define BM_APBH_DEVSEL_CH14 0x30000000
#define BF_APBH_DEVSEL_CH14(v)  \
	(((v) << 28) & BM_APBH_DEVSEL_CH14)
#define BP_APBH_DEVSEL_CH13      26
#define BM_APBH_DEVSEL_CH13 0x0C000000
#define BF_APBH_DEVSEL_CH13(v)  \
	(((v) << 26) & BM_APBH_DEVSEL_CH13)
#define BP_APBH_DEVSEL_CH12      24
#define BM_APBH_DEVSEL_CH12 0x03000000
#define BF_APBH_DEVSEL_CH12(v)  \
	(((v) << 24) & BM_APBH_DEVSEL_CH12)
#define BP_APBH_DEVSEL_CH11      22
#define BM_APBH_DEVSEL_CH11 0x00C00000
#define BF_APBH_DEVSEL_CH11(v)  \
	(((v) << 22) & BM_APBH_DEVSEL_CH11)
#define BP_APBH_DEVSEL_CH10      20
#define BM_APBH_DEVSEL_CH10 0x00300000
#define BF_APBH_DEVSEL_CH10(v)  \
	(((v) << 20) & BM_APBH_DEVSEL_CH10)
#define BP_APBH_DEVSEL_CH9      18
#define BM_APBH_DEVSEL_CH9 0x000C0000
#define BF_APBH_DEVSEL_CH9(v)  \
	(((v) << 18) & BM_APBH_DEVSEL_CH9)
#define BP_APBH_DEVSEL_CH8      16
#define BM_APBH_DEVSEL_CH8 0x00030000
#define BF_APBH_DEVSEL_CH8(v)  \
	(((v) << 16) & BM_APBH_DEVSEL_CH8)
#define BP_APBH_DEVSEL_CH7      14
#define BM_APBH_DEVSEL_CH7 0x0000C000
#define BF_APBH_DEVSEL_CH7(v)  \
	(((v) << 14) & BM_APBH_DEVSEL_CH7)
#define BP_APBH_DEVSEL_CH6      12
#define BM_APBH_DEVSEL_CH6 0x00003000
#define BF_APBH_DEVSEL_CH6(v)  \
	(((v) << 12) & BM_APBH_DEVSEL_CH6)
#define BP_APBH_DEVSEL_CH5      10
#define BM_APBH_DEVSEL_CH5 0x00000C00
#define BF_APBH_DEVSEL_CH5(v)  \
	(((v) << 10) & BM_APBH_DEVSEL_CH5)
#define BP_APBH_DEVSEL_CH4      8
#define BM_APBH_DEVSEL_CH4 0x00000300
#define BF_APBH_DEVSEL_CH4(v)  \
	(((v) << 8) & BM_APBH_DEVSEL_CH4)
#define BP_APBH_DEVSEL_CH3      6
#define BM_APBH_DEVSEL_CH3 0x000000C0
#define BF_APBH_DEVSEL_CH3(v)  \
	(((v) << 6) & BM_APBH_DEVSEL_CH3)
#define BP_APBH_DEVSEL_CH2      4
#define BM_APBH_DEVSEL_CH2 0x00000030
#define BF_APBH_DEVSEL_CH2(v)  \
	(((v) << 4) & BM_APBH_DEVSEL_CH2)
#define BP_APBH_DEVSEL_CH1      2
#define BM_APBH_DEVSEL_CH1 0x0000000C
#define BF_APBH_DEVSEL_CH1(v)  \
	(((v) << 2) & BM_APBH_DEVSEL_CH1)
#define BP_APBH_DEVSEL_CH0      0
#define BM_APBH_DEVSEL_CH0 0x00000003
#define BF_APBH_DEVSEL_CH0(v)  \
	(((v) << 0) & BM_APBH_DEVSEL_CH0)

#define HW_APBH_DMA_BURST_SIZE	(0x00000050)

#define BP_APBH_DMA_BURST_SIZE_CH15      30
#define BM_APBH_DMA_BURST_SIZE_CH15 0xC0000000
#define BF_APBH_DMA_BURST_SIZE_CH15(v) \
	(((v) << 30) & BM_APBH_DMA_BURST_SIZE_CH15)
#define BP_APBH_DMA_BURST_SIZE_CH14      28
#define BM_APBH_DMA_BURST_SIZE_CH14 0x30000000
#define BF_APBH_DMA_BURST_SIZE_CH14(v)  \
	(((v) << 28) & BM_APBH_DMA_BURST_SIZE_CH14)
#define BP_APBH_DMA_BURST_SIZE_CH13      26
#define BM_APBH_DMA_BURST_SIZE_CH13 0x0C000000
#define BF_APBH_DMA_BURST_SIZE_CH13(v)  \
	(((v) << 26) & BM_APBH_DMA_BURST_SIZE_CH13)
#define BP_APBH_DMA_BURST_SIZE_CH12      24
#define BM_APBH_DMA_BURST_SIZE_CH12 0x03000000
#define BF_APBH_DMA_BURST_SIZE_CH12(v)  \
	(((v) << 24) & BM_APBH_DMA_BURST_SIZE_CH12)
#define BP_APBH_DMA_BURST_SIZE_CH11      22
#define BM_APBH_DMA_BURST_SIZE_CH11 0x00C00000
#define BF_APBH_DMA_BURST_SIZE_CH11(v)  \
	(((v) << 22) & BM_APBH_DMA_BURST_SIZE_CH11)
#define BP_APBH_DMA_BURST_SIZE_CH10      20
#define BM_APBH_DMA_BURST_SIZE_CH10 0x00300000
#define BF_APBH_DMA_BURST_SIZE_CH10(v)  \
	(((v) << 20) & BM_APBH_DMA_BURST_SIZE_CH10)
#define BP_APBH_DMA_BURST_SIZE_CH9      18
#define BM_APBH_DMA_BURST_SIZE_CH9 0x000C0000
#define BF_APBH_DMA_BURST_SIZE_CH9(v)  \
	(((v) << 18) & BM_APBH_DMA_BURST_SIZE_CH9)
#define BP_APBH_DMA_BURST_SIZE_CH8      16
#define BM_APBH_DMA_BURST_SIZE_CH8 0x00030000
#define BF_APBH_DMA_BURST_SIZE_CH8(v)  \
	(((v) << 16) & BM_APBH_DMA_BURST_SIZE_CH8)
#define BV_APBH_DMA_BURST_SIZE_CH8__BURST0 0x0
#define BV_APBH_DMA_BURST_SIZE_CH8__BURST4 0x1
#define BV_APBH_DMA_BURST_SIZE_CH8__BURST8 0x2
#define BP_APBH_DMA_BURST_SIZE_CH7      14
#define BM_APBH_DMA_BURST_SIZE_CH7 0x0000C000
#define BF_APBH_DMA_BURST_SIZE_CH7(v)  \
	(((v) << 14) & BM_APBH_DMA_BURST_SIZE_CH7)
#define BP_APBH_DMA_BURST_SIZE_CH6      12
#define BM_APBH_DMA_BURST_SIZE_CH6 0x00003000
#define BF_APBH_DMA_BURST_SIZE_CH6(v)  \
	(((v) << 12) & BM_APBH_DMA_BURST_SIZE_CH6)
#define BP_APBH_DMA_BURST_SIZE_CH5      10
#define BM_APBH_DMA_BURST_SIZE_CH5 0x00000C00
#define BF_APBH_DMA_BURST_SIZE_CH5(v)  \
	(((v) << 10) & BM_APBH_DMA_BURST_SIZE_CH5)
#define BP_APBH_DMA_BURST_SIZE_CH4      8
#define BM_APBH_DMA_BURST_SIZE_CH4 0x00000300
#define BF_APBH_DMA_BURST_SIZE_CH4(v)  \
	(((v) << 8) & BM_APBH_DMA_BURST_SIZE_CH4)
#define BP_APBH_DMA_BURST_SIZE_CH3      6
#define BM_APBH_DMA_BURST_SIZE_CH3 0x000000C0
#define BF_APBH_DMA_BURST_SIZE_CH3(v)  \
	(((v) << 6) & BM_APBH_DMA_BURST_SIZE_CH3)
#define BV_APBH_DMA_BURST_SIZE_CH3__BURST0 0x0
#define BV_APBH_DMA_BURST_SIZE_CH3__BURST4 0x1
#define BV_APBH_DMA_BURST_SIZE_CH3__BURST8 0x2

#define BP_APBH_DMA_BURST_SIZE_CH2      4
#define BM_APBH_DMA_BURST_SIZE_CH2 0x00000030
#define BF_APBH_DMA_BURST_SIZE_CH2(v)  \
	(((v) << 4) & BM_APBH_DMA_BURST_SIZE_CH2)
#define BV_APBH_DMA_BURST_SIZE_CH2__BURST0 0x0
#define BV_APBH_DMA_BURST_SIZE_CH2__BURST4 0x1
#define BV_APBH_DMA_BURST_SIZE_CH2__BURST8 0x2
#define BP_APBH_DMA_BURST_SIZE_CH1      2
#define BM_APBH_DMA_BURST_SIZE_CH1 0x0000000C
#define BF_APBH_DMA_BURST_SIZE_CH1(v)  \
	(((v) << 2) & BM_APBH_DMA_BURST_SIZE_CH1)
#define BV_APBH_DMA_BURST_SIZE_CH1__BURST0 0x0
#define BV_APBH_DMA_BURST_SIZE_CH1__BURST4 0x1
#define BV_APBH_DMA_BURST_SIZE_CH1__BURST8 0x2

#define BP_APBH_DMA_BURST_SIZE_CH0      0
#define BM_APBH_DMA_BURST_SIZE_CH0 0x00000003
#define BF_APBH_DMA_BURST_SIZE_CH0(v)  \
	(((v) << 0) & BM_APBH_DMA_BURST_SIZE_CH0)
#define BV_APBH_DMA_BURST_SIZE_CH0__BURST0 0x0
#define BV_APBH_DMA_BURST_SIZE_CH0__BURST4 0x1
#define BV_APBH_DMA_BURST_SIZE_CH0__BURST8 0x2

#define HW_APBH_DEBUG	(0x00000060)

#define BP_APBH_DEBUG_RSVD      1
#define BM_APBH_DEBUG_RSVD 0xFFFFFFFE
#define BF_APBH_DEBUG_RSVD(v) \
	(((v) << 1) & BM_APBH_DEBUG_RSVD)
#define BM_APBH_DEBUG_GPMI_ONE_FIFO 0x00000001

/*
 *  multi-register-define name HW_APBH_CHn_CURCMDAR
 *              base 0x00000100
 *              count 16
 *              offset 0x70
 */
#define HW_APBH_CHn_CURCMDAR(n)	(0x00000100 + (n) * 0x70)
#define BP_APBH_CHn_CURCMDAR_CMD_ADDR      0
#define BM_APBH_CHn_CURCMDAR_CMD_ADDR 0xFFFFFFFF
#define BF_APBH_CHn_CURCMDAR_CMD_ADDR(v)   (v)

/*
 *  multi-register-define name HW_APBH_CHn_NXTCMDAR
 *              base 0x00000110
 *              count 16
 *              offset 0x70
 */
#define HW_APBH_CHn_NXTCMDAR(n)	(0x00000110 + (n) * 0x70)
#define BP_APBH_CHn_NXTCMDAR_CMD_ADDR      0
#define BM_APBH_CHn_NXTCMDAR_CMD_ADDR 0xFFFFFFFF
#define BF_APBH_CHn_NXTCMDAR_CMD_ADDR(v)   (v)

/*
 *  multi-register-define name HW_APBH_CHn_CMD
 *              base 0x00000120
 *              count 16
 *              offset 0x70
 */
#define HW_APBH_CHn_CMD(n)	(0x00000120 + (n) * 0x70)
#define BP_APBH_CHn_CMD_XFER_COUNT      16
#define BM_APBH_CHn_CMD_XFER_COUNT 0xFFFF0000
#define BF_APBH_CHn_CMD_XFER_COUNT(v) \
	(((v) << 16) & BM_APBH_CHn_CMD_XFER_COUNT)
#define BP_APBH_CHn_CMD_CMDWORDS      12
#define BM_APBH_CHn_CMD_CMDWORDS 0x0000F000
#define BF_APBH_CHn_CMD_CMDWORDS(v)  \
	(((v) << 12) & BM_APBH_CHn_CMD_CMDWORDS)
#define BP_APBH_CHn_CMD_RSVD1      9
#define BM_APBH_CHn_CMD_RSVD1 0x00000E00
#define BF_APBH_CHn_CMD_RSVD1(v)  \
	(((v) << 9) & BM_APBH_CHn_CMD_RSVD1)
#define BM_APBH_CHn_CMD_HALTONTERMINATE 0x00000100
#define BM_APBH_CHn_CMD_WAIT4ENDCMD 0x00000080
#define BM_APBH_CHn_CMD_SEMAPHORE 0x00000040
#define BM_APBH_CHn_CMD_NANDWAIT4READY 0x00000020
#define BM_APBH_CHn_CMD_NANDLOCK 0x00000010
#define BM_APBH_CHn_CMD_IRQONCMPLT 0x00000008
#define BM_APBH_CHn_CMD_CHAIN 0x00000004
#define BP_APBH_CHn_CMD_COMMAND      0
#define BM_APBH_CHn_CMD_COMMAND 0x00000003
#define BF_APBH_CHn_CMD_COMMAND(v)  \
	(((v) << 0) & BM_APBH_CHn_CMD_COMMAND)
#define BV_APBH_CHn_CMD_COMMAND__NO_DMA_XFER 0x0
#define BV_APBH_CHn_CMD_COMMAND__DMA_WRITE   0x1
#define BV_APBH_CHn_CMD_COMMAND__DMA_READ    0x2
#define BV_APBH_CHn_CMD_COMMAND__DMA_SENSE   0x3

/*
 *  multi-register-define name HW_APBH_CHn_BAR
 *              base 0x00000130
 *              count 16
 *              offset 0x70
 */
#define HW_APBH_CHn_BAR(n)	(0x00000130 + (n) * 0x70)
#define BP_APBH_CHn_BAR_ADDRESS      0
#define BM_APBH_CHn_BAR_ADDRESS 0xFFFFFFFF
#define BF_APBH_CHn_BAR_ADDRESS(v)   (v)

/*
 *  multi-register-define name HW_APBH_CHn_SEMA
 *              base 0x00000140
 *              count 16
 *              offset 0x70
 */
#define HW_APBH_CHn_SEMA(n)	(0x00000140 + (n) * 0x70)
#define BP_APBH_CHn_SEMA_RSVD2      24
#define BM_APBH_CHn_SEMA_RSVD2 0xFF000000
#define BF_APBH_CHn_SEMA_RSVD2(v) \
	(((v) << 24) & BM_APBH_CHn_SEMA_RSVD2)
#define BP_APBH_CHn_SEMA_PHORE      16
#define BM_APBH_CHn_SEMA_PHORE 0x00FF0000
#define BF_APBH_CHn_SEMA_PHORE(v)  \
	(((v) << 16) & BM_APBH_CHn_SEMA_PHORE)
#define BP_APBH_CHn_SEMA_RSVD1      8
#define BM_APBH_CHn_SEMA_RSVD1 0x0000FF00
#define BF_APBH_CHn_SEMA_RSVD1(v)  \
	(((v) << 8) & BM_APBH_CHn_SEMA_RSVD1)
#define BP_APBH_CHn_SEMA_INCREMENT_SEMA      0
#define BM_APBH_CHn_SEMA_INCREMENT_SEMA 0x000000FF
#define BF_APBH_CHn_SEMA_INCREMENT_SEMA(v)  \
	(((v) << 0) & BM_APBH_CHn_SEMA_INCREMENT_SEMA)

/*
 *  multi-register-define name HW_APBH_CHn_DEBUG1
 *              base 0x00000150
 *              count 16
 *              offset 0x70
 */
#define HW_APBH_CHn_DEBUG1(n)	(0x00000150 + (n) * 0x70)
#define BM_APBH_CHn_DEBUG1_REQ 0x80000000
#define BM_APBH_CHn_DEBUG1_BURST 0x40000000
#define BM_APBH_CHn_DEBUG1_KICK 0x20000000
#define BM_APBH_CHn_DEBUG1_END 0x10000000
#define BM_APBH_CHn_DEBUG1_SENSE 0x08000000
#define BM_APBH_CHn_DEBUG1_READY 0x04000000
#define BM_APBH_CHn_DEBUG1_LOCK 0x02000000
#define BM_APBH_CHn_DEBUG1_NEXTCMDADDRVALID 0x01000000
#define BM_APBH_CHn_DEBUG1_RD_FIFO_EMPTY 0x00800000
#define BM_APBH_CHn_DEBUG1_RD_FIFO_FULL 0x00400000
#define BM_APBH_CHn_DEBUG1_WR_FIFO_EMPTY 0x00200000
#define BM_APBH_CHn_DEBUG1_WR_FIFO_FULL 0x00100000
#define BP_APBH_CHn_DEBUG1_RSVD1      5
#define BM_APBH_CHn_DEBUG1_RSVD1 0x000FFFE0
#define BF_APBH_CHn_DEBUG1_RSVD1(v)  \
	(((v) << 5) & BM_APBH_CHn_DEBUG1_RSVD1)
#define BP_APBH_CHn_DEBUG1_STATEMACHINE      0
#define BM_APBH_CHn_DEBUG1_STATEMACHINE 0x0000001F
#define BF_APBH_CHn_DEBUG1_STATEMACHINE(v)  \
	(((v) << 0) & BM_APBH_CHn_DEBUG1_STATEMACHINE)
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__IDLE            0x00
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__REQ_CMD1        0x01
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__REQ_CMD3        0x02
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__REQ_CMD2        0x03
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__XFER_DECODE     0x04
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__REQ_WAIT        0x05
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__REQ_CMD4        0x06
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__PIO_REQ         0x07
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__READ_FLUSH      0x08
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__READ_WAIT       0x09
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__WRITE           0x0C
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__READ_REQ        0x0D
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__CHECK_CHAIN     0x0E
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__XFER_COMPLETE   0x0F
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__TERMINATE       0x14
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__WAIT_END        0x15
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__WRITE_WAIT      0x1C
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__HALT_AFTER_TERM 0x1D
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__CHECK_WAIT      0x1E
#define BV_APBH_CHn_DEBUG1_STATEMACHINE__WAIT_READY      0x1F

/*
 *  multi-register-define name HW_APBH_CHn_DEBUG2
 *              base 0x00000160
 *              count 16
 *              offset 0x70
 */
#define HW_APBH_CHn_DEBUG2(n)	(0x00000160 + (n) * 0x70)
#define BP_APBH_CHn_DEBUG2_APB_BYTES      16
#define BM_APBH_CHn_DEBUG2_APB_BYTES 0xFFFF0000
#define BF_APBH_CHn_DEBUG2_APB_BYTES(v) \
	(((v) << 16) & BM_APBH_CHn_DEBUG2_APB_BYTES)
#define BP_APBH_CHn_DEBUG2_AHB_BYTES      0
#define BM_APBH_CHn_DEBUG2_AHB_BYTES 0x0000FFFF
#define BF_APBH_CHn_DEBUG2_AHB_BYTES(v)  \
	(((v) << 0) & BM_APBH_CHn_DEBUG2_AHB_BYTES)

#define HW_APBH_VERSION	(0x00000800)

#define BP_APBH_VERSION_MAJOR      24
#define BM_APBH_VERSION_MAJOR 0xFF000000
#define BF_APBH_VERSION_MAJOR(v) \
	(((v) << 24) & BM_APBH_VERSION_MAJOR)
#define BP_APBH_VERSION_MINOR      16
#define BM_APBH_VERSION_MINOR 0x00FF0000
#define BF_APBH_VERSION_MINOR(v)  \
	(((v) << 16) & BM_APBH_VERSION_MINOR)
#define BP_APBH_VERSION_STEP      0
#define BM_APBH_VERSION_STEP 0x0000FFFF
#define BF_APBH_VERSION_STEP(v)  \
	(((v) << 0) & BM_APBH_VERSION_STEP)



#endif /* _REGS_DMA_APBH_H_ */
