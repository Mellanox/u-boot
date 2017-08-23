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

#ifndef _DDR_H_
#define _DDR_H_

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
#define	  IS_DDR3_LOW_GROUP(__speed_bin, __type)\
		((!IS_DDR4_DEVICE(__type)) &&\
		((__speed_bin == DDR3_800D)  || (__speed_bin == DDR3_800E)  ||\
		 (__speed_bin == DDR3_1066E) || (__speed_bin == DDR3_1066F) ||\
		 (__speed_bin == DDR3_1066G) || (__speed_bin == DDR3_1333F) ||\
		 (__speed_bin == DDR3_1333G) || (__speed_bin == DDR3_1333H) ||\
		 (__speed_bin == DDR3_1333J)))

#define	  IS_DDR4_LOW_GROUP(__speed_bin, __type)\
		((IS_DDR4_DEVICE(__type)) &&\
		((__speed_bin == DDR4_1600J) || (__speed_bin == DDR4_1600K) ||\
		 (__speed_bin == DDR4_1600L) || (__speed_bin == DDR4_1866L) ||\
		 (__speed_bin == DDR4_1866M) || (__speed_bin == DDR4_1866N) ||\
		 (__speed_bin == DDR4_2133N) || (__speed_bin == DDR4_2133P) ||\
		 (__speed_bin == DDR4_2133R)))


#define	  IS_DDR4_1600_GROUP(__speed_bin)\
		((__speed_bin == DDR4_1600J) || (__speed_bin == DDR4_1600K) ||\
		 (__speed_bin == DDR4_1600L))

#define	  IS_DDR4_1866_GROUP(__speed_bin)\
		((__speed_bin == DDR4_1866L) || (__speed_bin == DDR4_1866M) ||\
		 (__speed_bin == DDR4_1866N))

#define	  IS_DDR4_2133_GROUP(__speed_bin)\
		((__speed_bin == DDR4_2133N) || (__speed_bin == DDR4_2133P) ||\
		 (__speed_bin == DDR4_2133R))

#define	  IS_DDR4_2400_GROUP(__speed_bin)\
		((__speed_bin == DDR4_2400P) || (__speed_bin == DDR4_2400R) ||\
		 (__speed_bin == DDR4_2400T) || (__speed_bin == DDR4_2400U))

#define	  IS_DDR4_2666_GROUP(__speed_bin)\
		((__speed_bin == DDR4_2666T) || (__speed_bin == DDR4_2666U) ||\
		 (__speed_bin == DDR4_2666V) || (__speed_bin == DDR4_2666W))

#define		DDR_ROUND_UP(n, d)\
		((((n % d) > (d / 50)) || (d < 50))\
				? DIV_ROUND_UP(n, d):((n) / d))

#define	  GET_MRS_DATA_0(__num, __data)\
		((MR_CMD << MR_CMD_OFFSET) + (__num << MR_NUM_OFFSET) + __data)


#define	  GET_MRS_DATA_1(__last)\
		((__last << MR_NUM_OFFSET) + 0x1000 + MR_DELAY)

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
#define EMEM_MI_NUM_OF_BLOCKS				12
#define EMEM_MI_REG_IND_DATA0				0x0
#define EMEM_MI_REG_IND_CMD				0x10
#define EMEM_MI_REG_IND_ADDR				0x14
#define EMEM_MI_REG_IND_STS				0x15
#define EMEM_MI_REG_DEVICE_PROPERTIES			0x104
#define EMEM_MI_REG_DRAM_ECC				0x107
#define EMEM_MI_REG_CACHE_MI_IF				0x110
#define EMEM_MI_REG_CFG					0x131
#define EMEM_MI_REG_RD_SORT_THR_3			0x139
#define EMEM_MI_REG_RD_SORT_THR_7			0x13D
#define EMEM_MI_REG_RD_SORT_THR_8			0x13E
#define EMEM_MI_REG_WR_SORT_THR_3			0x145
#define EMEM_MI_REG_WR_SORT_DATA_THR_3			0x151
#define EMEM_MI_REG_AVG_LAT_CALC			0x1AF
#define EMEM_MI_REG_SM_RD_WR_WIN			0x1C2
#define EMEM_MI_REG_RD_REQ_SM	                        0x1C4
#define EMEM_MI_REG_WR_RD_REQ_SM			0x1C5
#define EMEM_MI_REG_RD_WR_SM_STARVATION_PREVENTION	0x1C9
#define EMEM_MI_REG_RANK_OVERRIDE			0x1F9
#define EMEM_MI_REG_RDWR_SM_PROFILE_0			0x240
#define EMEM_MI_REG_RDWR_SM_PROFILE_1			0x241
#define EMEM_MI_REG_RDWR_SM_PROFILE_2			0x242
#define EMEM_MI_REG_RDWR_SM_PROFILE_3			0x243
#define EMEM_MI_REG_RDWR_SM_PROFILE_4			0x244
#define EMEM_MI_REG_RDWR_SM_PROFILE_5			0x245
#define EMEM_MI_REG_RDWR_SM_PROFILE_6			0x246
#define EMEM_MI_REG_RDWR_SM_PROFILE_7			0x247

/* EMEM_MC block */
#define EMEM_MC_NUM_OF_BLOCKS		12
#define EMEM_MC_NUM_OF_CONTROLLERS	(EMEM_MC_NUM_OF_BLOCKS * 2)

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

#define MC_MI_IF_INIT_VAL 0xF7FF113C

#define IND_STS_REG_ADDR		0x15
#define IND_STS_RDY			(1 << 0)
#define IND_STS_SUCCESS			(1 << 1)

#define	PUB_RIDR_REG_ADDR		0x0
#define	PUB_PIR_REG_ADDR		0x1
#define	PUB_PGCR0_REG_ADDR		0x4
#define	PUB_PGCR1_REG_ADDR		0x5
#define	PUB_PGCR2_REG_ADDR		0x6
#define	PUB_PGCR3_REG_ADDR		0x7
#define	PUB_PGCR4_REG_ADDR		0x8
#define	PUB_PGCR5_REG_ADDR		0x9
#define	PUB_PGCR6_REG_ADDR		0xA
#define	PUB_PGCR7_REG_ADDR		0xB
#define	PUB_PGSR0_REG_ADDR		0xC
#define	PUB_PGSR1_REG_ADDR		0xD
#define	PUB_PTR0_REG_ADDR		0x10
#define	PUB_PTR1_REG_ADDR		0x11
#define	PUB_PTR2_REG_ADDR		0x12
#define	PUB_PTR3_REG_ADDR		0x13
#define	PUB_PTR4_REG_ADDR		0x14
#define	PUB_PLLCR_REG_ADDR		0x20
#define	PUB_DXCCR_REG_ADDR		0x22
#define	PUB_DSGCR_REG_ADDR		0x24
#define	PUB_ODTCR_REG_ADDR		0x26
#define	PUB_AACR_REG_ADDR		0x28
#define	PUB_GPR0_REG_ADDR		0x30
#define	PUB_GPR1_REG_ADDR		0x31
#define	PUB_DCR_REG_ADDR		0x40
#define	PUB_DTPR0_REG_ADDR		0x44
#define	PUB_DTPR1_REG_ADDR		0x45
#define	PUB_DTPR2_REG_ADDR		0x46
#define	PUB_DTPR3_REG_ADDR		0x47
#define	PUB_DTPR4_REG_ADDR		0x48
#define	PUB_DTPR5_REG_ADDR		0x49
#define	PUB_DTPR6_REG_ADDR		0x4A
#define	PUB_SCHCR0_REG_ADDR		0x5A
#define	PUB_SCHCR1_REG_ADDR		0x5B
#define	PUB_MR0_REG_ADDR		0x60
#define	PUB_MR1_REG_ADDR		0x61
#define	PUB_MR2_REG_ADDR		0x62
#define	PUB_MR3_REG_ADDR		0x63
#define	PUB_MR4_REG_ADDR		0x64
#define	PUB_MR5_REG_ADDR		0x65
#define	PUB_MR6_REG_ADDR		0x66
#define	PUB_DTCR0_REG_ADDR		0x80
#define PUB_BISTRR_REG_ADDR		0x100
#define PUB_BISTWCR_REG_ADDR		0x101
#define PUB_BISTLSR_REG_ADDR		0x105
#define PUB_BISTAR1_REG_ADDR		0x107
#define PUB_BISTAR2_REG_ADDR		0x108
#define PUB_BISTAR4_REG_ADDR		0x10A
#define PUB_BISTUDPR_REG_ADDR		0x10B
#define PUB_BISTGSR_REG_ADDR		0x10C
#define PUB_RANKIDR_REG_ADDR		0x137
#define PUB_ACBDLR0_REG_ADDR		0x150
#define PUB_ACMDLR0_REG_ADDR		0x168
#define PUB_ACMDLR1_REG_ADDR		0x168
#define PUB_IOVCR0_REG_ADDR		0x148
#define	PUB_VTCR0_REG_ADDR		0x14A
#define	PUB_VTCR1_REG_ADDR		0x14B
#define	PUB_ZQCR_REG_ADDR		0x1A0
#define	PUB_ZQ0PR_REG_ADDR		0x1A1
#define	PUB_ZQ1PR_REG_ADDR		0x1A5

#define	PUB_DX0GCR0_REG_ADDR		0x1C0

#define	PUB_DX0GCR4_REG_ADDR		0x1C4
#define	PUB_DX1GCR4_REG_ADDR		0x204
#define	PUB_DX2GCR4_REG_ADDR		0x244
#define	PUB_DX3GCR4_REG_ADDR		0x284

#define PUB_BISTRR_REG_HW_DEF_VAL (0xF << 19)

#define	PUB_DX0GCR5_REG_ADDR		0x1C5
#define	PUB_DX1GCR5_REG_ADDR		0x205
#define	PUB_DX2GCR5_REG_ADDR		0x245
#define	PUB_DX3GCR5_REG_ADDR		0x285

#define	PUB_DX0GCR6_REG_ADDR		0x1C6
#define	PUB_DX1GCR6_REG_ADDR		0x206
#define	PUB_DX2GCR6_REG_ADDR		0x246
#define	PUB_DX3GCR6_REG_ADDR		0x286

#define PUB_DX0BDLR2_REG_ADDR		0x1D2

#define	PUB_DX0LCDLR0_REG_ADDR		0x1E0
#define	PUB_DX1LCDLR0_REG_ADDR		0x220
#define	PUB_DX2LCDLR0_REG_ADDR		0x260
#define	PUB_DX3LCDLR0_REG_ADDR		0x2A0

#define	PUB_DX0LCDLR1_REG_ADDR		0x1E1
#define	PUB_DX1LCDLR1_REG_ADDR		0x221
#define	PUB_DX2LCDLR1_REG_ADDR		0x261
#define	PUB_DX3LCDLR1_REG_ADDR		0x2A1

#define	PUB_DX0LCDLR2_REG_ADDR		0x1E2
#define	PUB_DX1LCDLR2_REG_ADDR		0x222
#define	PUB_DX2LCDLR2_REG_ADDR		0x262
#define	PUB_DX3LCDLR2_REG_ADDR		0x2A2

#define	PUB_DX0LCDLR3_REG_ADDR		0x1E3
#define	PUB_DX1LCDLR3_REG_ADDR		0x223
#define	PUB_DX2LCDLR3_REG_ADDR		0x263
#define	PUB_DX3LCDLR3_REG_ADDR		0x2A3

#define	PUB_DX0LCDLR4_REG_ADDR		0x1E4
#define	PUB_DX1LCDLR4_REG_ADDR		0x224
#define	PUB_DX2LCDLR4_REG_ADDR		0x264
#define	PUB_DX3LCDLR4_REG_ADDR		0x2A4

#define	PUB_DX0LCDLR5_REG_ADDR		0x1E5
#define	PUB_DX1LCDLR5_REG_ADDR		0x225
#define	PUB_DX2LCDLR5_REG_ADDR		0x265
#define	PUB_DX3LCDLR5_REG_ADDR		0x2A5

#define	PUB_DX0BDLR0_REG_ADDR		0x1D0
#define	PUB_DX1BDLR0_REG_ADDR		0x210
#define	PUB_DX2BDLR0_REG_ADDR		0x250
#define	PUB_DX3BDLR0_REG_ADDR		0x290

#define	PUB_DX0BDLR1_REG_ADDR		0x1D1
#define	PUB_DX1BDLR1_REG_ADDR		0x211
#define	PUB_DX2BDLR1_REG_ADDR		0x251
#define	PUB_DX3BDLR1_REG_ADDR		0x291

#define	PUB_DX0BDLR3_REG_ADDR		0x1D4
#define	PUB_DX1BDLR3_REG_ADDR		0x214
#define	PUB_DX2BDLR3_REG_ADDR		0x254
#define	PUB_DX3BDLR3_REG_ADDR		0x294

#define	PUB_DX0BDLR4_REG_ADDR		0x1D5
#define	PUB_DX1BDLR4_REG_ADDR		0x215
#define	PUB_DX2BDLR4_REG_ADDR		0x255
#define	PUB_DX3BDLR4_REG_ADDR		0x295


#define	PUB_DX0MDLR0_REG_ADDR		0x1E8
#define	PUB_DX1MDLR0_REG_ADDR		0x228
#define	PUB_DX2MDLR0_REG_ADDR		0x268
#define	PUB_DX3MDLR0_REG_ADDR		0x2A8

#define PUB_DX0MDLR1_REG_ADDR		0x1E9

#define PUB_DX0GTR0_REG_ADDR		0x1F0
#define PUB_DX1GTR0_REG_ADDR		0x230
#define PUB_DX2GTR0_REG_ADDR		0x270
#define PUB_DX3GTR0_REG_ADDR		0x2B0

#define	PUB_BISTMSKR1_REG_ADDR		0x103
#define	PUB_VTDR_REG_ADDR		0x08F


/* set of ddr errors that uboot raises to the cp */
 enum ddr_errors {
	NO_ERROR,
	PLL_LOCK_ERROR,
	PUB_CONECTION_ERROR,
	PHY_INIT_FAILED,
	TAP_DELAY_0,
	DDR_TRAINING_FAILED,
	POST_DDR_TRAINING_FAILED,
	BIST_FAILED,
	VREF_TRAINING_FAILED,
};

#define BYTE_LANE_0_OR_1_PUB_REG(__reg)\
	((__reg == PUB_DX0GTR0_REG_ADDR) || (__reg == PUB_DX1GTR0_REG_ADDR) ||\
	(__reg == PUB_DX0LCDLR0_REG_ADDR) || (__reg == PUB_DX1LCDLR0_REG_ADDR) ||\
	(__reg == PUB_DX0LCDLR1_REG_ADDR) || (__reg == PUB_DX1LCDLR1_REG_ADDR) ||\
	(__reg == PUB_DX0LCDLR2_REG_ADDR) || (__reg == PUB_DX1LCDLR2_REG_ADDR) ||\
	(__reg == PUB_DX0LCDLR3_REG_ADDR) || (__reg == PUB_DX1LCDLR3_REG_ADDR) ||\
	(__reg == PUB_DX0LCDLR4_REG_ADDR) || (__reg == PUB_DX1LCDLR4_REG_ADDR) ||\
	(__reg == PUB_DX0LCDLR5_REG_ADDR) || (__reg == PUB_DX1LCDLR5_REG_ADDR))

#define BYTE_LANE_2_OR_3_PUB_REG(__reg)\
	((__reg == PUB_DX2GTR0_REG_ADDR) || (__reg == PUB_DX3GTR0_REG_ADDR) ||\
	(__reg == PUB_DX2LCDLR0_REG_ADDR) || (__reg == PUB_DX3LCDLR0_REG_ADDR) ||\
	(__reg == PUB_DX2LCDLR1_REG_ADDR) || (__reg == PUB_DX3LCDLR1_REG_ADDR) ||\
	(__reg == PUB_DX2LCDLR2_REG_ADDR) || (__reg == PUB_DX3LCDLR2_REG_ADDR) ||\
	(__reg == PUB_DX2LCDLR3_REG_ADDR) || (__reg == PUB_DX3LCDLR3_REG_ADDR) ||\
	(__reg == PUB_DX2LCDLR4_REG_ADDR) || (__reg == PUB_DX3LCDLR4_REG_ADDR) ||\
	(__reg == PUB_DX2LCDLR5_REG_ADDR) || (__reg == PUB_DX3LCDLR5_REG_ADDR))

#define MEM_VREF_MAX_VAL		1200

struct pub_regs {
	u32	address;
	u32	init_val;
};

struct pub_regs_synop {
	u32	address;
	u32	val;
};

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
	DDR_966_MHz,	/* ND */
	DDR_1000_MHz,	/* ND */
	DDR_1033_MHz,	/* ND */
	DDR_1066_MHz,
	DDR_1100_MHz,	/* ND */
	DDR_1133_MHz,	/* ND */
	DDR_1166_MHz,	/* ND */
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
	bool			ddr_double_refresh; /* doubled refresh rates */
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

union pub_pir {
	u32	reg;
	struct {
		u32	reserved31:1;

	} fields;
};

union pub_pgcr0 {
	u32	reg;
	struct {
		u32 adcp:1;
		u32 x40scwddl:2;
		u32 x40scwdl:2;
		u32 phyfrst:1;
		u32 oscacdl:2;
		u32 oscwddl:2;
		u32 reserved21:1;
		u32 oscwdl:2;
		u32 dtosel:5;
		u32 acwlpon:1;
		u32 ocsdiv:4;
		u32 oscen:1;
		u32 dltst:1;
		u32 dltmode:1;
		u32 clrtstat:1;
		u32 clrperr:1;
		u32 initfsmbyp:1;
		u32 pllfsmbyp:1;
		u32 clrzcal:1;
		u32 icpc:1;
	} fields;
};

union pub_int_lb_port_status {
	u32	reg;
	struct {
		u32 reserved7_31:25;
		u32 scnd_err:4;
		u32 scnd_timeout:1;
		u32 frst_err:1;
		u32 frst_timeout:1;
	} fields;
};

union pub_pgcr1 {
	u32	reg;
	struct {
		u32	lbmode:1;
		u32	lbgdqs:2;
		u32	lbdqss:1;
		u32	iolb:1;
		u32	lyldtm:1;
		u32	phyhrst:1;
		u32	acvldtrn:1;
		u32	acvlddly:3;
		u32	res19_20:2;
		u32	updmstrc0:1;
		u32	lpmstrc0:1;
		u32	acpddc:1;
		u32	dual_chn:1;
		u32	fdepth:2;
		u32	lpfdepth:2;
		u32	lpfen:1;
		u32	mdlen:1;
		u32	ioddram:2;
		u32	pub_mode:1;
		u32	ddlbyp_mode:2;
		u32	wluncrt:1;
		u32	wlstep:1;
		u32	wlmode:1;
		u32	dtomode:1;
	} fields;
};

union pub_pgcr6 {
	u32	reg;
	struct {
		u32 reserved3:8;
		u32 dldlmt:8;
		u32 reserved2:2;
		u32 acdlvt:1;
		u32 acbvt:1;
		u32 odtbvt:1;
		u32 ckebvt:1;
		u32 csnbvt:1;
		u32 ckbvt:1;
		u32 reserved:6;
		u32	fvt:1;
		u32	inhvt:1;
	} fields;
};

union pub_pgcr7 {
	u32	reg;
	struct {
		u32 reserved31:1;
		u32 dxrsvd:6;
		u32 dxrclkmd:1;
		u32 dxqsgsel:1;
		u32 dxdtosel:2;
		u32 dxgsmd:1;
		u32 dxddlldt:1;
		u32 dxqsdbyp:1;
		u32 dxgbyp:1;
		u32 dxtmode:1;
		u32 reserved8_15:8;
		u32 acrsvd:3;
		u32 acrclkmd:1;
		u32 acdldt:1;
		u32 acrsvd2:1;
		u32 acdtosel:1;
		u32 actmode:1;
	} fields;
};

union pub_dx_x_lcdlr1 {
	u32	reg;
	struct {
		u32 reserved2:7;
		u32 x4wdqd:9;
		u32 reserved:7;
		u32	wdqd:9;
	} fields;
};


union pub_dx_x_lcdlr3 {
	u32	reg;
	struct {
		u32 reserved2:7;
		u32 x4rdqsd:9;
		u32 reserved:7;
		u32 rdqsd:9;
	} fields;
};

union pub_dx_x_lcdlr4 {
	u32	reg;
	struct {
		u32 reserved2:7;
		u32 x4rdqsnd:9;
		u32 reserved:7;
		u32 rdqsnd:9;
	} fields;
};

union pub_dx_x_lcdlr5 {
	u32	reg;
	struct {
		u32 reserved25_31:7;
		u32 x4dqsgsd:9;
		u32 reserved9_15:7;
		u32 dqsgsd:9;
	} fields;
};

union pub_dx_x_bdlr0 {
	u32	reg;
	struct {
		u32 reserved:2;
		u32 dq3wbd:6;
		u32 reserved2:2;
		u32 dq2wbd:6;
		u32 reserved3:2;
		u32 dq1wbd:6;
		u32 reserved4:2;
		u32 dq0wbd:6;
	} fields;
};

union pub_dx_x_bdlr1 {
	u32	reg;
	struct {
		u32 reserved:2;
		u32 dq7wbd:6;
		u32 reserved2:2;
		u32 dq6wbd:6;
		u32 reserved3:2;
		u32 dq5wbd:6;
		u32 reserved4:2;
		u32 dq4wbd:6;
	} fields;
};

union pub_dx_x_bdlr3 {
	u32	reg;
	struct {
		u32 reserved:2;
		u32 dq3rbd:6;
		u32 reserved2:2;
		u32 dq2rbd:6;
		u32 reserved3:2;
		u32 dq1rbd:6;
		u32 reserved4:2;
		u32 dq0rbd:6;
	} fields;
};

union pub_dx_x_bdlr4 {
	u32	reg;
	struct {
		u32 reserved:2;
		u32 dq7rbd:6;
		u32 reserved2:2;
		u32 dq6rbd:6;
		u32 reserved3:2;
		u32 dq5rbd:6;
		u32 reserved4:2;
		u32 dq4rbd:6;
	} fields;
};




union pub_dx_x_mdlr0 {
	u32	reg;
	struct {
		u32 reserved2:7;
		u32 tprd:9;
		u32 reserved:7;
		u32	iprd:9;
	} fields;
};


union pub_dx_x_mdlr1 {
	u32	reg;
	struct {
		u32 reserved9_31:23;
		u32 mdld:9;
	} fields;
};


union pub_bistmskr1 {
	u32	reg;
	struct {
		u32 dmmsk:4;
		u32 reserved25_27:3;
		u32 parinmsk:1;
		u32 odtmsk:8;
		u32 ckemsk:8;
		u32 bamsk:4;
		u32	x4dmmsk:4;
	} fields;
};



union pub_vtdr {
	u32	reg;
	struct {
		u32 reserved30_31:2;
		u32 hvrefmx:6;
		u32 reserved22_23:2;
		u32 hvrefmn:6;
		u32 reserved14_15:2;
		u32 dvrefmx:6;
		u32 reserved6_7:2;
		u32 dvrefmn:6;
	} fields;
};

union pub_dxccr {
	u32	reg;
	struct {
		u32 x4mode:1;
		u32 x4dqsmd:1;
		u32 rkloop:1;
		u32 reserved24_28:5;
		u32 dxdccbyp:1;
		u32 qscnten:1;
		u32 udqiom:1;
		u32 reserved18_20:3;
		u32 msbudq:3;
		u32 dxsr:2;
		u32 dqsnres:4;
		u32 dqsres:4;
		u32 dqsglb:2;
		u32 mdlen:1;
		u32 dxiom:1;
		u32 dxodt:1;
	} fields;
};

union pub_pllcr {
	u32	reg;
	struct {
		u32	pll_byp:1;
		u32	pll_rst:1;
		u32	pll_pd:1;
		u32	reserved25_28:4;
		u32	lockps:1;
		u32	lockcs:1;
		u32	lockds:1;
		u32	rgvint:1;
		u32	frq_sel:2;
		u32	rg_shift:1;
		u32	cppc:4;
		u32	cpic:2;
		u32	g_shift:1;
		u32	atoen:4;
		u32	atc:4;
		u32	dtc:3;
	} fields;
};

union pub_dsgcr {
	u32	reg;
	struct {
		u32	reserved24_31:8;
		u32	phy_zuen:1;
		u32	rr_mode:1;
		u32	rstoe:1;
		u32	sdr_mode:2;
		u32	wrr_mode:1;
		u32	atoae:1;
		u32	dtooe:1;
		u32	dtoiom:1;
		u32	dtopdr:1;
		u32	dtopdd:1;
		u32	dtoodt:1;
		u32	paud:4;
		u32	dqsgx:2;
		u32	cuaen:1;
		u32	lp_pll_pd:1;
		u32	lp_iopd:1;
		u32	ctlzuen:1;
		u32	bdi_sen:1;
		u32	puren:1;
	} fields;
};

union pub_dcr {
	u32	reg;
	struct {
		u32	reserved31:1;
		u32	ubg:1;
		u32	udimm:1;
		u32	ddr2t:1;
		u32	nosra:1;
		u32	reserved18_26:9;
		u32	byte_mask:8;
		u32	ddr_type:2;
		u32	mprdq:1;
		u32	pdq:3;
		u32	ddr8bnk:1;
		u32	ddrmd:3;
	} fields;
};

union pub_dtcr0 {
	u32	reg;
	struct {
		u32	rfshdt:4;
		u32	reserved26_27:2;
		u32	dtdrs:2;
		u32	dtexg:1;
		u32	dtexd:1;
		u32	dtdstp:1;
		u32	dtden:1;
		u32	dtdbs:4;
		u32	dtrdbitr:2;
		u32	dtdbc:1;
		u32	dtwbddm:1;
		u32	rfshent:4;
		u32	dtcmpd:1;
		u32	dtmpr:1;
		u32	reserved4_5:2;
		u32	dtrptn:4;
	} fields;
};

union pub_vtcr0 {
	u32	reg;
	struct {
		u32 tvref:3;
		u32 dven:1;
		u32 pdaen:1;
		u32 reserved22_26:5;
		u32 dvss:4;
		u32 dvmax:6;
		u32 dvmin:6;
		u32 dvinit:6;
	} fields;
};

union pub_vtcr1 {
	u32	reg;
	struct {
		u32	hvss:4;
		u32	hv_max:6;
		u32	hv_min:6;
		u32	vwcr:4;
		u32	reserved11:1;
		u32	shrnk:2;
		u32	shren:1;
		u32	tvrefio:3;
		u32	eoff:2;
		u32	e_num:1;
		u32	hven:1;
		u32	hvio:1;
	} fields;
};

union pub_zqcr {
	u32	reg;
	struct {
		u32	zctrl_upper:4;
		u32	force_zcat_vt_updat:1;
		u32	dis_mon_lin_comp:1;
		u32	pu_odt_only:1;
		u32	asym_drv_en:1;
		u32	iodlmt:7;
		u32	avg_en:1;
		u32	avg_max:2;
		u32	zcalt:3;
		u32	pg_wait:3;
		u32	reserved3_7:5;
		u32	zqpd:1;
		u32	term_off:1;
		u32	reserved0:1;
	} fields;
};

union pub_zqxpr {
	u32	reg;
	struct {
		u32	reserved28_31:4;
		u32	zctrl_upper:4;
		u32	pd_drv_adjust:2;
		u32	pu_drv_adjust:2;
		u32	zprog_pu_odt_only:4;
		u32	zprog_asym_drv_pd:4;
		u32	zprog_asym_drv_pu:4;
		u32	zqdiv:8;
	} fields;
};

union pub_dtpr0 {
	u32	reg;
	struct {
		u32	reserved28_31:4;
		u32	trrd:4;
		u32	reserved23:1;
		u32	tras:7;
		u32	reserved15:1;
		u32	trp:7;
		u32	reserved4_7:4;
		u32	trtp:4;
	} fields;
};

union pub_dtpr1 {
	u32	reg;
	struct {
		u32	reserved30_31:2;
		u32	twlmrd:6;
		u32	reserved22_23:2;
		u32	tfaw:6;
		u32	reserved11_15:5;
		u32	tmod:3;
		u32	reserved2_7:6;
		u32	tmrd:2;
	} fields;
};

union pub_dtpr2 {
	u32	reg;
	struct {
		u32	reserved29_31:3;
		u32	trtw:1;
		u32	reserved25_27:3;
		u32	trtodt:1;
		u32	reserved20_23:4;
		u32	tcke:4;
		u32	reserved10_15:6;
		u32	txs:10;
	} fields;
};

union pub_dtpr3 {
	u32	reg;
	struct {
		u32	tofdx:3;
		u32	tccd:3;
		u32	tdllk:10;
		u32	reserved11_15:5;
		u32	tdqsck_max:3;
		u32	reserved3_7:5;
		u32	tdqsck:3;
	} fields;
};

union pub_dtpr4 {
	u32	reg;
	struct {
		u32	reserved26_31:6;
		u32	trfc:10;
		u32	reserved12_15:4;
		u32	twlo:4;
		u32	reserved5_7:3;
		u32	txp:5;
	} fields;
};

union pub_dtpr5 {
	u32	reg;
	struct {
		u32	reserved24_31:8;
		u32	trc:8;
		u32	reserved15:1;
		u32	trcd:7;
		u32	reserved5_7:3;
		u32	twtr:5;
	} fields;
};

union pub_dtpr6 {
	u32 reg;
	struct {
		u32	pubwlen:1;
		u32	pubrlen:1;
		u32	reserved14_29:16;
		u32	pubwl:6;
		u32	reserved6_7:2;
		u32	pubrl:6;
	} fields;
};


union pub_mr0_ddr3 {
	u32 reg;
	struct {
		u32 reserved13_31:19;
		u32 ppd:1;
		u32 wr:3;
		u32 dll_reset:1;
		u32 tm:1;
		u32 cl4_6:3;
		u32 rbt:1;
		u32 cl2:1;
		u32 bl:2;
	} fields;
};

union pub_mr0_ddr4 {
	u32	reg;
	struct {
		u32	reserved14_31:18;
		u32	wr13:1;
		u32	cl12:1;
		u32	wr9_11:3;
		u32	dll_reset:1;
		u32	tm:1;
		u32	cl4_6:3;
		u32	rbt:1;
		u32	cl2:1;
		u32	bl:2;
	} fields;
};

union pub_mr1_ddr3 {
	u32	reg;
	struct {
		u32	reserved13_31:19;
		u32	qoff:1;
		u32	tdqs:1;
		u32	reserved10:1;
		u32	rtt_nom_9:1;
		u32	reserved8:1;
		u32	level:1;
		u32	rtt_nom_6:1;
		u32	dic_5:1;
		u32	al:2;
		u32	rtt_nom_2:1;
		u32	dic_1:1;
		u32	dll:1;
	} fields;
};

union pub_mr1_ddr4 {
	u32	reg;
	struct {
		u32	reserved11_31:21;
		u32	rtt_nom:3;
		u32	wr_level:1;
		u32	rfu:2;
		u32	al:2;
		u32	odic:2;
		u32	dll_en:1;
	} fields;
};

union pub_mr2_ddr3 {
	u32	reg;
	struct {
		u32	reserved11_31:21;
		u32	rtt_wr:2;
		u32	reserved8:1;
		u32	srt:1;
		u32	asr:1;
		u32	cwl:3;
		u32	pasr:3;
	} fields;
};

union pub_mr2_ddr4 {
	u32	reg;
	struct {
		u32 reserved12_31:20;
		u32 rtt_wr:3;
		u32 rfu_8:1;
		u32 lpasr:2;
		u32 cwl:3;
		u32 rfu_0:3;
	} fields;
};

union pub_mr6 {
	u32	reg;
	struct {
		u32 reserved13_31:19;
		u32 tccd_l:3;
		u32 rfu:2;
		u32 vrefdq_tr_en:1;
		u32 vrefdq_tr_rng:1;
		u32 vrefdq_tr_val:6;
	} fields;
};

union pub_acmdlr0 {
	u32	reg;
	struct {
		u32	reserved25_31:7;
		u32	tprd:9;
		u32	reserved9_15:7;
		u32	iprd:9;
	} fields;
};


union pub_acmdlr1 {
	u32	reg;
	struct {
		u32 reserved9_31:23;
		u32 mdld:9;
	} fields;
};

union pub_pgsr0 {
	u32 reg;
	struct {
		u32 aplock:1;
		u32 srderr:1;
		u32 cawrn:1;
		u32 caerr:1;
		u32 weerr:1;
		u32 reerr:1;
		u32 wderr:1;
		u32 rderr:1;
		u32 wlaerr:1;
		u32 qsgerr:1;
		u32 wlerr:1;
		u32 zcerr:1;
		u32 verr:1;
		u32 reserved15_18:4;
		u32 vdone:1;
		u32 srddone:1;
		u32 cadone:1;
		u32 wedone:1;
		u32 redone:1;
		u32 wddone:1;
		u32 rddone:1;
		u32 wladone:1;
		u32 qsgdone:1;
		u32 wldone:1;
		u32 didone:1;
		u32 zcdone:1;
		u32 dcdone:1;
		u32 pldone:1;
		u32 idone:1;
	} fields;
};

union pub_acbdlr0 {
	u32	reg;
	struct {
		u32 reserved30_31:2;
		u32 ck3bd:6;
		u32 reserved22_23:2;
		u32 ck2bd:6;
		u32 reserved14_15:2;
		u32 ck1bd:6;
		u32 reserved6_7:2;
		u32 ck0bd:6;
	} fields;
};

union pub_bistgsr {
	u32	reg;
	struct {
		u32 reserved30_31:2;
		u32 rasber:2;
		u32 dmber:8;
		u32 x4dmber:9;
		u32 bdxerr:9;
		u32 bacerr:1;
		u32 bdone:1;
	} fields;
};

union pub_bistrr {
	u32	reg;
	struct {
		u32 reserved27_31:5;
		u32 bccsel:2;
		u32 bcksel:2;
		u32 bdxsel:4;
		u32 bdpat:2;
		u32 bdmen:1;
		u32 bacen:1;
		u32 bdxen:1;
		u32 bsonf:1;
		u32 nfail:8;
		u32 binf:1;
		u32 bmode:1;
		u32 binst:3;

	} fields;
};

union pub_bistar1 {
	u32 reg;
	struct {
		u32 reserved20_31:12;
		u32 bmrank:4;
		u32 bainc:12;
		u32 brank:4;
	} fields;
};

union pub_bistar2 {
	u32 reg;
	struct {
		u32 bmbank:4;
		u32 reserved12_27:16;
		u32 bmcol:12;
	} fields;
};

union pub_iovcr0 {
	u32 reg;
	struct {
		u32 acrefiom:3;
		u32 acrefpen:1;
		u32 acrefeen:2;
		u32 acrefsen:1;
		u32 acrefien:1;
		u32 reserved22_23:2;
		u32 acvrefesel:6;
		u32 reserved14_15:2;
		u32 acvrefssel:6;
		u32 reserved6_7:2;
		u32 acvrefisel:6;
	} fields;
};

union pub_dx_n_bdlr2 {
	u32 reg;
	struct {
		u32 reserved22_31:10;
		u32 dsoebd:6;
		u32 reserved0_15:16;
	} fields;
};

union pub_dx_n_gcr4 {
	u32 reg;
	struct {
		u32 dxrefiom:3;
		u32 dxrefpen:1;
		u32 dxrefeen:2;
		u32 dxrefsen:1;
		u32 reserved22_24:3;
		u32 dxrefesel:6;
		u32 reserved14_15:2;
		u32 dxrefssel:6;
		u32 reserved6_7:2;
		u32 dxrefien:4;
		u32 dxrefimon:2;
	} fields;
};

union pub_dx_n_gcr0 {
	u32 reg;
	struct {
		u32 reserved1_31:31;
		u32 dxen:1;
	} fields;
};

union pub_dx_n_gcr5 {
	u32 reg;
	struct {
		u32 reserved30_31:2;
		u32 dxrefiselr3:6;
		u32 reserved22_23:2;
		u32 dxrefiselr2:6;
		u32 reserved14_15:2;
		u32 dxrefiselr1:6;
		u32 reserved6_7:2;
		u32 dxrefiselr0:6;
	} fields;
};

union pub_dx_x_gcr6 {
	u32 reg;
	struct {
		u32 reserved30_31:2;
		u32 dxdqvrefr3:6;
		u32 reserved22_23:2;
		u32 dxdqvrefr2:6;
		u32 reserved14_15:2;
		u32 dxdqvrefr1:6;
		u32 reserved6_7:2;
		u32 dxdqvrefr0:6;
	} fields;
};

union pub_rankidr {
	u32 reg;
	struct {
		u32 reserved20_31:12;
		u32 rankrid:4;
		u32 reserved4_15:12;
		u32 rankwid:4;
	} fields;
};

union pub_schcr0 {
	u32 reg;
	struct {
		u32 reserved25_31:7;
		u32 schdqv:9;
		u32 reserved12_15:4;
		u32 sp_cmd:4;
		u32 cmd:4;
		u32 schtrig:4;
	} fields;
};

union pub_schcr1 {
	u32 reg;
	struct {
		u32 scrnk:4;
		u32 scaddr:20;
		u32 scbg:2;
		u32 scbk:2;
		u32 reserved3:1;
		u32 allrank:1;
		u32 reserved0_1:2;
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

int get_vref_index(const u32 *table, u32 size, u32 input);
int configure_emem(void);
void emem_mc_write_indirect_reg(unsigned int block_id, unsigned int address,
								unsigned int data_size, unsigned int data_0,
								unsigned int data_1, unsigned int cmd);
int emem_mc_indirect_reg_read_synop(u32 block, u32 address);
void emem_mc_indirect_reg_write_synop_data0(u32 block, u32 address, u32 data_0);
int read_sdram_chunk(u32 mc_bitmask, u32 bg, u32 b, u32 row, u32 col, u32 size,
								u32 *data);
int write_sdram_chunk(u32 mc_bitmask, u32 bg, u32 b, u32 row, u32 col, u32 size,
								u32 *data);
int get_max_row_num(void);
int post_ddr_training(u32 size);
void configure_crg_emi_rst(enum crg_emi_rst_phase phase);
int set_ddr_freq(void);
int apb_ifc_rst(void);
void configure_phy(void);
int ddr_training(void);
int phy_init(void);
void set_pir_val(u32 pir_val);
#ifndef CONFIG_NPS_DDR_DEBUG
#define init_ddr_phy_record_DB()
#else
extern void init_ddr_phy_record_DB(void);
int do_print_ddr_config(cmd_tbl_t *cmdtp, int flag, int argc,
							char * const argv[]);
int do_ddr_pause(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_pup_fail_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif

#endif /* _DDR_H_ */
