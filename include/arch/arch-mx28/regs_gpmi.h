#ifndef _REGS_GPMI_H_
  #define _REGS_GPMI_H_




#define HW_GPMI_CTRL0	(0x00000000)
#define HW_GPMI_CTRL0_SET	(0x00000004)
#define HW_GPMI_CTRL0_CLR	(0x00000008)
#define HW_GPMI_CTRL0_TOG	(0x0000000c)

#define BM_GPMI_CTRL0_SFTRST 0x80000000
#define BV_GPMI_CTRL0_SFTRST__RUN   0x0
#define BV_GPMI_CTRL0_SFTRST__RESET 0x1
#define BM_GPMI_CTRL0_CLKGATE 0x40000000
#define BV_GPMI_CTRL0_CLKGATE__RUN     0x0
#define BV_GPMI_CTRL0_CLKGATE__NO_CLKS 0x1
#define BM_GPMI_CTRL0_RUN 0x20000000
#define BV_GPMI_CTRL0_RUN__IDLE 0x0
#define BV_GPMI_CTRL0_RUN__BUSY 0x1
#define BM_GPMI_CTRL0_DEV_IRQ_EN 0x10000000
#define BM_GPMI_CTRL0_LOCK_CS 0x08000000
#define BV_GPMI_CTRL0_LOCK_CS__DISABLED 0x0
#define BV_GPMI_CTRL0_LOCK_CS__ENABLED  0x1

#define BM_GPMI_CTRL0_UDMA 0x04000000
#define BV_GPMI_CTRL0_UDMA__DISABLED 0x0
#define BV_GPMI_CTRL0_UDMA__ENABLED  0x1
#define BP_GPMI_CTRL0_COMMAND_MODE      24
#define BM_GPMI_CTRL0_COMMAND_MODE 0x03000000
#define BF_GPMI_CTRL0_COMMAND_MODE(v)  \
	(((v) << 24) & BM_GPMI_CTRL0_COMMAND_MODE)
#define BV_GPMI_CTRL0_COMMAND_MODE__WRITE            0x0
#define BV_GPMI_CTRL0_COMMAND_MODE__READ             0x1
#define BV_GPMI_CTRL0_COMMAND_MODE__READ_AND_COMPARE 0x2
#define BV_GPMI_CTRL0_COMMAND_MODE__WAIT_FOR_READY   0x3
#define BM_GPMI_CTRL0_WORD_LENGTH 0x00800000
#define BV_GPMI_CTRL0_WORD_LENGTH__16_BIT 0x0
#define BV_GPMI_CTRL0_WORD_LENGTH__8_BIT  0x1

#define BP_GPMI_CTRL0_CS      20
#define BM_GPMI_CTRL0_CS 0x00700000
#define BF_GPMI_CTRL0_CS(v)  \
	(((v) << 20) & BM_GPMI_CTRL0_CS)
#define BP_GPMI_CTRL0_ADDRESS      17
#define BM_GPMI_CTRL0_ADDRESS 0x000E0000
#define BF_GPMI_CTRL0_ADDRESS(v)  \
	(((v) << 17) & BM_GPMI_CTRL0_ADDRESS)
#define BV_GPMI_CTRL0_ADDRESS__NAND_DATA 0x0
#define BV_GPMI_CTRL0_ADDRESS__NAND_CLE  0x1
#define BV_GPMI_CTRL0_ADDRESS__NAND_ALE  0x2
#define BM_GPMI_CTRL0_ADDRESS_INCREMENT 0x00010000
#define BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED 0x0
#define BV_GPMI_CTRL0_ADDRESS_INCREMENT__ENABLED  0x1
#define BP_GPMI_CTRL0_XFER_COUNT      0
#define BM_GPMI_CTRL0_XFER_COUNT 0x0000FFFF
#define BF_GPMI_CTRL0_XFER_COUNT(v)  \
	(((v) << 0) & BM_GPMI_CTRL0_XFER_COUNT)

#define HW_GPMI_COMPARE	(0x00000010)

#define BP_GPMI_COMPARE_MASK      16
#define BM_GPMI_COMPARE_MASK 0xFFFF0000
#define BF_GPMI_COMPARE_MASK(v) \
	(((v) << 16) & BM_GPMI_COMPARE_MASK)
#define BP_GPMI_COMPARE_REFERENCE      0
#define BM_GPMI_COMPARE_REFERENCE 0x0000FFFF
#define BF_GPMI_COMPARE_REFERENCE(v)  \
	(((v) << 0) & BM_GPMI_COMPARE_REFERENCE)

#define HW_GPMI_ECCCTRL	(0x00000020)
#define HW_GPMI_ECCCTRL_SET	(0x00000024)
#define HW_GPMI_ECCCTRL_CLR	(0x00000028)
#define HW_GPMI_ECCCTRL_TOG	(0x0000002c)

#define BP_GPMI_ECCCTRL_HANDLE      16
#define BM_GPMI_ECCCTRL_HANDLE 0xFFFF0000
#define BF_GPMI_ECCCTRL_HANDLE(v) \
	(((v) << 16) & BM_GPMI_ECCCTRL_HANDLE)
#define BM_GPMI_ECCCTRL_RSVD2 0x00008000
#define BP_GPMI_ECCCTRL_ECC_CMD      13
#define BM_GPMI_ECCCTRL_ECC_CMD 0x00006000
#define BF_GPMI_ECCCTRL_ECC_CMD(v)  \
	(((v) << 13) & BM_GPMI_ECCCTRL_ECC_CMD)

#define BV_GPMI_ECCCTRL_ECC_CMD__DECODE   0x0
#define BV_GPMI_ECCCTRL_ECC_CMD__ENCODE   0x1
#define BV_GPMI_ECCCTRL_ECC_CMD__RESERVE2 0x2
#define BV_GPMI_ECCCTRL_ECC_CMD__RESERVE3 0x3

#define BM_GPMI_ECCCTRL_ENABLE_ECC 0x00001000
#define BV_GPMI_ECCCTRL_ENABLE_ECC__ENABLE  0x1
#define BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE 0x0
#define BP_GPMI_ECCCTRL_RSVD1      9
#define BM_GPMI_ECCCTRL_RSVD1 0x00000E00
#define BF_GPMI_ECCCTRL_RSVD1(v)  \
	(((v) << 9) & BM_GPMI_ECCCTRL_RSVD1)
#define BP_GPMI_ECCCTRL_BUFFER_MASK      0
#define BM_GPMI_ECCCTRL_BUFFER_MASK 0x000001FF
#define BF_GPMI_ECCCTRL_BUFFER_MASK(v)  \
	(((v) << 0) & BM_GPMI_ECCCTRL_BUFFER_MASK)
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_AUXONLY 0x100
#define BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE    0x1FF

#define HW_GPMI_ECCCOUNT	(0x00000030)

#define BP_GPMI_ECCCOUNT_RSVD2      16
#define BM_GPMI_ECCCOUNT_RSVD2 0xFFFF0000
#define BF_GPMI_ECCCOUNT_RSVD2(v) \
	(((v) << 16) & BM_GPMI_ECCCOUNT_RSVD2)
#define BP_GPMI_ECCCOUNT_COUNT      0
#define BM_GPMI_ECCCOUNT_COUNT 0x0000FFFF
#define BF_GPMI_ECCCOUNT_COUNT(v)  \
	(((v) << 0) & BM_GPMI_ECCCOUNT_COUNT)

#define HW_GPMI_PAYLOAD	(0x00000040)

#define BP_GPMI_PAYLOAD_ADDRESS      2
#define BM_GPMI_PAYLOAD_ADDRESS 0xFFFFFFFC
#define BF_GPMI_PAYLOAD_ADDRESS(v) \
	(((v) << 2) & BM_GPMI_PAYLOAD_ADDRESS)
#define BP_GPMI_PAYLOAD_RSVD0      0
#define BM_GPMI_PAYLOAD_RSVD0 0x00000003
#define BF_GPMI_PAYLOAD_RSVD0(v)  \
	(((v) << 0) & BM_GPMI_PAYLOAD_RSVD0)

#define HW_GPMI_AUXILIARY	(0x00000050)

#define BP_GPMI_AUXILIARY_ADDRESS      2
#define BM_GPMI_AUXILIARY_ADDRESS 0xFFFFFFFC
#define BF_GPMI_AUXILIARY_ADDRESS(v) \
	(((v) << 2) & BM_GPMI_AUXILIARY_ADDRESS)
#define BP_GPMI_AUXILIARY_RSVD0      0
#define BM_GPMI_AUXILIARY_RSVD0 0x00000003
#define BF_GPMI_AUXILIARY_RSVD0(v)  \
	(((v) << 0) & BM_GPMI_AUXILIARY_RSVD0)

#define HW_GPMI_CTRL1	(0x00000060)
#define HW_GPMI_CTRL1_SET	(0x00000064)
#define HW_GPMI_CTRL1_CLR	(0x00000068)
#define HW_GPMI_CTRL1_TOG	(0x0000006c)

#define BP_GPMI_CTRL1_RSVD2	25
#define BM_GPMI_CTRL1_RSVD2	0xFE000000
#define BF_GPMI_CTRL1_RSVD2(v) \
		(((v) << 25) & BM_GPMI_CTRL1_RSVD2)
#define BM_GPMI_CTRL1_DECOUPLE_CS 0x01000000
#define BP_GPMI_CTRL1_WRN_DLY_SEL      22
#define BM_GPMI_CTRL1_WRN_DLY_SEL 0x00C00000
#define BF_GPMI_CTRL1_WRN_DLY_SEL(v)  \
	(((v) << 22) & BM_GPMI_CTRL1_WRN_DLY_SEL)
#define BM_GPMI_CTRL1_RSVD1 0x00200000
#define BM_GPMI_CTRL1_TIMEOUT_IRQ_EN 0x00100000
#define BM_GPMI_CTRL1_GANGED_RDYBUSY 0x00080000
#define BM_GPMI_CTRL1_BCH_MODE 0x00040000



#define BP_GPMI_CTRL1_DLL_ENABLE	17
#define BM_GPMI_CTRL1_DLL_ENABLE 0x00020000
#define BP_GPMI_CTRL1_HALF_PERIOD       16
#define BM_GPMI_CTRL1_HALF_PERIOD 0x00010000
#define BP_GPMI_CTRL1_RDN_DELAY      12
#define BM_GPMI_CTRL1_RDN_DELAY 0x0000F000
#define BF_GPMI_CTRL1_RDN_DELAY(v)  \
	(((v) << 12) & BM_GPMI_CTRL1_RDN_DELAY)
#define BM_GPMI_CTRL1_DMA2ECC_MODE 0x00000800
#define BM_GPMI_CTRL1_DEV_IRQ 0x00000400
#define BM_GPMI_CTRL1_TIMEOUT_IRQ 0x00000200
#define BM_GPMI_CTRL1_BURST_EN 0x00000100

#define BM_GPMI_CTRL1_ABORT_WAIT_REQUEST 0x00000080
#define BP_GPMI_CTRL1_ABORT_WAIT_FOR_READY_CHANNEL      4
#define BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY_CHANNEL 0x00000070
#define BF_GPMI_CTRL1_ABORT_WAIT_FOR_READY_CHANNEL(v)  \
	(((v) << 4) & BM_GPMI_CTRL1_ABORT_WAIT_FOR_READY_CHANNEL)

#define BM_GPMI_CTRL1_DEV_RESET 0x00000008
#define BV_GPMI_CTRL1_DEV_RESET__ENABLED  0x0
#define BV_GPMI_CTRL1_DEV_RESET__DISABLED 0x1
#define BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY 0x00000004
#define BV_GPMI_CTRL1_ATA_IRQRDY_POLARITY__ACTIVELOW  0x0
#define BV_GPMI_CTRL1_ATA_IRQRDY_POLARITY__ACTIVEHIGH 0x1
#define BM_GPMI_CTRL1_CAMERA_MODE 0x00000002
#define BM_GPMI_CTRL1_GPMI_MODE 0x00000001
#define BV_GPMI_CTRL1_GPMI_MODE__NAND 0x0
#define BV_GPMI_CTRL1_GPMI_MODE__ATA  0x1

#define HW_GPMI_TIMING0	(0x00000070)

#define BP_GPMI_TIMING0_RSVD1      24
#define BM_GPMI_TIMING0_RSVD1 0xFF000000
#define BF_GPMI_TIMING0_RSVD1(v) \
	(((v) << 24) & BM_GPMI_TIMING0_RSVD1)
#define BP_GPMI_TIMING0_ADDRESS_SETUP      16
#define BM_GPMI_TIMING0_ADDRESS_SETUP 0x00FF0000
#define BF_GPMI_TIMING0_ADDRESS_SETUP(v)  \
	(((v) << 16) & BM_GPMI_TIMING0_ADDRESS_SETUP)
#define BP_GPMI_TIMING0_DATA_HOLD      8
#define BM_GPMI_TIMING0_DATA_HOLD 0x0000FF00
#define BF_GPMI_TIMING0_DATA_HOLD(v)  \
	(((v) << 8) & BM_GPMI_TIMING0_DATA_HOLD)
#define BP_GPMI_TIMING0_DATA_SETUP      0
#define BM_GPMI_TIMING0_DATA_SETUP 0x000000FF
#define BF_GPMI_TIMING0_DATA_SETUP(v)  \
	(((v) << 0) & BM_GPMI_TIMING0_DATA_SETUP)

#define HW_GPMI_TIMING1	(0x00000080)

#define BP_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT      16
#define BM_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT 0xFFFF0000
#define BF_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT(v) \
	(((v) << 16) & BM_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT)
#define BP_GPMI_TIMING1_RSVD1      0
#define BM_GPMI_TIMING1_RSVD1 0x0000FFFF
#define BF_GPMI_TIMING1_RSVD1(v)  \
	(((v) << 0) & BM_GPMI_TIMING1_RSVD1)

#define HW_GPMI_TIMING2	(0x00000090)



#define BP_GPMI_TIMING2_RSVD1      27
#define BM_GPMI_TIMING2_RSVD1 0xF8000000
#define BF_GPMI_TIMING2_RSVD1(v) \
	(((v) << 27) & BM_GPMI_TIMING2_RSVD1)
#define BP_GPMI_TIMING2_READ_LATENCY      24
#define BM_GPMI_TIMING2_READ_LATENCY 0x07000000
#define BF_GPMI_TIMING2_READ_LATENCY(v)  \
	(((v) << 24) & BM_GPMI_TIMING2_READ_LATENCY)
#define BP_GPMI_TIMING2_RSVD0      21
#define BM_GPMI_TIMING2_RSVD0 0x00E00000
#define BF_GPMI_TIMING2_RSVD0(v)  \
	(((v) << 21) & BM_GPMI_TIMING2_RSVD0)
#define BP_GPMI_TIMING2_CE_DELAY      16
#define BM_GPMI_TIMING2_CE_DELAY 0x001F0000
#define BF_GPMI_TIMING2_CE_DELAY(v)  \
	(((v) << 16) & BM_GPMI_TIMING2_CE_DELAY)
#define BP_GPMI_TIMING2_PREAMBLE_DELAY      12
#define BM_GPMI_TIMING2_PREAMBLE_DELAY 0x0000F000
#define BF_GPMI_TIMING2_PREAMBLE_DELAY(v)  \
	(((v) << 12) & BM_GPMI_TIMING2_PREAMBLE_DELAY)
#define BP_GPMI_TIMING2_POSTAMBLE_DELAY      8
#define BM_GPMI_TIMING2_POSTAMBLE_DELAY 0x00000F00
#define BF_GPMI_TIMING2_POSTAMBLE_DELAY(v)  \
	(((v) << 8) & BM_GPMI_TIMING2_POSTAMBLE_DELAY)
#define BP_GPMI_TIMING2_CMDADD_PAUSE      4
#define BM_GPMI_TIMING2_CMDADD_PAUSE 0x000000F0
#define BF_GPMI_TIMING2_CMDADD_PAUSE(v)  \
	(((v) << 4) & BM_GPMI_TIMING2_CMDADD_PAUSE)
#define BP_GPMI_TIMING2_DATA_PAUSE      0
#define BM_GPMI_TIMING2_DATA_PAUSE 0x0000000F
#define BF_GPMI_TIMING2_DATA_PAUSE(v)  \
	(((v) << 0) & BM_GPMI_TIMING2_DATA_PAUSE)


#define HW_GPMI_DATA	(0x000000a0)

#define BP_GPMI_DATA_DATA      0
#define BM_GPMI_DATA_DATA 0xFFFFFFFF
#define BF_GPMI_DATA_DATA(v)   (v)

#define HW_GPMI_STAT	(0x000000b0)


#define BP_GPMI_STAT_READY_BUSY      24
#define BM_GPMI_STAT_READY_BUSY 0xFF000000
#define BF_GPMI_STAT_READY_BUSY(v) \
	(((v) << 24) & BM_GPMI_STAT_READY_BUSY)
#define BP_GPMI_STAT_RDY_TIMEOUT      16
#define BM_GPMI_STAT_RDY_TIMEOUT 0x00FF0000
#define BF_GPMI_STAT_RDY_TIMEOUT(v)  \
	(((v) << 16) & BM_GPMI_STAT_RDY_TIMEOUT)
#define BM_GPMI_STAT_DEV7_ERROR 0x00008000
#define BM_GPMI_STAT_DEV6_ERROR 0x00004000
#define BM_GPMI_STAT_DEV5_ERROR 0x00002000
#define BM_GPMI_STAT_DEV4_ERROR 0x00001000
#define BM_GPMI_STAT_DEV3_ERROR 0x00000800
#define BM_GPMI_STAT_DEV2_ERROR 0x00000400
#define BM_GPMI_STAT_DEV1_ERROR 0x00000200
#define BM_GPMI_STAT_DEV0_ERROR 0x00000100
#define BP_GPMI_STAT_RSVD1      5
#define BM_GPMI_STAT_RSVD1 0x000000E0
#define BF_GPMI_STAT_RSVD1(v)  \
	(((v) << 5) & BM_GPMI_STAT_RSVD1)
#define BM_GPMI_STAT_ATA_IRQ 0x00000010
#define BM_GPMI_STAT_INVALID_BUFFER_MASK 0x00000008
#define BM_GPMI_STAT_FIFO_EMPTY 0x00000004
#define BV_GPMI_STAT_FIFO_EMPTY__NOT_EMPTY 0x0
#define BV_GPMI_STAT_FIFO_EMPTY__EMPTY     0x1
#define BM_GPMI_STAT_FIFO_FULL 0x00000002
#define BV_GPMI_STAT_FIFO_FULL__NOT_FULL 0x0
#define BV_GPMI_STAT_FIFO_FULL__FULL     0x1
#define BM_GPMI_STAT_PRESENT 0x00000001
#define BV_GPMI_STAT_PRESENT__UNAVAILABLE 0x0
#define BV_GPMI_STAT_PRESENT__AVAILABLE   0x1


#define HW_GPMI_DEBUG	(0x000000c0)


#define BP_GPMI_DEBUG_WAIT_FOR_READY_END      24
#define BM_GPMI_DEBUG_WAIT_FOR_READY_END 0xFF000000
#define BF_GPMI_DEBUG_WAIT_FOR_READY_END(v) \
	(((v) << 24) & BM_GPMI_DEBUG_WAIT_FOR_READY_END)
#define BP_GPMI_DEBUG_DMA_SENSE      16
#define BM_GPMI_DEBUG_DMA_SENSE 0x00FF0000
#define BF_GPMI_DEBUG_DMA_SENSE(v)  \
	(((v) << 16) & BM_GPMI_DEBUG_DMA_SENSE)
#define BP_GPMI_DEBUG_DMAREQ      8
#define BM_GPMI_DEBUG_DMAREQ 0x0000FF00
#define BF_GPMI_DEBUG_DMAREQ(v)  \
	(((v) << 8) & BM_GPMI_DEBUG_DMAREQ)
#define BP_GPMI_DEBUG_CMD_END      0
#define BM_GPMI_DEBUG_CMD_END 0x000000FF
#define BF_GPMI_DEBUG_CMD_END(v)  \
	(((v) << 0) & BM_GPMI_DEBUG_CMD_END)


#define HW_GPMI_VERSION	(0x000000d0)

#define BP_GPMI_VERSION_MAJOR      24
#define BM_GPMI_VERSION_MAJOR 0xFF000000
#define BF_GPMI_VERSION_MAJOR(v) \
	(((v) << 24) & BM_GPMI_VERSION_MAJOR)
#define BP_GPMI_VERSION_MINOR      16
#define BM_GPMI_VERSION_MINOR 0x00FF0000
#define BF_GPMI_VERSION_MINOR(v)  \
	(((v) << 16) & BM_GPMI_VERSION_MINOR)
#define BP_GPMI_VERSION_STEP      0
#define BM_GPMI_VERSION_STEP 0x0000FFFF
#define BF_GPMI_VERSION_STEP(v)  \
	(((v) << 0) & BM_GPMI_VERSION_STEP)

#define HW_GPMI_DEBUG2	(0x000000e0)


#define BP_GPMI_DEBUG2_RSVD1      28
#define BM_GPMI_DEBUG2_RSVD1 0xF0000000
#define BF_GPMI_DEBUG2_RSVD1(v) \
	(((v) << 28) & BM_GPMI_DEBUG2_RSVD1)
#define BP_GPMI_DEBUG2_UDMA_STATE      24
#define BM_GPMI_DEBUG2_UDMA_STATE 0x0F000000
#define BF_GPMI_DEBUG2_UDMA_STATE(v)  \
	(((v) << 24) & BM_GPMI_DEBUG2_UDMA_STATE)
#define BM_GPMI_DEBUG2_BUSY 0x00800000
#define BV_GPMI_DEBUG2_BUSY__DISABLED 0x0
#define BV_GPMI_DEBUG2_BUSY__ENABLED  0x1
#define BP_GPMI_DEBUG2_PIN_STATE      20
#define BM_GPMI_DEBUG2_PIN_STATE 0x00700000
#define BF_GPMI_DEBUG2_PIN_STATE(v)  \
	(((v) << 20) & BM_GPMI_DEBUG2_PIN_STATE)
#define BV_GPMI_DEBUG2_PIN_STATE__PSM_IDLE   0x0
#define BV_GPMI_DEBUG2_PIN_STATE__PSM_BYTCNT 0x1
#define BV_GPMI_DEBUG2_PIN_STATE__PSM_ADDR   0x2
#define BV_GPMI_DEBUG2_PIN_STATE__PSM_STALL  0x3
#define BV_GPMI_DEBUG2_PIN_STATE__PSM_STROBE 0x4
#define BV_GPMI_DEBUG2_PIN_STATE__PSM_ATARDY 0x5
#define BV_GPMI_DEBUG2_PIN_STATE__PSM_DHOLD  0x6
#define BV_GPMI_DEBUG2_PIN_STATE__PSM_DONE   0x7
#define BP_GPMI_DEBUG2_MAIN_STATE      16
#define BM_GPMI_DEBUG2_MAIN_STATE 0x000F0000
#define BF_GPMI_DEBUG2_MAIN_STATE(v)  \
	(((v) << 16) & BM_GPMI_DEBUG2_MAIN_STATE)
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_IDLE   0x0
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_BYTCNT 0x1
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_WAITFE 0x2
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_WAITFR 0x3
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_DMAREQ 0x4
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_DMAACK 0x5
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_WAITFF 0x6
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_LDFIFO 0x7
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_LDDMAR 0x8
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_RDCMP  0x9
#define BV_GPMI_DEBUG2_MAIN_STATE__MSM_DONE   0xA
#define BP_GPMI_DEBUG2_SYND2GPMI_BE      12
#define BM_GPMI_DEBUG2_SYND2GPMI_BE 0x0000F000
#define BF_GPMI_DEBUG2_SYND2GPMI_BE(v)  \
	(((v) << 12) & BM_GPMI_DEBUG2_SYND2GPMI_BE)
#define BM_GPMI_DEBUG2_GPMI2SYND_VALID 0x00000800
#define BM_GPMI_DEBUG2_GPMI2SYND_READY 0x00000400
#define BM_GPMI_DEBUG2_SYND2GPMI_VALID 0x00000200
#define BM_GPMI_DEBUG2_SYND2GPMI_READY 0x00000100
#define BM_GPMI_DEBUG2_VIEW_DELAYED_RDN 0x00000080
#define BM_GPMI_DEBUG2_UPDATE_WINDOW 0x00000040
#define BP_GPMI_DEBUG2_RDN_TAP      0
#define BM_GPMI_DEBUG2_RDN_TAP 0x0000003F
#define BF_GPMI_DEBUG2_RDN_TAP(v)  \
	(((v) << 0) & BM_GPMI_DEBUG2_RDN_TAP)


#define HW_GPMI_DEBUG3	(0x000000f0)

#define BP_GPMI_DEBUG3_APB_WORD_CNTR      16
#define BM_GPMI_DEBUG3_APB_WORD_CNTR 0xFFFF0000
#define BF_GPMI_DEBUG3_APB_WORD_CNTR(v) \
	(((v) << 16) & BM_GPMI_DEBUG3_APB_WORD_CNTR)
#define BP_GPMI_DEBUG3_DEV_WORD_CNTR      0
#define BM_GPMI_DEBUG3_DEV_WORD_CNTR 0x0000FFFF
#define BF_GPMI_DEBUG3_DEV_WORD_CNTR(v)  \
	(((v) << 0) & BM_GPMI_DEBUG3_DEV_WORD_CNTR)


#define GPMI_NFC_COMMAND_BUFFER_SIZE (10)



















#endif

