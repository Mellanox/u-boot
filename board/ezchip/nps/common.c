/*
*	Copyright (C) 2015 EZchip, Inc. (www.ezchip.com)
*
* 	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License version 2 as
*	published by the Free Software Foundation.
*/

#include <asm/arcregs.h>
#include <asm/io.h>
#include "common.h"

void flash_to_mem(unsigned int src_addr, unsigned int dst_addr, unsigned int size)
{
	/* source address */
	write_aux_reg(COPY_ACCELERATOR_SRC_AUX_REG, src_addr);
	/* destination address */
	write_aux_reg(COPY_ACCELERATOR_DST_AUX_REG, dst_addr);
	/* size */
	write_aux_reg(COPY_ACCELERATOR_SIZE_AUX_REG, size);
	/* command register */
	write_aux_reg(COPY_ACCELERATOR_CMD_AUX_REG, ACCELERATOR_CMD_TRIGGER);
}

void write_cluster_register(u32 reg, u32 sub_block, u32 y, u32 x, u32 value)
{
	u32 reg_addr;

	reg_addr = CONFIGURATION_BASE + (x << 20) + (y << 16) + (sub_block << 10) + (reg << 2);

	out_be32((void*)reg_addr, value);
}

void write_non_cluster_register(u32 block, u32 reg, u32 value)
{
	u32 reg_addr;

	reg_addr = CONFIGURATION_BASE + (block << 14) + (reg << 2);

	out_be32((void*)reg_addr, value);
}

unsigned int read_non_cluster_register(unsigned int block, unsigned int reg)
{
	unsigned int reg_addr;

	reg_addr = CONFIGURATION_BASE + (block << 14) + (reg << 2);

	return in_be32((void*)reg_addr);
}
