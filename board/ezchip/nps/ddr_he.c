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

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include "common.h"
#include "ddr_he.h"
#include "nps.h"
#include "chip.h"

static void configure_emem_mi(void);

#define emem_mc_indirect_reg_write_apb_data0_data1(block, address,\
						data_0, data_1) \
			emem_mc_write_indirect_reg(block, address, 2,\
						data_0, data_1, 0x101)

#define emem_mc_indirect_reg_write_mc(block, address, data_0, data_1)	\
		do {							\
			emem_mc_write_indirect_reg(block, address, 2,	\
						data_0, data_1, 0x1);	\
			emem_mc_write_indirect_reg(block, address, 2,	\
						data_0, data_1, 0x11);	\
		} while (0)

static const int emem_mi_block_id[EMEM_MI_NUM_OF_BLOCKS] = {
	0x610, 0x611, 0x612, 0x613, 0x614, 0x615,
	0x710, 0x711, 0x712, 0x713, 0x714, 0x715
};

const int emem_mc_block_id[EMEM_MC_NUM_OF_BLOCKS] = {
	0x618, 0x619, 0x61a, 0x61b, 0x61c, 0x61d,
	0x718, 0x719, 0x71a, 0x71b, 0x71c, 0x71d
};

void configure_crg_emi_rst(enum crg_emi_rst_phase phase)
{
	int reg;
	unsigned int value, bitmask = 0;

	switch (phase) {
	case EMI_RST_PHASE_1:
		bitmask |= CRG_EMI_RST_GRST_N;
		bitmask |= CRG_EMI_RST_UMI_CFG_RST_N;
		break;
	case EMI_RST_PHASE_2:
		bitmask |= CRG_EMI_RST_UMI_PHY_RST_N;
		bitmask |= CRG_EMI_RST_UMI_IFC_RST_N;
		break;
	case EMI_RST_PHASE_3:
		bitmask |= CRG_EMI_RST_SRST_N;
		break;
	default:
		break;
	}

	for (reg = CRG_REG_EMI_RST_START; reg <= CRG_REG_EMI_RST_END ; reg++) {
		value = read_non_cluster_reg(CRG_BLOCK_ID, reg);
		value |= bitmask;
		write_non_cluster_reg(CRG_BLOCK_ID, reg, value);
	}
}

/* External Memory Interface */
static void configure_emem_mi(void)
{
	int i, block_id;
	if(get_debug())
		printf("configure_emem_mi\n");
	for (i = 0; i < EMEM_MI_NUM_OF_BLOCKS ; i++) {
		write_non_cluster_reg(emem_mi_block_id[i],
				EMEM_MI_REG_DRAM_ECC, 0x11);
	}

	for (i = 0; i < EMEM_MI_NUM_OF_BLOCKS ; i++) {
		write_non_cluster_reg(emem_mi_block_id[i],
				EMEM_MI_REG_RANK_OVERRIDE, 0xD);
	}

	for (i = 0; i < EMEM_MI_NUM_OF_BLOCKS; i++)
		write_non_cluster_reg(emem_mi_block_id[i],
					EMEM_MI_REG_CACHE_MI_IF, 0x33333);

	for (i = 0; i < EMEM_MI_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mi_block_id[i];

		write_non_cluster_reg(block_id,
				EMEM_MI_REG_CFG, 0x86B14143);
#ifdef CONFIG_NPS_EMEM_DDR4_8BIT
		write_non_cluster_reg(block_id,
				EMEM_MI_REG_DEVICE_PROPERTIES, 0x11);
#endif
		write_non_cluster_reg(block_id,
				EMEM_MI_REG_RD_SORT_THR_3, 0x6419);
		write_non_cluster_reg(block_id,
				EMEM_MI_REG_RD_SORT_THR_7, 0xC846);
		write_non_cluster_reg(block_id,
				EMEM_MI_REG_RD_SORT_THR_8, 0x6423);
		write_non_cluster_reg(block_id,
				EMEM_MI_REG_WR_SORT_THR_3, 0x6149);
		write_non_cluster_reg(block_id,
				EMEM_MI_REG_WR_SORT_DATA_THR_3, 0xD80064);
#if defined(CONFIG_NPS_EMEM_DDR4_8BIT) || defined(CONFIG_NPS_EMEM_DDR4_16BIT)
		write_non_cluster_reg(block_id,
				EMEM_MI_REG_WR_RD_REQ_SM, 0x8309E09E);
#endif
	}
}

static int check_indirect_status(unsigned int block_id)
{
	int i;
	unsigned int value;

	/* read indirect status until ready bit is on */
	for (i = 0 ; i < INDIRECT_RETRY_NUM; i++) {
		value = read_non_cluster_reg(block_id, IND_STS_REG_ADDR);
		if (value & IND_STS_RDY)
			break;
	}

	if (i == INDIRECT_RETRY_NUM) {
		error("indirect status retry number exceeded");
		return -EBUSY;
	}

	/* check if success bit is on */
	value = read_non_cluster_reg(block_id, IND_STS_REG_ADDR);
	if (!(value & IND_STS_SUCCESS)) {
		error("indirect status result was failure");
		return -EIO;
	}

	return 0;
}

void emem_mc_write_indirect_reg(unsigned int block_id,
		unsigned int address, unsigned int data_size,
		unsigned int data_0, unsigned int data_1,
		unsigned int cmd)
{
	int i;

	for (i = 0 ; i < data_size; i++) {
		switch (i) {
		case 0:
			write_non_cluster_reg(block_id,
					EMEM_MC_REG_IND_DATA0, data_0);
			break;
		case 1:
			write_non_cluster_reg(block_id,
					EMEM_MC_REG_IND_DATA1, data_1);
			break;
		default:
			error("invalid indirect data size");
			break;
		}
	}

	write_non_cluster_reg(block_id, EMEM_MC_REG_IND_ADDR, address);
	write_non_cluster_reg(block_id, EMEM_MC_REG_IND_CMD, cmd);
	if (check_indirect_status(block_id) < 0)
		error("indirect register=0x%x write failed on block=0x%x",
				address, block_id);
}

#if defined(CONFIG_NPS_EMEM_DDR4_8BIT) || defined(CONFIG_NPS_EMEM_DDR4_16BIT)
/* External Memory Controller DDR4 */
static void configure_emem_mc_ddr4(void)
{
	int i, block_id, try;
	unsigned int value;

	configure_crg_emi_rst(EMI_RST_PHASE_1);

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		write_non_cluster_reg(block_id,
				EMEM_MC_REG_EZREF_CNTR, 0x1020033);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_EZREF_TIMING, 0x1B331421);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_TIMING1, 0x1B331523);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_TIMING2, 0x50C09);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_TIMING3, 0x191B1C0C);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_RFRSH_PARAMS_1X, 0x014E1248);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_RFRSH_PARAMS_2X, 0x01300924);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_RFRSH_PARAMS_4X, 0x01210492);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_CALIB_PARAMS, 0x400000);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_DDR_PROPERTIES, 0x3530);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MNG_CMD_PARAMS, 0xA0178);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MC_DDR_IF, 0x0);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MC_MI_IF, 0xF7FF813C);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MC_SYNC1, 0x79021208);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MC_SYNC2, 0x208);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_EXT_MC_LATENCY, 0xD0D);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MRS_RFRSH_ACK, 0x8);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_PHY_UPDATE, 0x4801);
	}

	configure_crg_emi_rst(EMI_RST_PHASE_2);

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		write_non_cluster_reg(block_id,
				EMEM_MC_REG_APB_IFC, 0x4000002);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_APB_IFC, 0x84000002);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_APB_IFC, 0x84000003);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_APB_IFC, 0x84000001);
	}

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		emem_mc_indirect_reg_write_apb_data0(block_id,
				0x40, 0x40C);
		emem_mc_indirect_reg_write_apb_data0(block_id,
				0x5, 0x204C620);
		emem_mc_indirect_reg_write_apb_data0(block_id,
				0x14B, 0xFC00172);
		emem_mc_indirect_reg_write_apb_data0(block_id,
				0x80, 0x800021C3);
		emem_mc_indirect_reg_write_apb_data0(block_id,
				0x24, 0x64641A);
		emem_mc_indirect_reg_write_apb_data0(block_id,
				0x20, 0x38000);
		emem_mc_indirect_reg_write_apb_data0(block_id,
				0x10, 0x14008D0);
		emem_mc_indirect_reg_write_apb_data0(block_id,
				0x11, 0x4200030);
		emem_mc_indirect_reg_write_apb_data0(block_id,
				0x1, 0x33);

		/* Wait for Synopsys PHY initialization done */
		for (try = 0; try < EMEM_MC_PHY_INIT_RETRY; try++) {
			emem_mc_write_indirect_reg(block_id, 0x11,
						0, 0, 0, 0x300);
			value = read_non_cluster_reg(block_id,
						EMEM_MC_REG_IND_DATA0);
			if (value & EMEM_MC_REG_IND_STS_PHY_RDY)
				break;
		}

		if (try == EMEM_MC_PHY_INIT_RETRY)
			error("Failed to init EMEM_MC PHY");

		emem_mc_indirect_reg_write_apb_data0(block_id, 0x1, 0x40001);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x60, 0xA34);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x61, 0x9);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x62, 0x218);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x63, 0x0);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x64, 0x0);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x65, 0x0);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x66, 0x800);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x67, 0x0);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x44, 0x627100A);
		emem_mc_indirect_reg_write_apb_data0(block_id,
						0x45, 0x281A0300);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x46, 0x70200);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x47, 0x2000000);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x48, 0x1390C08);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x49, 0x37100A);
	}

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++)
		write_non_cluster_reg(emem_mc_block_id[i],
				EMEM_MC_REG_PHY_CTRL, 0x1);

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		emem_mc_indirect_reg_write_mc(block_id, 0x0, 0x4460000, 0x1004);
		emem_mc_indirect_reg_write_mc(block_id, 0x0, 0x44C0800, 0x1004);
		emem_mc_indirect_reg_write_mc(block_id, 0x0, 0x44A0000, 0x1004);
		emem_mc_indirect_reg_write_mc(block_id, 0x0, 0x4480000, 0x1004);
		emem_mc_indirect_reg_write_mc(block_id, 0x0, 0x4440218, 0x1004);
		emem_mc_indirect_reg_write_mc(block_id, 0x0, 0x4420009, 0x1004);
		emem_mc_indirect_reg_write_mc(block_id,
					0x0, 0x4400B34, 0x21004);
		emem_mc_indirect_reg_write_mc(block_id,
					0x0, 0x5C00400, 0x2100C);
	}

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		emem_mc_indirect_reg_write_apb_data0(block_id, 0x137, 0x0);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x1E2, 0x4D);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x1F0, 0x20002);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x222, 0x4D);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x230, 0x20002);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x137, 0x1);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x262, 0x4D);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x270, 0x20002);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x2A2, 0x4D);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x2B0, 0x20002);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x137, 0x0);
	}

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++)
		write_non_cluster_reg(emem_mc_block_id[i],
				EMEM_MC_REG_MC_DDR_IF, 0x30003);

	configure_crg_emi_rst(EMI_RST_PHASE_3);
}
#else
/* External Memory Controller DDR3 */
static void configure_emem_mc_ddr3(void)
{
	int i, block_id, try;
	unsigned int value;

	configure_crg_emi_rst(EMI_RST_PHASE_1);

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		write_non_cluster_reg(block_id,
				EMEM_MC_REG_EZREF_CNTR, 0x2040067);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_EZREF_TIMING, 0x2042718);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_TIMING1, 0x172F121D);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_TIMING2, 0x50B0B);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_TIMING3, 0x1517190C);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_RFRSH_PARAMS_1X, 0x1459040);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_RFRSH_PARAMS_2X, 0x1459040);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_RFRSH_PARAMS_4X, 0x1459040);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_CALIB_PARAMS, 0x2B0000);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_DDR_PROPERTIES, 0x4130);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MNG_CMD_PARAMS, 0xA0178);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MC_DDR_IF, 0x0);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MC_MI_IF, 0x77FF813C);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MC_SYNC1, 0x7A021208);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MC_SYNC2, 0x208);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_EXT_MC_LATENCY, 0xC0C);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_MRS_RFRSH_ACK, 0x8);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_PHY_UPDATE, 0x4801);
	}

	configure_crg_emi_rst(EMI_RST_PHASE_2);

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		write_non_cluster_reg(block_id,
				EMEM_MC_REG_APB_IFC, 0x4000002);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_APB_IFC, 0x84000002);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_APB_IFC, 0x84000003);
		write_non_cluster_reg(block_id,
				EMEM_MC_REG_APB_IFC, 0x84000001);
	}

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		emem_mc_indirect_reg_write_apb_data0(block_id,
						0x5, 0x204C620);
		emem_mc_indirect_reg_write_apb_data0(block_id,
						0x14B, 0xFC00172);
		emem_mc_indirect_reg_write_apb_data0(block_id,
						0x80, 0x80002183);
		emem_mc_indirect_reg_write_apb_data0(block_id,
						0x24, 0x64641A);
		emem_mc_indirect_reg_write_apb_data0(block_id,
						0x20, 0x38000);
		emem_mc_indirect_reg_write_apb_data0(block_id,
						0x10, 0x14008D0);
		emem_mc_indirect_reg_write_apb_data0(block_id,
						0x11, 0x4200030);
		emem_mc_indirect_reg_write_apb_data0(block_id,
						0x1, 0x33);

		/* Wait for Synopsys PHY initialization done */
		for (try = 0; try < EMEM_MC_PHY_INIT_RETRY; try++) {
			emem_mc_write_indirect_reg(block_id,
						0x11, 0, 0, 0, 0x300);
			value = read_non_cluster_reg(block_id,
					EMEM_MC_REG_IND_DATA0);
			if (value & EMEM_MC_REG_IND_STS_PHY_RDY)
				break;
		}

		if (try == EMEM_MC_PHY_INIT_RETRY)
			error("Failed to init EMEM_MC PHY");

		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x1, 0x40001);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x60, 0x24);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x61, 0x8);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x62, 0x2A8);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x63, 0x0);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x44, 0x6240E08);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x45, 0x281B0400);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x46, 0x60200);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x47, 0x2000000);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x48, 0x116081A);
		emem_mc_indirect_reg_write_apb_data0(block_id,
					0x49, 0x320E08);
	}

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++)
		write_non_cluster_reg(emem_mc_block_id[i],
				EMEM_MC_REG_PHY_CTRL, 0x1);

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		emem_mc_indirect_reg_write_mc(block_id,
					0x0, 0x44402A8, 0x1004);
		emem_mc_indirect_reg_write_mc(block_id,
					0x0, 0x4460000, 0x1004);
		emem_mc_indirect_reg_write_mc(block_id,
					0x0, 0x4420008, 0x1004);
		emem_mc_indirect_reg_write_mc(block_id,
					0x0, 0x4400124, 0x21004);
		emem_mc_indirect_reg_write_mc(block_id,
					0x0, 0x5C00400, 0x2100C);
	}

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++) {
		block_id = emem_mc_block_id[i];

		emem_mc_indirect_reg_write_apb_data0_data1(block_id,
						0x137, 0x0, 0x0);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x1E2, 0x52);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x1F0, 0x20002);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x222, 0x52);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x230, 0x20002);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x137, 0x1);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x262, 0x52);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x270, 0x20002);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x2A2, 0x52);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x2B0, 0x20002);
		emem_mc_indirect_reg_write_apb_data0(block_id, 0x137, 0x0);
	}

	for (i = 0; i < EMEM_MC_NUM_OF_BLOCKS ; i++)
		write_non_cluster_reg(emem_mc_block_id[i],
				EMEM_MC_REG_MC_DDR_IF, 0x30003);

	configure_crg_emi_rst(EMI_RST_PHASE_3);
}
#endif

void configure_emem(void)
{
#if defined(CONFIG_NPS_EMEM_DDR4_8BIT) || defined(CONFIG_NPS_EMEM_DDR4_16BIT)
	configure_emem_mc_ddr4();
#else
	configure_emem_mc_ddr3();
#endif
	configure_emem_mi();
}
