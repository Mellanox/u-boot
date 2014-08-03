/*
*	Copyright (C) 2015 EZchip, Inc. (www.ezchip.com)
*
* 	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License version 2 as
*	published by the Free Software Foundation.
*/

#ifndef _COMMON_H_
#define _COMMON_H_

/* CTOP Aux registers */
#define AUX_REGS_BASE					0xFFFFF800
#define COPY_ACCELERATOR_DST_AUX_REG	(AUX_REGS_BASE + 0xA4)
#define COPY_ACCELERATOR_SRC_AUX_REG	(AUX_REGS_BASE + 0xA8)
#define COPY_ACCELERATOR_SIZE_AUX_REG	(AUX_REGS_BASE + 0xAC)
#define COPY_ACCELERATOR_CMD_AUX_REG	(AUX_REGS_BASE + 0xB0)

#define ACCELERATOR_CMD_TRIGGER		1


void flash_to_mem(unsigned int src_addr, unsigned int dst_addr, unsigned int size);

#define CONFIGURATION_BASE			0xF6000000

#define NON_CLUSTER_REG_ADDR(BLOCK,REG) (CONFIGURATION_BASE + (BLOCK << 14) + (REG << 2))

void write_cluster_register(unsigned int reg, unsigned int sub_block, unsigned int y, unsigned int x, unsigned int value);
void write_non_cluster_register(unsigned int block, unsigned int reg, unsigned int value);
unsigned int read_non_cluster_register(unsigned int block, unsigned int reg);

#endif /* _COMMON_H_ */
