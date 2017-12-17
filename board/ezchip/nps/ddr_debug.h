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

#ifndef _DDR_DEBUG_H_
#define _DDR_DEBUG_H_

#include <common.h>


#define PUB_REG_NAME_MAX_LEN 80
#define PUB_DDR_PHY_REGS_NUMBER 296

typedef struct _pub_reg_record {
	char name[PUB_REG_NAME_MAX_LEN];
	u32 address;
	u32 val;
} pub_reg_record;


typedef struct _pub_ddr_phy_port_record {
	pub_reg_record reg_record[PUB_DDR_PHY_REGS_NUMBER];
} pub_ddr_phy_port_record;



int do_select_pup_emi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_mem_write_loop(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_mem_read_loop(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_phy_vref_prob(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_mem_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_mem_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ato_probing(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_dto_probing(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_sdram_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_sdram_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_phy_bist(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_marg_and_d2(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_vref(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_edit_results(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ddr_basic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_fifo_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_post_training(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#ifndef CONFIG_TARGET_NPS_HE
int do_configure_emem(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif
int do_ddr_training_steps(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_get_ddr_freq(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
void init_ddr_phy_record_DB(void);
int parse_UM_port_format(char * const UMstr, u32 *pside, u32 *pnum);
void print_pub_dump(u32 block_idx);
int ddr_int_loop(void);
int do_ddr_diagnostic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

#endif
