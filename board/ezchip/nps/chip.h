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

#ifndef _CHIP_H_
#define _CHIP_H_

/* CIU block */
#define CIU_WEST_BLOCK_ID		0x8
#define CIU_EAST_BLOCK_ID		0x28
#define CIU_REG_GLB_MSID_CFG_1_20	0xA4
#define CIU_REG_GLB_MSID_CFG_2_20	0xC4
#define CIU_GLB_MSID_CFG_1_MSID_L2	0xB2
#define CIU_GLB_MSID_CFG_1_MSID_OFFSET	24
#define CIU_GLB_MSID_CFG_1_SIZE_OFFSET	1
#define CIU_REG_GLB_MSID_CFG_2_VAL	0xC200000F

/* L2C block */
#define L2C_NUM_OF_BLOCKS		16
#define L2C_REG_DDR_TYPE		0x101
#define L2C_REG_L2CC_GLOBAL_CFG		0x208
#define L2C_REG_IND_DATA0		0x0
#define L2C_REG_IND_DATA1		0x1
#define L2C_REG_IND_DATA2		0x2
#define L2C_REG_IND_CMD			0x10
#define L2C_REG_IND_ADDR		0x14
#define L2C_REG_PERFEVT			0xE1

/* MTM block */
#define MTM_NUM_OF_BLOCKS		16
#define MTM_REG_CFG			0x81

/* CRG block */
#define CRG_BLOCK_ID			0x480
#define CRG_REG_SYS_PLL_PLLCFG	0x291
#define CRG_REG_BOOT_CFG		0x10e
#define CRG_REG_EMI_RST_START		0x12F
#define CRG_REG_EMI_RST_END		0x13A
#define CRG_EMI_RST_SRST_N		(1 << 0)
#define CRG_EMI_RST_GRST_N		(1 << 1)
#define CRG_EMI_RST_UMI_PHY_RST_N	(1 << 3)
#define CRG_EMI_RST_UMI_CFG_RST_N	(1 << 4)
#define CRG_EMI_RST_UMI_IFC_RST_N	(1 << 5)
#define CRG_REG_GEN_PURP_0		0x1BF
#define CRG_REG_GEN_PURP_1		0x1C0
#define CRG_REG_GEN_PURP_2		0x1C1
#define CRG_GEN_PURP_0_SYNC_BIT		(1 << 0)
#define CRG_REG_BIST_STATUS		0x28E

/* NPC_CRG block */
#define NPC_CRG_BLOCK_ID		0x38
#define NPC_CRG_REG_MTM_CLK_0		0x90

void configure_l2c(void);
void configure_mtm(void);
void configure_ciu(void);
void check_mbist_result(void);
unsigned long get_board_sys_clk(void);
int get_l2c_block_id(int block_id);

union crg_bist_status {
	u32 reg;
	struct {
		u32 reserved:30;
		u32 sfp_fail:1;
		u32 sfp_ready:1;
	} fields;
};

union ms_dfn_data_0 {
	u32 reg;
	struct {
		u32 size_lsb:12;
		u32 offset:20;
	} fields;
};

union ms_dfn_data_1 {
	u32 reg;
	struct {
		u32 data:24;
		u32 size_msb:8;
	} fields;
};

union boot_cfg {
	u32 value;
	struct {
		u32 reserved:17, cfg_btldr_addr:1,
		cfg_mem_rep_byp:1, cfg_spare:1, cfg_btldr_cfg:2,
		cfg_wdog_en:1, cfg_btldr_cpha:1, cfg_btldr_cpol:1,
		cfg_ref_clk_freq:1, cfg_tm_pll:3, cfg_sys_pll_cfg:3;
	} fields;
};

union sys_pll_pllcfg {
	u32 value;
	struct {
		u32 reserved:11, pll_out_divcnt:6,
		prog_fb_div255:8, prog_fb_div4:1,
		pll_refcnt:6;
	} fields;
};

#endif /* _CHIP_H_ */
