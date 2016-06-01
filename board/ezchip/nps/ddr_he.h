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

#ifndef _DDR_HE_H_
#define _DDR_HE_H_

#define DDR_PHY_BIST_RETRY_NUM	1000


#define INVALID_VAL		0xFFFFFFFF
#define CHUNK_SIZE_IN_BYTES			32
#define CHUNK_SIZE_IN_WORDS			8
#define DATA_QUEUES				16
#define	DDR_ADDRESS_SIZE			16
#define	NUMBER_OF_MC_IN_BLOCK			2
#define GET_NUMBER_OF_BANKS(__ddr_type)\
	((__ddr_type == DDR4_8BIT)?16:8)

#define   IS_16X_EXT_MEM_DEVICE(__ExtMemType)\
		((__ExtMemType == DDR4_16BIT) || (__ExtMemType == DDR3_16BIT))
#define   IS_DDR4_DEVICE(__ExtMemType)\
		((__ExtMemType == DDR4_8BIT) || (__ExtMemType == DDR4_16BIT))

#define		DDR_ROUND_UP(n, d)\
		((((n % d) > (d / 50)) || (d < 50))\
				? DIV_ROUND_UP(n, d):((n) / d))

#define GET_EMEM_MC_BLOCK_ADDR(__block)\
	((__block < (EMEM_MC_NUM_OF_BLOCKS / 2))?(EMEM_MC_0_ADDR + block):\
			(EMEM_MC_6_ADDR + (block % (EMEM_MC_NUM_OF_BLOCKS / 2))))


#define emem_mc_indirect_reg_write_apb_data0(block, address, data_0) \
		emem_mc_write_indirect_reg(block, address, 1, data_0, 0, 0x101)

#define emem_mc_indirect_reg_write_mrs(block, mc, data_0, data_1)\
	emem_mc_write_indirect_reg(block, 0, 2, data_0, data_1, (mc * 0x10) + 1)

#define EMEM_MC_IND_CMD_OP_WRITE				1
#define EMEM_MC_IND_CMD_OP_READ					0

#define NUM_OF_16BIT_DEVICES					2
#define NUM_OF_8BIT_DEVICES					4

#define INDIRECT_RETRY_NUM		1000

#define MR_DELAY			0x4
#define MR_CMD				0x22
#define ZQCL_MR_CMD			0x2E
#define ZQCL_MR_DATA			0x400
#define MR_CMD_OFFSET			21
#define MR_NUM_OFFSET			17
/* EMEM_MI block */
#define EMEM_MI_NUM_OF_BLOCKS		12
#define EMEM_MI_REG_IND_DATA0		0x0
#define EMEM_MI_REG_IND_CMD		0x10
#define EMEM_MI_REG_IND_ADDR		0x14
#define EMEM_MI_REG_IND_STS		0x15
#define EMEM_MI_REG_DEVICE_PROPERTIES	0x104
#define EMEM_MI_REG_DRAM_ECC		0x107
#define EMEM_MI_REG_CACHE_MI_IF		0x110
#define EMEM_MI_REG_CFG			0x131
#define EMEM_MI_REG_RD_SORT_THR_3	0x139
#define EMEM_MI_REG_RD_SORT_THR_7	0x13D
#define EMEM_MI_REG_RD_SORT_THR_8	0x13E
#define EMEM_MI_REG_WR_SORT_THR_3	0x145
#define EMEM_MI_REG_WR_SORT_DATA_THR_3	0x151
#define EMEM_MI_REG_WR_RD_REQ_SM	0x1C5
#define EMEM_MI_REG_RANK_OVERRIDE	0x1F9

/* EMEM_MC block */
#define EMEM_MC_NUM_OF_BLOCKS		12
#define EMEM_MC_NUM_OF_CONTROLLERS	(EMEM_MC_NUM_OF_BLOCKS * 2)
#define	NUM_OF_8BIT_DEVICES		4
#define	NUM_OF_16BIT_DEVICES		2

#define EMEM_MC_0_ADDR			0x618
#define EMEM_MC_1_ADDR			0x619
#define EMEM_MC_2_ADDR			0x61A
#define EMEM_MC_3_ADDR			0x61B
#define EMEM_MC_4_ADDR			0x61C
#define EMEM_MC_5_ADDR			0x61D
#define EMEM_MC_6_ADDR			0x718
#define EMEM_MC_7_ADDR			0x719
#define EMEM_MC_8_ADDR			0x71A
#define EMEM_MC_9_ADDR			0x71B
#define EMEM_MC_10_ADDR			0x71C
#define EMEM_MC_11_ADDR			0x71D

#define	IFC_BIST_CFG_ADDR		0x1A0
#define	IFC_BIST_BASE_ADDR		0x1A1
#define	IFC_BIST_HIGH_ADDR		0x1A2
#define	IFC_BIST_TIMING_PARAMS_ADDR	0x1A3
#define	IFC_BIST_MASK_0_ADDR		0x1A4
#define	IFC_BIST_MASK_1_ADDR		0x1A5
#define	IFC_BIST_EN_ADDR		0x1A6
#define	IFC_BIST_STAT_MUX_ADDR		0x1A7
#define	IFC_BIST_STATUS_ADDR		0x1A8
#define	IFC_BIST_ERR_ADDR		0x1A9
#define	IFC_BIST_ERR_CNTR_ADDR		0x1AA
#define	IFC_BIST_ERR_DATA_0_ADDR	0x1AB
#define	IFC_BIST_ERR_DATA_16_ADDR	0x1BB

#define EMEM_MC_PHY_INIT_RETRY		0x100000
#define EMEM_MC_REG_TIMING1		0x100
#define EMEM_MC_REG_TIMING2		0x101
#define EMEM_MC_REG_TIMING3		0x102
#define EMEM_MC_REG_RFRSH_PARAMS_1X	0x103
#define EMEM_MC_REG_RFRSH_PARAMS_2X	0x104
#define EMEM_MC_REG_RFRSH_PARAMS_4X	0x105
#define EMEM_MC_REG_CALIB_PARAMS	0x106
#define EMEM_MC_REG_DDR_PROPERTIES	0x10A
#define EMEM_MC_REG_APB_IFC		0x110
#define EMEM_MC_REG_MNG_CMD_PARAMS	0x113
#define EMEM_MC_REG_MC_DDR_IF		0x116
#define EMEM_MC_REG_MC_MI_IF		0x117
#define EMEM_MC_REG_MC_SYNC1		0x118
#define EMEM_MC_REG_MC_SYNC2		0x119
#define EMEM_MC_REG_EXT_MC_LATENCY	0x11B
#define EMEM_MC_REG_MRS_RFRSH_ACK	0x11C
#define EMEM_MC_MC_ADDR_MIRROR		0x11D
#define EMEM_MC_REG_PHY_CTRL		0x120
#define EMEM_MC_REG_PHY_UPDATE		0x121
#define EMEM_MC_REG_EZREF_CNTR		0x162
#define EMEM_MC_REG_EZREF_TIMING	0x163
#define EMEM_MC_REG_IND_DATA0		0x0
#define EMEM_MC_REG_IND_DATA1		0x1
#define EMEM_MC_REG_IND_CMD		0x10
#define EMEM_MC_REG_IND_ADDR		0x14
#define EMEM_MC_REG_IND_STS		0x15
#define EMEM_MC_REG_IND_STS_RDY		(1 << 0)
#define EMEM_MC_REG_IND_STS_SUCCESS	(1 << 1)
#define EMEM_MC_REG_IND_STS_PHY_RDY	(1 << 0)

#define EMEM_MC_IND_REG_MC_PUP_CTRL	0x0
#define EMEM_MC_IND_REG_PUB_CTRL	0x10
#define EMEM_MC_IND_REG_PUB_STATUS	0x11

#define IND_STS_REG_ADDR		0x15
#define IND_STS_RDY			(1 << 0)
#define IND_STS_SUCCESS			(1 << 1)

enum ddr_type {
	DDR3_8BIT,
	DDR3_16BIT,
	DDR4_8BIT,
	DDR4_16BIT,
};

enum ddr_speed_bin {
	DDR3_800D = 0,
	DDR3_800E,
	DDR3_1066E,
	DDR3_1066F,
	DDR3_1066G,
	DDR3_1333F,
	DDR3_1333G,
	DDR3_1333H,
	DDR3_1333J,
	DDR3_1600G,
	DDR3_1600H,
	DDR3_1600J,
	DDR3_1600K,
	DDR3_1866J,
	DDR3_1866K,
	DDR3_1866L,
	DDR3_1866M,
	DDR3_2133K,
	DDR3_2133L,
	DDR3_2133M,
	DDR3_2133N,
	NUM_OF_DDR3_SPEED_BIN,
	DDR4_1600J = 0,
	DDR4_1600K,
	DDR4_1600L,
	DDR4_1866L,
	DDR4_1866M,
	DDR4_1866N,
	DDR4_2133N,
	DDR4_2133P,
	DDR4_2133R,
	DDR4_2400P,
	DDR4_2400R,
	DDR4_2400T,
	DDR4_2400U,
	DDR4_2666T,
	DDR4_2666U,
	DDR4_2666V,
	DDR4_2666W,
	NUM_OF_DDR4_SPEED_BIN
};

enum ddr_pll_freq {
	DDR_400_MHz = 0,
	DDR_533_MHz,
	DDR_667_MHz,
	DDR_800_MHz,
	DDR_933_MHz,
	DDR_1066_MHz,
	DDR_1200_MHz,
	DDR_1333_MHz,
	DDR_1466_MHz,
	DDR_1600_MHz,
	NON_DEFAULT
};

#define NUM_OF_DEFAULTS_FREQ	NON_DEFAULT

enum rzq {
	DISABLED,
	RZQ_DIV_1,
	RZQ_DIV_2,
	RZQ_DIV_3,
	RZQ_DIV_4,
	RZQ_DIV_5,
	RZQ_DIV_6,
	RZQ_DIV_7,
	RZQ_DIV_8,
	RZQ_DIV_9,
	RZQ_DIV_10,
	RZQ_DIV_11,
	RZQ_DIV_12,
};

struct clk_psec {
	u32	ck_0;
	u32	ck_1;
};

struct ddr_params {
	enum	ddr_type	type;
	u32			size;	/* in MB */
	u32			active_devices;
	u32			clock_frequency;
	bool			training_bypass;
	enum	ddr_speed_bin	speed_bin;
	u32			tfaw;
	u32			clk_ref; /* MHz */
	u32			brtd[EMEM_MC_NUM_OF_CONTROLLERS];
	bool			address_mirror[EMEM_MC_NUM_OF_CONTROLLERS];
	u32			tcase;
	bool			asr_en;
	enum rzq		phy_wr_drv;
	enum rzq		phy_rd_odt;
	enum rzq		mem_dic;
	enum rzq		mem_rtt_nom;
	enum rzq		mem_rtt_wr;
	u32			mem_vref;
	u32			phy_rd_vref;
	struct clk_psec		clk[EMEM_MC_NUM_OF_BLOCKS];
	enum	ddr_pll_freq	pll_freq; /* derived from clock_frequency */
};

struct gpr_setup {
	u32 low_block;
	u32 high_block;
	u32 low_data_0;
	u32 low_data_1;
	u32 high_data_0;
	u32 high_data_1;
};


/*	registers	*/

union crg_emi_rst {
		u32 reg;
		struct {
			u32 core_keep_rst:1;
			u32 reserved23_30:8;
			u32 core_umi_div_rst_mask:1;
			u32 core_umi_ifc_rst_msk:1;
			u32 core_umi_cfg_rst_msk:1;
			u32 core_umi_phy_rst_msk:1;
			u32 core_rstiso_msk:1;
			u32 core_grst_msk:1;
			u32 core_srst_msk:1;
			u32 reserved7_15:9;
			u32 umi_div_rst_n:1;
			u32 umi_ifc_rst_n:1;
			u32 umi_cfg_rst_n:1;
			u32 umi_phy_rst_n:1;
			u32 rst_iso_n:1;
			u32 grst_n:1;
			u32 srst_n:1;
		} fields;
};

union  crg_ddr_x_pll_cntr {
	u32 reg;
	struct {
		u32 reserved31:1;
		u32 halt:1;
		u32 reserved14_29:16;
		u32 pll_lock_latched:1;
		u32 dll_lock_latched:1;
		u32 pll_lock:1;
		u32 reserved1_10:10;
		u32 nreset:1;
	} fields;
};

union  crg_ddr_x_pll_pllcfg {
	u32 reg;
	struct {
		u32 reserved21_31:11;
		u32 pll_out_divcnt:6;
		u32 prog_fbdiv255:8;
		u32 prog_fb_div4:1;
		u32 pll_refcnt:6;
	} fields;
};

union  emem_mc_ind_cmd {
	u32 reg;
	struct {
		u32 reserved14_31:18;
		u32 ifc_bist_mc_selector:2;
		u32 reserved11:1;
		u32 mem_id:3;
		u32 reserved5_7:3;
		u32 mc_id:1;
		u32 reserved1_3:3;
		u32 op:1;
	} fields;
};

union emem_mc_mc_ddr_if {
	u32 reg;
	struct {
		u32	ref_hot_update:1;
		u32	stat_cntr_en:1;
		u32	ezref_stat_cntr_en:1;
		u32	ezref_hot_update:1;
		u32	reserved24_27:4;
		u32	fast_ezref_en:2;
		u32	ezref_en:2;
		u32	reserved19:1;
		u32	ref_switch:1;
		u32	ref_en:2;
		u32	reserved9_15:7;
		u32	calib_en:1;
		u32	reserved6_7:2;
		u32	ppr_en:2;
		u32	reserved2_3:2;
		u32	mc_en:2;
	} fields;
};

union emem_mc_phy_ctrl {
	u32 reg;
	struct {
		u32	reserved5_31:27;
		u32	ddr_self_refresh:1;
		u32	reserved1_3:3;
		u32	ck_en:1;
	} fields;
};

union emem_mc_timing1 {
	u32 reg;
	struct {
		u32	wr_al:8;
		u32	rd_al:8;
		u32	rda_trp_gap:8;
		u32	wra_trp_gap:8;
	} fields;
};

union emem_mc_timing2 {
	u32 reg;
	struct {
		u32	reserved24_31:8;
		u32	rd_wr_gap:8;
		u32	wr_rd_gap_lo:8;
		u32	wr_rd_gap_sh:8;
	} fields;
};

union emem_mc_timing3 {
	u32 reg;
	struct {
		u32	dqs_wr_en:8;
		u32	dqs_rd_en:8;
		u32	trc:8;
		u32	tfaw:8;
	} fields;
};

union emem_mc_sync1 {
	u32 reg;
	struct {
		u32 tfaw_mult_en:1;
		u32 d_phase_en:1;
		u32 a_phase_en:1;
		u32 phase_skew_en:1;
		u32 d_phase_cfg_th:1;
		u32 a_phase_cfg_th:3;
		u32 reserved18_23:6;
		u32 d_phase_cfg:2;
		u32 reserved14_15:2;
		u32 ezref_phase_cfg:6;
		u32 reserved6_7:2;
		u32 a_phase_cfg:6;
	} fields;
};

union emem_mc_sync2 {
	u32 reg;
	struct {
		u32 reserved11_31:21;
		u32 use_ezref_phase:1;
		u32 ezref_phase:2;
		u32 reserved6_7:2;
		u32 mi_ack_phase:6;
	} fields;
};

union emem_mc_rfrsh_params_1x {
	u32	reg;
	struct {
		u32 reserved31:1;
		u32 mode:3;
		u32 rfrsh_burst:4;
		u32 trfc:9;
		u32 trefi:15;
	} fields;
};

union emem_mc_rfrsh_params_2x {
	u32	reg;
	struct {
		u32 reserved29_31:3;
		u32 rfrsh_burst:5;
		u32 trfc:9;
		u32 trefi:15;
	} fields;
};

union emem_mc_rfrsh_params_4x {
	u32	reg;
	struct {
		u32 reserved30_31:2;
		u32 rfrsh_burst:6;
		u32 trfc:9;
		u32 trefi:15;
	} fields;
};

union emem_mc_calib_params {
	u32	reg;
	struct {
		u32 debug_mode:1;
		u32 reserved26_30:5;
		u32 tzqcs:10;
		u32 reserved10_15:6;
		u32 th:10;
	} fields;
};

union emem_mc_ddr_properties {
	u32	reg;
	struct {
		u32 reserved16_31:16;
		u32 page_num:4;
		u32 reserved11:1;
		u32 gen:1;
		u32 dq_pins:2;
		u32 reserved7:1;
		u32 dram_size:3;
		u32 reserved2_3:2;
		u32 work_mode:2;
	} fields;
};

union emem_mc_mng_cmd_params {
	u32	reg;
	struct {
		u32 reserved20_31:12;
		u32 ras_cas_if:4;
		u32 reserved10_15:6;
		u32 master_slave:2;
		u32 cas_fifo_th:4;
		u32 mrs_fifo_th:4;
	} fields;
};

union emem_mc_phy_update {
	u32	reg;
	struct {
		u32 reserved16_31:16;
		u32 maxpw:4;
		u32 reserved0_11:12;
	} fields;
};

union emem_mc_mc_mi_if {
	u32	reg;
	struct {
		u32 mc_highest_bank:4;
		u32 cmd_switch_th:8;
		u32 swap_priority_on_prefare_long:1;
		u32 swap_priority_on_starve_for_short:1;
		u32 swap_priority_on_starve_for_long:1;
		u32 out_of_order:1;
		u32 bank_rdy_pre:4;
		u32 ret_data_gap:4;
		u32 chosen_mi:1;
		u32 ack_data_gap:3;
		u32 arb_algorithm:1;
		u32 ack_addr_gap:3;
	} fields;
};

union emem_mc_ext_mc_latency {
	u32	reg;
	struct {
		u32	reserved13_31:19;
		u32	ext_mc_rl1:5;
		u32	reserved5_7:3;
		u32	ext_mc_rl0:5;
	} fields;
};

union emem_mc_mrs_rfrsh_ack {
	u32	reg;
	struct {
		u32	reserved6_31:26;
		u32	phase1:3;
		u32	phase0:3;
	} fields;
};

union emem_mc_ezref_cntr {
	u32	reg;
	struct {
		u32	th2:16;
		u32	th1:16;
	} fields;
};

union emem_mc_ezref_timing {
	u32	reg;
	struct {
		u32	trefi:16;
		u32	sectors_debug_mode:1;
		u32	mask_bg_req:3;
		u32	tpre:4;
		u32	reserved5_7:3;
		u32	tras:5;
	} fields;
};

union emem_mc_apb_ifc {
	u32	reg;
	struct {
		u32	cont_clk_div_en:1;
		u32	cfg_clk_div:3;
		u32	cont_rl_cfg:4;
		u32	reserved2_23:22;
		u32	cont_rst:1;
		u32	cont_clk_en:1;
	} fields;
};

#define DDR_N_PLL_PLLCFG_ADDR		0x29C
#define DDR_S_PLL_PLLCFG_ADDR		0x29F
#define DDR_S_PLL_CNTR_ADDR		0x29E
#define DDR_N_PLL_CNTR_ADDR		0x29B

enum crg_emi_rst_phase {
	EMI_RST_PHASE_1,
	EMI_RST_PHASE_2,
	EMI_RST_PHASE_3,
};

void configure_emem(void);
void emem_mc_write_indirect_reg(unsigned int block_id, unsigned int address,
								unsigned int data_size, unsigned int data_0,
								unsigned int data_1, unsigned int cmd);
void configure_crg_emi_rst(enum crg_emi_rst_phase phase);

#endif /* _DDR_HE_H_ */
