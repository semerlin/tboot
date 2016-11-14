#ifndef _GPMI_H_
  #define _GPMI_H_

#include "types.h"


/* gpmi命令缓冲区大小 */
#define GPMI_COMMAND_BUFFER_SIZE      (10)

/* ECC布局 */
#define GPMI_ECC_METADATA_SIZE        (10)
#define GPMI_ECC_BLOCK_SIZE           (512)

#if 0
static inline uint32_t gpmi_nfc_get_blk_mark_bit_ofs(uint32_t page_data_size, uint32_t ecc_strength)
{
	uint32_t chunk_data_size_in_bits;
	uint32_t chunk_ecc_size_in_bits;
	uint32_t chunk_total_size_in_bits;
	uint32_t block_mark_chunk_number;
	uint32_t block_mark_chunk_bit_offset;
	uint32_t block_mark_bit_offset;

	/* 4096 bits */
	chunk_data_size_in_bits = GPMI_NFC_CHUNK_DATA_CHUNK_SIZE * 8;
	/* 208 bits */
	chunk_ecc_size_in_bits  = GPMI_NFC_CHUNK_ECC_SIZE_IN_BITS(ecc_strength);

	/* 4304 bits */
	chunk_total_size_in_bits = chunk_data_size_in_bits + chunk_ecc_size_in_bits;

	/* Compute the bit offset of the block mark within the physical page. */
	/* 4096 * 8 = 32768 bits */
	block_mark_bit_offset = page_data_size * 8;

	/* Subtract the metadata bits. */
	/* 32688 bits */
	block_mark_bit_offset -= GPMI_NFC_METADATA_SIZE * 8;

	/*
	 * Compute the chunk number (starting at zero) in which the block mark
	 * appears.
	 */
	/* 7 */
	block_mark_chunk_number = block_mark_bit_offset / chunk_total_size_in_bits;

	/*
	 * Compute the bit offset of the block mark within its chunk, and
	 * validate it.
	 */
	/* 2560 bits */
	block_mark_chunk_bit_offset = block_mark_bit_offset - (block_mark_chunk_number * chunk_total_size_in_bits);

	if (block_mark_chunk_bit_offset > chunk_data_size_in_bits)
		return 1;

	/*
	 * Now that we know the chunk number in which the block mark appears,
	 * we can subtract all the ECC bits that appear before it.
	 */
	/* 31232 bits */
	block_mark_bit_offset -= block_mark_chunk_number * chunk_ecc_size_in_bits;

	return block_mark_bit_offset;
}
#endif

static inline uint32_t gpmi_nfc_get_ecc_strength(uint32_t page_data_size, uint32_t page_oob_size)
{
    if(2048 == page_data_size)
        return 8;
	else if (4096 == page_data_size)
	{
		if (128 == page_oob_size)
			return 8;
		else if (218 == page_oob_size)
			return 16;
		else
			return 0;
	}
	else
		return 0;
}

/* gpmi信息结构体 */
struct gpmi_info
{
    /* 当前操作的芯片 */
	int32_t cur_chip;

	/* data区数据缓冲区 */
	uint8_t  *data_buf;

	/* oob数据缓冲区 */
	uint8_t  *oob_buf;

    /* 每页ecc块数量 */
	uint32_t ecc_chunk_cnt;

	/* ecc强度 */
	uint32_t ecc_strength;

	/* 块好/坏状态偏移地址 */
	uint32_t aux_status_ofs;
};

















#endif /* _GPMI_H_ */

