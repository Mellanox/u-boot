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

#ifndef _BIST_H_
#define _BIST_H_

#include <command.h>
#include <asm/arcregs.h>
#include <asm/io.h>
#include "ddr.h"
#include "common.h"

#define BIST_TFTP_MODE				10
#define BIST_RANDOM_MODE			11

/* BIST */
#define	PATTERN_LINE_LENGTH			8
#define	BIST_MAX_PATTERN_LENGTH_IN_BYTES	2048
#define	BIST_PATTERN_SRAM_ID			0x2
#define	BIST_ADDR_INC_TYPE_DISCRETE_BANK	1
#define	ERR_REG_COUPLES				16
#define	MAX_BIST_BLOCK_SIZE			128
#define	NUMBER_OF_LINES_IN_CHUNK		8
#define  BIST_GET_DQ(__bit_ind)\
	((__bit_ind % PATTERN_LINE_LENGTH) +\
		(PATTERN_LINE_LENGTH * (__bit_ind / (DATA_QUEUES * 2))))

enum bist_operation_mode {
	COMPARE_ONLY,
	WRITE_ONLY,
	WRITE_COMPARE,
	ERROR_OP_MODE
};

enum bist_repetition_mode {
	ENDLESS,
	SINGLE,
	UNTIL_ERROR,
	ERROR_REP_MODE
};

union	dq_mask {
	u8	mask;
	struct {
		u8	reserved:4;
		u8	second_falling:1;
		u8	second_rising:1;
		u8	first_falling:1;
		u8	first_rising:1;
	} fields;
};

struct dq_error_info {
	u32 clocking;
	u32 from;
	u32 to;
};

struct bist_mc_result {
	u32	errors_num;
	u32	error_bank;
	u32	error_bank_group;
	u32	error_address;
	u32	error_data_queues[DATA_QUEUES];
	u8	error_data[MAX_BIST_BLOCK_SIZE];
	u8	expected_data[MAX_BIST_BLOCK_SIZE];
};

struct bist_parameters {
	u32				mc_mask;
	u32				pattern_length;
	u8				*pattern;
	enum	bist_repetition_mode	rep_mode;
	enum	bist_operation_mode	op_mode;
	u32				address;
	/* length of each mc in bytes */
	u32				length;
	union	dq_mask			dq_masks[DATA_QUEUES];
};

union emem_mc_ifc_bist_err_data {
	u32	reg;
	struct {
		u32	byte_24_31:8;
		u32	byte_16_23:8;
		u32	byte_8_15:8;
		u32	byte_0_7:8;
	} fields;
};

union emem_mc_ifc_bist_timing_params {
	u32	reg;
	struct {
		u32 reserved21_31:11;
		u32 ack_data_gap:5;
		u32 reserved13_15:3;
		u32 ack_addr_gap:5;
		u32 reserved0_7:8;
	} fields;
};

union emem_mc_ifc_bist_cfg {
	u32	reg;
	struct {
		u32 reserved20_31:12;
		u32 ptrn_seq:1;
		u32 ptrn_mode:1;
		u32 rd_enable:1;
		u32 wr_enable:1;
		u32 seq_len:9;
		u32 inc_addr_operand:2;
		u32 addr_inc_type:1;
		u32 test_mode:2;
		u32 rep_num:2;
	} fields;
};

union emem_mc_ifc_bist_en {
	u32	reg;
	struct {
		u32	safe_cfg:1;
		u32	reserved2_30:29;
		u32	val:2;
	} fields;
};

union emem_mc_ifc_bist_status {
	u32	reg;
	struct {
		u32	reserved24_31:8;
		u32	error_partner_bank:4;
		u32	chunk:4;
		u32	sram_entry_error:8;
		u32	error_bank:4;
		u32	reserved2_3:2;
		u32	test_status:1;
		u32	end_of_test:1;
	} fields;
};
static inline void swap_line(u8* line)
{
	swap(line[0], line[3]);
	swap(line[1], line[2]);
	swap(line[4], line[7]);
	swap(line[5], line[6]);
}

extern const int emem_mc_block_id[EMEM_MC_NUM_OF_BLOCKS];
extern struct ddr_params  current_ddr_params;
/* TODO: go over input parameters validation */
/* TODO : check if should be moved to static */
int do_ddr_bist(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_bus_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_bist_results(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_bist_loop(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int start_bist(struct bist_parameters *parameters);
int run_default_bist(u32 skip_mc_mask, int size);
int init_ddr_by_bist(u32 skip_mc_mask, int size);
void set_full_ddr_size(int size);
#endif /* _BIST_H_ */
