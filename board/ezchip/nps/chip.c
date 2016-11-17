/*
* Copyright (c) 2016, Mellanox Technologies. All rights reserved.
*
* This software is available to you under a choice of one of two
* licenses.  You may choose to be licensed under the terms of the GNU
* General Public License (GPL) Version 2, available from the file
* COPYING in the main directory of this source tree, or the
* OpenIB.org BSD license below:
*
*     Redistribution and use in source and binary forms, with or
*     without modification, are permitted provided that the following
*     conditions are met:
*
*      - Redistributions of source code must retain the above
*        copyright notice, this list of conditions and the following
*        disclaimer.
*
*      - Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following
*        disclaimer in the documentation and/or other materials
*        provided with the distribution.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "common.h"
#include "chip.h"

static const int l2c_block_id[L2C_NUM_OF_BLOCKS] = {
	0x600, 0x601, 0x602, 0x603, 0x604, 0x605, 0x606, 0x607,
	0x700, 0x701, 0x702, 0x703, 0x704, 0x705, 0x706, 0x707
};

static const int mtm_block_id[MTM_NUM_OF_BLOCKS] = {
	0x0, 0x1, 0x2, 0x3, 0x10, 0x11, 0x12, 0x13,
	0x20, 0x21, 0x22, 0x23, 0x30, 0x31, 0x32, 0x33
};

int get_l2c_block_id(int block_id)
{
	return l2c_block_id[block_id];
}

/* Level 2 Cache */
void configure_l2c(void)
{
	int i, block_id;

	for (i = 0; i < L2C_NUM_OF_BLOCKS ; i++) {
		block_id = l2c_block_id[i];

		write_non_cluster_reg(block_id, L2C_REG_IND_DATA0, 0x10001);
		write_non_cluster_reg(block_id, L2C_REG_IND_ADDR, 0x32);
		write_non_cluster_reg(block_id, L2C_REG_IND_CMD, 0x109);

		write_non_cluster_reg(block_id, L2C_REG_IND_DATA0, 0x202);
		write_non_cluster_reg(block_id, L2C_REG_IND_ADDR, 0x32);
		write_non_cluster_reg(block_id, L2C_REG_IND_CMD, 0x209);

		write_non_cluster_reg(block_id, L2C_REG_IND_DATA0, 0xC);
		write_non_cluster_reg(block_id, L2C_REG_IND_ADDR, 0x0);
		write_non_cluster_reg(block_id, L2C_REG_IND_CMD, 0x309);

		write_non_cluster_reg(block_id, L2C_REG_IND_DATA0, 0x76543210);
		write_non_cluster_reg(block_id, L2C_REG_IND_ADDR, 0x10);
		write_non_cluster_reg(block_id, L2C_REG_IND_CMD, 0x309);

		write_non_cluster_reg(block_id, L2C_REG_IND_DATA0, 0xBA98);
		write_non_cluster_reg(block_id, L2C_REG_IND_ADDR, 0x20);
		write_non_cluster_reg(block_id, L2C_REG_IND_CMD, 0x309);
#ifdef CONFIG_TARGET_NPS_SIM
		write_non_cluster_reg(block_id, L2C_REG_DDR_TYPE, 0x1);

		write_non_cluster_reg(block_id, L2C_REG_IND_DATA0, 0x0E000000);
		write_non_cluster_reg(block_id, L2C_REG_IND_DATA1, 0x33BABA03);
		write_non_cluster_reg(block_id, L2C_REG_IND_DATA2, 0x1);
		write_non_cluster_reg(block_id, L2C_REG_IND_ADDR, 0x32);
		write_non_cluster_reg(block_id, L2C_REG_IND_CMD, 0x2);
#endif
		write_non_cluster_reg(block_id, L2C_REG_L2CC_GLOBAL_CFG, 0x101);
		write_non_cluster_reg(block_id, L2C_REG_PERFEVT, 0x2F000000);
	}
}

/* Multi Thread Manager */
void configure_mtm(void)
{
	int x, y, i;

	for (x = 0; x < NUM_OF_CLUSTER_PER_COL_ROW; x++)
		for (y = 0; y < NUM_OF_CLUSTER_PER_COL_ROW; y++)
			for (i = 0; i < MTM_NUM_OF_BLOCKS ; i++) {
				/* enable core mtm_clk for all cores except 0 */
				if ((x + y + i) > 0)
					write_cluster_reg(x, y,
							  NPC_CRG_BLOCK_ID,
							  NPC_CRG_REG_MTM_CLK_0 + i,
							  0x1);

				/* enable mtm address scrambling */
				write_cluster_reg(x, y,
					mtm_block_id[i], MTM_REG_CFG, 0x1);
			}
}

/* Cluster Interface Unit */
void configure_ciu(void)
{
	int x, y;
	unsigned int msid_8k_size;
	unsigned int glb_msid_cfg_1;
	unsigned int glb_msid_cfg_2 = CIU_REG_GLB_MSID_CFG_2_VAL;

	/* address space in 8KB granularity */
	msid_8k_size = (CONFIG_NPS_KERNEL_MSID_SIZE / (8 * 1024));

	glb_msid_cfg_1 = (msid_8k_size << CIU_GLB_MSID_CFG_1_SIZE_OFFSET);
	glb_msid_cfg_1 |= (CIU_GLB_MSID_CFG_1_MSID_L2 <<
				CIU_GLB_MSID_CFG_1_MSID_OFFSET);

	/* configure CIU on all clusters */
	for (x = 0; x < NUM_OF_CLUSTER_PER_COL_ROW; x++)
		for (y = 0; y < NUM_OF_CLUSTER_PER_COL_ROW; y++) {
			/* CIU West block */
			write_cluster_reg(x, y,
				CIU_WEST_BLOCK_ID,
				CIU_REG_GLB_MSID_CFG_1_20,
				glb_msid_cfg_1);
			write_cluster_reg(x, y,
				CIU_WEST_BLOCK_ID,
				CIU_REG_GLB_MSID_CFG_2_20,
				glb_msid_cfg_2);

			/* CIU East block */
			write_cluster_reg(x, y,
				CIU_EAST_BLOCK_ID,
				CIU_REG_GLB_MSID_CFG_1_20,
				glb_msid_cfg_1);
			write_cluster_reg(x, y,
				CIU_EAST_BLOCK_ID,
				CIU_REG_GLB_MSID_CFG_2_20,
				glb_msid_cfg_2);
		}
}

/* Mbist is automatically activated by HW at powerup
 * this function checks the result and print an error
 * message in case of failure. */

void check_mbist_result(void)
{
	u32 	retries = 25, i;
	union	crg_bist_status	bist_status;
	union	crg_gen_purp_0 gen_purp_0;

	bist_status.reg = 0;
	bist_status.reg = read_non_cluster_reg(CRG_BLOCK_ID, CRG_REG_BIST_STATUS);

	for (i = 0 ; i < retries; i++) {
		if (bist_status.fields.sfp_ready == 1) {
			if (bist_status.fields.sfp_fail == 1) {
		                gen_purp_0.reg = read_non_cluster_reg(CRG_BLOCK_ID, CRG_REG_GEN_PURP_0);
                		gen_purp_0.fields.mbist_failed = 1;
		                write_non_cluster_reg(CRG_BLOCK_ID, CRG_REG_GEN_PURP_0, gen_purp_0.reg);
				error("Error: Internal memory bist failed");
			}
			break;
		}
		udelay(100);
	}
	if (i == retries) {
		gen_purp_0.reg = read_non_cluster_reg(CRG_BLOCK_ID, CRG_REG_GEN_PURP_0);
		gen_purp_0.fields.mbist_not_finished = 1;
        	write_non_cluster_reg(CRG_BLOCK_ID, CRG_REG_GEN_PURP_0, gen_purp_0.reg);
		error("Error: Internal memory bist did not end after timeout");
	}
}
