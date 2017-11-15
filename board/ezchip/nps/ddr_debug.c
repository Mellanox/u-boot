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

#include <console.h>
#include "common.h"
#include "ddr_debug.h"
#include "ddr.h"
#include "chip.h"


#define BYTE_LANE_LEN		8
#define BYTE_LANES_NUM		4
#define RESULT_FIELDS_NUM	4
#define BIT_ENTRIES_NUM		32

#define TR_VAL			0
#define RIGHT_EDGE		1
#define LEFT_EDGE		2
#define BL_TAP_PS		3

#define WARNING_RET_VAL		0x00000005

#define D2_TAP_DELTA		10

#define D2_VREF_TR_VAL 0
#define D2_WDQD_TR_VAL 1
#define D2_BL_IPRD 2
#define D2_BL_TPRD 3
#define D2_BL_TAP_PS 4
#define D2_W_RESULT_FIELDS_NUM 5

#define D2_VREF_MAX_VAL 73

#define EYE_DEBUG_BL_INDEX 2
#define EYE_DEBUG_VREF_INDEX 30
#define EYE_DEBUG_DLL_INDEX 10
#define D2_R_VREF_TR_VAL 0
#define D2_R_RDQSD_TR_VAL 1
#define D2_R_RDQSND_TR_VAL 2
#define D2_R_BL_IPRD 3
#define D2_R_BL_TPRD 4
#define D2_R_BL_TAP_PS 5
#define D2_R_RESULT_FIELDS_NUM	6
#define D2_R_VREF_MAX_VAL 63

static int g_temp_try = 0;
static int g_temp_try_2d = 0;


static void show_write_centralization_margin(int block);
static void show_read_centralization_margin(int block);
static void show_write_skew_margin(int block, bool print_enable);
static void show_read_skew_margin(int block, bool print_enable);
static void set_bdlr_bd_field(int block, int bl_index, int bit_index, int nibble, int bd_field);
static int get_bdlr_bd_field(int block, int bl_index, int bit_index, int nibble);
static int phy_bist_run(u32 mc_mask, u32 *results);
static void phy_bist_setup(u32 mc_mask);

static void d2_write_eye_plotting(int block, bool print_enable);
static void d2_read_eye_plotting(int block, bool print_enable);

extern u32 get_optimal_vref_index(const u32 *table_1, const u32 *table_2,
		u32 size_1, u32 size_2, u32 vref_prec, u32 *index);
extern const int emem_mc_block_id[EMEM_MC_NUM_OF_BLOCKS];
extern const u32 phy_rd_vref_dq[64];
extern struct ddr_params  current_ddr_params;
extern const u32 tCK2[NUM_OF_DEFAULTS_FREQ];
extern const u32 vref_dq_2[51];
extern const u32 vref_dq_1[51];
extern union pub_mr6 mr6;
extern int g_EZsim_mode;
extern union pub_mr6 mr6_rank_vref[EMEM_MC_NUM_OF_BLOCKS][2];

static u32 res_rbd[BIT_ENTRIES_NUM][RESULT_FIELDS_NUM];
static u32 res_wbd[BIT_ENTRIES_NUM][RESULT_FIELDS_NUM];
static u32 vref_read_max[BYTE_LANES_NUM];
static u32 vref_read_min[BYTE_LANES_NUM];
static u32 vref_write_max[BYTE_LANES_NUM];
static u32 vref_write_min[BYTE_LANES_NUM];
static u32 left_margin[BYTE_LANES_NUM];
static u32 right_margin[BYTE_LANES_NUM];
static u32 res_vrtrdqsd[BYTE_LANES_NUM][D2_R_RESULT_FIELDS_NUM];
static u32 res_w_tr_val[BYTE_LANES_NUM];
static u32 res_vrtwdqd[BYTE_LANES_NUM][D2_W_RESULT_FIELDS_NUM];

static pub_ddr_phy_port_record ddr_phy_port_record_init = {
		{{ "RIDR"  , 0x000 , 0x0 },
		{ "PIR"   , 0x001 , 0x0 },
		{ "PGCR0" , 0x004 , 0x0 },
		{ "PGCR1" , 0x005 , 0x0 },
		{ "PGCR2" , 0x006 , 0x0 },
		{ "PGCR3" , 0x007 , 0x0 },
		{ "PGCR4" , 0x008 , 0x0 },
		{ "PGCR5" , 0x009 , 0x0 },
		{ "PGCR6" , 0x00A , 0x0 },
		{ "PGCR7" , 0x00B , 0x0 },
		{ "PGSR0" , 0x00C , 0x0 },
		{ "PGSR1" , 0x00D , 0x0 },
		{ "PTR0"  , 0x010 , 0x0 },
		{ "PTR1"  , 0x011 , 0x0 },
		{ "PTR2"  , 0x012 , 0x0 },
		{ "PTR3"  , 0x013 , 0x0 },
		{ "PTR4"  , 0x014 , 0x0 },
		{ "PLLCR" , 0x020 , 0x0 },
		{ "DXCCR" , 0x022 , 0x0 },
		{ "DSGCR" , 0x024 , 0x0 },
		{ "ODTCR" , 0x026 , 0x0 },
		{ "AACR"  , 0x028 , 0x0 },
		{ "GPR0"  , 0x030 , 0x0 },
		{ "GPR1"  , 0x031 , 0x0 },
		{ "DCR"   , 0x040 , 0x0 },
		{ "DTPR0" , 0x044 , 0x0 },
		{ "DTPR1" , 0x045 , 0x0 },
		{ "DTPR2" , 0x046 , 0x0 },
		{ "DTPR3" , 0x047 , 0x0 },
		{ "DTPR4" , 0x048 , 0x0 },
		{ "DTPR5" , 0x049 , 0x0 },
		{ "DTPR6" , 0x04A , 0x0 },
		{ "SCHCR0", 0x05A , 0x0 },
		{ "SCHCR1", 0x05B , 0x0 },
		{ "MR0"       , 0x060 , 0x0 },
		{ "MR1"       , 0x061 , 0x0 },
		{ "MR2"       , 0x062 , 0x0 },
		{ "MR3"       , 0x063 , 0x0 },
		{ "MR4"       , 0x064 , 0x0 },
		{ "MR5"       , 0x065 , 0x0 },
		{ "MR6"       , 0x066 , 0x0 },
		{ "MR7"       , 0x067 , 0x0 },
		{ "DTCR0"     , 0x080 , 0x0 },
		{ "DTCR1"     , 0x081 , 0x0 },
		{ "DTAR0"     , 0x082 , 0x0 },
		{ "DTAR1"     , 0x083 , 0x0 },
		{ "DTAR2"     , 0x084 , 0x0 },
		{ "DTDR0"     , 0x086 , 0x0 },
		{ "DTDR1"     , 0x087 , 0x0 },
		{ "DTEDR0"    , 0x08C , 0x0 },
		{ "DTEDR1"    , 0x08D , 0x0 },
		{ "VTDR"      , 0x08F , 0x0 },
		{ "DQSDR0"    , 0x094 , 0x0 },
		{ "DQSDR1"    , 0x095 , 0x0 },
		{ "DQSDR2"    , 0x096 , 0x0 },
		{ "BISTRR"    , 0x100 , 0x0 },
		{ "BISTWCR"   , 0x101 , 0x0 },
		{ "BISTMSKR0" , 0x102 , 0x0 },
		{ "BISTMSKR1" , 0x103 , 0x0 },
		{ "BISTMSKR2" , 0x104 , 0x0 },
		{ "BISTLSR"   , 0x105 , 0x0 },
		{ "BISTAR0"   , 0x106 , 0x0 },
		{ "BISTAR1"   , 0x107 , 0x0 },
		{ "BISTAR2"   , 0x108 , 0x0 },
		{ "BISTAR3"   , 0x109 , 0x0 },
		{ "BISTAR4"   , 0x10A , 0x0 },
		{ "BISTUDPR"  , 0x10B , 0x0 },
		{ "BISTGSR"   , 0x10C , 0x0 },
		{ "BISTWER0"  , 0x10D , 0x0 },
		{ "BISTWER1"  , 0x10E , 0x0 },
		{ "BISTBER0"  , 0x10F , 0x0 },
		{ "BISTBER1"  , 0x110 , 0x0 },
		{ "BISTBER2"  , 0x111 , 0x0 },
		{ "BISTBER3"  , 0x112 , 0x0 },
		{ "BISTBER4"  , 0x113 , 0x0 },
		{ "BISTWCSR"  , 0x114 , 0x0 },
		{ "BISTFWER0" , 0x115 , 0x0 },
		{ "BISTFWER1" , 0x116 , 0x0 },
		{ "BISTFWER2" , 0x117 , 0x0 },
		{ "BISTBER5"  , 0x118 , 0x0 },
		{ "RANKIDR"   , 0x137 , 0x0 },
		{ "RIOCR0"    , 0x138 , 0x0 },
		{ "RIOCR1"    , 0x139 , 0x0 },
		{ "RIOCR2"    , 0x13A , 0x0 },
		{ "RIOCR3"    , 0x13B , 0x0 },
		{ "RIOCR4"    , 0x13C , 0x0 },
		{ "RIOCR5"    , 0x13D , 0x0 },
		{ "ACIOCR0"   , 0x140 , 0x0 },
		{ "ACIOCR1"   , 0x141 , 0x0 },
		{ "ACIOCR2"   , 0x142 , 0x0 },
		{ "ACIOCR3"   , 0x143 , 0x0 },
		{ "ACIOCR4"   , 0x144 , 0x0 },
		{ "ACIOCR5"   , 0x145 , 0x0 },
		{ "IOVCR0"    , 0x148 , 0x0 },
		{ "IOVCR1"    , 0x149 , 0x0 },
		{ "VTCR0"     , 0x14A , 0x0 },
		{ "VTCR1"     , 0x14B , 0x0 },
		{ "ACBDLR0"   , 0x150 , 0x0 },
		{ "ACBDLR1"   , 0x151 , 0x0 },
		{ "ACBDLR2"   , 0x152 , 0x0 },
		{ "ACBDLR3"   , 0x153 , 0x0 },
		{ "ACBDLR4"   , 0x154 , 0x0 },
		{ "ACBDLR5"   , 0x155 , 0x0 },
		{ "ACBDLR6"   , 0x156 , 0x0 },
		{ "ACBDLR7"   , 0x157 , 0x0 },
		{ "ACBDLR8"   , 0x158 , 0x0 },
		{ "ACBDLR9"   , 0x159 , 0x0 },
		{ "ACBDLR10"  , 0x15A , 0x0 },
		{ "ACBDLR11"  , 0x15B , 0x0 },
		{ "ACBDLR12"  , 0x15C , 0x0 },
		{ "ACBDLR13"  , 0x15D , 0x0 },
		{ "ACBDLR14"  , 0x15E , 0x0 },
		{ "ACLCDLR"   , 0x160 , 0x0 },
		{ "ACMDLR0"   , 0x168 , 0x0 },
		{ "ACMDLR1"   , 0x169 , 0x0 },
		{ "ZQCR"      , 0x1A0 , 0x0 },
		{ "ZQ0PR"     , 0x1A1 , 0x0 },
		{ "ZQ0DR"     , 0x1A2 , 0x0 },
		{ "ZQ0SR"     , 0x1A3 , 0x0 },
		{ "ZQ1PR"     , 0x1A5 , 0x0 },
		{ "ZQ1DR"     , 0x1A6 , 0x0 },
		{ "ZQ1SR"     , 0x1A7 , 0x0 },
		{ "DX0GCR0"   , 0x1C0 , 0x0 },
		{ "DX0GCR1"   , 0x1C1 , 0x0 },
		{ "DX0GCR2"   , 0x1C2 , 0x0 },
		{ "DX0GCR3"   , 0x1C3 , 0x0 },
		{ "DX0GCR4"   , 0x1C4 , 0x0 },
		{ "DX0GCR5"   , 0x1C5 , 0x0 },
		{ "DX0GCR6"   , 0x1C6 , 0x0 },
		{ "DX0GCR7"   , 0x1C7 , 0x0 },
		{ "DX0GCR8"   , 0x1C8 , 0x0 },
		{ "DX0GCR9"   , 0x1C9 , 0x0 },
		{ "DX0BDLR0"  , 0x1D0 , 0x0 },
		{ "DX0BDLR1"  , 0x1D1 , 0x0 },
		{ "DX0BDLR2"  , 0x1D2 , 0x0 },
		{ "DX0BDLR3"  , 0x1D4 , 0x0 },
		{ "DX0BDLR4"  , 0x1D5 , 0x0 },
		{ "DX0BDLR5"  , 0x1D6 , 0x0 },
		{ "DX0BDLR6"  , 0x1D8 , 0x0 },
		{ "DX0BDLR7"  , 0x1D9 , 0x0 },
		{ "DX0BDLR8"  , 0x1DA , 0x0 },
		{ "DX0BDLR9"  , 0x1DB , 0x0 },
		{ "DX0LCDLR0" , 0x1E0 , 0x0 },
		{ "DX0LCDLR1" , 0x1E1 , 0x0 },
		{ "DX0LCDLR2" , 0x1E2 , 0x0 },
		{ "DX0LCDLR3" , 0x1E3 , 0x0 },
		{ "DX0LCDLR4" , 0x1E4 , 0x0 },
		{ "DX0LCDLR5" , 0x1E5 , 0x0 },
		{ "DX0MDLR0"  , 0x1E8 , 0x0 },
		{ "DX0MDLR1"  , 0x1E9 , 0x0 },
		{ "DX0GTR0"   , 0x1F0 , 0x0 },
		{ "DX0GTR1"   , 0x1F1 , 0x0 },
		{ "DX0GTR2"   , 0x1F2 , 0x0 },
		{ "DX0GTR3"   , 0x1F3 , 0x0 },
		{ "DX0RSR0"   , 0x1F4 , 0x0 },
		{ "DX0RSR1"   , 0x1F5 , 0x0 },
		{ "DX0RSR2"   , 0x1F6 , 0x0 },
		{ "DX0RSR3"   , 0x1F7 , 0x0 },
		{ "DX0GSR0"   , 0x1F8 , 0x0 },
		{ "DX0GSR1"   , 0x1F9 , 0x0 },
		{ "DX0GSR2"   , 0x1FA , 0x0 },
		{ "DX0GSR3"   , 0x1FB , 0x0 },
		{ "DX0GSR4"   , 0x1FC , 0x0 },
		{ "DX0GSR5"   , 0x1FD , 0x0 },
		{ "DX0GSR6"   , 0x1FE , 0x0 },
		{ "DX1GCR0"   , 0x200 , 0x0 },
		{ "DX1GCR1"   , 0x201 , 0x0 },
		{ "DX1GCR2"   , 0x202 , 0x0 },
		{ "DX1GCR3"   , 0x203 , 0x0 },
		{ "DX1GCR4"   , 0x204 , 0x0 },
		{ "DX1GCR5"   , 0x205 , 0x0 },
		{ "DX1GCR6"   , 0x206 , 0x0 },
		{ "DX1GCR7"   , 0x207 , 0x0 },
		{ "DX1GCR8"   , 0x208 , 0x0 },
		{ "DX1GCR9"   , 0x209 , 0x0 },
		{ "DX1BDLR0"  , 0x210 , 0x0 },
		{ "DX1BDLR1"  , 0x211 , 0x0 },
		{ "DX1BDLR2"  , 0x212 , 0x0 },
		{ "DX1BDLR3"  , 0x214 , 0x0 },
		{ "DX1BDLR4"  , 0x215 , 0x0 },
		{ "DX1BDLR5"  , 0x216 , 0x0 },
		{ "DX1BDLR6"  , 0x218 , 0x0 },
		{ "DX1BDLR7"  , 0x219 , 0x0 },
		{ "DX1BDLR8"  , 0x21A , 0x0 },
		{ "DX1BDLR9"  , 0x21B , 0x0 },
		{ "DX1LCDLR0" , 0x220 , 0x0 },
		{ "DX1LCDLR1" , 0x221 , 0x0 },
		{ "DX1LCDLR2" , 0x222 , 0x0 },
		{ "DX1LCDLR3" , 0x223 , 0x0 },
		{ "DX1LCDLR4" , 0x224 , 0x0 },
		{ "DX1LCDLR5" , 0x225 , 0x0 },
		{ "DX1MDLR0"  , 0x228 , 0x0 },
		{ "DX1MDLR1"  , 0x229 , 0x0 },
		{ "DX1GTR0"   , 0x230 , 0x0 },
		{ "DX1GTR1"   , 0x231 , 0x0 },
		{ "DX1GTR2"   , 0x232 , 0x0 },
		{ "DX1GTR3"   , 0x233 , 0x0 },
		{ "DX1RSR0"   , 0x234 , 0x0 },
		{ "DX1RSR1"   , 0x235 , 0x0 },
		{ "DX1RSR2"   , 0x236 , 0x0 },
		{ "DX1RSR3"   , 0x237 , 0x0 },
		{ "DX1GSR0"   , 0x238 , 0x0 },
		{ "DX1GSR1"   , 0x239 , 0x0 },
		{ "DX1GSR2"   , 0x23A , 0x0 },
		{ "DX1GSR3"   , 0x23B , 0x0 },
		{ "DX1GSR4"   , 0x23C , 0x0 },
		{ "DX1GSR5"   , 0x23D , 0x0 },
		{ "DX1GSR6"   , 0x23E , 0x0 },
		{ "DX2GCR0"   , 0x240 , 0x0 },
		{ "DX2GCR1"   , 0x241 , 0x0 },
		{ "DX2GCR2"   , 0x242 , 0x0 },
		{ "DX2GCR3"   , 0x243 , 0x0 },
		{ "DX2GCR4"   , 0x244 , 0x0 },
		{ "DX2GCR5"   , 0x245 , 0x0 },
		{ "DX2GCR6"   , 0x246 , 0x0 },
		{ "DX2GCR7"   , 0x247 , 0x0 },
		{ "DX2GCR8"   , 0x248 , 0x0 },
		{ "DX2GCR9"   , 0x249 , 0x0 },
		{ "DX2BDLR0"  , 0x250 , 0x0 },
		{ "DX2BDLR1"  , 0x251 , 0x0 },
		{ "DX2BDLR2"  , 0x252 , 0x0 },
		{ "DX2BDLR3"  , 0x254 , 0x0 },
		{ "DX2BDLR4"  , 0x255 , 0x0 },
		{ "DX2BDLR5"  , 0x256 , 0x0 },
		{ "DX2BDLR6"  , 0x258 , 0x0 },
		{ "DX2BDLR7"  , 0x259 , 0x0 },
		{ "DX2BDLR8"  , 0x25A , 0x0 },
		{ "DX2BDLR9"  , 0x25B , 0x0 },
		{ "DX2LCDLR0" , 0x260 , 0x0 },
		{ "DX2LCDLR1" , 0x261 , 0x0 },
		{ "DX2LCDLR2" , 0x262 , 0x0 },
		{ "DX2LCDLR3" , 0x263 , 0x0 },
		{ "DX2LCDLR4" , 0x264 , 0x0 },
		{ "DX2LCDLR5" , 0x265 , 0x0 },
		{ "DX2MDLR0"  , 0x268 , 0x0 },
		{ "DX2MDLR1"  , 0x269 , 0x0 },
		{ "DX2GTR0"   , 0x270 , 0x0 },
		{ "DX2GTR1"   , 0x271 , 0x0 },
		{ "DX2GTR2"   , 0x272 , 0x0 },
		{ "DX2GTR3"   , 0x273 , 0x0 },
		{ "DX2RSR0"   , 0x274 , 0x0 },
		{ "DX2RSR1"   , 0x275 , 0x0 },
		{ "DX2RSR2"   , 0x276 , 0x0 },
		{ "DX2RSR3"   , 0x277 , 0x0 },
		{ "DX2GSR0"   , 0x278 , 0x0 },
		{ "DX2GSR1"   , 0x279 , 0x0 },
		{ "DX2GSR2"   , 0x27A , 0x0 },
		{ "DX2GSR3"   , 0x27B , 0x0 },
		{ "DX2GSR4"   , 0x27C , 0x0 },
		{ "DX2GSR5"   , 0x27D , 0x0 },
		{ "DX2GSR6"   , 0x27E , 0x0 },
		{ "DX3GCR0"   , 0x280 , 0x0 },
		{ "DX3GCR1"   , 0x281 , 0x0 },
		{ "DX3GCR2"   , 0x282 , 0x0 },
		{ "DX3GCR3"   , 0x283 , 0x0 },
		{ "DX3GCR4"   , 0x284 , 0x0 },
		{ "DX3GCR5"   , 0x285 , 0x0 },
		{ "DX3GCR6"   , 0x286 , 0x0 },
		{ "DX3GCR7"   , 0x287 , 0x0 },
		{ "DX3GCR8"   , 0x288 , 0x0 },
		{ "DX3GCR9"   , 0x289 , 0x0 },
		{ "DX3BDLR0"  , 0x290 , 0x0 },
		{ "DX3BDLR1"  , 0x291 , 0x0 },
		{ "DX3BDLR2"  , 0x292 , 0x0 },
		{ "DX3BDLR3"  , 0x294 , 0x0 },
		{ "DX3BDLR4"  , 0x295 , 0x0 },
		{ "DX3BDLR5"  , 0x296 , 0x0 },
		{ "DX3BDLR6"  , 0x298 , 0x0 },
		{ "DX3BDLR7"  , 0x299 , 0x0 },
		{ "DX3BDLR8"  , 0x29A , 0x0 },
		{ "DX3BDLR9"  , 0x29B , 0x0 },
		{ "DX3LCDLR0" , 0x2A0 , 0x0 },
		{ "DX3LCDLR1" , 0x2A1 , 0x0 },
		{ "DX3LCDLR2" , 0x2A2 , 0x0 },
		{ "DX3LCDLR3" , 0x2A3 , 0x0 },
		{ "DX3LCDLR4" , 0x2A4 , 0x0 },
		{ "DX3LCDLR5" , 0x2A5 , 0x0 },
		{ "DX3MDLR0"  , 0x2A8 , 0x0 },
		{ "DX3MDLR1"  , 0x2A9 , 0x0 },
		{ "DX3GTR0"   , 0x2B0 , 0x0 },
		{ "DX3GTR1"   , 0x2B1 , 0x0 },
		{ "DX3GTR2"   , 0x2B2 , 0x0 },
		{ "DX3GTR3"   , 0x2B3 , 0x0 },
		{ "DX3RSR0"   , 0x2B4 , 0x0 },
		{ "DX3RSR1"   , 0x2B5 , 0x0 },
		{ "DX3RSR2"   , 0x2B6 , 0x0 },
		{ "DX3RSR3"   , 0x2B7 , 0x0 },
		{ "DX3GSR0"   , 0x2B8 , 0x0 },
		{ "DX3GSR1"   , 0x2B9 , 0x0 },
		{ "DX3GSR2"   , 0x2BA , 0x0 },
		{ "DX3GSR3"   , 0x2BB , 0x0 },
		{ "DX3GSR4"   , 0x2BC , 0x0 },
		{ "DX3GSR5"   , 0x2BD , 0x0 },
		{ "DX3GSR6"   , 0x2BE , 0x0 }}
};

pub_ddr_phy_port_record ddr_phy_record[EMEM_MC_NUM_OF_BLOCKS];



static const int pub_dx_x_mdlr0_addr[] = {
	PUB_DX0MDLR0_REG_ADDR, PUB_DX1MDLR0_REG_ADDR,
	PUB_DX2MDLR0_REG_ADDR, PUB_DX3MDLR0_REG_ADDR
};

static const int pub_dx_x_lcdlr1_addr[] = {
	PUB_DX0LCDLR1_REG_ADDR, PUB_DX1LCDLR1_REG_ADDR,
	PUB_DX2LCDLR1_REG_ADDR, PUB_DX3LCDLR1_REG_ADDR,
};

static const int pub_dx_x_lcdlr3_addr[] = {
	PUB_DX0LCDLR3_REG_ADDR, PUB_DX1LCDLR3_REG_ADDR,
	PUB_DX2LCDLR3_REG_ADDR, PUB_DX3LCDLR3_REG_ADDR,
};

static const int pub_dx_x_lcdlr4_addr[] = {
	PUB_DX0LCDLR4_REG_ADDR, PUB_DX1LCDLR4_REG_ADDR,
	PUB_DX2LCDLR4_REG_ADDR, PUB_DX3LCDLR4_REG_ADDR,
};

static const int pub_dx_x_bdlr0_addr[] = {
	PUB_DX0BDLR0_REG_ADDR, PUB_DX1BDLR0_REG_ADDR,
	PUB_DX2BDLR0_REG_ADDR, PUB_DX3BDLR0_REG_ADDR,
};

static const int pub_dx_x_bdlr1_addr[] = {
	PUB_DX0BDLR1_REG_ADDR, PUB_DX1BDLR1_REG_ADDR,
	PUB_DX2BDLR1_REG_ADDR, PUB_DX3BDLR1_REG_ADDR,
};

static const int pub_dx_x_bdlr3_addr[] = {
	PUB_DX0BDLR3_REG_ADDR, PUB_DX1BDLR3_REG_ADDR,
	PUB_DX2BDLR3_REG_ADDR, PUB_DX3BDLR3_REG_ADDR,
};

static const int pub_dx_x_bdlr4_addr[] = {
	PUB_DX0BDLR4_REG_ADDR, PUB_DX1BDLR4_REG_ADDR,
	PUB_DX2BDLR4_REG_ADDR, PUB_DX3BDLR4_REG_ADDR,
};


static const int pub_dx_x_gcr5_addr[] = {
	PUB_DX0GCR5_REG_ADDR, PUB_DX1GCR5_REG_ADDR,
	PUB_DX2GCR5_REG_ADDR, PUB_DX3GCR5_REG_ADDR
};

static struct gpr_setup gpr[EMEM_MC_NUM_OF_BLOCKS] = {
		{0x618, 0x61A, 0x0, 0x1, 0x1, 0x0},
		{0x619, 0x61A, 0x0, 0x1, 0x1, 0x0},
		{0x61A, 0x61A, 0x0, 0x0, 0x0, 0x0},
		{0x61A, 0x61B, 0x3, 0x0, 0x0, 0x1},
		{0x61A, 0x61C, 0x3, 0x0, 0x0, 0x1},
		{0x61A, 0x61D, 0x3, 0x0, 0x0, 0x1},
		{0x718, 0x71A, 0x0, 0x1, 0x1, 0x0},
		{0x719, 0x71A, 0x0, 0x1, 0x1, 0x0},
		{0x71A, 0x71A, 0x0, 0x0, 0x0, 0x0},
		{0x71A, 0x71B, 0x3, 0x0, 0x0, 0x1},
		{0x71A, 0x71C, 0x3, 0x0, 0x0, 0x1},
		{0x71A, 0x71D, 0x3, 0x0, 0x0, 0x1}
};

#define PUB_RECORD_GET( _port_, _idx_) \
	(&(ddr_phy_record[_port_].reg_record[_idx_]))

void init_ddr_phy_record_DB(void)
{
	u32 port;

	for (port = 0; port < EMEM_MC_NUM_OF_BLOCKS; port++)
		memcpy(&ddr_phy_record[port],
				&ddr_phy_port_record_init,
				sizeof(ddr_phy_port_record_init));
}

int find_ddr_phy_record(int port, bool addr_mode, int address, char *name)
{
	u32 idx, arr_size, i;
	bool match = false;

	arr_size = sizeof(ddr_phy_record[port]) /
			sizeof(ddr_phy_record[port].reg_record[0]);
	if (arr_size != PUB_DDR_PHY_REGS_NUMBER)
		printf("ERROR, update PUB_DDR_PHY_REGS_NUMBER to %d\n", arr_size);

	for (idx = 0; idx < arr_size; idx++) {
		if (addr_mode)
			match = (PUB_RECORD_GET(port, idx)->address == address);
		else {
			for (i = 0; i < PUB_REG_NAME_MAX_LEN; i++) {
				if ( name[i] != PUB_RECORD_GET(port, idx)->name[i]) {
					break;
				}
				if ( name[i] == 0){
					match = true;
					break;
				}
			}
		}

		if (match) {
			if(get_debug())
				printf("TEMP PRINT: FOUND INDEX = %d\n", idx);
			return idx;
		}
	}

	return INVALID_VAL;
}

/*********************
 * Internal service functions *
**********************/

static void show_write_centralization_margin(int block)
{
	u32 bl_index, wdqd_tr_res, bl_iprd, bl_tap_fantaSec, tck_index, rank_index;
	u32 phy_side, phy_num, block_idx, mc_mask;
	u32 bist_results[EMEM_MC_NUM_OF_BLOCKS];
	u32 res_udqd[BYTE_LANES_NUM][RESULT_FIELDS_NUM];
	u32 ret_status = 0;

	union pub_pgcr6 pgcr6;
	union pub_dx_x_lcdlr1 dx_x_lcdlr1;
	union pub_dx_x_mdlr0 dx_x_mdlr0;
	union pub_bistrr bistrr;
	union pub_bistar1 bistar1;

	int status;

	phy_side = (block >= emem_mc_block_id[6]) ? 1 : 0;
	phy_num = (block >= emem_mc_block_id[6]) ?
			(block - emem_mc_block_id[6]) :
			(block - emem_mc_block_id[0]);
	block_idx = (phy_side * 6) + phy_num;
	mc_mask = (1 << block_idx);

	/* 1) */
	if (get_debug())
		printf("Step 1\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 1;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 2) */
	if (get_debug())
		printf("Step 2\n");
	if (get_debug())
		printf("Debug print: Call phy_bist_setup, mc_mask = 0x%08X\n",
				mc_mask);
	phy_bist_setup(mc_mask);

	/* 3) */
	if (get_debug())
		printf("Step 3\n");
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		dx_x_mdlr0.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_mdlr0_addr[bl_index]);

		if (g_temp_try) {
			dx_x_mdlr0.fields.iprd = 13;
			printf("temp_try print: temp_try mode, ");
			printf("dx_x_mdlr0.fields.iprd = %d\n",
						dx_x_mdlr0.fields.iprd);
		}

		bl_iprd = dx_x_mdlr0.fields.iprd;

		/* 5) */
		if (get_debug())
			printf("Step 5\n");
		tck_index = current_ddr_params.pll_freq;
		bl_tap_fantaSec = (tCK2[tck_index] / (2 * bl_iprd));

		if (get_debug()) {
			printf("Debug print: bl_tap_fantaSec = ");
			printf("tCK2 /(2 * bl_iprd) = ");
			printf("%u / (2 * %u) = %u\n",
					tCK2[tck_index], bl_iprd, bl_tap_fantaSec);
		}
		/* 6) */
		if (get_debug())
			printf("Step 6\n");
		dx_x_lcdlr1.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr1_addr[bl_index]);

		if (g_temp_try) {
			dx_x_lcdlr1.fields.wdqd = 8;
			printf("temp_try print: temp_try mode ,");
			printf("dx_x_lcdlr1.fields.wdqd = %d\n",
						dx_x_lcdlr1.fields.wdqd);
		}

		wdqd_tr_res = dx_x_lcdlr1.fields.wdqd;
		res_udqd[bl_index][TR_VAL] = dx_x_lcdlr1.fields.wdqd;
		res_udqd[bl_index][BL_TAP_PS] = bl_tap_fantaSec;

		/* 7).*/
		if (get_debug())
			printf("Step 7\n");
		bistrr.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTRR_REG_ADDR);
		bistrr.fields.bdxsel = bl_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTRR_REG_ADDR, bistrr.reg);

		/* 8) */
		if (get_debug())
			printf("Step 8\n");
		rank_index = (bl_index / 2);

		/* 9) */
		if (get_debug())
			printf("Step 9\n");
		bistar1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTAR1_REG_ADDR);
		bistar1.fields.brank = rank_index;
		bistar1.fields.bmrank = rank_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTAR1_REG_ADDR, bistar1.reg);

		/* 10) */
		/* 11) */

		/* NOTE: LCDLR1 wdqd field equals to wdqd_tr_res */
		if (get_debug())
			printf("Step 10+11\n");
		if (get_debug()) {
			printf("Debug print: Run right bist,");
			printf("byte %u, wdqd = %u\n",
					bl_index, dx_x_lcdlr1.fields.wdqd);
			printf("Debug print: mc_mask = 0x%08X\n", mc_mask);
		}
		status = phy_bist_run(mc_mask, bist_results);
		if(status)
			return;
		else if(bist_results[block_idx] != 0) {
			if (get_debug())
				printf("Debug print: phy_bist_run %s\n",
					(bist_results[block_idx]) ? "failed":"passed");
		}
		while ((bist_results[block_idx] == 0) &&
				(dx_x_lcdlr1.fields.wdqd > 0)) {
			dx_x_lcdlr1.fields.wdqd--;
			emem_mc_indirect_reg_write_synop_data0(block,
					pub_dx_x_lcdlr1_addr[bl_index],
							dx_x_lcdlr1.reg);
			if (get_debug()) {
				printf("Debug print: Run right bist,");
				printf("byte %u, wdqd = %u\n",
					bl_index, dx_x_lcdlr1.fields.wdqd);
				printf("Debug print: mc_mask = 0x%08X\n",
								mc_mask);
			}
			status = phy_bist_run(mc_mask, bist_results);
			if(status)
				return;
			else if(bist_results[block_idx] != 0) {
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");
			}
		}

		res_udqd[bl_index][RIGHT_EDGE] = dx_x_lcdlr1.fields.wdqd;

		/* 12) */
		if (get_debug())
			printf("Step 12\n");
		dx_x_lcdlr1.fields.wdqd = wdqd_tr_res;
		emem_mc_indirect_reg_write_synop_data0(
				block, pub_dx_x_lcdlr1_addr[bl_index],
							dx_x_lcdlr1.reg);

		/* 13) */
		/* 14) */
		/* NOTE: LCDLR1 wdqd field equals to wdqd_tr_res */
		if (get_debug())
			printf("Step 13+14\n");

		/* senity check */
		if (wdqd_tr_res > bl_iprd) {
			printf("ERROR: wdqd_tr_res %u > bl_iprd %u\n",
					wdqd_tr_res, bl_iprd);
			ret_status = 1;
		}

		if (get_debug())
			printf("Debug: Run left bist, byte %u, wdqd = %u\n",
					bl_index, dx_x_lcdlr1.fields.wdqd);
		if (get_debug())
			printf("Debug print: mc_mask = 0x%08X\n", mc_mask);
		status = phy_bist_run(mc_mask, bist_results);
		if(status)
			return;
		else if(bist_results[block_idx] != 0) {
			if (get_debug())
				printf("Debug print: phy_bist_run %s\n",
					(bist_results[block_idx]) ? "failed":"passed");
		}

		while ((bist_results[block_idx] == 0) &&
				(dx_x_lcdlr1.fields.wdqd < bl_iprd)) {
			dx_x_lcdlr1.fields.wdqd++;
			emem_mc_indirect_reg_write_synop_data0(block,
					pub_dx_x_lcdlr1_addr[bl_index],
					dx_x_lcdlr1.reg);
			if (get_debug()) {
				printf("Debug print: Run left bist, ");
				printf("byte %u, wdqd = %u\n", bl_index,
						dx_x_lcdlr1.fields.wdqd);
				printf("Debug print: mc_mask = 0x%08X\n",
								mc_mask);
			}
			status = phy_bist_run(mc_mask, bist_results);
			if(status)
				return;
			else if(bist_results[block_idx] != 0) {
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");
			}
		}

		res_udqd[bl_index][LEFT_EDGE] = dx_x_lcdlr1.fields.wdqd;

		/* 15) */
		if (get_debug())
			printf("Step 15\n");
		dx_x_lcdlr1.fields.wdqd = wdqd_tr_res;
		emem_mc_indirect_reg_write_synop_data0(block,
				pub_dx_x_lcdlr1_addr[bl_index],
						dx_x_lcdlr1.reg);

		/* 16) */
		if (get_debug())
			printf("Step 16\n");
	}

	/* 17)*/
	if (get_debug())
		printf("Step 17\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 0;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 18) */
	if (get_debug())
		printf("Step 18\n");
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_BISTRR_REG_ADDR, PUB_BISTRR_REG_HW_DEF_VAL);

	/* 19) */
	if (get_debug())
		printf("Step 19\n");

	printf("\nShow WRITE centralization margin for DDR PHY UM%u_%u_:\n\n",
			phy_side, phy_num);
	printf("status = %s\n", ((ret_status) ? "FAIL" : "PASS"));
	printf("%-10s %-18s %-18s %-18s %-18s\n",
			"byte_lane",
			"TR_VAL",
			"RIGHT_EDGE",
			"LEFT_EDGE",
			"BL_TAP_PS");

	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {
		printf("%-10u %-18u %-18u %-18u %u.%03u\n",
				bl_index,
				res_udqd[bl_index][TR_VAL],
				res_udqd[bl_index][RIGHT_EDGE],
				res_udqd[bl_index][LEFT_EDGE],
				(res_udqd[bl_index][BL_TAP_PS]/1000),
				(res_udqd[bl_index][BL_TAP_PS]%1000));
	}
	printf("\n");
}

static void show_read_centralization_margin(int block)
{
	u32 rdqsd_tr_res, rdqsnd_tr_res;
	u32 bl_index, bl_iprd, bl_tap_fantaSec, tck_index, rank_index;
	u32 phy_side, phy_num, block_idx, mc_mask;
	u32 bist_results[EMEM_MC_NUM_OF_BLOCKS];
	u32 res_rdqsd[BYTE_LANES_NUM][RESULT_FIELDS_NUM];
	u32 res_rdqsdn[BYTE_LANES_NUM][RESULT_FIELDS_NUM];
	u32 ret_status = 0;
	int status;

	union pub_pgcr6 pgcr6;
	union pub_dx_x_lcdlr3 dx_x_lcdlr3;
	union pub_dx_x_lcdlr4 dx_x_lcdlr4;
	union pub_dx_x_mdlr0 dx_x_mdlr0;
	union pub_bistrr bistrr;
	union pub_bistar1 bistar1;

	phy_side = (block >= emem_mc_block_id[6]) ? 1 : 0;
	phy_num = (block >= emem_mc_block_id[6]) ?
			(block - emem_mc_block_id[6]) :
				(block - emem_mc_block_id[0]);
	block_idx = (phy_side * 6) + phy_num;
	mc_mask = (1 << block_idx);


	/* 1) */
	if (get_debug())
		printf("Step 1\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 1;

	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 2) */
	if (get_debug())
		printf("Step 2\n");
	if (get_debug())
		printf("Debug print: Call phy_bist_setup, mc_mask = 0x%08X\n",
				mc_mask);
	phy_bist_setup(mc_mask);

	/* 3) */
	if (get_debug())
		printf("Step 3\n");
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {
		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		dx_x_mdlr0.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_mdlr0_addr[bl_index]);

		if (g_temp_try) {
			dx_x_mdlr0.fields.iprd = 8;
			printf("temp_try print: temp_try mode, ");
			printf("dx_x_mdlr0.fields.iprd = %d\n",
						dx_x_mdlr0.fields.iprd);
		}

		bl_iprd = dx_x_mdlr0.fields.iprd;

		/* 5) */
		if (get_debug())
			printf("Step 5\n");
		tck_index = current_ddr_params.pll_freq;
		bl_tap_fantaSec = (tCK2[tck_index] / (2 * bl_iprd));

		if (get_debug()) {
			printf("Debug print: bl_tap_fantaSec =");
			printf("tCK2 /(2 * bl_iprd) = ");
			printf("%u / (2 * %u) = %u\n",
					tCK2[tck_index], bl_iprd, bl_tap_fantaSec);
		}
		/* 6) */
		if (get_debug())
			printf("Step 6\n");
		dx_x_lcdlr3.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr3_addr[bl_index]);

		if (g_temp_try) {
			dx_x_lcdlr3.fields.rdqsd = 4;
			printf("temp_try print: temp_try mode ,");
			printf("dx_x_lcdlr3.fields.rdqsd = %d\n",
						dx_x_lcdlr3.fields.rdqsd);
		}
		rdqsd_tr_res = dx_x_lcdlr3.fields.rdqsd;
		res_rdqsd[bl_index][TR_VAL] = dx_x_lcdlr3.fields.rdqsd;

		dx_x_lcdlr4.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr4_addr[bl_index]);

		if (g_temp_try) {
			dx_x_lcdlr4.fields.rdqsnd = 6;
			printf("temp_try print: temp_try mode ,");
			printf("dx_x_lcdlr4.fields.rdqsnd = %d\n",
						dx_x_lcdlr4.fields.rdqsnd);
		}

		rdqsnd_tr_res = dx_x_lcdlr4.fields.rdqsnd;
		res_rdqsdn[bl_index][TR_VAL] = dx_x_lcdlr4.fields.rdqsnd;

		res_rdqsd[bl_index][BL_TAP_PS] = bl_tap_fantaSec;
		res_rdqsdn[bl_index][BL_TAP_PS] = bl_tap_fantaSec;

		/* 7) */
		if (get_debug())
			printf("Step 7\n");
		bistrr.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTRR_REG_ADDR);
		bistrr.fields.bdxsel = bl_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTRR_REG_ADDR, bistrr.reg);

		/* 8) */
		if (get_debug())
			printf("Step 8\n");
		rank_index = (bl_index / 2);

		/* 9) */
		if (get_debug())
			printf("Step 9\n");
		bistar1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTAR1_REG_ADDR);
		bistar1.fields.brank = rank_index;
		bistar1.fields.bmrank = rank_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTAR1_REG_ADDR, bistar1.reg);

		/* 10) */
		/* 11) */
		/* NOTE:
		 *	LCDLR3 rdqsd field equals to rdqsd_tr_res
		 *	LCDLR4 rdqsnd field equals to rdqsnd_tr_res */
		if (get_debug())
			printf("Step 10+11\n");
		if (get_debug()) {
			printf("Debug print: Run right bist, byte %u, ",
								bl_index);
			printf("rdqsd = %u, rdqsnd = %u\n",
					dx_x_lcdlr3.fields.rdqsd,
					dx_x_lcdlr4.fields.rdqsnd);
		}
		if (get_debug())
			printf("Debug print: mc_mask = 0x%08X\n", mc_mask);
		status = phy_bist_run(mc_mask, bist_results);
		if(status)
			return;
		else if(bist_results[block_idx] != 0) {
			if (get_debug())
				printf("Debug print: phy_bist_run %s\n",
					(bist_results[block_idx]) ? "failed":"passed");
		}

		while ((bist_results[block_idx] == 0) &&
				(dx_x_lcdlr3.fields.rdqsd > 0) &&
				(dx_x_lcdlr4.fields.rdqsnd > 0)) {
			dx_x_lcdlr3.fields.rdqsd--;
			dx_x_lcdlr4.fields.rdqsnd--;
			emem_mc_indirect_reg_write_synop_data0(block,
					pub_dx_x_lcdlr3_addr[bl_index],
							dx_x_lcdlr3.reg);
			emem_mc_indirect_reg_write_synop_data0(block,
					pub_dx_x_lcdlr4_addr[bl_index],
							dx_x_lcdlr4.reg);

			if (get_debug()) {
				printf("Debug: Run right bist, byte %u, ",
								bl_index);
				printf("rdqsd = %u, rdqsnd = %u\n",
						dx_x_lcdlr3.fields.rdqsd,
						dx_x_lcdlr4.fields.rdqsnd);
			}
			if (get_debug())
				printf("Debug print: mc_mask = 0x%08X\n",
								mc_mask);
			status = phy_bist_run(mc_mask, bist_results);
			if(status)
				return;
			else if(bist_results[block_idx] != 0) {
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");
			}
		}

		res_rdqsd[bl_index][RIGHT_EDGE] = dx_x_lcdlr3.fields.rdqsd;
		res_rdqsdn[bl_index][RIGHT_EDGE] = dx_x_lcdlr4.fields.rdqsnd;

		/* 12) */
		if (get_debug())
			printf("Step 12\n");
		dx_x_lcdlr3.fields.rdqsd = rdqsd_tr_res;
		dx_x_lcdlr4.fields.rdqsnd = rdqsnd_tr_res;
		emem_mc_indirect_reg_write_synop_data0(
				block, pub_dx_x_lcdlr3_addr[bl_index],
				dx_x_lcdlr3.reg);
		emem_mc_indirect_reg_write_synop_data0(
				block, pub_dx_x_lcdlr4_addr[bl_index],
				dx_x_lcdlr4.reg);

		/*	13) */
		/*	14) */

		/* NOTE:
		 *	LCDLR3 rdqsd field equals to rdqsd_tr_res
		 *	LCDLR4 rdqsnd field equals to rdqsnd_tr_res */

		if (get_debug())
			printf("Step 13+14\n");
		/* senity check */
		if (rdqsd_tr_res > bl_iprd) {
			printf("ERROR: rdqsd_tr_res %u > bl_iprd %u\n",
					rdqsd_tr_res, bl_iprd);
			ret_status = 1;
		}
		if (rdqsnd_tr_res > bl_iprd) {
			printf("ERROR: rdqsnd_tr_res %u > bl_iprd %u\n",
					rdqsnd_tr_res, bl_iprd);
			ret_status = 1;
		}

		if (get_debug()) {
			printf("Debug print: Run Left bist, byte %u, ",
								bl_index);
			printf("rdqsd = %u, rdqsnd = %u\n",
					dx_x_lcdlr3.fields.rdqsd,
					dx_x_lcdlr4.fields.rdqsnd);
		}
		if (get_debug())
			printf("Debug print: mc_mask = 0x%08X\n", mc_mask);
		status = phy_bist_run(mc_mask, bist_results);
		if(status)
			return;
		else if(bist_results[block_idx] != 0) {
			if (get_debug())
				printf("Debug print: phy_bist_run %s\n",
					(bist_results[block_idx]) ? "failed":"passed");
		}

		while ((bist_results[block_idx] == 0) &&
				(dx_x_lcdlr3.fields.rdqsd < bl_iprd) &&
				(dx_x_lcdlr4.fields.rdqsnd < bl_iprd)) {
			dx_x_lcdlr3.fields.rdqsd++;
			dx_x_lcdlr4.fields.rdqsnd++;
			emem_mc_indirect_reg_write_synop_data0(block,
					pub_dx_x_lcdlr3_addr[bl_index],
					dx_x_lcdlr3.reg);
			emem_mc_indirect_reg_write_synop_data0(block,
					 pub_dx_x_lcdlr4_addr[bl_index],
					 dx_x_lcdlr4.reg);
			if (get_debug()) {
				printf("Debug: Run Left bist, byte %u, ",
								bl_index);
				printf("rdqsd = %u, rdqsnd = %u\n",
						dx_x_lcdlr3.fields.rdqsd,
						dx_x_lcdlr4.fields.rdqsnd);
			}
			if (get_debug())
				printf("Debug: mc_mask = 0x%08X\n", mc_mask);
			status = phy_bist_run(mc_mask, bist_results);
			if(status)
				return;
			else if(bist_results[block_idx] != 0) {
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");
			}
		}

		res_rdqsd[bl_index][LEFT_EDGE] = dx_x_lcdlr3.fields.rdqsd;
		res_rdqsdn[bl_index][LEFT_EDGE] = dx_x_lcdlr4.fields.rdqsnd;


		/* 15) */
		if (get_debug())
			printf("Step 15\n");
		dx_x_lcdlr3.fields.rdqsd = rdqsd_tr_res;
		dx_x_lcdlr4.fields.rdqsnd = rdqsnd_tr_res;
		emem_mc_indirect_reg_write_synop_data0(
					block, pub_dx_x_lcdlr3_addr[bl_index],
					dx_x_lcdlr3.reg);
		emem_mc_indirect_reg_write_synop_data0(
					block, pub_dx_x_lcdlr4_addr[bl_index],
					dx_x_lcdlr4.reg);


		/* 16) */
		if (get_debug())
			printf("Step 16\n");
	}

	/* 17) */
	if (get_debug())
		printf("Step 17\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 0;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 18) */
	if (get_debug())
		printf("Step 18\n");
	/* read_apb gets default reg value */
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_BISTRR_REG_ADDR, PUB_BISTRR_REG_HW_DEF_VAL);

	/* 19) */
	if (get_debug())
		printf("Step 19\n");

	printf("\nShow READ centralization margin for DDR PHY UM%u_%u_:\n\n",
			phy_side, phy_num);
	printf("status = %s\n", ((ret_status) ? "FAIL" : "PASS"));

	printf("RES_RDQSD TABLE:\n");
	printf("%-10s %-18s %-18s %-18s %-18s\n",
			"byte_lane",
			"TR_VAL",
			"RIGHT_EDGE",
			"LEFT_EDGE",
			"BL_TAP_PS");
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {
		printf("%-10u %-18u %-18u %-18u %u.%03u\n",
				bl_index,
				res_rdqsd[bl_index][TR_VAL],
				res_rdqsd[bl_index][RIGHT_EDGE],
				res_rdqsd[bl_index][LEFT_EDGE],
				(res_rdqsd[bl_index][BL_TAP_PS]/1000),
				(res_rdqsd[bl_index][BL_TAP_PS]%1000));
	}
	printf("\n");

	printf("RES_RDQSND TABLE:\n");
	printf("%-10s %-18s %-18s %-18s %-18s\n",
			"byte_lane",
			"TR_VAL",
			"RIGHT_EDGE",
			"LEFT_EDGE",
			"BL_TAP_PS");
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {
		printf("%-10u %-18u %-18u %-18u %u.%03u\n",
				bl_index,
				res_rdqsdn[bl_index][TR_VAL],
				res_rdqsdn[bl_index][RIGHT_EDGE],
				res_rdqsdn[bl_index][LEFT_EDGE],
				(res_rdqsdn[bl_index][BL_TAP_PS]/1000),
				(res_rdqsdn[bl_index][BL_TAP_PS]%1000));
	}
	printf("\n");
}


static int get_bdlr_bd_field(
		int block,
		int bl_index,
		int bit_index,
		int nibble)
{
	union pub_dx_x_bdlr0 dx_x_bdlr0;
	union pub_dx_x_bdlr1 dx_x_bdlr1;
	union pub_dx_x_bdlr3 dx_x_bdlr3;
	union pub_dx_x_bdlr4 dx_x_bdlr4;
	int bd_field;

	if (nibble == 1) {
		dx_x_bdlr1.reg = emem_mc_indirect_reg_read_synop(
			block, pub_dx_x_bdlr1_addr[bl_index]);

		switch (bit_index) {
		case 4:
			bd_field = dx_x_bdlr1.fields.dq4wbd;
			break;
		case 5:
			bd_field = dx_x_bdlr1.fields.dq5wbd;
			break;
		case 6:
			bd_field = dx_x_bdlr1.fields.dq6wbd;
			break;
		default:
			bd_field = dx_x_bdlr1.fields.dq7wbd;
			break;
		}
	} else if (nibble == 0) {
		dx_x_bdlr0.reg = emem_mc_indirect_reg_read_synop(
			block, pub_dx_x_bdlr0_addr[bl_index]);

		switch (bit_index) {
		case 0:
			bd_field = dx_x_bdlr0.fields.dq0wbd;
			break;
		case 1:
			bd_field = dx_x_bdlr0.fields.dq1wbd;
			break;
		case 2:
			bd_field = dx_x_bdlr0.fields.dq2wbd;
			break;
		default:
			bd_field = dx_x_bdlr0.fields.dq3wbd;
			break;
		}
	} else if (nibble == 4) {
		dx_x_bdlr4.reg = emem_mc_indirect_reg_read_synop(
			block, pub_dx_x_bdlr4_addr[bl_index]);

		switch (bit_index) {
		case 4:
			bd_field = dx_x_bdlr4.fields.dq4rbd;
			break;
		case 5:
			bd_field = dx_x_bdlr4.fields.dq5rbd;
			break;
		case 6:
			bd_field = dx_x_bdlr4.fields.dq6rbd;
			break;
		default:
			bd_field = dx_x_bdlr4.fields.dq7rbd;
			break;
		}
	} else {
		/* nibble = 3 */
		dx_x_bdlr3.reg = emem_mc_indirect_reg_read_synop(
			block, pub_dx_x_bdlr3_addr[bl_index]);

		switch (bit_index) {
		case 0:
			bd_field = dx_x_bdlr3.fields.dq0rbd;
			break;
		case 1:
			bd_field = dx_x_bdlr3.fields.dq1rbd;
			break;
		case 2:
			bd_field = dx_x_bdlr3.fields.dq2rbd;
			break;
		default:
			bd_field = dx_x_bdlr3.fields.dq3rbd;
			break;
		}
	}

	return bd_field;
}

static void set_bdlr_bd_field(int block, int bl_index, int bit_index,
						int nibble, int bd_field)
{
	union pub_dx_x_bdlr0 dx_x_bdlr0;
	union pub_dx_x_bdlr1 dx_x_bdlr1;
	union pub_dx_x_bdlr3 dx_x_bdlr3;
	union pub_dx_x_bdlr4 dx_x_bdlr4;

	if (nibble == 1) {
		dx_x_bdlr1.reg = emem_mc_indirect_reg_read_synop(
			block, pub_dx_x_bdlr1_addr[bl_index]);

		switch (bit_index) {
		case 4:
			dx_x_bdlr1.fields.dq4wbd = bd_field;
			break;
		case 5:
			dx_x_bdlr1.fields.dq5wbd = bd_field;
			break;
		case 6:
			dx_x_bdlr1.fields.dq6wbd = bd_field;
			break;
		default:
			dx_x_bdlr1.fields.dq7wbd = bd_field;
			break;
		}
		emem_mc_indirect_reg_write_synop_data0(block,
				pub_dx_x_bdlr1_addr[bl_index], dx_x_bdlr1.reg);
	} else if (nibble == 0) {

		dx_x_bdlr0.reg = emem_mc_indirect_reg_read_synop(
			block, pub_dx_x_bdlr0_addr[bl_index]);

		switch (bit_index) {
		case 0:
			dx_x_bdlr0.fields.dq0wbd = bd_field;
			break;
		case 1:
			dx_x_bdlr0.fields.dq1wbd = bd_field;
			break;
		case 2:
			dx_x_bdlr0.fields.dq2wbd = bd_field;
			break;
		default:
			dx_x_bdlr0.fields.dq3wbd = bd_field;
			break;
		}
		emem_mc_indirect_reg_write_synop_data0(block,
			pub_dx_x_bdlr0_addr[bl_index], dx_x_bdlr0.reg);
	} else if (nibble == 4) {
		dx_x_bdlr4.reg = emem_mc_indirect_reg_read_synop(
			block, pub_dx_x_bdlr4_addr[bl_index]);

		switch (bit_index) {
		case 4:
			dx_x_bdlr4.fields.dq4rbd = bd_field;
			break;
		case 5:
			dx_x_bdlr4.fields.dq5rbd = bd_field;
			break;
		case 6:
			dx_x_bdlr4.fields.dq6rbd = bd_field;
			break;
		default:
			dx_x_bdlr4.fields.dq7rbd = bd_field;
			break;
		}
		emem_mc_indirect_reg_write_synop_data0(block,
				pub_dx_x_bdlr4_addr[bl_index], dx_x_bdlr4.reg);
	} else {
		/* nibble = 3 */
		dx_x_bdlr3.reg = emem_mc_indirect_reg_read_synop(
					block, pub_dx_x_bdlr3_addr[bl_index]);

		/*if (get_debug())
			printf("TEMP_PRINT: nibble = 3!!!\n"); */

		switch (bit_index) {
		case 0:
			dx_x_bdlr3.fields.dq0rbd = bd_field;
			break;
		case 1:
			dx_x_bdlr3.fields.dq1rbd = bd_field;
			break;
		case 2:
			dx_x_bdlr3.fields.dq2rbd = bd_field;
			/*printf("TEMP_PRINT: dx_x_bdlr3.fields.dq2rbd = bd_field!!!\n"); */
			break;
		default:
			dx_x_bdlr3.fields.dq3rbd = bd_field;
			break;
		}
		emem_mc_indirect_reg_write_synop_data0(block,
				pub_dx_x_bdlr3_addr[bl_index], dx_x_bdlr3.reg);
		/* printf("TEMP_PRINT: emem_mc_indirect_reg_write_synop_data0(block, pub_dx_x_bdlr3_addr[bl_index], dx_x_bdlr3.reg);!!!\n"); */

	}
}


static void show_write_skew_margin(int block, bool print_enable)
{
	u32 bl_index, bl_iprd, bl_tap_fantaSec, tck_index, rank_index;
	u32 bit_index, nibble, res_index, dqwbd_tr_res, dqwbd_tr_cur, eindex;
	u32 phy_side, phy_num, block_idx, mc_mask;
	u32 bist_results[EMEM_MC_NUM_OF_BLOCKS];
	u32 ret_status = 0;

	int status;

	union pub_pgcr6 pgcr6;
	union pub_dx_x_mdlr0 dx_x_mdlr0;
	union pub_bistrr bistrr;
	union pub_bistar1 bistar1;

	phy_side = (block >= emem_mc_block_id[6]) ? 1 : 0;
	phy_num = (block >= emem_mc_block_id[6]) ?
			(block - emem_mc_block_id[6]) :
			(block - emem_mc_block_id[0]);
	block_idx = (phy_side * 6) + phy_num;
	mc_mask = (1 << block_idx);

	/* 1) */
	if (get_debug())
		printf("Step 1\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 1;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 2) */
	if (get_debug())
		printf("Step 2\n");
	if (get_debug())
		printf("Debug print: Call phy_bist_setup, mc_mask = 0x%08X\n",
				mc_mask);
	phy_bist_setup(mc_mask);

	/* 3) */
	if (get_debug())
		printf("Step 3\n");
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		dx_x_mdlr0.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_mdlr0_addr[bl_index]);

		if (g_temp_try) {
			dx_x_mdlr0.fields.iprd = 8;
			printf("temp_try print: temp_try mode, ");
			printf("dx_x_mdlr0.fields.iprd = %d\n",
						dx_x_mdlr0.fields.iprd);
		}

		bl_iprd = dx_x_mdlr0.fields.iprd;

		/* 5) */
		/* NOTE: for more precision,
		 use (psec * 1000) resolution in tck and bl_iprd values.*/
		if (get_debug())
			printf("Step 5\n");
		tck_index = current_ddr_params.pll_freq;
		bl_tap_fantaSec = (tCK2[tck_index] / (2 * bl_iprd));

		if (get_debug()) {
			printf("Debug print: bl_tap_fantaSec = ");
			printf("tCK2 /(2 * bl_iprd) = ");
			printf("%u / (2 * %u) = %u\n",
					tCK2[tck_index], bl_iprd, bl_tap_fantaSec);
		}
		/* 6) */
		if (get_debug())
			printf("Step 6\n");
		bistrr.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTRR_REG_ADDR);
		bistrr.fields.bdxsel = bl_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTRR_REG_ADDR, bistrr.reg);

		/* 7) */
		if (get_debug())
			printf("Step 7\n");
		rank_index = (bl_index / 2);

		/* 8) */
		if (get_debug())
			printf("Step 8\n");
		bistar1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTAR1_REG_ADDR);
		bistar1.fields.brank = rank_index;
		bistar1.fields.bmrank = rank_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTAR1_REG_ADDR, bistar1.reg);


		/* 9) */
		if (get_debug())
			printf("Step 9\n");
		for (bit_index = 0; bit_index < 8; bit_index++) {

			/* 10).*/
			if (get_debug())
				printf("Step 10\n");
			nibble = (bit_index / 4); /* can be 0 or 1 */

			/* 11) */
			if (get_debug())
				printf("Step 11\n");
			res_index = (8 * bl_index) + bit_index;

			/* 12) */
			if (get_debug())
				printf("Step 12\n");

			dqwbd_tr_res = get_bdlr_bd_field(block,
					bl_index, bit_index, nibble);

			if (g_temp_try) {
				dqwbd_tr_res = 3;
				printf("temp_try print: temp_try mode, ");
				printf("dqwbd_tr_res = %d\n", dqwbd_tr_res);
			}

			res_wbd[res_index][TR_VAL] = dqwbd_tr_res;
			res_wbd[res_index][BL_TAP_PS] = bl_tap_fantaSec;

			/* 13) */
			/* 14) */
			/* NOTE: store dqwbd_tr_res value in dqwbd_tr_cur */
			if (get_debug())
				printf("Step 13+14\n");
			dqwbd_tr_cur = dqwbd_tr_res;

			if (get_debug()) {
				printf("Debug print: Run right bist, ");
				printf("byte %u, bit %u, wbd = %u\n",
					bl_index, bit_index, dqwbd_tr_cur);
			}
			if (get_debug())
				printf("Debug print: mc_mask = 0x%08X\n",
								mc_mask);
			status = phy_bist_run(mc_mask, bist_results);
			if(status)
				return;
			else if(bist_results[block_idx] != 0) {
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");
			}

			while ((bist_results[block_idx] == 0)
						&& (dqwbd_tr_cur > 0)) {
				dqwbd_tr_cur--;
				set_bdlr_bd_field(block, bl_index, bit_index,
							nibble, dqwbd_tr_cur);
				if (get_debug()) {
					printf("Debug print: Run right bist, ");
					printf("byte %u, bit %u, wbd = %u\n",
						bl_index, bit_index, dqwbd_tr_cur);
				}
				if (get_debug())
					printf("Debug: mc_mask = 0x%08X\n",
								mc_mask);
				status = phy_bist_run(mc_mask, bist_results);
				if(status)
					return;
				else if(bist_results[block_idx] != 0) {
					if (get_debug())
						printf("Debug print: phy_bist_run %s\n",
							(bist_results[block_idx]) ? "failed":"passed");
				}
			}

			res_wbd[res_index][RIGHT_EDGE] = dqwbd_tr_cur;

			/* 15) */
			if (get_debug())
				printf("Step 15\n");

			set_bdlr_bd_field(block, bl_index, bit_index,
							nibble, dqwbd_tr_res);

			/* 16) */
			/* 17) */
			/* NOTE: store dqwbd_tr_res value in dqwbd_tr_cur */
			if (get_debug())
				printf("Step 16+17\n");

			if (dqwbd_tr_res > bl_iprd) {
				printf("ERROR: dqwbd_tr_res %u > bl_iprd %u\n",
						dqwbd_tr_res, bl_iprd);
				ret_status = 1;
			}
			dqwbd_tr_cur = dqwbd_tr_res;

			if (get_debug()) {
				printf("Debug print: Run left bist, ");
				printf("byte %u, bit %u, wbd = %u\n",
					bl_index, bit_index, dqwbd_tr_cur);
			}
			if (get_debug())
				printf("Debug print: mc_mask = 0x%08X\n",
								mc_mask);
			status = phy_bist_run(mc_mask, bist_results);
			if(status)
				return;
			else if(bist_results[block_idx] != 0) {
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");
			}

			while ((bist_results[block_idx] == 0)
						&& (dqwbd_tr_cur < bl_iprd)) {
				dqwbd_tr_cur++;
				set_bdlr_bd_field(block, bl_index, bit_index,
							nibble, dqwbd_tr_cur);

				if (get_debug()) {
					printf("Debug print: Run left bist, ");
					printf("byte %u, bit %u, wbd = %u\n",
					bl_index, bit_index, dqwbd_tr_cur);
				}
				if (get_debug())
					printf("Debug: mc_mask = 0x%08X\n",
								mc_mask);
				status = phy_bist_run(mc_mask, bist_results);
				if(status)
					return;
				else if(bist_results[block_idx] != 0) {
					if (get_debug())
						printf("Debug print: phy_bist_run %s\n",
							(bist_results[block_idx]) ? "failed":"passed");
				}
			}

			res_wbd[res_index][LEFT_EDGE] = dqwbd_tr_cur;

			/* 18) */
			if (get_debug())
				printf("Step 18\n");

			set_bdlr_bd_field(block, bl_index, bit_index,
							nibble, dqwbd_tr_res);
			/* 19) */
			/* 20) */
			if (get_debug())
				printf("Step 19+20\n");
		}
	}

	/* 21) */
	if (get_debug())
		printf("Step 21\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 0;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 22) */
	if (get_debug())
		printf("Step 22\n");
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_BISTRR_REG_ADDR, PUB_BISTRR_REG_HW_DEF_VAL);

	/* 23) */
	if (get_debug())
		printf("Step 23\n");

	if (print_enable) {
		printf("\nShow WRITE skew margin for DDR PHY UM%u_%u_:\n\n",
				phy_side, phy_num);
		printf("status = %s\n", ((ret_status) ? "FAIL" : "PASS"));

		printf("%-10s %-18s %-18s %-18s %-18s\n",
				"bit_entry",
				"TR_VAL",
				"RIGHT_EDGE",
				"LEFT_EDGE",
				"BL_TAP_PS");

		for (eindex = 0; eindex < BIT_ENTRIES_NUM; eindex++) {
			printf("%-10u %-18u %-18u %-18u %u.%03u\n",
					eindex,
					res_wbd[eindex][TR_VAL],
					res_wbd[eindex][RIGHT_EDGE],
					res_wbd[eindex][LEFT_EDGE],
					(res_wbd[eindex][BL_TAP_PS]/1000),
					(res_wbd[eindex][BL_TAP_PS]%1000));
		}
		printf("\n");
	}
}

static void show_read_skew_margin(int block, bool print_enable)
{
	u32 bl_index, bl_iprd, bl_tap_fantaSec, tck_index, rank_index;
	u32 bit_index, nibble, res_index, dqrbd_tr_res, dqrbd_tr_cur, eindex;
	u32 phy_side, phy_num, block_idx, mc_mask;
	u32 bist_results[EMEM_MC_NUM_OF_BLOCKS];
	u32 ret_status = 0;
	int status;

	union pub_pgcr6 pgcr6;
	union pub_dx_x_mdlr0 dx_x_mdlr0;
	union pub_bistrr bistrr;
	union pub_bistar1 bistar1;

	phy_side = (block >= emem_mc_block_id[6]) ? 1 : 0;
	phy_num = (block >= emem_mc_block_id[6]) ?
			(block - emem_mc_block_id[6]) :
			(block - emem_mc_block_id[0]);
	block_idx = (phy_side * 6) + phy_num;
	mc_mask = (1 << block_idx);

	/* 1) */
	if (get_debug())
		printf("Step 1\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 1;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 2) */
	if (get_debug())
		printf("Step 2\n");
	if (get_debug())
		printf("Debug print: Call phy_bist_setup, mc_mask = 0x%08X\n",
				mc_mask);
	phy_bist_setup(mc_mask);

	/* 3) */
	if (get_debug())
		printf("Step 3\n");
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		dx_x_mdlr0.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_mdlr0_addr[bl_index]);

		if (g_temp_try) {
			dx_x_mdlr0.fields.iprd = 8;
			printf("temp_try print: temp_try mode, ");
			printf("dx_x_mdlr0.fields.iprd = %d\n",
						dx_x_mdlr0.fields.iprd);
		}

		bl_iprd = dx_x_mdlr0.fields.iprd;

		/* 5) */
		if (get_debug())
			printf("Step 5\n");
		tck_index = current_ddr_params.pll_freq;
		bl_tap_fantaSec = (tCK2[tck_index] / (2 * bl_iprd));

		if (get_debug()) {
			printf("Debug print: bl_tap_fantaSec =");
			printf(" tCK2 /(2 * bl_iprd) =");
			printf("%u / (2 * %u) = %u\n",
					tCK2[tck_index], bl_iprd, bl_tap_fantaSec);
		}
		/* 6) */

		if (get_debug())
			printf("Step 6\n");
		bistrr.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTRR_REG_ADDR);
		bistrr.fields.bdxsel = bl_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTRR_REG_ADDR, bistrr.reg);

		/* 7) */
		if (get_debug())
			printf("Step 7\n");
		rank_index = (bl_index / 2);

		/* 8) */

		if (get_debug())
			printf("Step 8\n");
		bistar1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTAR1_REG_ADDR);
		bistar1.fields.brank = rank_index;
		bistar1.fields.bmrank = rank_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTAR1_REG_ADDR, bistar1.reg);


		/* 9) */
		if (get_debug())
			printf("Step 9\n");
		for (bit_index = 0; bit_index < 8; bit_index++) {

			/* 10) */
			if (get_debug())
				printf("Step 10\n");
			nibble = 3 + (bit_index / 4); /* can be 3 or 4 */

			/* 11) */
			if (get_debug())
				printf("Step 11\n");
			res_index = (8 * bl_index) + bit_index;

			/* 12) */
			if (get_debug())
				printf("Step 12\n");

			dqrbd_tr_res = get_bdlr_bd_field(block,
					bl_index, bit_index, nibble);

			if (g_temp_try) {
				dqrbd_tr_res = 3;
				printf("temp_try print: temp_try mode, ");
				printf("dqrbd_tr_res = %d\n", dqrbd_tr_res);
			}

			res_rbd[res_index][TR_VAL] = dqrbd_tr_res;
			res_rbd[res_index][BL_TAP_PS] = bl_tap_fantaSec;

			/* 13) */
			/* 14) */

			/* NOTE: store dqrbd_tr_res value in dqrbd_tr_cur */
			if (get_debug())
				printf("Step 13+14\n");

			dqrbd_tr_cur = dqrbd_tr_res;

			if (get_debug()) {
				printf("Debug print: Run right bist, ");
				printf("byte %u, bit %u, wbd = %u\n",
					bl_index, bit_index, dqrbd_tr_cur);
			}
			if (get_debug())
				printf("Debug: mc_mask = 0x%08X\n", mc_mask);
			status = phy_bist_run(mc_mask, bist_results);
			if(status)
				return;
			else if(bist_results[block_idx] != 0) {
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");
			}
			while ((bist_results[block_idx] == 0) &&
						(dqrbd_tr_cur > 0)) {
				dqrbd_tr_cur--;
				set_bdlr_bd_field(block, bl_index, bit_index,
							nibble, dqrbd_tr_cur);

				if (get_debug()) {
					printf("Debug print: Run right bist, ");
					printf("byte %u, bit %u, wbd = %u\n",
						bl_index, bit_index, dqrbd_tr_cur);
				}
				if (get_debug())
					printf("Debug: mc_mask = 0x%08X\n",
								mc_mask);
				status = phy_bist_run(mc_mask, bist_results);
				if(status)
					return;
				else if(bist_results[block_idx] != 0) {
					if (get_debug())
						printf("Debug print: phy_bist_run %s\n",
							(bist_results[block_idx]) ? "failed":"passed");
				}
			}

			res_rbd[res_index][RIGHT_EDGE] = dqrbd_tr_cur;

			/* 15) */
			if (get_debug())
				printf("Step 15\n");

			set_bdlr_bd_field(block, bl_index,
					bit_index, nibble, dqrbd_tr_res);

			/* 16) */
			/* 17) */
			/* NOTE: store dqrbd_tr_res value in dqrbd_tr_cur */
			if (get_debug())
				printf("Step 16+17\n");

			if (dqrbd_tr_res > bl_iprd) {
				printf("ERROR: dqrbd_tr_res %u > bl_iprd %u\n",
						dqrbd_tr_res, bl_iprd);
				ret_status = 1;
			}

			dqrbd_tr_cur = dqrbd_tr_res;

			if (get_debug()) {
				printf("Debug print: Run left bist, ");
				printf("byte %u, bit %u, wbd = %u\n",
					bl_index, bit_index, dqrbd_tr_cur);
			}
			if (get_debug())
				printf("Debug print: mc_mask = 0x%08X\n",
								mc_mask);
			status = phy_bist_run(mc_mask, bist_results);
			if(status)
				return;
			else if(bist_results[block_idx] != 0) {
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");
			}

			while ((bist_results[block_idx] == 0) &&
					(dqrbd_tr_cur < bl_iprd)) {
				dqrbd_tr_cur++;
				set_bdlr_bd_field(block, bl_index,
					bit_index, nibble, dqrbd_tr_cur);

				if (get_debug()) {
					printf("Debug print: Run left bist, ");
					printf("byte %u, bit %u, wbd = %u\n",
						bl_index, bit_index, dqrbd_tr_cur);
				}
				if (get_debug())
					printf("Debug: mc_mask = 0x%08X\n",
								mc_mask);
				status = phy_bist_run(mc_mask, bist_results);
				if(status)
					return;
				else if(bist_results[block_idx] != 0) {
					if (get_debug())
						printf("Debug print: phy_bist_run %s\n",
							(bist_results[block_idx]) ? "failed":"passed");
				}
			}
			res_rbd[res_index][LEFT_EDGE] = dqrbd_tr_cur;
			/* 18) */
			if (get_debug())
				printf("Step 18\n");

			set_bdlr_bd_field(block, bl_index,
					bit_index, nibble, dqrbd_tr_res);

			/* 19) */
			/* 20) */
			if (get_debug())
				printf("Step 19+20\n");
		}
	}

	/* 21) */
	if (get_debug())
		printf("Step 21\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 0;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 22) */
	if (get_debug())
		printf("Step 22\n");
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_BISTRR_REG_ADDR, PUB_BISTRR_REG_HW_DEF_VAL);

	/* 23) */
	if (get_debug())
		printf("Step 23\n");

	if (print_enable) {
		printf("\nShow READ skew margin for DDR PHY UM%u_%u:\n\n",
				phy_side, phy_num);
		printf("status = %s\n", ((ret_status) ? "FAIL" : "PASS"));

		printf("%-10s %-18s %-18s %-18s %-18s\n",
				"bit_entry",
				"TR_VAL",
				"RIGHT_EDGE",
				"LEFT_EDGE",
				"BL_TAP_PS");

		for (eindex = 0; eindex < BIT_ENTRIES_NUM; eindex++) {
			printf("%-10u %-18u %-18u %-18u %u.%03u\n",
					eindex,
					res_rbd[eindex][TR_VAL],
					res_rbd[eindex][RIGHT_EDGE],
					res_rbd[eindex][LEFT_EDGE],
					(res_rbd[eindex][BL_TAP_PS]/1000),
					(res_rbd[eindex][BL_TAP_PS]%1000));
		}
		printf("\n");
	}
}

int phy_bist_run(u32 mc_mask, u32 *results)
{
	union pub_pgcr0 pgcr0;
	union pub_bistrr bistrr;
	union pub_bistgsr bistgsr;
	const u32 seed = 0x1234ABCD;
	u32 ifc, retries;

	for (ifc = 0; ifc < EMEM_MC_NUM_OF_BLOCKS; ifc++) {
		if (GET_BIT(mc_mask, ifc) == 0)
			continue;
		bistrr.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[ifc], PUB_BISTRR_REG_ADDR);
		bistrr.fields.binst = 0x2;
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
					PUB_BISTRR_REG_ADDR, bistrr.reg);
		udelay(5);
		bistrr.fields.binst = 0x3;
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
					PUB_BISTRR_REG_ADDR, bistrr.reg);
		pgcr0.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[ifc], PUB_PGCR0_REG_ADDR);
		pgcr0.fields.phyfrst = 0;
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
					PUB_PGCR0_REG_ADDR, pgcr0.reg);
		udelay(5);
		pgcr0.fields.phyfrst = 1;
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
					PUB_PGCR0_REG_ADDR, pgcr0.reg);
		udelay(5);
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
						PUB_BISTLSR_REG_ADDR, seed);
		bistrr.fields.binst = 0x1;
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
					PUB_BISTRR_REG_ADDR, bistrr.reg);
		for (retries = 0; retries < INDIRECT_RETRY_NUM; retries++) {
			udelay(1000);
			bistgsr.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[ifc], PUB_BISTGSR_REG_ADDR);
			if (bistgsr.fields.bdone == 1)
				break;
		}
		if (retries == INDIRECT_RETRY_NUM && !g_EZsim_mode) {
			error("phy_bist_run : retries exceeded (%d)\n", ifc);
			return -1;
		}
		bistrr.fields.binst = 0x2;
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
					PUB_BISTRR_REG_ADDR, bistrr.reg);

		bistgsr.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[ifc], PUB_BISTGSR_REG_ADDR);
		results[ifc] = bistgsr.fields.bdxerr;
	}

	return 0;
}

void phy_bist_setup(u32 mc_mask)
{
	union pub_bistrr bistrr;
	union pub_bistar1 bistar1;
	union pub_bistar2 bistar2;
	u32 ifc;

	for (ifc = 0; ifc < EMEM_MC_NUM_OF_BLOCKS; ifc++) {
		if (GET_BIT(mc_mask, ifc) == 0)
			continue;
		bistrr.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[ifc], PUB_BISTRR_REG_ADDR);
		bistrr.fields.bmode = 0x1;
		bistrr.fields.bdpat = 0x2;
		bistrr.fields.bdxen = 0x1;
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
					PUB_BISTRR_REG_ADDR, bistrr.reg);

		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
						PUB_BISTWCR_REG_ADDR, 0x20);

		bistar1.reg = emem_mc_indirect_reg_read_synop(
						emem_mc_block_id[ifc],
						PUB_BISTAR1_REG_ADDR);
		bistar1.fields.bainc = 0x8;
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
					PUB_BISTAR1_REG_ADDR, bistar1.reg);

		bistar2.fields.bmbank = GET_NUMBER_OF_BANKS(current_ddr_params.type) - 1;
		bistar2.fields.bmcol = 0x3FF;
		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
					PUB_BISTAR2_REG_ADDR, bistar2.reg);

		emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[ifc],
				PUB_BISTAR4_REG_ADDR, get_max_row_num());
	}
}

/* if phy_table = 0 -> lookup in vref_dq_2
 * if phy_table = 1 -> lookup in phy_rd_vref_dq */
int find_vref_dq(u32 vref_prec, u32 phy_table)
{
	u32 vref_idx;
	u32 table_size, value, prev_value;

	table_size = (phy_table) ? 64 : 51;

	for (vref_idx = 0; vref_idx < table_size; vref_idx++) {

		value = (phy_table) ? phy_rd_vref_dq[vref_idx] : vref_dq_2[vref_idx];
		if (vref_prec < value)
			break;
	}

	/* prec < table[0] case  */
	if (vref_idx == 0)
		return vref_idx;

	/* prec > table[table_size] case  */
	if (vref_idx == table_size)
		return (vref_idx - 1);

	/* common case */
	prev_value = (phy_table) ?
		phy_rd_vref_dq[vref_idx - 1] : vref_dq_2[vref_idx - 1];
	if ((vref_prec - prev_value) < (value - vref_prec))
		vref_idx--;

	return vref_idx;
}

void ato_probing(u32 um, u32 ato_msel, u32 ato_isel)
{
	u32 block = um;
	union pub_pllcr pllcr;
	union pub_dsgcr dsgcr;

	pllcr.reg = emem_mc_indirect_reg_read_synop(
							emem_mc_block_id[block], PUB_PLLCR_REG_ADDR);
	pllcr.fields.atoen = ato_msel;
	pllcr.fields.atc = ato_isel;
	emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[block],
											PUB_PLLCR_REG_ADDR, pllcr.reg);

	dsgcr.reg = emem_mc_indirect_reg_read_synop(
							emem_mc_block_id[block], PUB_DSGCR_REG_ADDR);
	dsgcr.fields.atoae = 1;
	emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[block],
											PUB_DSGCR_REG_ADDR, dsgcr.reg);
}

void dto_probing(u32 um, u32 dto_msel, u32 dto_isel)
{
	u32 block = um;
	union pub_pgcr0 pgcr0;
	union pub_pgcr1 pgcr1;
	union pub_pgcr7 pgcr7;
	union pub_pllcr pllcr;
	union pub_dsgcr dsgcr;

	/* ===== 1 ===== */
	pgcr0.reg = emem_mc_indirect_reg_read_synop(
							emem_mc_block_id[block], PUB_PGCR0_REG_ADDR);
	pgcr0.fields.dtosel = dto_msel;
	emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[block],
											PUB_PGCR0_REG_ADDR, pgcr0.reg);
	pgcr1.reg = emem_mc_indirect_reg_read_synop(
							emem_mc_block_id[block], PUB_PGCR1_REG_ADDR);
	pgcr1.fields.dtomode = GET_BIT(dto_isel,3);
	emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[block],
													PUB_PGCR1_REG_ADDR, pgcr1.reg);
	pgcr7.reg = emem_mc_indirect_reg_read_synop(
							emem_mc_block_id[block], PUB_PGCR7_REG_ADDR);
	pgcr7.fields.dxdtosel = GET_BITS(dto_isel, 0, 2);
	pgcr7.fields.acdtosel = GET_BIT(dto_isel, 0);
	emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[block],
													PUB_PGCR7_REG_ADDR, pgcr7.reg);
	pllcr.reg = emem_mc_indirect_reg_read_synop(
								emem_mc_block_id[block], PUB_PLLCR_REG_ADDR);
	pllcr.fields.dtc = GET_BITS(dto_isel, 0, 3);
	emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[block],
												PUB_PLLCR_REG_ADDR, pllcr.reg);
	/* ===== 2 ===== */
	emem_mc_indirect_reg_write_synop_data0(gpr[block].low_block,
											PUB_GPR0_REG_ADDR, gpr[block].low_data_0);

	emem_mc_indirect_reg_write_synop_data0(gpr[block].low_block,
											PUB_GPR1_REG_ADDR, gpr[block].low_data_1);

	emem_mc_indirect_reg_write_synop_data0(gpr[block].high_block,
											PUB_GPR0_REG_ADDR, gpr[block].high_data_0);

	emem_mc_indirect_reg_write_synop_data0(gpr[block].low_block,
											PUB_GPR1_REG_ADDR, gpr[block].high_data_1);
	/* ===== 3 ===== */
	if(block < (EMEM_MC_NUM_OF_BLOCKS / 2)) {
		dsgcr.reg = emem_mc_indirect_reg_read_synop(EMEM_MC_2_ADDR,
								PUB_DSGCR_REG_ADDR);
		dsgcr.fields.dtopdd = 0;
		dsgcr.fields.dtooe = 1;

		emem_mc_indirect_reg_write_synop_data0(EMEM_MC_2_ADDR,
							PUB_DSGCR_REG_ADDR, dsgcr.reg);
	} else {
		dsgcr.reg = emem_mc_indirect_reg_read_synop(EMEM_MC_8_ADDR,
								PUB_DSGCR_REG_ADDR);
		dsgcr.fields.dtopdd = 0;
		dsgcr.fields.dtooe = 1;
		emem_mc_indirect_reg_write_synop_data0(EMEM_MC_8_ADDR,
						PUB_DSGCR_REG_ADDR, dsgcr.reg);
	}
}


int phy_vref_prob(u32 bl_index,u32 side,u32 num)
{
	u32 block;
	u32 addr;
	union pub_iovcr0 iovcr0;
	union pub_dx_n_gcr4 dx_n_gcr4;

	block = ((EMEM_MC_NUM_OF_BLOCKS / 2) * side) + num;
	/* 1)	Set a value of 0x0 to the ACREFPEN field of the IOVCR0 PUB register.*/
	iovcr0.reg = emem_mc_indirect_reg_read_synop(emem_mc_block_id[block],
							PUB_IOVCR0_REG_ADDR);
	iovcr0.fields.acrefpen = 0;
	emem_mc_indirect_reg_write_synop_data0(emem_mc_block_id[block],
					PUB_IOVCR0_REG_ADDR, iovcr0.reg);

	/* 2)	Set a value of 0x0 to the DXREFPEN field of the DX0GCR4 PUB register. */
	dx_n_gcr4.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[block],
				PUB_DX0GCR4_REG_ADDR);
	dx_n_gcr4.fields.dxrefpen = 0;
	emem_mc_indirect_reg_write_synop_data0(
		emem_mc_block_id[block],
		PUB_DX0GCR4_REG_ADDR,
		dx_n_gcr4.reg);


	/* 3)	Set a value of 0x0 to the DXREFPEN field of the DX1GCR4 PUB register. */
	dx_n_gcr4.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[block],
				PUB_DX1GCR4_REG_ADDR);
	dx_n_gcr4.fields.dxrefpen = 0;
	emem_mc_indirect_reg_write_synop_data0(
		emem_mc_block_id[block],
		PUB_DX1GCR4_REG_ADDR,
		dx_n_gcr4.reg);



	/* 4)	Set a value of 0x0 to the DXREFPEN field of the DX2GCR4 PUB register. */
	dx_n_gcr4.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[block],
				PUB_DX2GCR4_REG_ADDR);
	dx_n_gcr4.fields.dxrefpen = 0;
	emem_mc_indirect_reg_write_synop_data0(
		emem_mc_block_id[block],
		PUB_DX2GCR4_REG_ADDR,
		dx_n_gcr4.reg);


	/* 5)	Set a value of 0x0 to the DXREFPEN field of the DX3GCR4 PUB register. */
	dx_n_gcr4.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[block],
				PUB_DX3GCR4_REG_ADDR);
	dx_n_gcr4.fields.dxrefpen = 0;
	emem_mc_indirect_reg_write_synop_data0(
		emem_mc_block_id[block],
		PUB_DX3GCR4_REG_ADDR,
		dx_n_gcr4.reg);



	/* 6)	Wait for a minimum time of 0.5 uSec.*/
	udelay(1);

	/*7)	Set a value of 0x1 to the DXREFPEN field of the DX<bl_index>GCR4 PUB register.*/
	switch (bl_index) {
	case 0:
		addr = PUB_DX0GCR4_REG_ADDR;
		break;
	case 1:
		addr = PUB_DX1GCR4_REG_ADDR;
		break;
	case 2:
		addr = PUB_DX2GCR4_REG_ADDR;
		break;
	case 3:
		addr = PUB_DX3GCR4_REG_ADDR;
		break;
	default:
		printf("ERROR: wrong bl_index value %d\n", bl_index);
		return -1;
	}

	dx_n_gcr4.reg = emem_mc_indirect_reg_read_synop(
				emem_mc_block_id[block],
				addr);
	dx_n_gcr4.fields.dxrefpen = 1;
	emem_mc_indirect_reg_write_synop_data0(
		emem_mc_block_id[block],
		addr,
		dx_n_gcr4.reg);

	return 0;
}


/*******************
 * debug commands  *
*******************/

int do_edit_results(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 block, block_idx, reg_data, wdtap_flag;
	u32 side = 0, num = 0, bl_index, bit_index, nibble;
	int tap_delay, tr_res, bl_iprd;
	int status = 0;

	union pub_dx_x_lcdlr1 dx_x_lcdlr1;
	union pub_dx_x_lcdlr3 dx_x_lcdlr3;
	union pub_dx_x_lcdlr4 dx_x_lcdlr4;

	union pub_dx_x_mdlr0 dx_x_mdlr0;

	status = parse_UM_port_format(argv[1], &side, &num);
	if (status)
		return status;

	/* get bl_index parametr */
	bl_index = (u32)simple_strtol(argv[2], NULL, 10);
	if (bl_index > 3) {
		printf("ERROR: Wrong bl_index value %u, Should be 0-3\n", bl_index);
		return 1;
	}

	block_idx = (side * 6) + num;
	block = emem_mc_block_id[block_idx];

	if (!strcmp("edit_wctap", argv[0])) {

		/* get tap_delay parametr */
		tap_delay = (int)simple_strtol(argv[3], NULL, 10);

		/* 1) */
		if (get_debug())
			printf("Step 1\n");
		reg_data = read_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF);

		/* 2) */
		if (get_debug())
			printf("Step 2\n");
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, 0x0);

		/* 3) */
		if (get_debug())
			printf("Step 3\n");
		dx_x_mdlr0.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_mdlr0_addr[bl_index]);
		if (g_temp_try) {
			dx_x_mdlr0.fields.iprd = 8;
			printf("temp_try print: temp_try mode, ");
			printf("dx_x_mdlr0.fields.iprd = %d\n",
						dx_x_mdlr0.fields.iprd);
		}
		bl_iprd = dx_x_mdlr0.fields.iprd;

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		dx_x_lcdlr1.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr1_addr[bl_index]);

		if (g_temp_try) {
			dx_x_lcdlr1.fields.wdqd = 2;
			printf("temp_try print: temp_try mode ,");
			printf("dx_x_lcdlr1.fields.wdqd = %d\n",
						dx_x_lcdlr1.fields.wdqd);
		}

		tr_res = dx_x_lcdlr1.fields.wdqd;

		if ((tr_res + tap_delay) > bl_iprd) {
			printf("WARNING: (tr_res %d + tap_delay %d) > bl_iprd %d\n",
					tr_res, tap_delay, bl_iprd);
			tr_res = bl_iprd;
			status = WARNING_RET_VAL;

		} else if ( (tr_res + tap_delay) < 0) {
			printf("WARNING: (tr_res %d + tap_delay %d) < 0\n",
					tr_res, tap_delay);
			tr_res = 0;
			status = WARNING_RET_VAL;
		} else {
			tr_res = tr_res + tap_delay;
		}
		printf("Set new tr_res = %d\n", tr_res);

		dx_x_lcdlr1.fields.wdqd = tr_res;
		emem_mc_indirect_reg_write_synop_data0(block,
				pub_dx_x_lcdlr1_addr[bl_index], dx_x_lcdlr1.reg);

		/* 5) */
		if (get_debug())
			printf("Step 5\n");
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, reg_data);

	} else if (!strcmp("edit_rctap", argv[0])) {

		/* get tap_delay parametr */
		tap_delay = (int)simple_strtol(argv[3], NULL, 10);

		/* 1) */
		if (get_debug())
			printf("Step 1\n");
		reg_data = read_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF);

		/* 2) */
		if (get_debug())
			printf("Step 2\n");
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, 0x0);

		/* 3) */
		if (get_debug())
			printf("Step 3\n");
		dx_x_mdlr0.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_mdlr0_addr[bl_index]);
		if (g_temp_try) {
			dx_x_mdlr0.fields.iprd = 8;
			printf("temp_try print: temp_try mode, ");
			printf("dx_x_mdlr0.fields.iprd = %d\n",
						dx_x_mdlr0.fields.iprd);
		}
		bl_iprd = dx_x_mdlr0.fields.iprd;

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		dx_x_lcdlr3.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr3_addr[bl_index]);

		if (g_temp_try) {
			dx_x_lcdlr3.fields.rdqsd = 4;
			printf("temp_try print: temp_try mode ,");
			printf("dx_x_lcdlr3.fields.rdqsd = %d\n",
						dx_x_lcdlr3.fields.rdqsd);
		}
		tr_res = dx_x_lcdlr3.fields.rdqsd;

		if ((tr_res + tap_delay) > bl_iprd) {
			printf("WARNING: (tr_res %d + tap_delay %d) > bl_iprd %d\n",
					tr_res, tap_delay, bl_iprd);
			tr_res = bl_iprd;
			status = WARNING_RET_VAL;

		} else if ((tr_res + tap_delay) < 0) {
			printf("WARNING: (tr_res %d + tap_delay %d) < 0\n",
					tr_res, tap_delay);
			tr_res = 0;
			status = WARNING_RET_VAL;

		} else {
			tr_res = tr_res + tap_delay;
		}
		printf("Set new tr_res = %d\n", tr_res);

		dx_x_lcdlr3.fields.rdqsd = tr_res;
		emem_mc_indirect_reg_write_synop_data0(block,
				pub_dx_x_lcdlr3_addr[bl_index], dx_x_lcdlr3.reg);


		/* 5) */
		if (get_debug())
			printf("Step 5\n");
		dx_x_lcdlr4.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr4_addr[bl_index]);

		if (g_temp_try) {
			dx_x_lcdlr4.fields.rdqsnd = 6;
			printf("temp_try print: temp_try mode ,");
			printf("dx_x_lcdlr4.fields.rdqsnd = %d\n",
						dx_x_lcdlr4.fields.rdqsnd);
		}
		tr_res = dx_x_lcdlr4.fields.rdqsnd;

		if ((tr_res + tap_delay) > bl_iprd) {
			printf("WARNING: (tr_res %d + tap_delay %d) > bl_iprd %d\n",
					tr_res, tap_delay, bl_iprd);
			tr_res = bl_iprd;
			status = WARNING_RET_VAL;
		} else if ((tr_res + tap_delay) < 0) {
			printf("WARNING: (tr_res %d + tap_delay %d) < 0\n",
					tr_res, tap_delay);
			tr_res = 0;
			status = WARNING_RET_VAL;
		} else {
			tr_res = tr_res + tap_delay;
		}
		printf("Set new tr_res = %d\n", tr_res);

		dx_x_lcdlr4.fields.rdqsnd = tr_res;
		emem_mc_indirect_reg_write_synop_data0(
					block, pub_dx_x_lcdlr4_addr[bl_index],
					dx_x_lcdlr4.reg);

		/* 6) */
		if (get_debug())
			printf("Step 6\n");
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, reg_data);

	} else {
		/* "edit_wdtap" and "edit_rdtap" */
		wdtap_flag = (!strcmp("edit_wdtap", argv[0]));

		/* get bit_index parameter */
		bit_index = (int)simple_strtol(argv[3], NULL, 10);
		if (bit_index > 7) {
			printf("ERROR: Wrong bit_index value %u, Should be 0-7\n", bit_index);
			return 1;
		}

		/* get tap_delay parameter */
		tap_delay = (int)simple_strtol(argv[4], NULL, 10);

		/* 1) */
		if (get_debug())
			printf("Step 1\n");
		reg_data = read_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF);

		/* 2) */
		if (get_debug())
			printf("Step 2\n");
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, 0x0);

		/* 3) */
		if (get_debug())
			printf("Step 3\n");
		if (wdtap_flag)
			nibble = (bit_index / 4); /* can be 0 or 1 */
		else
			nibble = 3 + (bit_index / 4); /* can be 3 or 4 */

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		tr_res = get_bdlr_bd_field(block, bl_index, bit_index, nibble);
		if (g_temp_try) {
			tr_res = 3;
			printf("temp_try print: temp_try mode, tr_res = %d\n", tr_res);
		}

		if ((tr_res + tap_delay) > 0x3F) {
			printf("(WARNING: tr_res %d + tap_delay %d) > 0x3F\n", tr_res, tap_delay);
			tr_res = 0x3F;
			status = WARNING_RET_VAL;

		} else if ((tr_res + tap_delay) < 0) {
			printf("WARNING: (tr_res %d + tap_delay %d) < 0\n", tr_res, tap_delay);
			tr_res = 0;
			status = WARNING_RET_VAL;

		} else {
			tr_res = tr_res + tap_delay;
		}
		printf("Set new tr_res = %d\n", tr_res);

		set_bdlr_bd_field(block, bl_index, bit_index, nibble, tr_res);

		/* 5) */
		if (get_debug())
			printf("Step 5\n");
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, reg_data);
	}

	return status;
}

int do_vref(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 block, block_idx, reg_data;
	u32 side = 0, num = 0, bl_index;
	u32 vref_val, sdram_table_entry, sdram_vref_val, phy_table;
	u32 mc_id, mr6_temp, mr_data_0, mr_data_1;
	u32 host_table_entry, vrefdq_isel_val,sdram_vref_rng, sdram_vref_data;
	int status = 0;

	union pub_dx_n_gcr5 dx_x_gcr5;

	status = parse_UM_port_format(argv[1], &side, &num);
	if (status)
		return status;

	/* get bl_index parameter */
	bl_index = (u32)simple_strtol(argv[2], NULL, 10);
	if (bl_index > 3) {
		printf("ERROR: Wrong bl_index value %u, Should be 0-3\n", bl_index);
		return 1;
	}

	/* get vref_val parameter */
	vref_val = (u32)simple_strtol(argv[3], NULL, 10);

	block_idx = (side * 6) + num;
	block = emem_mc_block_id[block_idx];

	if (!strcmp("sdram_vref", argv[0])) {

		phy_table = 0;

		/* 1) */
		if (get_debug())
			printf("Step 1\n");
		reg_data = read_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF);

		/* 2) */
		if (get_debug())
			printf("Step 2\n");
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, 0x0);

		/* 3) */
		if (get_debug())
			printf("Step 3\n");
		mc_id = bl_index / 2;

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		/* multiply by 100 to emulate percentage,
		 * and multiply by 100 again, to fit vref_dq_2[] tables */
		sdram_table_entry = ( vref_val * 100 * 100 / 1200 );

		/* 5) */
		if (get_debug())
			printf("Step 5\n");

		/*sdram_vref_val = find_vref_dq(sdram_table_entry, phy_table);*/
		sdram_vref_rng = get_optimal_vref_index(vref_dq_1, vref_dq_2,
					ARRAY_SIZE(vref_dq_1),
					ARRAY_SIZE(vref_dq_2),
					sdram_table_entry,
					&sdram_vref_val);
		if (get_debug())
			printf("Debug print: sdram_vref_val = %d\n", sdram_vref_val);

		/* 6 */
		if (get_debug())
			printf("Step 6\n");
		if (get_debug())
			printf("Debug print: old MR6_DATA = 0x%08X\n", mr6.reg);
		sdram_vref_data = (0x40 * sdram_vref_rng) + sdram_vref_val;
		mr6_temp = (mr6.reg & 0xFF00) + 0x0080 + sdram_vref_data;
		if (get_debug())
			printf("Debug print: MR6_TEMP = 0x%08X\n", mr6_temp);
		/* 7 */
		if (get_debug())
			printf("Step 7\n");
		mr_data_0 = (MR_CMD << MR_CMD_OFFSET) + (6 << 17) + mr6_temp;
		mr_data_1 = 0x21004;	/* bit 17 = last */
		emem_mc_indirect_reg_write_mrs( block, mc_id, mr_data_0, mr_data_1);

		/* 8) */
		if (get_debug())
			printf("Step 8\n");
		udelay(1);

		/* 9 */
		if (get_debug())
			printf("Step 9\n");
		mr6_temp = (mr6.reg & 0xFF00) + sdram_vref_data;
		if (get_debug())
			printf("Debug print: scnd MR6_TEMP = 0x%08X\n", mr6_temp);

		/* 10) */
		if (get_debug())
			printf("Step 10\n");
		mr_data_0 = (MR_CMD << MR_CMD_OFFSET) + (6 << 17) + mr6_temp;
		mr_data_1 = 0x21004;	/* bit 17 = last */
		emem_mc_indirect_reg_write_mrs( block, mc_id, mr_data_0, mr_data_1);

		/* 11) */
		if (get_debug())
			printf("Step 11\n");
		udelay(1);

		/* 12) */
		if (get_debug())
			printf("Step 12\n");
		mr6_rank_vref[block_idx][mc_id].reg =
				(mr6_rank_vref[block_idx][mc_id].reg & 0xFF00) + sdram_vref_data;
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, reg_data);

	} else {

		phy_table = 1;

		/* 1) */
		if (get_debug())
			printf("Step 1\n");
		reg_data = read_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF);

		/* 2) */
		if (get_debug())
			printf("Step 2\n");
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, 0x0);

		/* 3) */
		if (get_debug())
			printf("Step 3\n");
		/*rank_index = bl_index / 2;*/

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		/* multiply by 100 to emulate percentage,
		 * and multiply by 100 again, to fit vref_dq_2[] tables */
		host_table_entry = ( vref_val * 100 * 100 / 1200);

		/* 5) */
		if (get_debug())
			printf("Step 5\n");
		vrefdq_isel_val = find_vref_dq(host_table_entry, phy_table);
		if (get_debug())
			printf("Debug print: vrefdq_isel_val = %d\n", vrefdq_isel_val);

		/* 6) */
		if (get_debug())
			printf("Step 6 - removed\n");

		/* 7) */
		if (get_debug())
			printf("Step 7\n");
		dx_x_gcr5.reg = emem_mc_indirect_reg_read_synop(block,
				pub_dx_x_gcr5_addr[bl_index]);
		dx_x_gcr5.fields.dxrefiselr1 = vrefdq_isel_val;
		dx_x_gcr5.fields.dxrefiselr0 = vrefdq_isel_val;
		emem_mc_indirect_reg_write_synop_data0(block,
					pub_dx_x_gcr5_addr[bl_index], dx_x_gcr5.reg);

		/* 8) */
		if (get_debug())
			printf("Step 8\n");
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, reg_data);


	}

	return status;
}

int do_marg_and_d2(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 block, block_idx, start_block_idx, end_block_idx, reg_data;
	u32 side = 0, num = 0;
	u32 all = 0;
	int status = 0;

	if (!strcmp(argv[1], "ALL"))
		all = 1;
	else {
		status = parse_UM_port_format(argv[1], &side, &num);
		if (status)
			return status;
	}

	if (all) {
		start_block_idx = 0;
		end_block_idx = 11;
	} else {
		start_block_idx = (side * 6) + num;
		end_block_idx = start_block_idx;
	}

	for (block_idx = start_block_idx; block_idx <= end_block_idx;
								block_idx++) {

		block = emem_mc_block_id[block_idx];
		/* 3)	Before running a specific DDR PHY Interface,
		 *		the EMEM_MC main enable register MC_DDR_IF
		 *		(Register Address 0x116) should be saved,
		 *		and the register enables should be cleared. */
		reg_data = read_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF);
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, 0x0);

		if (!strcmp("show_wcm", argv[0]))
			show_write_centralization_margin(block);
		else if (!strcmp("show_rcm", argv[0]))
			show_read_centralization_margin(block);
		else if (!strcmp("show_wsm", argv[0]))
			show_write_skew_margin(block, true);
		else if (!strcmp("show_rsm", argv[0]))
			show_read_skew_margin(block, true);
		else if (!strcmp("write_2d", argv[0]))
			d2_write_eye_plotting(block, true);
		else
			d2_read_eye_plotting(block, true);

		/* 4)	After running a specific DDR PHY Interface,
		 *	the EMEM_MC main enable register MC_DDR_IF
		 *	(Register Address 0x116) data should be recovered. */
		write_non_cluster_reg(block, EMEM_MC_REG_MC_DDR_IF, reg_data);
	}

	return status;
}

int do_phy_bist(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 mc_mask;
	u32 bist_results[EMEM_MC_NUM_OF_BLOCKS];

	mc_mask = (u32)simple_strtol(argv[1], NULL, 16);
	phy_bist_setup(mc_mask);
	phy_bist_run(mc_mask, bist_results);

	return 0;
}

int do_ato_probing(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 side, ato_msel, ato_isel, um;

	if (!strcmp(argv[1], "ALL"))
		um = 0x3;
	else if (!memcmp("UM", argv[1], 2)) {
		side = argv[1][2] - '0';
		if (side > 1) {
			printf("ERROR: Wrong <side> value, in the UM<side>");
			printf("parameter. Should be 0-1\n");
			return 1;
		}
		um = ((EMEM_MI_NUM_OF_BLOCKS / 2) * side) + 2;
	} else {
		printf("ERROR: wrong parameters, ");
		printf("should be ALL or UM<side>_<num>\n");
		return 1;
	}
	ato_msel = (u32)simple_strtol(argv[2], NULL, 16);
	ato_isel = (u32)simple_strtol(argv[3], NULL, 16);
	ato_probing(um, ato_msel, ato_isel);

	return 0;
}

int do_dto_probing(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 num, side, dto_msel, dto_isel, um;

	if (!memcmp("UM", argv[1], 2)) {
		side = argv[1][2] - '0';
		num = argv[1][4] - '0';
		if (side > 1) {
			printf("ERROR: Wrong <side> value, in the UM<side>");
			printf("parameter. Should be 0-1\n");
			return 1;
		}
		if (num > 5) {
			printf("ERROR: Wrong <num> value, in the UM<side>_<num> ");
			printf("parameter. Should be 0-5\n");
			return 1;
		}
		um = (side * 6) + num;
	} else {
		printf("ERROR: wrong parameters, ");
		printf("should be ALL or UM<side>_<num>\n");
		return 1;
	}

	dto_msel = (u32)simple_strtol(argv[2], NULL, 16);
	dto_isel = (u32)simple_strtol(argv[3], NULL, 16);
	dto_probing(um, dto_msel, dto_isel);

	return 0;
}

int do_sdram_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 mc_bitmask, bank_group, bank, row, col, size;
	u32 data[32];

	mc_bitmask = simple_strtol(argv[1], NULL, 16);
	bank_group = simple_strtol(argv[2], NULL, 10);
	bank = simple_strtol(argv[3], NULL, 10);
	row = simple_strtol(argv[4], NULL, 10);
	col = simple_strtol(argv[5], NULL, 10);
	size = simple_strtol(argv[6], NULL, 10);

	write_sdram_chunk(mc_bitmask, bank_group, bank, row, col, size, data);
	return 0;
}

int do_sdram_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 mc_bitmask, bank_group, bank, row, col, size;
	u32 data[32];

	mc_bitmask = simple_strtol(argv[1], NULL, 16);
	bank_group = simple_strtol(argv[2], NULL, 10);
	bank = simple_strtol(argv[3], NULL, 10);
	row = simple_strtol(argv[4], NULL, 10);
	col = simple_strtol(argv[5], NULL, 10);
	size = simple_strtol(argv[6], NULL, 10);

	read_sdram_chunk(mc_bitmask, bank_group, bank, row, col, size, data);

	return 0;
}

int parse_UM_port_format(char * const UMstr, u32 *pside, u32 *pnum)
{
	if (!memcmp("UM", UMstr, 2) && (UMstr[3] == '_')) {
		*pside = UMstr[2] - '0';
		*pnum = UMstr[4] - '0';
		if (*pside > 1) {
			printf("ERROR: Wrong <side> value, in the UM<side>_<num> ");
			printf("parameter. Should be 0-1\n");
			return 1;
		}
		if (*pnum > 5) {
			printf("ERROR: Wrong <num> value, in the UM<side>_<num> ");
			printf("parameter. Should be 0-5\n");
			return 1;
		}
	}
	else {
		printf("ERROR: wrong UM<side>_<num> parameter format\n");
		return 1;
	}
	return 0;
}

static void print_reg(char *name, u32 address, u32 val) {
	printf("%-15s 0x%08X     0x%08X\n", name, address, val );
}

void print_pub_dump(u32 block_idx)
{
	u32 block, reg_idx, read_val, register_cnt = 0;

	block = emem_mc_block_id[block_idx];

	for (reg_idx = 0; reg_idx <= (PUB_DDR_PHY_REGS_NUMBER - 1); reg_idx++) {
		while ( PUB_RECORD_GET(block_idx, reg_idx)->address != register_cnt )
			print_reg("Reserved", register_cnt++, 0x0);
		register_cnt++;
		read_val = emem_mc_indirect_reg_read_synop(block,
				PUB_RECORD_GET(block_idx, reg_idx)->address);
		print_reg(PUB_RECORD_GET(block_idx, reg_idx)->name,
				PUB_RECORD_GET(block_idx, reg_idx)->address,
				read_val);
	}
}

int do_ddr_training_steps(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int status;
	u32 ddr_training_pir_val;

	if (!memcmp("0x", argv[1], 2) || !memcmp("0X", argv[1], 2)) {
		ddr_training_pir_val = (u32)simple_strtol(argv[1], NULL, 16);
		set_pir_val(ddr_training_pir_val);
		if (ddr_training_pir_val == INVALID_VAL)
			printf("run ddr_training with default steps set\n");
		else
			printf("run ddr_training with steps set = 0x%08X\n", ddr_training_pir_val);
	}
	else {
		printf("Wrong value format, should be 0x<val>");
		return -1;
	}
	status = ddr_training();
	return status;
}

int do_ddr_basic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 block, block_idx, reg_idx = 0, first_reg_idx = 0, last_reg_idx = 0;
	u32 side = 0, num = 0;
	u32 cli_reg_addr = 0, read_val, write_val;
	bool addr_mode, all = false;
	char *cli_name = NULL;
	int status = 0;

	status = parse_UM_port_format(argv[1], &side, &num);
	if (status)
		return status;

	block_idx = (side * 6) + num;
	block = emem_mc_block_id[block_idx];

	/* argv[2] = 0x<addr> or name or ALL*/
	if (argc > 2 ) {

		all = (!strcmp(argv[2], "ALL"));

		if (all)
		{
			first_reg_idx = 0;
			last_reg_idx = (PUB_DDR_PHY_REGS_NUMBER - 1);

			printf("all case\n");
		} else {
			addr_mode = (!memcmp("0x", argv[2], 2) || !memcmp("0X", argv[2], 2));
			printf("address mode = %d\n", addr_mode);
			if (addr_mode)
				cli_reg_addr = (u32)simple_strtol(argv[2], NULL, 16);
			else
				cli_name = argv[2];
			printf("cli_reg_addr = 0x%x\n", cli_reg_addr);
			reg_idx = find_ddr_phy_record(block_idx, addr_mode, cli_reg_addr, cli_name);
			printf("reg_idx = %d\n", reg_idx);
			if (reg_idx == INVALID_VAL) {
				printf("Error, invalid register %s\n", (addr_mode ? "address": "name"));
				return 1;
			}

			first_reg_idx = reg_idx;
			last_reg_idx = reg_idx;
		}
	}

	if (!strcmp("pub_record", argv[0])) {

		for (reg_idx = first_reg_idx; reg_idx <= last_reg_idx; reg_idx++ ) {
			PUB_RECORD_GET(block_idx, reg_idx)->val =
					emem_mc_indirect_reg_read_synop(block,
					PUB_RECORD_GET(block_idx, reg_idx)->address);

			if (get_debug()) {
				printf("Debug print: RECORD info: \n");
				print_reg(PUB_RECORD_GET(block_idx, reg_idx)->name,
						PUB_RECORD_GET(block_idx, reg_idx)->address,
						PUB_RECORD_GET(block_idx, reg_idx)->val);
			}

		}
	} else if (!strcmp("pub_restore", argv[0])) {

		for (reg_idx = first_reg_idx; reg_idx <= last_reg_idx; reg_idx++ ) {

			/* Prohibit restore for Registers: PIR, SCHCR0 & BISTRR.*/
			if ((PUB_RECORD_GET(block_idx, reg_idx)->address == PUB_PIR_REG_ADDR) ||
				(PUB_RECORD_GET(block_idx, reg_idx)->address == PUB_SCHCR0_REG_ADDR) ||
				(PUB_RECORD_GET(block_idx, reg_idx)->address == PUB_BISTRR_REG_ADDR)) {
				if (get_debug())
					printf("Debug print: Skip %s reg restore\n",
							PUB_RECORD_GET(block_idx, reg_idx)->name);
				continue;
			}

			write_val = PUB_RECORD_GET(block_idx, reg_idx)->val;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_RECORD_GET(block_idx, reg_idx)->address,
					write_val);
			if (get_debug()) {
				printf("Debug print: RESTORE info: \n");
				print_reg(PUB_RECORD_GET(block_idx, reg_idx)->name,
						PUB_RECORD_GET(block_idx, reg_idx)->address,
						write_val);
			}
		}

	} else if (!strcmp("pub_dump", argv[0])) {

		print_pub_dump(block_idx);

	} else if (!strcmp("pub_write", argv[0])){

		write_val = (u32)simple_strtol(argv[3], NULL, 16);
		emem_mc_indirect_reg_write_synop_data0( block,
				PUB_RECORD_GET(block_idx, reg_idx)->address, write_val);
		if (get_debug()) {
			printf("Debug print: WRITE info: \n");
			print_reg(PUB_RECORD_GET(block_idx, reg_idx)->name,
					PUB_RECORD_GET(block_idx, reg_idx)->address,
					write_val);
		}
	} else {
		/* pub_read */
		read_val = emem_mc_indirect_reg_read_synop(block,
				PUB_RECORD_GET(block_idx, reg_idx)->address);
		print_reg(PUB_RECORD_GET(block_idx, reg_idx)->name,
				PUB_RECORD_GET(block_idx, reg_idx)->address,
				read_val);
	}

	return 0;
}

int do_mem_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 mc_mask, bank_group, bank, row, col, size;
	u32 data[CHUNK_SIZE_IN_WORDS] = { 0 };

	mc_mask = (u32)simple_strtol(argv[1], NULL, 16);
	bank_group = (u32)simple_strtol(argv[2], NULL, 10);
	bank = (u32)simple_strtol(argv[3], NULL, 10);
	row = (u32)simple_strtol(argv[4], NULL, 10);
	col = (u32)simple_strtol(argv[5], NULL, 10);
	size = (u32)simple_strtol(argv[6], NULL, 10);

	if (size == 16){
		data[0] = (u32)simple_strtol(argv[7], NULL, 16);
		data[1] = (u32)simple_strtol(argv[8], NULL, 16);
		data[2] = (u32)simple_strtol(argv[9], NULL, 16);
		data[3] = (u32)simple_strtol(argv[10], NULL, 16);
	}
	else if (size == 32){
		data[0] = (u32)simple_strtol(argv[7], NULL, 16);
		data[1] = (u32)simple_strtol(argv[8], NULL, 16);
		data[2] = (u32)simple_strtol(argv[9], NULL, 16);
		data[3] = (u32)simple_strtol(argv[10], NULL, 16);
		data[4] = (u32)simple_strtol(argv[11], NULL, 16);
		data[5] = (u32)simple_strtol(argv[12], NULL, 16);
		data[6] = (u32)simple_strtol(argv[13], NULL, 16);
		data[7] = (u32)simple_strtol(argv[14], NULL, 16);
	}
	else {
		printf("ERROR: wrong side val %d, should be 16 or 32\n", size);
		return -1;
	}

	write_sdram_chunk(mc_mask, bank_group, bank, row, col, size, data);

	return 0;
}

int do_mem_write_loop(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 mc_mask, bank_group, bank, row, col, size;
	u32 data[CHUNK_SIZE_IN_WORDS] = { 0 };
	int i;

	mc_mask = (u32)simple_strtol(argv[1], NULL, 16);
	bank_group = (u32)simple_strtol(argv[2], NULL, 10);
	bank = (u32)simple_strtol(argv[3], NULL, 10);
	row = (u32)simple_strtol(argv[4], NULL, 10);
	col = (u32)simple_strtol(argv[5], NULL, 10);
	size = (u32)simple_strtol(argv[6], NULL, 10);

	if (size == 16){
		data[0] = (u32)simple_strtol(argv[7], NULL, 16);
		data[1] = (u32)simple_strtol(argv[8], NULL, 16);
		data[2] = (u32)simple_strtol(argv[9], NULL, 16);
		data[3] = (u32)simple_strtol(argv[10], NULL, 16);
	}
	else if (size == 32){
		data[0] = (u32)simple_strtol(argv[7], NULL, 16);
		data[1] = (u32)simple_strtol(argv[8], NULL, 16);
		data[2] = (u32)simple_strtol(argv[9], NULL, 16);
		data[3] = (u32)simple_strtol(argv[10], NULL, 16);
		data[4] = (u32)simple_strtol(argv[11], NULL, 16);
		data[5] = (u32)simple_strtol(argv[12], NULL, 16);
		data[6] = (u32)simple_strtol(argv[13], NULL, 16);
		data[7] = (u32)simple_strtol(argv[14], NULL, 16);
	}
	else {
		printf("ERROR: wrong side val %d, should be 16 or 32\n", size);
		return -1;
	}

	while (1) {
		for (i = 0; i < 10000; i++ )
			write_sdram_chunk(mc_mask, bank_group, bank, row, col, size, data);
		if(ctrlc())
			break;
	}

	return 0;
}

int do_mem_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 mc_mask, bank_group, bank, row, col, byte, line, size, num;
	u32 data[CHUNK_SIZE_IN_WORDS];
	u8 *byte_data;

	mc_mask = (u32)simple_strtol(argv[1], NULL, 16);
	bank_group = (u32)simple_strtol(argv[2], NULL, 10);
	bank = (u32)simple_strtol(argv[3], NULL, 10);
	row = (u32)simple_strtol(argv[4], NULL, 10);
	col = (u32)simple_strtol(argv[5], NULL, 10);
	size = (u32)simple_strtol(argv[6], NULL, 10);

	read_sdram_chunk(mc_mask, bank_group, bank, row, col, size, data);

	byte_data = (u8*)data;

	num = (size == 32)?4:2;

	for (line = 0; line < num; line++) {
		swap(byte_data[0], byte_data[3]);
		swap(byte_data[1], byte_data[2]);
		swap(byte_data[4], byte_data[7]);
		swap(byte_data[5], byte_data[6]);

		for (byte = 0; byte < 8; byte++) {
			printf("\t0x%x", *byte_data++);
		}
		printf("\n");
	}

	return 0;
}

int do_mem_read_loop(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 mc_mask, bank_group, bank, row, col, byte, line, size, num;
	u32 data[CHUNK_SIZE_IN_WORDS];
	u8 *byte_data;

	mc_mask = (u32)simple_strtol(argv[1], NULL, 16);
	bank_group = (u32)simple_strtol(argv[2], NULL, 10);
	bank = (u32)simple_strtol(argv[3], NULL, 10);
	row = (u32)simple_strtol(argv[4], NULL, 10);
	col = (u32)simple_strtol(argv[5], NULL, 10);
	size = (u32)simple_strtol(argv[6], NULL, 10);

	while (1) {
		read_sdram_chunk(mc_mask, bank_group, bank, row, col, size, data);
		if(ctrlc())
			break;
	}

	byte_data = (u8*)data;

	num = (size == 32)?4:2;
	for (line = 0; line < num; line++) {
		for (byte = 0; byte < 8; byte++) {
			printf("\t0x%x", *byte_data++);
		}
		printf("\n");
	}

	return 0;
}


int do_phy_vref_prob(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 bl_index, side = 0, num = 0;

	parse_UM_port_format(argv[1], &side, &num);
	bl_index = (u32)simple_strtol(argv[2], NULL, 10);

	phy_vref_prob(bl_index, side, num);

	return 0;
}

int do_post_training(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 size;

	size = (u32)simple_strtol(argv[1], NULL, 10);
	configure_crg_emi_rst(EMI_RST_PHASE_3);
	post_ddr_training(size);

	return 0;
}
#ifndef CONFIG_TARGET_NPS_HE
int do_configure_emem(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return configure_emem();
}
#endif

#define	D2_WRITE_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(_vref_index_) \
	(((_vref_index_) > 24) ? vref_dq_1[(_vref_index_) - (73 - 50)] : vref_dq_2[(_vref_index_)])

#define	D2_READ_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(_vref_index_) \
	(phy_rd_vref_dq[(_vref_index_)])

static void d2_write_eye_plotting(int block, bool print_enable)
{
	u32 phy_side, phy_num, block_idx, mc_mask;
	u32 bl_index, bl_iprd, bl_tprd, bl_tap_fantaSec, tck_index, rank_index;
	u32 wdqd_tr_res, vref_tr_res, mr6_temp, retries;
	int vref_index, dll_index, vref_range, vref_range_val;
	u32 bist_results[EMEM_MC_NUM_OF_BLOCKS];

	u32 res_wr_2D[BYTE_LANES_NUM][D2_VREF_MAX_VAL+1][512];
	u32 i, j, k, cross_vref_idx;
	u32 status = 0;

	u32 central_left_margin;
	u32 central_right_margin;
	u32 vref_high_margin;
	u32 vref_low_margin;
	u32 central_left_margin_ps_mul1000;
	u32 central_right_margin_ps_mul1000;
	u32 vref_high_margin_volt_mul100;
	u32 vref_low_margin_volt_mul100;

	u32 max_tap[BYTE_LANES_NUM];

	u32 central_left_position = 0;
	u32 central_right_position = 0;
	u32 vref_high_position = 0;
	u32 vref_low_position = 0;

	u32 central_left_prev = 0;
	u32 central_right_prev = 0;
	u32 vref_high_prev = 0;
	u32 vref_low_prev = 0;

	u32 percent_val, percent_val_found;

	union pub_pgcr6 pgcr6;
	union pub_dx_x_mdlr0 dx_x_mdlr0;
	union pub_dx_x_lcdlr1 dx_x_lcdlr1;
	union pub_bistrr bistrr;
	union pub_bistar1 bistar1;
	union pub_pgcr1 pgcr1;
	union pub_schcr1 schcr1;
	union pub_schcr0 schcr0;

	for(i = 0; i < BYTE_LANES_NUM; i++)
		for(j = 0; j < D2_VREF_MAX_VAL+1; j++)
			for( k = 0; k < 512; k++)
				res_wr_2D[i][j][k] = 0xf;

	phy_side = (block >= emem_mc_block_id[6]) ? 1 : 0;
	phy_num = (block >= emem_mc_block_id[6]) ?
			(block - emem_mc_block_id[6]) :
			(block - emem_mc_block_id[0]);
	block_idx = (phy_side * 6) + phy_num;
	mc_mask = (1 << block_idx);

	/* 1) */
	if (get_debug())
		printf("Step 1\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 1;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 2) */
	if (get_debug())
		printf("Step 2\n");
	if (get_debug())
		printf("Debug print: Call phy_bist_setup, mc_mask = 0x%08X\n",
				mc_mask);
	phy_bist_setup(mc_mask);

	/* 3) */
	if (get_debug())
		printf("Step 3\n");
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {
		if (g_temp_try && (bl_index != EYE_DEBUG_BL_INDEX))
			continue;

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		dx_x_mdlr0.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_mdlr0_addr[bl_index]);

		if (g_temp_try) {
			dx_x_mdlr0.fields.iprd = 13;
			dx_x_mdlr0.fields.iprd = 15;
			printf("temp_try print: temp_try mode, ");
			printf("dx_x_mdlr0.fields.iprd = %d\n",
						dx_x_mdlr0.fields.iprd);
			printf("dx_x_mdlr0.fields.tprd = %d\n",
						dx_x_mdlr0.fields.tprd);
		}
		bl_iprd = dx_x_mdlr0.fields.iprd;
		bl_tprd = dx_x_mdlr0.fields.tprd;
		res_vrtwdqd[bl_index][D2_BL_IPRD] = bl_iprd;
		res_vrtwdqd[bl_index][D2_BL_TPRD] = bl_tprd;

		max_tap[bl_index] = bl_iprd + D2_TAP_DELTA;

		/* 5) */
		if (get_debug())
			printf("Step 5\n");
		tck_index = current_ddr_params.pll_freq;
		bl_tap_fantaSec = (tCK2[tck_index] / (2 * bl_iprd));

		if (get_debug()) {
			printf("Debug print: bl_tap_fantaSec = ");
			printf("tCK2 /(2 * bl_iprd) = ");
			printf("%u / (2 * %u *10) = %u\n",
					tCK2[tck_index], bl_iprd, bl_tap_fantaSec);
		}

		/* 6) */
		if (get_debug())
			printf("Step 6\n");
		dx_x_lcdlr1.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr1_addr[bl_index]);

		if (g_temp_try) {
			dx_x_lcdlr1.fields.wdqd = 8;
			printf("temp_try print: temp_try mode ,");
			printf("dx_x_lcdlr1.fields.wdqd = %d\n",
						dx_x_lcdlr1.fields.wdqd);
		}
		wdqd_tr_res = dx_x_lcdlr1.fields.wdqd;
		res_vrtwdqd[bl_index][D2_WDQD_TR_VAL] = dx_x_lcdlr1.fields.wdqd;
		res_vrtwdqd[bl_index][D2_BL_TAP_PS] = bl_tap_fantaSec;

		/* 7) */
		if (get_debug())
			printf("Step 7\n");
		rank_index = (bl_index / 2);

		/* 8) */
		if (get_debug())
			printf("Step 8\n");
		vref_tr_res = (mr6_rank_vref[block_idx][rank_index].reg & 0x7F);/* bits 6..0, corresponds to mr6_rank_vref fields vrefdq_tr_rng + vrefdq_tr_val */
		res_vrtwdqd[bl_index][D2_VREF_TR_VAL] = vref_tr_res;

		/* 9) */
		if (get_debug())
			printf("Step 9\n");
		bistrr.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTRR_REG_ADDR);
		bistrr.fields.bdxsel = bl_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTRR_REG_ADDR, bistrr.reg);

		/* 10) */
		if (get_debug())
			printf("Step 10\n");
		bistar1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTAR1_REG_ADDR);
		bistar1.fields.brank = rank_index;
		bistar1.fields.bmrank = rank_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTAR1_REG_ADDR, bistar1.reg);

		/* 11)	Set vref_index loop variable parameter to a value of 50. */
		/* 12)	Set dll_index loop variable parameter to a value of 0. */
		/* NOTE: STEP 12 loop is actually done after step 13 */
		if (get_debug())
			printf("Step 11+12\n");
		for (vref_index = D2_VREF_MAX_VAL; vref_index >= 0; vref_index--) {

			if (g_temp_try_2d && (vref_index != EYE_DEBUG_VREF_INDEX))
				continue;

			/* 13) */
			if (get_debug())
				printf("Step 13\n");

			/* a)	Calculate a binary variable vref_range, set the vref_range variable to 1 if the vref_index loop variable is less than 25, otherwise set to 0. */
			if (get_debug())
				printf("Step 13.a\n");
			vref_range = (vref_index >= 25) ? 0 : 1;

			/* b)	Calculate a variable vref_range_val = ( 64 * vref_range ) + ( vref_index - ( 23 * (1 - vref_range ) ) ) */
			if (get_debug())
				printf("Step 13.b\n");
			vref_range_val = (64 * vref_range) + (vref_index - (23 * (1 - vref_range)));

			/* c)	Calculate a variable MR6_TEMP = ( MR6_DATA & 0xFF00 ) + 0x0080 + vref_range_val.*/
			if (get_debug())
				printf("Step 13.c\n");
			if (get_debug())
				printf("Debug print: MR6_DATA = 0x%08X\n", mr6.reg);
			mr6_temp = (mr6.reg & 0xFF00) + 0x0080 + vref_range_val;
			if (get_debug())
				printf("Debug print: MR6_TEMP = 0x%08X\n", mr6_temp);

			/*d)	Set the value of 0x1 to the PUBMODE field of the PGCR1 PUB register.*/
			if (get_debug())
				printf("Step 13.d\n");
			pgcr1.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_PGCR1_REG_ADDR);
			pgcr1.fields.pub_mode = 1;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_PGCR1_REG_ADDR, pgcr1.reg);

			/* e)	Set the value of 0x1 to the CMD field of the SCHCR0 PUB register. */
			if (get_debug())
				printf("Step 13.e\n");
			schcr0.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_SCHCR0_REG_ADDR);
			schcr0.fields.cmd = 1;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_SCHCR0_REG_ADDR, schcr0.reg);


			/*f)	Set the value of 0x0 to the ALLRANK field of the SCHCR1 PUB register.*/
			if (get_debug())
				printf("Step 13.f\n");
			schcr1.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_SCHCR1_REG_ADDR);
			schcr1.fields.allrank = 0;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_SCHCR1_REG_ADDR, schcr1.reg);

			/* g)	Set the value of 0x2 to the SCBK field of the SCHCR1 PUB register. */
			if (get_debug())
				printf("Step 13.g\n");
			schcr1.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_SCHCR1_REG_ADDR);
			schcr1.fields.scbk = 2;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_SCHCR1_REG_ADDR, schcr1.reg);

			/* h)	Set the value of 0x1 to the SCBG field of the SCHCR1 PUB register.*/
			if (get_debug())
				printf("Step 13.h\n");
			schcr1.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_SCHCR1_REG_ADDR);
			schcr1.fields.scbg = 1;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_SCHCR1_REG_ADDR, schcr1.reg);


			/* i)	Set the value of the MR6_TEMP variable to the SCADDR field of the SCHCR1 PUB register. */
			if (get_debug())
				printf("Step 13.i\n");
			schcr1.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_SCHCR1_REG_ADDR);
			schcr1.fields.scaddr = mr6_temp;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_SCHCR1_REG_ADDR, schcr1.reg);


			/* j) Set the value of the rank_index variable to the SCRANK field of the SCHCR1 PUB register. */
			if (get_debug())
				printf("Step 13.j \n");
			schcr1.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_SCHCR1_REG_ADDR);
			schcr1.fields.scrnk = rank_index;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_SCHCR1_REG_ADDR, schcr1.reg);


			/* k)	Set the value of 0x1 to the SCHTRIG field of the SCHCR0 PUB register.*/
			if (get_debug())
				printf("Step 13.k\n");
			schcr0.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_SCHCR0_REG_ADDR);
			schcr0.fields.schtrig = 1;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_SCHCR0_REG_ADDR, schcr0.reg);


			/* l)	Read the SCHCR0 PUB register and wait until the SCHTRIG field value is back to 0x0.*/
			/* The wait should have a timeout of 1 uSec.*/
			if (get_debug())
				printf("Step 13.l\n");
			for (retries = 0; retries < DDR_PHY_BIST_RETRY_NUM; retries++) {
				udelay(1);
				schcr0.reg = emem_mc_indirect_reg_read_synop(
						block, PUB_SCHCR0_REG_ADDR);
				if (schcr0.fields.schtrig == 0)
					break;
			}
			if (retries == DDR_PHY_BIST_RETRY_NUM && !g_EZsim_mode) {
				error("frst D2: retries exceeded (port %d)\n", block);
				status = 1;
			}

			/* m)	Set the value of 0x0 to the PUBMODE field of the PGCR1 PUB register.*/
			if (get_debug())
				printf("Step 13.m\n");
			pgcr1.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_PGCR1_REG_ADDR);
			pgcr1.fields.pub_mode = 0;
			emem_mc_indirect_reg_write_synop_data0(
					block, PUB_PGCR1_REG_ADDR, pgcr1.reg);

			for (dll_index = 0; dll_index <= max_tap[bl_index]; dll_index++) {

				if (g_temp_try_2d && (dll_index != EYE_DEBUG_DLL_INDEX))
					continue;


				/* 14)	Write the value of the dll_index to the WDQD field of the DX<bl_index>LCDLR1 PUB register. */
				if (get_debug())
					printf("Step 14\n");
				dx_x_lcdlr1.reg = emem_mc_indirect_reg_read_synop(
						block, pub_dx_x_lcdlr1_addr[bl_index]);
				dx_x_lcdlr1.fields.wdqd = dll_index;
				emem_mc_indirect_reg_write_synop_data0(
						block, pub_dx_x_lcdlr1_addr[bl_index], dx_x_lcdlr1.reg);

				/* 15)	Run the DDR PHY BIST as described in section ?15.3.2 on page 15-79 and get the BIST status. */
				if (get_debug())
					printf("Step 15\n");
				phy_bist_run(mc_mask, bist_results);
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");

				/* 16)	Perform the following checks and operations:
					a)	If the BIST Status is not 0, then write a value of 0x0 to the RES_WR_2D<bl_index>[verf_index][dll_index].
					b)	If the BIST Status is 0, then write a value of 0x1 to the RES_WR_2D<bl_index>[verf_index][dll_index]. */
				if (get_debug())
					printf("Step 16\n");
				res_wr_2D[bl_index][vref_index][dll_index] = (bist_results[block_idx]) ? 0 : 1;


				/*
				17)	Increment the dll_index loop variable parameter by 1 and go to step 14 if the dll_index parameter value is less or equal then the BL_IPRD variable.
				18)	Decrement the vref_index loop variable parameter by 1 and go to step 12 if the vref_index parameter value is less than 0.
				*/
				if (get_debug())
					printf("Step 17+18\n");
			}
		}

		/* 19)	Restore the value of the WDQD field of the DX<bl_index>LCDLR1 PUB register using the WDQD_TR_RES recorded value. */
		if (get_debug())
			printf("Step 19\n");
		dx_x_lcdlr1.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr1_addr[bl_index]);
		dx_x_lcdlr1.fields.wdqd = wdqd_tr_res;
		emem_mc_indirect_reg_write_synop_data0(
				block, pub_dx_x_lcdlr1_addr[bl_index], dx_x_lcdlr1.reg);

		/*
		20)	Restore the VREF SDRAM Training Results. */
		if (get_debug())
			printf("Step 20\n");

		/* a)	Calculate a variable MR6_TEMP = ( MR6_DATA & 0xFF00 ) + 0x0080 + VREF_TR_RES.*/
		if (get_debug())
			printf("Step 20.a\n");
		if (get_debug())
			printf("Debug print: MR6_DATA = 0x%08X\n", mr6.reg);
		mr6_temp = (mr6.reg & 0xFF00) + 0x0080 + vref_tr_res;
		if (get_debug())
			printf("Debug print: MR6_TEMP = 0x%08X\n", mr6_temp);

		/* b)	Set the value of 0x1 to the PUBMODE field of the PGCR1 PUB register. */
		if (get_debug())
			printf("Step 20.b\n");
		pgcr1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_PGCR1_REG_ADDR);
		pgcr1.fields.pub_mode = 1;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_PGCR1_REG_ADDR, pgcr1.reg);


		/* c)	Set the value of the MR6_TEMP variable to the SCADDR field of the SCHCR1 PUB register */
		if (get_debug())
			printf("Step 20.c\n");

		schcr1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_SCHCR1_REG_ADDR);
		schcr1.fields.scaddr = mr6_temp;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_SCHCR1_REG_ADDR, schcr1.reg);

		/* d)	Set the value of 0x1 to the SCHTRIG field of the SCHCR0 PUB register. */
		if (get_debug())
			printf("Step 20.d\n");
		schcr0.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_SCHCR0_REG_ADDR);
		schcr0.fields.schtrig = 1;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_SCHCR0_REG_ADDR, schcr0.reg);

		/* e)	Read the SCHCR0 PUB register and wait until the SCHTRG field value is back to 0x0.
				The wait should have a timeout of 1 uSec */
		if (get_debug())
			printf("Step 20.e\n");
		for (retries = 0; retries < DDR_PHY_BIST_RETRY_NUM; retries++) {
			udelay(1);
			schcr0.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_SCHCR0_REG_ADDR);
			if (schcr0.fields.schtrig == 0)
				break;
		}
		if (retries == DDR_PHY_BIST_RETRY_NUM && !g_EZsim_mode) {
			error("scnd D2: retries exceeded (port %d)\n", block);
			status = 1;
		}


		/* f)	Calculate a variable MR6_TEMP = ( MR6_DATA & 0xFF00 ) + VREF_TR_RES. */
		if (get_debug())
			printf("Step 20.f\n");
		if (get_debug())
			printf("Debug print: MR6_DATA = 0x%08X\n", mr6.reg);
		mr6_temp = (mr6.reg & 0xFF00) + vref_tr_res;
		if (get_debug())
			printf("Debug print: MR6_TEMP = 0x%08X\n", mr6_temp);


		/* g)	Set the value of the MR6_TEMP variable to the SCADDR field of the SCHCR1 PUB register */
		if (get_debug())
			printf("Step 20.g\n");

		schcr1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_SCHCR1_REG_ADDR);
		schcr1.fields.scaddr = mr6_temp;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_SCHCR1_REG_ADDR, schcr1.reg);

		/* h)	Set the value of 0x1 to the SCHTRIG field of the SCHCR0 PUB register. */
		if (get_debug())
			printf("Step 20.h\n");
		schcr0.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_SCHCR0_REG_ADDR);
		schcr0.fields.schtrig = 1;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_SCHCR0_REG_ADDR, schcr0.reg);


		/* i)	Read the SCHCR0 PUB register and wait until the SCHTRIG field value is back to 0x0.
		The wait should have a time-out of 1 uSec.*/
		if (get_debug())
			printf("Step 20.i\n");
		for (retries = 0; retries < DDR_PHY_BIST_RETRY_NUM; retries++) {
			udelay(1);
			schcr0.reg = emem_mc_indirect_reg_read_synop(
					block, PUB_SCHCR0_REG_ADDR);
			if (schcr0.fields.schtrig == 0)
				break;

		}
		if (retries == DDR_PHY_BIST_RETRY_NUM && !g_EZsim_mode) {
			error("thrd D2: retries exceeded (port %d)\n", block);
			status = 1;
		}

		/* j)	Set the value of 0x0 to the PUBMODE field of the PGCR1 PUB register. */
		if (get_debug())
			printf("Step 20.j\n");
		pgcr1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_PGCR1_REG_ADDR);
		pgcr1.fields.pub_mode = 0;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_PGCR1_REG_ADDR, pgcr1.reg);


		/* 21)	Increment the bl_index loop variable parameter by 1 and go to step 4 if the bl_index parameter value is less than 4. */
		if (get_debug())
			printf("Step 21\n");

	}	/* bl_index loop */



	/*22)	Set a value of 0x0 to the INHVT field of the PGCR6 PUB register.*/
	if (get_debug())
		printf("Step 22\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 0;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 23)	Restore the BISTRR PUB register to its default value. */
	if (get_debug())
		printf("Step 23\n");
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_BISTRR_REG_ADDR, PUB_BISTRR_REG_HW_DEF_VAL);

	/* 24)	Return the RES_VRTWDQD and the 4 RES_WR_2D<bl_index> arrays information as a result for the specific DDR PHY Interface. */
	if (get_debug())
		printf("Step 24\n");

	if (print_enable) {
		printf("\nPer-Byte-Lane WRITE Data Eye Plotting for DDR PHY UM%u_%u:\n\n",
				phy_side, phy_num);
		printf("status = %s\n", ((status) ? "FAIL" : "PASS"));
		printf("%-10s %-15s %-15s %-15s %-15s %-15s\n",
				"byte_lane",
				"VREF_TR_VAL",
				"WDQD_TR_VAL",
				"BL_IPRD",
				"BL_TPRD",
				"BL_TAP_PS");

		for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {
			printf("%-10u %-15u %-15u %-15u %-15u %u.%03u\n",
					bl_index,
					res_vrtwdqd[bl_index][D2_VREF_TR_VAL],
					res_vrtwdqd[bl_index][D2_WDQD_TR_VAL],
					res_vrtwdqd[bl_index][D2_BL_IPRD],
					res_vrtwdqd[bl_index][D2_BL_TPRD],
					(res_vrtwdqd[bl_index][D2_BL_TAP_PS] / 1000),
					(res_vrtwdqd[bl_index][D2_BL_TAP_PS] % 1000));
		}
	}
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {
		if (print_enable)
			printf("byte_lane %d 2D WRITE representation:\n\n", bl_index);

		central_left_position = INVALID_VAL;
		central_right_position = INVALID_VAL;
		vref_high_position = INVALID_VAL;
		vref_low_position = INVALID_VAL;

		central_left_prev = 0;
		central_right_prev = 0;
		vref_high_prev = 0;
		vref_low_prev = 0;

		for (vref_index = D2_VREF_MAX_VAL; vref_index >= 0; vref_index--) {

			percent_val = D2_WRITE_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(vref_index);

			/* find cross vref index */
			vref_range = (res_vrtwdqd[bl_index][D2_VREF_TR_VAL] >> 6) & 1;
			vref_range_val = res_vrtwdqd[bl_index][D2_VREF_TR_VAL] & 0x3F;

			if (vref_range == 0 ) {
				/* range 1 */
				if (vref_range_val >= 2 )
				{
					cross_vref_idx = vref_range_val + 23;
				}
				else {
					/* NOTE:
					 * range 1 table name:  vref_dq_1
					 * range 2 table name:  vref_dq_2 */

					/* 1. find percentage value of vref_range_val in range 1 table */
					percent_val_found = vref_dq_1[vref_range_val];

					/* 2. find nearest index value to percentage value  in range 2 table */
					cross_vref_idx = get_vref_index(vref_dq_2, ARRAY_SIZE(vref_dq_2), percent_val_found);
				}
			}
			else {
				/* range 2 */
				if (vref_range_val <= 24 ) {
					cross_vref_idx = vref_range_val;
				}
				else {
					/* NOTE:
					 * range 1 table name:  vref_dq_1
					 * range 2 table name:  vref_dq_2 */

					/* 1. find percentage value of vref_range_val in range 2 table */
					percent_val_found = vref_dq_2[vref_range_val];

					/* 2. find nearest index value to percentage value  in range 1 table, and add 23 */
					cross_vref_idx = get_vref_index(vref_dq_1, ARRAY_SIZE(vref_dq_1), percent_val_found) + 23;
				}
			}
			if (print_enable)
				printf("%02d (%2d.%02d%%) ", vref_index, (percent_val/100), (percent_val%100));
			for (dll_index = 0; dll_index < 512; dll_index++) {
				if (dll_index == res_vrtwdqd[bl_index][D2_WDQD_TR_VAL]) {
					if ( vref_index >= cross_vref_idx  ) {
						/* check vref_high_position, last one catches */
						if ((vref_high_prev == 0) &&  (res_wr_2D[bl_index][vref_index][dll_index] == 1)) {
							vref_high_position = vref_index;
						}
						vref_high_prev = res_wr_2D[bl_index][vref_index][dll_index];
					}
					else {
						/* check vref_low_position, first one catches  */
						if ((vref_low_prev == 1) &&  (res_wr_2D[bl_index][vref_index][dll_index] == 0) && (vref_low_position == INVALID_VAL)) {
							vref_low_position = vref_index;
						}
						vref_low_prev = res_wr_2D[bl_index][vref_index][dll_index];
					}
				}
				else if ((vref_index == cross_vref_idx ) &&
						 ((res_wr_2D[bl_index][vref_index][dll_index] == 0) ||
						  (res_wr_2D[bl_index][vref_index][dll_index] == 1))) {

					if (dll_index <= res_vrtwdqd[bl_index][D2_WDQD_TR_VAL]) {
						/* check central_left_position, last one catches */
						if ((central_left_prev == 0) &&  (res_wr_2D[bl_index][vref_index][dll_index] == 1)) {
							central_left_position = dll_index;
						}
						central_left_prev = res_wr_2D[bl_index][vref_index][dll_index];
					}
					else {
						/* check central_right_position, first one catches */
						if ((central_right_prev == 1) &&  (res_wr_2D[bl_index][vref_index][dll_index] == 0) && (central_right_position == INVALID_VAL)) {
							central_right_position = dll_index;
						}
						central_right_prev = res_wr_2D[bl_index][vref_index][dll_index];
					}
				}
				if (print_enable) {
					if ((vref_index == cross_vref_idx ) &&
						(dll_index == res_vrtwdqd[bl_index][D2_WDQD_TR_VAL])) {
						printf("*");
					}
					else if (res_wr_2D[bl_index][vref_index][dll_index] == 0) {
						printf("X");
					}
					else if (res_wr_2D[bl_index][vref_index][dll_index] == 1) {
						if (vref_index == cross_vref_idx )
								printf("-");
						else if (dll_index == res_vrtwdqd[bl_index][D2_WDQD_TR_VAL])
							printf("|");
						else
							printf(" ");
					}
				}
			}
			if (print_enable)
				printf("\n");
		}
		res_w_tr_val[bl_index] = cross_vref_idx;
		if (print_enable) {
			printf("            0");
			for (dll_index = 1; dll_index < res_vrtwdqd[bl_index][D2_WDQD_TR_VAL]; dll_index++)
				printf(" ");
			printf("%02d", res_vrtwdqd[bl_index][D2_WDQD_TR_VAL]);
			for (dll_index = res_vrtwdqd[bl_index][D2_WDQD_TR_VAL] + 3 ;
				 dll_index < max_tap[bl_index];
				 dll_index++)
				printf(" ");
			printf("%02d", max_tap[bl_index]);
			printf("\n\n");
		}
		/* treat corner cases */
		if ( (central_left_position == 0xFFFFFFFF) ||  (central_left_position == 0) ) {
			central_left_position = 1;
		}
		if ( central_right_position == 0xFFFFFFFF ) {
			central_right_position = max_tap[bl_index];
		}

		if ( (vref_high_position == 0xFFFFFFFF) || (vref_high_position == D2_VREF_MAX_VAL) ) {
			vref_high_position = D2_VREF_MAX_VAL -1;
		}

		if ( vref_low_position == 0xFFFFFFFF ) {
			vref_low_position = 0;
		}

		/* tune central_right_position and vref_low_position: they currently represent first 0 and not last 1 */
		central_right_position--;
		vref_low_position++;

		/* print right and left margins in pSec, high and low margins in voltage percents */

		 central_left_margin  = (res_vrtwdqd[bl_index][D2_WDQD_TR_VAL] - central_left_position);
		 central_right_margin = (central_right_position - res_vrtwdqd[bl_index][D2_WDQD_TR_VAL] );

		 vref_high_margin = (vref_high_position - cross_vref_idx);
		 vref_low_margin  = (cross_vref_idx - vref_low_position );

		 central_left_margin_ps_mul1000 = central_left_margin * res_vrtwdqd[bl_index][D2_BL_TAP_PS];
		 central_right_margin_ps_mul1000 =  central_right_margin * res_vrtwdqd[bl_index][D2_BL_TAP_PS];

		 vref_high_margin_volt_mul100 = D2_WRITE_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(vref_high_position + 1) -
				 D2_WRITE_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(cross_vref_idx);

		 vref_low_margin_volt_mul100 = D2_WRITE_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(cross_vref_idx) -
				 D2_WRITE_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(vref_low_position - 1);

		 if (print_enable) {
			printf("%-26s %u.%03u pSec (%u taps)\n",
					"central_left_margin = ",
					(central_left_margin_ps_mul1000 / 1000),
					(central_left_margin_ps_mul1000 % 1000),
					central_left_margin );

			printf("%-26s %u.%03u pSec (%u taps)\n",
					"central_right_margin = ",
					(central_right_margin_ps_mul1000 / 1000),
					(central_right_margin_ps_mul1000 % 1000),
					central_right_margin );

			printf("%-26s %u.%02u%% (%u indexes)\n",
					"vref_high_margin = ",
					(vref_high_margin_volt_mul100 / 100),
					(vref_high_margin_volt_mul100 % 100),
					vref_high_margin );

			printf("%-26s %u.%02u%% (%u indexes)\n",
					"vref_low_margin = ",
					(vref_low_margin_volt_mul100 / 100),
					(vref_low_margin_volt_mul100 % 100),
					vref_low_margin );
			printf("\n\n");
		}
		left_margin[bl_index] = central_left_margin;
		right_margin[bl_index] = central_right_margin;
		vref_write_max[bl_index] = vref_high_margin;
		vref_write_min[bl_index] = vref_low_margin;
	}
	if(print_enable)
		printf("\n");

	return;
}

static void d2_read_eye_plotting(int block, bool print_enable)
{
#if 0
	u32 bl_index, bl_iprd, bl_tprd, bl_tap_fantaSec, tck_index, rank_index;
	u32 rdqsd_tr_res, rdqsnd_tr_res, vref_tr_res;
	u32 phy_side, phy_num, block_idx, mc_mask;
	int vref_index, dll_index;
	u32 vref_gcr5;

	u32 bist_results[EMEM_MC_NUM_OF_BLOCKS];

	u32 res_rd_2D[BYTE_LANES_NUM][D2_R_VREF_MAX_VAL+1][512];
	u32 i, j, k;
	u32 status = 0;

	u32 central_left_margin;
	u32 central_right_margin;
	u32 vref_high_margin;
	u32 vref_low_margin;
	u32 central_left_margin_ps_mul1000;
	u32 central_right_margin_ps_mul1000;
	u32 vref_high_margin_volt_mul100;
	u32 vref_low_margin_volt_mul100;

	u32 max_tap[BYTE_LANES_NUM];

	u32 central_left_position = 0;
	u32 central_right_position = 0;
	u32 vref_high_position = 0;
	u32 vref_low_position = 0;

	u32 central_left_prev = 0;
	u32 central_right_prev = 0;
	u32 vref_high_prev = 0;
	u32 vref_low_prev = 0;

	u32 percent_val;
	u32 min_rdqs;
	u32 rdqsd_delta;
	u32 rdqsnd_delta;

	union pub_pgcr6 pgcr6;
	union pub_dx_x_mdlr0 dx_x_mdlr0;
	union pub_dx_x_lcdlr3 dx_x_lcdlr3;
	union pub_dx_x_lcdlr4 dx_x_lcdlr4;

	union pub_dx_n_gcr5 dx_x_gcr5;

	union pub_bistrr bistrr;
	union pub_bistar1 bistar1;

	for(i = 0; i < BYTE_LANES_NUM; i++)
		for(j = 0; j < D2_R_VREF_MAX_VAL+1; j++)
			for( k = 0; k < 512; k++)
				res_rd_2D[i][j][k] = 0xf;

	phy_side = (block >= emem_mc_block_id[6]) ? 1 : 0;
	phy_num = (block >= emem_mc_block_id[6]) ?
			(block - emem_mc_block_id[6]) :
			(block - emem_mc_block_id[0]);
	block_idx = (phy_side * 6) + phy_num;
	mc_mask = (1 << block_idx);

	/* 1) */
	if (get_debug())
		printf("Step 1\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 1;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 2) */
	if (get_debug())
		printf("Step 2\n");
	if (get_debug())
		printf("Debug print: Call phy_bist_setup, mc_mask = 0x%08X\n",
				mc_mask);
	phy_bist_setup(mc_mask);

	/* 3) */
	if (get_debug())
		printf("Step 3\n");
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {

		if (g_temp_try && (bl_index != EYE_DEBUG_BL_INDEX))
			continue;

		/* 4) */
		if (get_debug())
			printf("Step 4\n");
		dx_x_mdlr0.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_mdlr0_addr[bl_index]);

		if (g_temp_try) {
			dx_x_mdlr0.fields.iprd = 13;
			dx_x_mdlr0.fields.tprd = 15;
			printf("temp_try print: temp_try mode, ");
			printf("dx_x_mdlr0.fields.iprd = %d\n",
						dx_x_mdlr0.fields.iprd);
			printf("dx_x_mdlr0.fields.tprd = %d\n",
						dx_x_mdlr0.fields.tprd);
		}
		bl_iprd = dx_x_mdlr0.fields.iprd;
		bl_tprd = dx_x_mdlr0.fields.tprd;
		res_vrtrdqsd[bl_index][D2_R_BL_IPRD] = bl_iprd;
		res_vrtrdqsd[bl_index][D2_R_BL_TPRD] = bl_tprd;

		max_tap[bl_index] = bl_iprd + D2_TAP_DELTA;


		/* 5) */
		if (get_debug())
			printf("Step 5\n");
		tck_index = current_ddr_params.pll_freq;
		bl_tap_fantaSec = (tCK2[tck_index] / (2 * bl_iprd));

		if (get_debug()) {
			printf("Debug print: bl_tap_fantaSec = ");
			printf("tCK2 /(2 * bl_iprd) = ");
			printf("%u / (2 * %u) = %u\n",
					tCK2[tck_index], bl_iprd, bl_tap_fantaSec);
		}


		/* 6) */
		if (get_debug())
			printf("Step 6\n");
		dx_x_lcdlr3.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr3_addr[bl_index]);

		if (g_temp_try) {
			dx_x_lcdlr3.fields.rdqsd = 8;
			printf("temp_try print: temp_try mode ,");
			printf("dx_x_lcdlr3.fields.rdqsd = %d\n",
					dx_x_lcdlr3.fields.rdqsd);
		}
		rdqsd_tr_res = dx_x_lcdlr3.fields.rdqsd;
		res_vrtrdqsd[bl_index][D2_R_RDQSD_TR_VAL] = dx_x_lcdlr3.fields.rdqsd;


		dx_x_lcdlr4.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr4_addr[bl_index]);

		if (g_temp_try) {
			dx_x_lcdlr4.fields.rdqsnd = 8;
			printf("temp_try print: temp_try mode ,");
			printf("dx_x_lcdlr4.fields.rdqsnd = %d\n",
					dx_x_lcdlr4.fields.rdqsnd);
		}
		rdqsnd_tr_res = dx_x_lcdlr4.fields.rdqsnd;
		res_vrtrdqsd[bl_index][D2_R_RDQSND_TR_VAL] = dx_x_lcdlr4.fields.rdqsnd;

		res_vrtrdqsd[bl_index][D2_R_BL_TAP_PS] = bl_tap_fantaSec;


		rdqsd_delta = 0;
		rdqsnd_delta = 0;
		if ( rdqsd_tr_res > rdqsnd_tr_res )
			rdqsd_delta = rdqsd_tr_res - rdqsnd_tr_res;
		if ( rdqsd_tr_res < rdqsnd_tr_res )
			rdqsnd_delta = rdqsnd_tr_res - rdqsd_tr_res;

		/* 7) */
		if (get_debug())
			printf("Step 7\n");
		rank_index = (bl_index / 2);

		/* 8) */
		if (get_debug())
			printf("Step 8\n");

		dx_x_gcr5.reg = emem_mc_indirect_reg_read_synop(block,
				pub_dx_x_gcr5_addr[bl_index]);
		vref_gcr5 = dx_x_gcr5.reg;
		switch (rank_index) {
		case 0:
			vref_tr_res = dx_x_gcr5.fields.dxrefiselr0;
			break;
		case 1:
			vref_tr_res = dx_x_gcr5.fields.dxrefiselr1;
			break;
		case 2:
			vref_tr_res = dx_x_gcr5.fields.dxrefiselr2;
			break;
		case 3:
			vref_tr_res = dx_x_gcr5.fields.dxrefiselr3;
			break;
		default:
			printf("ERROR: illegal rank_index %u\n", rank_index);
			vref_tr_res = 0;
			break;
		}
		res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL] = vref_tr_res;

		/* 9)*/
		if (get_debug())
			printf("Step 9\n");
		bistrr.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTRR_REG_ADDR);
		bistrr.fields.bdxsel = bl_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTRR_REG_ADDR, bistrr.reg);



		/* 10) */
		if (get_debug())
			printf("Step 10\n");
		bistar1.reg = emem_mc_indirect_reg_read_synop(
				block, PUB_BISTAR1_REG_ADDR);
		bistar1.fields.brank = rank_index;
		bistar1.fields.bmrank = rank_index;
		emem_mc_indirect_reg_write_synop_data0(
				block, PUB_BISTAR1_REG_ADDR, bistar1.reg);




		/* 11)	Set vref_index loop variable parameter to a value of 63. */
		/* 12)	Set dll_index loop variable parameter to a value of 0. */
		/* NOTE: STEP 12 loop is actually done after step 13 */
		if (get_debug())
			printf("Step 11+12\n");
		for (vref_index = D2_R_VREF_MAX_VAL; vref_index >= 0; vref_index--) {

			if (g_temp_try && (vref_index != EYE_DEBUG_VREF_INDEX))
				continue;

			/* 13) */
			if (get_debug())
				printf("Step 13\n");

			dx_x_gcr5.reg = emem_mc_indirect_reg_read_synop(block,
					pub_dx_x_gcr5_addr[bl_index]);
			dx_x_gcr5.fields.dxrefiselr0 = vref_index;
			dx_x_gcr5.fields.dxrefiselr1 = vref_index;
			dx_x_gcr5.fields.dxrefiselr2 = vref_index;
			dx_x_gcr5.fields.dxrefiselr3 = vref_index;
			emem_mc_indirect_reg_write_synop_data0(
					block, pub_dx_x_gcr5_addr[bl_index], dx_x_gcr5.reg);

			for (dll_index = 0; dll_index <= max_tap[bl_index]; dll_index++) {

				if (g_temp_try && (dll_index != EYE_DEBUG_DLL_INDEX))
					continue;

				/* 14) */
				if (get_debug())
					printf("Step 14\n");

				dx_x_lcdlr3.reg = emem_mc_indirect_reg_read_synop(
					block, pub_dx_x_lcdlr3_addr[bl_index]);
				dx_x_lcdlr3.fields.rdqsd = (dll_index + rdqsd_delta);
				emem_mc_indirect_reg_write_synop_data0(block,
					pub_dx_x_lcdlr3_addr[bl_index], dx_x_lcdlr3.reg);

				dx_x_lcdlr4.reg = emem_mc_indirect_reg_read_synop(
						block, pub_dx_x_lcdlr4_addr[bl_index]);
				dx_x_lcdlr4.fields.rdqsnd = (dll_index + rdqsnd_delta);
				emem_mc_indirect_reg_write_synop_data0(
							block, pub_dx_x_lcdlr4_addr[bl_index],
							dx_x_lcdlr4.reg);

				/* 15) */
				if (get_debug())
					printf("Step 15\n");
				phy_bist_run(mc_mask, bist_results);
				if (get_debug())
					printf("Debug print: phy_bist_run %s\n",
						(bist_results[block_idx]) ? "failed":"passed");

				/* 16) */
				if (get_debug())
					printf("Step 16\n");
				res_rd_2D[bl_index][vref_index][dll_index] = (bist_results[block_idx]) ? 0 : 1;

				/* 17) */
				/* 18) */
				if (get_debug())
					printf("Step 17+18\n");

			}
		}
		/* 19) */
		if (get_debug())
			printf("Step 19\n");
		dx_x_lcdlr3.reg = emem_mc_indirect_reg_read_synop(
			block, pub_dx_x_lcdlr3_addr[bl_index]);
		dx_x_lcdlr3.fields.rdqsd = rdqsd_tr_res;
		emem_mc_indirect_reg_write_synop_data0(block,
			pub_dx_x_lcdlr3_addr[bl_index], dx_x_lcdlr3.reg);

		dx_x_lcdlr4.reg = emem_mc_indirect_reg_read_synop(
				block, pub_dx_x_lcdlr4_addr[bl_index]);
		dx_x_lcdlr4.fields.rdqsnd = rdqsnd_tr_res;
		emem_mc_indirect_reg_write_synop_data0(
					block, pub_dx_x_lcdlr4_addr[bl_index],
					dx_x_lcdlr4.reg);

		/* 20) */
		if (get_debug())
			printf("Step 20\n");
		emem_mc_indirect_reg_write_synop_data0(
				block, pub_dx_x_gcr5_addr[bl_index], vref_gcr5);

		/* 21) */
		if (get_debug())
			printf("Step 21\n");
	}	/* bl_index loop */


	/*22)	Set a value of 0x0 to the INHVT field of the PGCR6 PUB register.*/
	if (get_debug())
		printf("Step 22\n");
	pgcr6.reg = emem_mc_indirect_reg_read_synop(
			block, PUB_PGCR6_REG_ADDR);
	pgcr6.fields.inhvt = 0;
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_PGCR6_REG_ADDR, pgcr6.reg);

	/* 23)	Restore the BISTRR PUB register to its default value. */
	if (get_debug())
		printf("Step 23\n");
	emem_mc_indirect_reg_write_synop_data0(
			block, PUB_BISTRR_REG_ADDR, PUB_BISTRR_REG_HW_DEF_VAL);



	/* 24)	Return the RES_VRTWDQD and the 4 RES_WR_2D<bl_index> arrays information as a result for the specific DDR PHY Interface. */
	if (get_debug())
		printf("Step 24\n");
	if(print_enable) {
		printf("\nPer-Byte-Lane READ Data Eye Plotting for DDR PHY UM%u_%u:\n\n",
				phy_side, phy_num);
		printf("status = %s\n", ((status) ? "FAIL" : "PASS"));
		printf("%-10s %-15s %-15s %-15s %-15s %-15s %-15s\n",
				"byte_lane",
				"VREF_TR_VAL",
				"RDQSD_TR_VAL",
				"RDQSND_TR_VAL",
				"BL_IPRD",
				"BL_TPRD",
				"BL_TAP_PS");

		for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {
			printf("%-10u %-15u %-15u %-15u %-15u %-15u %u.%03u\n",
					bl_index,
					res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL],
					res_vrtrdqsd[bl_index][D2_R_RDQSD_TR_VAL],
					res_vrtrdqsd[bl_index][D2_R_RDQSND_TR_VAL],
					res_vrtrdqsd[bl_index][D2_R_BL_IPRD],
					res_vrtrdqsd[bl_index][D2_R_BL_TPRD],
					(res_vrtrdqsd[bl_index][D2_R_BL_TAP_PS] / 1000),
					(res_vrtrdqsd[bl_index][D2_R_BL_TAP_PS] % 1000));
		}
	}
	for (bl_index = 0; bl_index < BYTE_LANES_NUM; bl_index++) {
		if (print_enable)
			printf("byte_lane %d 2D READ representation:\n\n", bl_index);

		min_rdqs = min(res_vrtrdqsd[bl_index][D2_R_RDQSD_TR_VAL],
				res_vrtrdqsd[bl_index][D2_R_RDQSND_TR_VAL]);

		central_left_position = INVALID_VAL;
		central_right_position = INVALID_VAL;
		vref_high_position = INVALID_VAL;
		vref_low_position = INVALID_VAL;

		central_left_prev = 0;
		central_right_prev = 0;
		vref_high_prev = 0;
		vref_low_prev = 0;

		for (vref_index = D2_R_VREF_MAX_VAL; vref_index >= 0; vref_index--) {

			percent_val = D2_READ_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX (vref_index);
			if (print_enable)
				printf("%02d (%2d.%02d%%) ", vref_index, (percent_val/100), (percent_val%100));

			for (dll_index = 0; dll_index < 512; dll_index++) {
				if (dll_index == min_rdqs) {
					if ( vref_index >= res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL]  ) {
						/* check vref_high_position, last one catches */
						if ((vref_high_prev == 0) &&  (res_rd_2D[bl_index][vref_index][dll_index] == 1)) {
							vref_high_position = vref_index;
						}
						vref_high_prev = res_rd_2D[bl_index][vref_index][dll_index];
					}
					else {
						/* check vref_low_position, first one catches  */
						if ((vref_low_prev == 1) &&  (res_rd_2D[bl_index][vref_index][dll_index] == 0) && ( vref_low_position == INVALID_VAL ) ) {
							vref_low_position = vref_index;
						}
						vref_low_prev = res_rd_2D[bl_index][vref_index][dll_index];
					}
				}
				else if ((vref_index == res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL] ) &&
						 ((res_rd_2D[bl_index][vref_index][dll_index] == 0) ||
						  (res_rd_2D[bl_index][vref_index][dll_index] == 1))) {

					if (dll_index <= min_rdqs) {
						/* check central_left_position, last one catches */
						if ((central_left_prev == 0) &&  (res_rd_2D[bl_index][vref_index][dll_index] == 1)) {
							central_left_position = dll_index;
						}
						central_left_prev = res_rd_2D[bl_index][vref_index][dll_index];
					}
					else {
						/* check central_right_position, first one catches */
						if ((central_right_prev == 1) &&  (res_rd_2D[bl_index][vref_index][dll_index] == 0) && (central_right_position == INVALID_VAL)) {
							central_right_position = dll_index;
						}
						central_right_prev = res_rd_2D[bl_index][vref_index][dll_index];
					}
				}
				if (print_enable) {
					if ((vref_index == res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL] ) &&
						(dll_index == min_rdqs)) {
						printf("*");
					}
					else if (res_rd_2D[bl_index][vref_index][dll_index] == 0) {
						printf("X");
					}
					else if (res_rd_2D[bl_index][vref_index][dll_index] == 1) {
						if (vref_index == res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL] )
								printf("-");
						else if (dll_index == min_rdqs)
							printf("|");
						else
							printf(" ");
					}
				}
			}
			if (print_enable)
				printf("\n");
		}
		if (print_enable) {
			printf("            0");
			for (dll_index = 1; dll_index < min_rdqs; dll_index++)
				printf(" ");
			printf("%02d", min_rdqs);
			for (dll_index = min_rdqs + 3 ;
				 dll_index < max_tap[bl_index];
				 dll_index++)
				printf(" ");
			printf("%02d", max_tap[bl_index]);
			printf("\n\n");
		}
		/* treat corner cases */
		if ( (central_left_position == 0xFFFFFFFF) ||  (central_left_position == 0) ) {
			central_left_position = 1;
		}
		if ( central_right_position == 0xFFFFFFFF ) {
			central_right_position = max_tap[bl_index];
		}

		if ( (vref_high_position == 0xFFFFFFFF) || (vref_high_position == D2_R_VREF_MAX_VAL) ) {
			vref_high_position = D2_R_VREF_MAX_VAL -1;
		}

		if ( vref_low_position == 0xFFFFFFFF ) {
			vref_low_position = 0;
		}

		/* tune central_right_position and vref_low_position: they currently represent first 0 and not last 1 */
		central_right_position--;
		vref_low_position++;


		/* print right and left margins in pSec, high and low margins in voltage percents */

		central_left_margin = min_rdqs - central_left_position;
		central_right_margin = central_right_position - min_rdqs;

		vref_high_margin = (vref_high_position - res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL]);
		vref_low_margin = (res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL] - vref_low_position);

		central_left_margin_ps_mul1000 =  central_left_margin * res_vrtrdqsd[bl_index][D2_R_BL_TAP_PS];
		central_right_margin_ps_mul1000 = central_right_margin * res_vrtrdqsd[bl_index][D2_R_BL_TAP_PS];


		vref_high_margin_volt_mul100 = D2_READ_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(vref_high_position+1) -
				 D2_READ_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL]);

		vref_low_margin_volt_mul100 = D2_READ_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(res_vrtrdqsd[bl_index][D2_R_VREF_TR_VAL]) -
				 D2_READ_FIND_VOLT_PERCENT_VAL_OF_VREF_IDX(vref_low_position-1);

		if (print_enable) {
			printf("%-26s %u.%03u pSec (%u taps)\n",
					"central_left_margin = ",
					(central_left_margin_ps_mul1000 / 1000),
					(central_left_margin_ps_mul1000 % 1000),
					central_left_margin );

			printf("%-26s %u.%03u pSec (%u taps)\n",
					"central_right_margin = ",
					(central_right_margin_ps_mul1000 / 1000),
					(central_right_margin_ps_mul1000 % 1000),
					central_right_margin );

			printf("%-26s %u.%02u%% (%u indexes)\n",
					"vref_high_margin = ",
					(vref_high_margin_volt_mul100 / 100),
					(vref_high_margin_volt_mul100 % 100),
					vref_high_margin );

			printf("%-26s %u.%02u%% (%u indexes)\n",
					"vref_low_margin = ",
					(vref_low_margin_volt_mul100 / 100),
					(vref_low_margin_volt_mul100 % 100),
					vref_low_margin );
			printf("\n\n");
		}
		left_margin[bl_index] = central_left_margin;
		right_margin[bl_index] = central_right_margin;
		vref_read_min[bl_index] = vref_low_margin;
		vref_read_max[bl_index] = vref_high_margin;
	}
	if(print_enable)
		printf("\n");
#endif
}

int do_ddr_diagnostic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if 0
	u32 row, col, phy_index, ifc, bit, reg_data, phy;
	u32 bl_tap_num[BYTE_LANE_LEN][BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 bl_tap_mod[BYTE_LANE_LEN][BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 pbs[BYTE_LANE_LEN][BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 vref_set[BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 vref_max[BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 vref_min[BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 eye_set[BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 eye_min[BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 eye_max[BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 eye_tap_num[BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];
	u32 eye_tap_mod[BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS];

	printf("Diagnostic results:\n");
	for (ifc = 0; ifc < EMEM_MC_NUM_OF_BLOCKS; ifc++) {
		reg_data = read_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF);
		write_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF, 0x0);
		show_read_skew_margin(emem_mc_block_id[ifc], false);
		write_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF, reg_data);
		for(bit = 0; bit < BIT_ENTRIES_NUM; bit ++) {
			phy_index = (ifc * BYTE_LANES_NUM) + (bit / BYTE_LANE_LEN);
			bl_tap_num[bit % BYTE_LANE_LEN][phy_index] = res_rbd[bit][BL_TAP_PS]/1000;
			bl_tap_mod[bit % BYTE_LANE_LEN][phy_index] = res_rbd[bit][BL_TAP_PS]%1000;
			pbs[bit % BYTE_LANE_LEN][phy_index] = res_rbd[bit][TR_VAL];
		}
	}

	printf("%d\tMHz\t\t\t", current_ddr_params.clock_frequency);
	for (ifc = 0; ifc < EMEM_MC_NUM_OF_BLOCKS; ifc++)
		for (phy = 0; phy < BYTE_LANES_NUM; phy++)
			printf("UM%d_%d:%d\t", ifc / (EMEM_MC_NUM_OF_BLOCKS / 2),
						ifc % (EMEM_MC_NUM_OF_BLOCKS / 2),
						phy);
	printf("\n");

	for (col = 0; col < BYTE_LANE_LEN; col++) {
		printf("Rx\tDQ%d\tBL_TAP\tshow_rsm\t", col);
		for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
			printf("%u.%03u\t", bl_tap_num[col][row], bl_tap_mod[col][row]);
		printf("\n");
	}

	for (col = 0; col < BYTE_LANE_LEN; col++) {
		printf("Rx\tDQ%d\tPBS\tshow_rsm\t", col);
		for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
			printf("%u\t", pbs[col][row]);
		printf("\n");
	}
	printf("\n");

	for (ifc = 0; ifc < EMEM_MC_NUM_OF_BLOCKS; ifc++) {
		reg_data = read_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF);
		write_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF, 0x0);
		show_write_skew_margin(emem_mc_block_id[ifc], false);
		write_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF, reg_data);
		for(bit = 0; bit < BIT_ENTRIES_NUM; bit ++) {
			phy_index = (ifc * BYTE_LANES_NUM) + (bit / BYTE_LANE_LEN);
			bl_tap_num[bit % BYTE_LANE_LEN][phy_index] = res_wbd[bit][BL_TAP_PS]/1000;
			bl_tap_mod[bit % BYTE_LANE_LEN][phy_index] = res_wbd[bit][BL_TAP_PS]%1000;
			pbs[bit % BYTE_LANE_LEN][phy_index] = res_wbd[bit][TR_VAL];
		}
	}

	for (col = 0; col < BYTE_LANE_LEN; col++) {
		printf("Tx\tDQ%d\tBL_TAP\tshow_wsm\t", col);
		for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
			printf("%u.%03u\t", bl_tap_num[col][row], bl_tap_mod[col][row]);
		printf("\n");
	}

	for (col = 0; col < BYTE_LANE_LEN; col++) {
		printf("Tx\tDQ%d\tPBS\tshow_wsm\t", col);
		for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
			printf("%u\t", pbs[col][row]);
		printf("\n");
	}

	for (ifc = 0; ifc < EMEM_MC_NUM_OF_BLOCKS; ifc++) {
		reg_data = read_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF);
		write_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF, 0x0);
		d2_read_eye_plotting(emem_mc_block_id[ifc], false);
		write_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF, reg_data);
		for(bit = 0; bit < BYTE_LANES_NUM; bit++) {
			phy_index = (ifc * 4) + bit;
			vref_set[phy_index] = res_vrtrdqsd[bit][D2_VREF_TR_VAL];
			vref_max[phy_index] = vref_set[phy_index] + vref_read_max[bit] + 1;
			vref_min[phy_index] = vref_set[phy_index] - (vref_read_min[bit] + 1);

			eye_tap_num[phy_index] = res_vrtrdqsd[bit][D2_R_BL_TAP_PS] / 1000;
			eye_tap_mod[phy_index] = res_vrtrdqsd[bit][D2_R_BL_TAP_PS] % 1000;
			eye_set[phy_index] = min(res_vrtrdqsd[bit][D2_R_RDQSD_TR_VAL],
						res_vrtrdqsd[bit][D2_R_RDQSND_TR_VAL]);
			eye_min[phy_index] = eye_set[phy_index] - (left_margin[bit] + 1);
			eye_max[phy_index] = eye_set[phy_index] + right_margin[bit] + 1;
		}
	}
	printf("\nVref\tset\tHost\tRead_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", vref_set[row]);
	printf("\nVref\tmax\tHost\tRead_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", vref_max[row]);
	printf("\nVref\tmin\tHost\tRead_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", vref_min[row]);

	printf("\nbl\ttap\teye\tRead_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u.%03u\t", eye_tap_num[row], eye_tap_mod[row]);

	printf("\nRx\tset\teye\tRead_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", eye_set[row]);
	printf("\nRx\tleft\teye\tRead_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", eye_min[row]);
	printf("\nRx\tright\teye\tRead_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", eye_max[row]);

	for (ifc = 0; ifc < EMEM_MC_NUM_OF_BLOCKS; ifc++) {
		reg_data = read_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF);
		write_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF, 0x0);
		d2_write_eye_plotting(emem_mc_block_id[ifc], false);
		write_non_cluster_reg(emem_mc_block_id[ifc], EMEM_MC_REG_MC_DDR_IF, reg_data);
		for(bit = 0; bit < BYTE_LANES_NUM; bit++) {
			phy_index = (ifc * 4) + bit;
			vref_set[phy_index] = res_w_tr_val[bit];
			vref_max[phy_index] = vref_set[phy_index] + vref_write_max[bit] + 1;
			vref_min[phy_index] = vref_set[phy_index] - (vref_write_min[bit] + 1);

			eye_tap_num[phy_index] = res_vrtwdqd[bit][D2_BL_TAP_PS] / 1000;
			eye_tap_mod[phy_index] = res_vrtwdqd[bit][D2_BL_TAP_PS] % 1000;
			eye_set[phy_index] = res_vrtwdqd[bit][D2_WDQD_TR_VAL];
			eye_min[phy_index] = eye_set[phy_index] - (left_margin[bit] + 1);
			eye_max[phy_index] = eye_set[phy_index] + right_margin[bit] + 1;
		}
	}
	printf("\n");
	printf("\nVref\tset\tDram\tWrite_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", vref_set[row]);
	printf("\nVref\tmax\tDram\tWrite_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", vref_max[row]);
	printf("\nVref\tmin\tDram\tWrite_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", vref_min[row]);

	printf("\nbl\ttap\teye\tWrite_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u.%03u\t", eye_tap_num[row], eye_tap_mod[row]);

	printf("\nTx\tset\teye\tWrite_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", eye_set[row]);
	printf("\nTx\tleft\teye\tWrite_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", eye_min[row]);
	printf("\nTx\tright\teye\tWrite_2d\t");
	for(row = 0; row < BYTE_LANES_NUM * EMEM_MC_NUM_OF_BLOCKS; row++)
		printf("%u\t", eye_max[row]);
#endif
	return 0;
}

