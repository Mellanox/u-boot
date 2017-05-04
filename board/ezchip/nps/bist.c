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

#include <malloc.h>
#include <console.h>
#include "bist.h"
#include "bist_pattern.h"
#include "ddr.h"
#include "nps.h"

#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

static char *get_param_value(char	*line, char c);
static void read_bist_sram_line(u8 *data, u32 line, u32 mc);
static void write_bist_sram_line(struct sram_line_write *sram_line_params, u32 mc, u8 *const curr_pattern);
static void configure_bist(struct bist_parameters	*parameters);
static void execute_bist(struct bist_parameters	*parameters);
static void read_expected_data(u32 current_mc,
				u32 sram_line,
				u8 *pattern,
				struct bist_mc_result *mc_result);
static u32 print_error_info(u32	current_mc,
				u32	rel_sram_error_line,
				u32	sram_error_line,
				struct	bist_mc_result	*mc_result);
static bool stop_bist(u32	mc_mask);
static void bist_loop_rand( u32 mc_mask, u32 iterations, bool is_fixed_seed, u32 seed);
static void enable_emem_mc_hw(u32	mc_mask, enum bist_operation_mode op_mode);
static long print_log(char	*format, ...);
static bool get_bist_parameters(struct bist_parameters *params,
					   u32 lram_address);
static int get_bist_results(u32 mc_mask, u8 *pattern, bool stop);
static bool set_bist_pattern(struct bist_parameters *parameters);
static void show_configurations(u32 type);
static void print_input_params(struct bist_parameters *params);
static int bist_loop(u32 mc_mask, u32 iterations);
static void read_bist_loop(u32 mc_mask, u32 iterations, u32 read_num, bool is_fixed_seed, u32 seed);


static u8 rand_pattern[BIST_MAX_PATTERN_SIZE];
static u32 loop_results[BIST_NUM_OF_PATTERNS][EMEM_MC_NUM_OF_CONTROLLERS];
static u32 current_config;

static struct bist_parameters rand_pattern_config = {
		0x0, BIST_MAX_PATTERN_SIZE, NULL, SINGLE, WRITE_COMPARE,
		0x0, (1<<30),
		{{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF},
		{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF} }
};

static struct bist_parameters bist_configs[BIST_NUM_OF_PATTERNS] = {
	{0x0, BIST_PATTERN_0_SIZE, NULL, SINGLE, WRITE_COMPARE, 0x0, (1<<30),
		{{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF},
		{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF} } },
	{0x0, BIST_PATTERN_1_SIZE, NULL, SINGLE, WRITE_COMPARE, 0x0, (1<<30),
		{{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF},
		{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF} } },
	{0x0, BIST_PATTERN_2_SIZE, NULL, SINGLE, WRITE_COMPARE, 0x0, (1<<30),
		{{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF},
		{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF} } },
	{0x0, BIST_PATTERN_3_SIZE, NULL, SINGLE, WRITE_COMPARE, 0x0, (1<<30),
		{{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF},
		{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF} } },
	{0x0, BIST_PATTERN_4_SIZE, NULL, SINGLE, WRITE_COMPARE, 0x0, (1<<30),
		{{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF},
		{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF} } },
	{0x0, BIST_PATTERN_5_SIZE, NULL, SINGLE, WRITE_COMPARE, 0x0, (1<<30),
		{{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF},
		{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF} } },
	{0x0, BIST_PATTERN_6_SIZE, NULL, SINGLE, WRITE_COMPARE, 0x0, (1<<30),
		{{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF},
		{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF} } },
	{0x0, BIST_PATTERN_7_SIZE, NULL, SINGLE, WRITE_COMPARE, 0x0, (1<<30),
		{{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF},
		{0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF}, {0xF} } },
};

static	u8	ifc_bist[24][BIST_MAX_PATTERN_SIZE];

extern struct ddr_params  current_ddr_params;

static char *get_param_value(char *line, char	c)
{
	char *val;

	val = strchr(line, c);

	return ++val;
}

static void show_configurations(u32 type)
{
	u32 line, dq, byte, index = 0;
	const u32 pattern_lines =
		bist_configs[type].pattern_length / PATTERN_LINE_LENGTH;
	u8 *patterns[BIST_NUM_OF_PATTERNS];

	if (type > BIST_NUM_OF_PATTERNS) {
		error("show_configurations: Invalid type(%d)\n", type);
		return;
	}

	patterns[index++] = bist_pattern_0;
	patterns[index++] = bist_pattern_1;
	patterns[index++] = bist_pattern_2;
	patterns[index++] = bist_pattern_3;
	patterns[index++] = bist_pattern_4;
	patterns[index++] = bist_pattern_5;
	patterns[index++] = bist_pattern_6;
	patterns[index++] = bist_pattern_7;

	printf("Bist configuration #%d:\n", type);
	switch (bist_configs[type].rep_mode) {
	case SINGLE:
		printf("\trepetition mode = SINGLE\n");
		break;
	case ENDLESS:
		printf("\trepetition mode = ENDLESS\n");
		break;
	case UNTIL_ERROR:
		printf("\trepetition mode = UNTIL_ERROR\n");
		break;
	default:
		printf("\tUnknown repetition mode\n");
		break;
	}
	switch (bist_configs[type].op_mode) {
	case COMPARE_ONLY:
		printf("\toperation mode = COMPARE_ONLY\n");
		break;
	case WRITE_ONLY:
		printf("\toperation mode = WRITE_ONLY\n");
		break;
	case WRITE_COMPARE:
		printf("\toperation mode = WRITE_COMPARE\n");
		break;
	default:
		printf("\tUnknown operation mode\n");
		break;
	}
	printf("\taddress = 0x%x\n", bist_configs[type].address);
	printf("\tlength = 0x%x\n", bist_configs[type].length);
	printf("\tpattern_length = %d\n", bist_configs[type].pattern_length);
	printf("dq masks =\n");
	for (dq = 0; dq < DATA_QUEUES / 2; dq++)
		printf("\t0x%x", bist_configs[type].dq_masks[dq].mask);
	printf("\n");
	for (dq = DATA_QUEUES / 2; dq < DATA_QUEUES; dq++)
		printf("\t0x%x", bist_configs[type].dq_masks[dq].mask);
	printf("\n");
	printf("pattern =\n");
	for (line = 0; line < pattern_lines; line++) {
		for (byte = 0; byte < PATTERN_LINE_LENGTH; byte++)
			printf("\t0x%x",
			patterns[type][(line * PATTERN_LINE_LENGTH) + byte]);
		printf("\n");
	}
}

bool set_bist_pattern(struct bist_parameters *parameters)
{
	u32 lines, line, mc_block, bit_offset;
	struct sram_line_write bist_pattern_line;
	union emem_mc_ind_cmd ind_cmd;
	u8 *curr_pattern;
	if(parameters->op_mode == COMPARE_ONLY)
		return true;
	/* input parameters validations */
	if (parameters->pattern == NULL) {
		printf("ERROR: BIST data pattern pointer is NULL\n");
		return false;
	}
	if (parameters->pattern_length == 0) {
		printf("ERROR: BIST data pattern length can't be 0\n");
		return false;
	}
	if (parameters->pattern_length % PATTERN_LINE_LENGTH) {
		printf("ERROR: Pattern length isn't in multiples of 8(%u)\n",
						parameters->pattern_length);
		return false;
	}
	if (parameters->pattern_length > BIST_MAX_PATTERN_LENGTH_IN_BYTES) {
		printf("ERROR: Pattern length is bigger than 2048 bytes(%u)\n",
						parameters->pattern_length);
		return false;
	}
	lines = parameters->pattern_length / PATTERN_LINE_LENGTH;

	ind_cmd.fields.op = EMEM_MC_IND_CMD_OP_WRITE;
	ind_cmd.fields.mem_id = BIST_PATTERN_SRAM_ID;

	/* bist sram line size in bytes */
	bist_pattern_line.line_size = PATTERN_LINE_LENGTH;
	bist_pattern_line.address_register = EMEM_MC_REG_IND_ADDR;
	bist_pattern_line.command_register = EMEM_MC_REG_IND_CMD;

	for (mc_block = 0; mc_block < EMEM_MC_NUM_OF_BLOCKS; mc_block++) {
		bit_offset = mc_block * 2;
		ind_cmd.fields.ifc_bist_mc_selector = GET_BITS(parameters->mc_mask, bit_offset, 2);
		if (ind_cmd.fields.ifc_bist_mc_selector == 0)
			continue;
		bist_pattern_line.command = ind_cmd.reg;
		bist_pattern_line.block = emem_mc_block_id[mc_block];
		bist_pattern_line.address = 0;
		curr_pattern = parameters->pattern;
		/*bist_pattern_line.data = parameters->pattern;*/
		for (line = 0; line < lines; line++) {
			write_bist_sram_line(&bist_pattern_line, bit_offset, curr_pattern);
			bist_pattern_line.address++;
			curr_pattern += PATTERN_LINE_LENGTH;
		}
		bist_pattern_line.block++;
	}

	return true;
}

int start_bist(struct bist_parameters *parameters)
{
	bool test_status;
	int status = 0;

	configure_bist(parameters);
	execute_bist(parameters);
	if (parameters->rep_mode == SINGLE) {
		test_status = stop_bist(parameters->mc_mask);
		if (!test_status) {
			printf("Unable to stop test at single mode\n");
			return -1;
		}
		if (parameters->op_mode != WRITE_ONLY)
			status = get_bist_results(parameters->mc_mask,
					 parameters->pattern,
					 true);
		enable_emem_mc_hw(parameters->mc_mask, parameters->op_mode);
	}

	return status;
}

static void enable_emem_mc_hw(u32 mc_mask, enum bist_operation_mode op_mode)
{
	u32	index, block_addr, active_mc;
	union	emem_mc_mc_mi_if	mc_mi_if;

	for (index = 0; index < EMEM_MC_NUM_OF_BLOCKS; index++) {
		block_addr = emem_mc_block_id[index];
		active_mc = GET_BITS(mc_mask, index * 2, 2);
		if (active_mc != 0) {
			write_non_cluster_reg(block_addr,
						IFC_BIST_EN_ADDR,
						0);
			/* Re-enable ARB_ALG */
			mc_mi_if.reg = read_cached_register(MC_MI_IF_0 + index);
			mc_mi_if.fields.arb_algorithm = 1;
			write_non_cluster_reg(block_addr,
						   EMEM_MC_REG_MC_MI_IF,
						   mc_mi_if.reg);
		}
	}
}

static
bool stop_bist(u32 mc_mask)
{
	const u32 number_of_retries = 0xFFFFFFFF;;
	u32 curr_mc, try, active_mc, block_addr, index;
	union emem_mc_ifc_bist_status bist_status;
	union emem_mc_ifc_bist_en ifc_bist_en;

	ifc_bist_en.reg = 0;
	ifc_bist_en.fields.safe_cfg = 1;
	for (index = 0; index < EMEM_MC_NUM_OF_BLOCKS; index++) {
		block_addr = emem_mc_block_id[index];
		active_mc = GET_BITS(mc_mask, (index * 2), 2);
		if (active_mc == 0)
			continue;
		write_non_cluster_reg(block_addr,
					   IFC_BIST_EN_ADDR,
					   ifc_bist_en.reg);
		for (curr_mc = 0; curr_mc < NUMBER_OF_MC_IN_BLOCK; curr_mc++) {
			if (GET_BIT(active_mc, curr_mc) == 0)
				continue;
			try = 0;
			write_non_cluster_reg(block_addr,
						IFC_BIST_STAT_MUX_ADDR,
						curr_mc);
			while (try < number_of_retries) {
				udelay(1000);
				bist_status.reg = read_non_cluster_reg(block_addr,
							      IFC_BIST_STATUS_ADDR);
				if (bist_status.fields.end_of_test == 1)
					break;
				try++;
			}
			if (try == number_of_retries) {
				error("Unable to stop bist at device %d\n", index);
				return false;
			}
		}
	}

	return true;
}

static
void	configure_bist(struct bist_parameters	*parameters)
{
	u32 bit_idx, active_mc, bi, base_address;
	u32 high_address, block_addr, index, bit;
	u32 bit_val, mask_0 = 0, mask_1 = 0;
	u32 length, length_in_bank, seq_len;
	u8 dq_mask_0, dq_mask_1;
	union emem_mc_ifc_bist_timing_params ifc_bist_timing_params;
	union emem_mc_ifc_bist_cfg ifc_bist_cfg;

	length = parameters->length;
	/* Update timing parameters */
	ifc_bist_timing_params.reg = 0;
	ifc_bist_timing_params.fields.ack_data_gap = 0x2;
	ifc_bist_timing_params.fields.ack_addr_gap = 0x5;

	/* Prepare configuration register. */
	ifc_bist_cfg.reg = 0;
	seq_len = parameters->pattern_length / PATTERN_LINE_LENGTH;
	ifc_bist_cfg.fields.seq_len = seq_len;
	ifc_bist_cfg.fields.rep_num = parameters->rep_mode;
	ifc_bist_cfg.fields.addr_inc_type = BIST_ADDR_INC_TYPE_DISCRETE_BANK;

	if (parameters->op_mode == COMPARE_ONLY ||
	    parameters->op_mode == WRITE_COMPARE)
		ifc_bist_cfg.fields.rd_enable = 1;
	if (parameters->op_mode == WRITE_COMPARE ||
	    parameters->op_mode == WRITE_ONLY)
		ifc_bist_cfg.fields.wr_enable = 1;

	base_address = parameters->address / DDR_ADDRESS_SIZE;
	/* Get the length in addresses inside a bank */
	length_in_bank = (length / (DDR_ADDRESS_SIZE * GET_NUMBER_OF_BANKS(current_ddr_params.type)));

	if ((seq_len != 1) && (seq_len != 2) && (seq_len != 4) && (seq_len != 8)
		&& (length_in_bank % (GET_NUMBER_OF_BANKS
				(current_ddr_params.type) * seq_len * 4))) {
		printf("ERROR : Test length is invalid\n");
		printf("length_in_bank %d, seq_len %d length %d\n", length_in_bank, seq_len, length);
		return;
	}
	high_address = base_address + length_in_bank - 4;

	/* Write BIST configuration to the relevant parameters. *
	 * All devices are configured with the same values      */
	for (bi = 0; bi < EMEM_MC_NUM_OF_BLOCKS; bi++) {
		block_addr = emem_mc_block_id[bi];
		active_mc = GET_BITS(parameters->mc_mask, bi * 2, 2);
		if (active_mc != 0) {
			write_non_cluster_reg(block_addr,
							IFC_BIST_EN_ADDR, 0);
			/* Write Base\High addr data */
			write_cached_register(IFC_BIST_BASE_ADDR_ID_0 + bi,
					      base_address);
			write_non_cluster_reg(block_addr,
						   IFC_BIST_HIGH_ADDR,
						   high_address);
			write_non_cluster_reg(block_addr,
						IFC_BIST_TIMING_PARAMS_ADDR,
						ifc_bist_timing_params.reg);
			/* implement DQ translation to bit mask. */
			for (index = 0; index < (DATA_QUEUES / 2); index++) {
				dq_mask_0 = parameters->dq_masks[index].mask;
				dq_mask_1 =
					parameters->dq_masks[index + 8].mask;
				for (bit = 0; bit < 4; bit++) {
					bit_idx = (bit * 8) + index;
					bit_val = GET_BIT(dq_mask_0, bit);
					WRITE_BIT(mask_0, bit_idx, bit_val);
					bit_val = GET_BIT(dq_mask_1, bit);
					WRITE_BIT(mask_1, bit_idx, bit_val);
				}
			}
			/* Write BIST mask registers */
			write_non_cluster_reg(block_addr,
						   IFC_BIST_MASK_0_ADDR,
						   mask_0);
			write_non_cluster_reg(block_addr,
						   IFC_BIST_MASK_1_ADDR,
						   mask_1);
			write_cached_register(IFC_BIST_CFG_ID_0 + bi,
					      ifc_bist_cfg.reg);
		}
	}
}

static	void	execute_bist(struct bist_parameters	*parameters)
{
	union emem_mc_mc_mi_if mc_mi_if;
	union emem_mc_ifc_bist_en ifc_bist_en;
	u32 block_addr, active_mc, index;

	for (index = 0 ; index < EMEM_MC_NUM_OF_BLOCKS ; index++) {
		block_addr = emem_mc_block_id[index];
		active_mc = GET_BITS(parameters->mc_mask, index * 2, 2);
		if (active_mc != 0) {
			/* Disable ARB_ALG */
			mc_mi_if.reg = read_cached_register(MC_MI_IF_0 + index);
			mc_mi_if.fields.arb_algorithm = 0;
			write_non_cluster_reg(block_addr,
						   EMEM_MC_REG_MC_MI_IF,
						   mc_mi_if.reg);
			/* start bist */
			ifc_bist_en.reg = 0;
			ifc_bist_en.fields.safe_cfg = 1;
			write_non_cluster_reg(block_addr,
						   IFC_BIST_EN_ADDR,
						   ifc_bist_en.reg);
			ifc_bist_en.fields.val = active_mc;
			write_non_cluster_reg(block_addr,
						   IFC_BIST_EN_ADDR,
						   ifc_bist_en.reg);
		}
	}
}

int get_bist_results(u32	mc_mask, u8 *pattern, bool stop)
{
	u32 index, ba, rel_mc, abs_mc, chunk, reg_addr, couples, err_regs;
	u32 reg, i = 0, status = 0;
	ulong total_log_length = 0;
	union emem_mc_ifc_bist_status ifc_bist_status;
	union emem_mc_ifc_bist_err_data ifc_bist_err_data;
	struct bist_mc_result mc_result[EMEM_MC_NUM_OF_CONTROLLERS];

	couples = (IS_DDR4_DEVICE(current_ddr_params.type)) ?
				ERR_REG_COUPLES : (ERR_REG_COUPLES / 2);
	err_regs = couples * 2;

	/* Get BIST status */
	for (index = 0; index < EMEM_MC_NUM_OF_BLOCKS; index++) {
		ba = emem_mc_block_id[index];
		for (rel_mc = 0; rel_mc < NUMBER_OF_MC_IN_BLOCK; rel_mc++) {
			abs_mc = (index * 2) + rel_mc;
			if (GET_BIT(mc_mask, abs_mc) == 0)
				continue;
			memzero((void *)&mc_result[abs_mc],
				sizeof(struct bist_mc_result));
			/* Write statistic display mux register */
			write_non_cluster_reg(ba,
						   IFC_BIST_STAT_MUX_ADDR,
						   rel_mc);
			mc_result[abs_mc].errors_num =
				read_non_cluster_reg(ba,
						IFC_BIST_ERR_CNTR_ADDR);
			if (mc_result[abs_mc].errors_num > 0) {
				status = -1;
				loop_results[current_config][abs_mc] +=1;
				mc_result[abs_mc].error_address =
					read_non_cluster_reg(ba,
							IFC_BIST_ERR_ADDR);
				ifc_bist_status.reg =
					read_non_cluster_reg(ba,
							IFC_BIST_STATUS_ADDR);
				chunk = ifc_bist_status.fields.chunk;
				mc_result[abs_mc].error_bank = ifc_bist_status.fields.error_bank;
				i = 0;
				for (reg = 0; reg < couples; reg++) {
					reg_addr = IFC_BIST_ERR_DATA_0_ADDR +
						   ((err_regs - 2) -
						    (reg * 2));
					ifc_bist_err_data.reg =
						read_non_cluster_reg(ba,
								   reg_addr);
					mc_result[abs_mc].error_data[i++] =
					   ifc_bist_err_data.fields.byte_0_7;
					mc_result[abs_mc].error_data[i++] =
					   ifc_bist_err_data.fields.byte_8_15;
					mc_result[abs_mc].error_data[i++] =
					   ifc_bist_err_data.fields.byte_16_23;
					mc_result[abs_mc].error_data[i++] =
					   ifc_bist_err_data.fields.byte_24_31;
					ifc_bist_err_data.reg =
						read_non_cluster_reg(ba,
								reg_addr + 1);
					mc_result[abs_mc].error_data[i++] =
					   ifc_bist_err_data.fields.byte_0_7;
					mc_result[abs_mc].error_data[i++] =
					   ifc_bist_err_data.fields.byte_8_15;
					mc_result[abs_mc].error_data[i++] =
					   ifc_bist_err_data.fields.byte_16_23;
					mc_result[abs_mc].error_data[i++] =
					   ifc_bist_err_data.fields.byte_24_31;
				}
				read_expected_data(abs_mc,
						ifc_bist_status.fields.sram_entry_error,
						pattern,
						&mc_result[abs_mc]);
				total_log_length += print_error_info(abs_mc,
									chunk,
				    ifc_bist_status.fields.sram_entry_error,
							&mc_result[abs_mc]);
			} else if(get_debug())
				total_log_length +=
				print_log(" Memory device %d: BIST passed\n",
					  abs_mc);
		}
	}
	setenv_hex("log_length", total_log_length);

	return status;
}

static
void	read_expected_data(u32 current_mc,
			u32 sram_line,
			u8 *pattern,
			struct bist_mc_result *mc_result)
{
	u32 line, seq_len, index, total_lines;
	u32  chunk_size;
/*
	u32 partial, total_lines, full_bank, base_addr;
*/
	u32 block_index;

	union emem_mc_ifc_bist_cfg ifc_bist_cfg;

	chunk_size = (IS_DDR4_DEVICE(current_ddr_params.type)) ?
			MAX_BIST_BLOCK_SIZE : (MAX_BIST_BLOCK_SIZE / 2);
	block_index = current_mc / 2;
/*	base_addr =
		read_cached_register(IFC_BIST_BASE_ADDR_ID_0 + block_index);*/
	ifc_bist_cfg.reg =
		read_cached_register(IFC_BIST_CFG_ID_0 + block_index);
	seq_len = ifc_bist_cfg.fields.seq_len;

	/* full cycles to all banks in lines */
	total_lines = (NUMBER_OF_LINES_IN_CHUNK * GET_NUMBER_OF_BANKS(current_ddr_params.type))
			* (mc_result->error_address >> 2);
	total_lines += GET_BITS(mc_result->error_bank, 0, 2) * NUMBER_OF_LINES_IN_CHUNK * 4;
	total_lines += (GET_BITS(mc_result->error_bank, 2, 2) / 2)
							* NUMBER_OF_LINES_IN_CHUNK * 2;
	line = (total_lines % seq_len);

	for (index = 0; index < chunk_size; index += PATTERN_LINE_LENGTH) {
		read_bist_sram_line(mc_result->expected_data + index,
					line,
					current_mc);
		line = (line + 1) % seq_len;
	}
}

static long	print_log(char	*format, ...)
{
	static char *log_ptr = (char *)DDR_CONFIG_LRAM_ADDRESS;
	long string_len;
	va_list arg_list;

	va_start(arg_list, format);
	string_len = vsprintf(log_ptr, format, arg_list);
	if (string_len < 0) {
		printf("unable to copy string to log buffer\n");
		return 0;
	}
	log_ptr += string_len;
	vprintf(format, arg_list);
	va_end(arg_list);
	return string_len;
}

static	u32	print_error_info(u32 current_mc,
				 u32 rel_sram_error_line,
				 u32 sram_error_line,
				 struct bist_mc_result *mc_result)
{
	u32 bit_val, bit, current_char, index, chunk_size;
	struct dq_error_info timing_mask[DATA_QUEUES];
	u8 err_ch, exp_ch;
	u32 log_length = 0;
	/* input parameter */
	u32 ddr_type = 0, ddr_size = 0;

	memzero(timing_mask, DATA_QUEUES * sizeof(struct dq_error_info));
	chunk_size = (IS_DDR4_DEVICE(current_ddr_params.type)) ?
			MAX_BIST_BLOCK_SIZE : (MAX_BIST_BLOCK_SIZE / 2);
	for (index = 0; index < BYTES_TO_BITS(PATTERN_LINE_LENGTH); index++) {
		current_char = (rel_sram_error_line * PATTERN_LINE_LENGTH) +
				(index / PATTERN_LINE_LENGTH);
		exp_ch = mc_result->expected_data[current_char];
		err_ch = mc_result->error_data[current_char];
		bit = index % PATTERN_LINE_LENGTH;
		if (GET_BIT(exp_ch, bit) != GET_BIT(err_ch, bit)) {
			bit_val = (1 << ((index / PATTERN_LINE_LENGTH) % 4));
			timing_mask[BIST_GET_DQ(index)].clocking |= bit_val;
			if(GET_BIT(exp_ch, bit))
				timing_mask[BIST_GET_DQ(index)].from |= bit_val;
			else
				timing_mask[BIST_GET_DQ(index)].to |= bit_val;
		}
	}
	log_length += print_log("Controller %d :\n", current_mc);
	log_length += print_log("-----------------\n");
	log_length += print_log("Error address\t\t: 0x%x\n",
					mc_result->error_address);
	if (IS_16X_EXT_MEM_DEVICE(ddr_type)) {
		log_length += print_log("Column error address\t: 0x%x\n",
				GET_BITS(mc_result->error_address, 0, 7));
		log_length += print_log("Row error address\t: 0x%x\n",
				GET_BITS(mc_result->error_address, 7, 25));
	} else {
		/*  In x8 devices that are bigger than 1GB
		 *  ( total memory size bigger than 48GB ),
		 *  cloumn field size is one more bit.*/
		if (ddr_size >= (1024 * 48)) {
			log_length += print_log("Column error address\t: 0x%x\n",
				GET_BITS(mc_result->error_address, 0, 8));
			log_length += print_log("Row error address\t: 0x%x\n",
				GET_BITS(mc_result->error_address, 8, 24));
		} else {
			log_length += print_log("Column error address\t: 0x%x\n",
				GET_BITS(mc_result->error_address, 0, 7));
			log_length += print_log("Row error address\t: 0x%x\n",
				GET_BITS(mc_result->error_address, 7, 25));
		}
	}
	log_length += print_log("Error bank\t\t: 0x%x\n",
						mc_result->error_bank);
	log_length += print_log("Sram line\t\t: 0x%x\n", sram_error_line);
	log_length += print_log("Number of errors\t: 0x%x ( dec %d )\n",
			mc_result->errors_num, mc_result->errors_num);
	log_length += print_log("Error DQs :\n");
	for (index = 0; index < DATA_QUEUES; index++) {
		if (timing_mask[index].clocking != 0) {
			log_length += print_log("          DQ %2d (",
				index + (DATA_QUEUES * (current_mc % 2)));
			if (GET_BIT(timing_mask[index].clocking, 0))
				log_length += print_log(" First Rising %d->%d",
				GET_BIT(timing_mask[index].from, 0),
				GET_BIT(timing_mask[index].to, 0));
			if (GET_BIT(timing_mask[index].clocking, 1))
				log_length += print_log(" First Falling %d->%d",
				GET_BIT(timing_mask[index].from, 1),
				GET_BIT(timing_mask[index].to, 1));
			if (GET_BIT(timing_mask[index].clocking, 2))
				log_length += print_log(" Second Rising %d->%d",
				GET_BIT(timing_mask[index].from, 2),
				GET_BIT(timing_mask[index].to, 2));
			if (GET_BIT(timing_mask[index].clocking, 3))
				log_length += print_log(" Second Falling %d->%d",
				GET_BIT(timing_mask[index].from, 3),
				GET_BIT(timing_mask[index].to, 3));
			log_length += print_log(")\n");
		}
	}
	log_length += print_log("\n");
	log_length += print_log("Error Data:\n\t");
	for (index = 0; index < chunk_size; index++) {
		if ((index % PATTERN_LINE_LENGTH) == 0) {
			if (rel_sram_error_line == (index / PATTERN_LINE_LENGTH) - 1)
				printf("]");
			if(index == (MAX_BIST_BLOCK_SIZE / 2))
				log_length += print_log("\n\t ");
			else
				log_length += print_log(" ");
			if (rel_sram_error_line == (index / PATTERN_LINE_LENGTH))
				printf("[");
		}
		log_length += print_log("%02X",
			mc_result->error_data[index]);
	}
	if (rel_sram_error_line == (index / PATTERN_LINE_LENGTH) - 1)
		printf("]");
/*	log_length += print_log("\n");
	log_length += print_log("              ");
	for (index = 0; index < chunk_size; index++) {
		if ((index % PATTERN_LINE_LENGTH) == 0)
			log_length += print_log(" ");
		line = index / PATTERN_LINE_LENGTH;
		if (line == rel_sram_error_line)
			log_length += print_log("--");
		else
			log_length += print_log("  ");
	}*/
	log_length += print_log("\n");
	log_length += print_log("Expected Data:\n\t");
	for (index = 0; index < chunk_size; index++) {
		if ((index % PATTERN_LINE_LENGTH) == 0) {
			if (rel_sram_error_line == (index / PATTERN_LINE_LENGTH) - 1)
				printf("]");
			if(index == (MAX_BIST_BLOCK_SIZE / 2))
				log_length += print_log("\n\t ");
			else
				log_length += print_log(" ");
			if (rel_sram_error_line == (index / PATTERN_LINE_LENGTH))
				printf("[");
		}
		log_length += print_log("%02X",
		      mc_result->expected_data[index]);
	}
	if (rel_sram_error_line == (index / PATTERN_LINE_LENGTH) - 1)
		printf("]");
/*	log_length += print_log("\n");
	log_length += print_log("              ");
	for (index = 0; index < chunk_size; index++) {
		if ((index % PATTERN_LINE_LENGTH) == 0)
			log_length += print_log(" ");
		line = index / PATTERN_LINE_LENGTH;
		if (line == rel_sram_error_line)
			log_length += print_log("--");
		else
			log_length += print_log("  ");
	}*/
	log_length += print_log("\n");
	return log_length;
}

static
bool	get_bist_parameters(struct bist_parameters *params, u32 lram_address)
{
	char *current_line;
	char *next_line;
	char *param;
	char *next_param;
	u32 b, idx;

	printf("=== get_bist_parameters ===\n");
	current_line = (char *)lram_address;
	next_line = strchr(current_line, '\n');
	*next_line = '\0';

	while (strcmp(current_line, "****") != 0) {
		remove_spaces(current_line);
		if (*current_line == '#' || strlen(current_line) == 0) {
			current_line = next_line+1;
			next_line = strchr(current_line, '\n');
			*next_line = '\0';
			continue;
		}
		if (strstr(current_line, "pattern") != NULL) {
			if (strstr(current_line, "length") != NULL) {
				param = get_param_value(current_line, '=');
				params->pattern_length =
					simple_strtol(param, NULL, 10);
			} else {
				params->pattern =
						malloc(params->pattern_length);
				if (params->pattern == NULL) {
					printf(" memory allocation failed");
					return false;
				}
				param = get_param_value(current_line, '=');
				for (b = 0; b < params->pattern_length; b++) {
					next_param = strchr(param, ',');
					*next_param = '\0';
					params->pattern[b] =
						simple_strtol(param, NULL, 16);
					param = next_param + 1;
				}
			}
		} else if (strstr(current_line, "rep") != NULL) {
			param = get_param_value(current_line, '=');
			if (strcasecmp(param, "single") == 0)
				params->rep_mode = SINGLE;
			else if (strcasecmp(param, "endless") == 0)
				params->rep_mode = ENDLESS;
			else
				params->rep_mode = UNTIL_ERROR;
		} else if (strstr(current_line, "dqmask") != NULL) {
			param = get_param_value(current_line, '=');
			for (idx = 0; idx < DATA_QUEUES; idx++) {
				next_param = strchr(param, ',');
				*next_param = '\0';
				params->dq_masks[idx].mask =
						simple_strtol(param, NULL, 16);
				param = next_param + 1;
			}
		} else if (strstr(current_line, "operation") != NULL) {
			param = get_param_value(current_line, '=');
			if (strcasecmp(param, "write_only") == 0)
				params->op_mode = WRITE_ONLY;
			else if (strcasecmp(param, "compare_only") == 0)
				params->op_mode = COMPARE_ONLY;
			else
				params->op_mode = WRITE_COMPARE;
		} else if (strstr(current_line, "address") != NULL) {
			param = get_param_value(current_line, '=');
			params->address = simple_strtol(param, NULL, 16);
		} else if (strstr(current_line, "length") != NULL) {
			param = get_param_value(current_line, '=');
			params->length = simple_strtol(param, NULL, 10);
		} else {
			printf("ERROR: Unrecognized input line(%s)\n"
							, current_line);
			return false;
		}
		current_line = next_line+1;
		next_line = strchr(current_line, '\n');
		*next_line = '\0';
	}
	print_input_params(params);
	return true;
}

static
void print_input_params(struct bist_parameters *params)
{
	u32 idx;

	printf("BIST parameters:\n");
	printf(" \tpattern length = %d\n", params->pattern_length);
	printf(" \tpattern = ");
	for (idx = 0; idx < params->pattern_length; idx++)
		printf("0x%x ", params->pattern[idx]);
	printf("\n");
	printf(" \tdq mask = ");
	for (idx = 0; idx < DATA_QUEUES; idx++)
		printf("0x%x ", params->dq_masks[idx].mask);
	printf("\n");
	printf(" \tlength = %d\n", params->length);
	if (params->op_mode == COMPARE_ONLY)
		printf(" \top_mode = COMPARE_ONLY\n");
	else if (params->op_mode == WRITE_ONLY)
		printf(" \top_mode = WRITE_ONLY\n");
	else
		printf(" \top_mode = WRITE_COMPARE\n");
	if (params->rep_mode == ENDLESS)
		printf(" \trep_mode = ENDLESS\n");
	else if (params->rep_mode == SINGLE)
		printf(" \trep_mode = SINGLE\n");
	else
		printf(" \trep_mode = UNTIL_ERROR\n");
	printf("\tmc_mask = 0x%x\n", params->mc_mask);
}

static int address_bus_test(u32 mc_mask)
{
	const u32 initial_addr = 1;

	u32 test_data[2][CHUNK_SIZE_IN_BYTES / sizeof(u32)];
	u32 mem_data[CHUNK_SIZE_IN_BYTES / sizeof(u32)];
	u32 ifc, col, bank, bank_group, high_addr, addr, a;
	const u32 size = 16;

	bank_group = 0;
	bank = 0;
	col = 0;
	high_addr = get_max_row_num();

	for (ifc = 0; ifc < EMEM_MC_NUM_OF_BLOCKS; ifc++) {
		if(GET_BITS(mc_mask, (ifc * 2), 2) == 0)
			continue;
		/*Prepare a pattern and an anti-pattern*/
		memset(test_data[0], 0xA5, CHUNK_SIZE_IN_BYTES);
		memset(test_data[1], 0x5A, CHUNK_SIZE_IN_BYTES);
		/* Write default pattern at each offset */
		for (addr = initial_addr; addr <= high_addr; addr <<= 1)
			write_sdram_chunk((1 << (ifc * 2)), bank_group, bank, addr,
							col, size, test_data[0]);
		/* Check for address bits stuck high. */
		write_sdram_chunk((1 << (ifc * 2)), bank_group, bank, 0, col, size,
								test_data[1]);
		for (addr = initial_addr; addr <= high_addr; addr <<= 1) {
			memset(mem_data, 0x0, CHUNK_SIZE_IN_BYTES);
			read_sdram_chunk((1 << (ifc * 2)), bank_group, bank, addr,
								col, size, mem_data);
			/* Check if the read data contains the.expected data. */
			if (memcmp(mem_data, test_data[0], size))
				printf("Address bus error (stuck high)0x%x, ifc %d\n",
								addr, ifc);
		}
		write_sdram_chunk((1 << (ifc * 2)), bank_group, bank, 0, col, size,
								test_data[0]);
		for (addr = initial_addr; addr <= high_addr; addr <<= 1) {
			/* Write the anti pattern. */
			write_sdram_chunk((1 << (ifc * 2)), bank_group, bank, addr,
							col, size, test_data[1]);
			/* Check if bit stuck low */
			memset(mem_data, 0x0, CHUNK_SIZE_IN_BYTES);
			read_sdram_chunk((1 << (ifc * 2)), bank_group, bank, 0,
								col, size, mem_data);
			/* Check if the read data contains the expected data. */
			if (memcmp(mem_data, test_data[0], size))
				printf("Address bus error (stuck low)0x%x, ifc %d\n",
									addr, ifc);
			/* Check for shortage bits */
			for (a = initial_addr; a <= high_addr; a <<= 1) {
				memset(mem_data, 0x0, CHUNK_SIZE_IN_BYTES);
				read_sdram_chunk((1 << (ifc * 2)), bank_group, bank,
							a, col, size, mem_data);
				/* compare read data and expected data.*/
				if ((a != addr) &&
						memcmp(mem_data, test_data[0],
							size))
					printf("Address bus(shortage)0x%x ifc %d\n",
									addr, ifc);
			}
			/* Restore the pattern. */
			write_sdram_chunk((1 << (ifc * 2)), bank_group, bank, addr,
							col, size, test_data[0]);
		}
	}

	return 0;
}

static int data_bus_test(u32 mc_mask)
{
	u32 w1_w0_test_data[8] = { 0 };
	u32 mem_data[8] = { 0 };
	u32 mc, bit, line, byte, offset, index;
	u32 data_size = CHUNK_SIZE_IN_BYTES / 2;
	struct dq_error_info timing_mask[DATA_QUEUES];

	memzero(timing_mask, DATA_QUEUES * sizeof(struct dq_error_info));
	for (mc = 0; mc < EMEM_MC_NUM_OF_CONTROLLERS; mc++) {
		if (GET_BIT(mc_mask, mc) == 0)
			continue;
		w1_w0_test_data[0] = 0x1;
		/* go over 128 bits of data */
		for (bit = 0; bit < BYTES_TO_BITS(data_size); bit++) {
			write_sdram_chunk((1 << mc), 0, 0, 0,
						0, data_size, w1_w0_test_data);
			memset(mem_data, 0x0, data_size);
			read_sdram_chunk((1 << mc), 0, 0, 0, 0, data_size, mem_data);
			/* Check if the read data contains the expected data. */
			if (memcmp(mem_data, w1_w0_test_data, data_size)) {
				/* Print error data */
				printf("Error in data bus mc(%d) bit(%d):\n",
								mc, bit);
				printf("\tExpected data:\n");
				for (line = 0; line < 2; line++) {
					for (byte = 0; byte < 8; byte++) {
						offset = (line * 8) + byte;
						printf("\t0x%02x", GET_BITS(
						w1_w0_test_data[offset / 4],
						(byte % 4) * 8, 8));
					}
					printf("\n");
				}
				printf("\tRead data:\n");
				for (line = 0; line < 2; line++) {
					for (byte = 0; byte < 8; byte++) {
						offset = (line * 8) + byte;
						printf("\t0x%02x", GET_BITS(
						mem_data[offset / 4],
						(byte % 4) * 8, 8));
					}
					printf("\n");
				}
				index = bit % 64;
				timing_mask[BIST_GET_DQ(index)].clocking |=
					(1 << ((index / PATTERN_LINE_LENGTH) % 4));
			}
			memset(w1_w0_test_data, 0x0, data_size);
			/* Prepare data for next iteration */
			w1_w0_test_data[bit / BYTES_TO_BITS(sizeof(u32))] =
					(1 << ((bit+ 1) % (BYTES_TO_BITS(sizeof(u32)))) );
		}
		for (index = 0; index < DATA_QUEUES; index++) {
			if (timing_mask[index].clocking != 0) {
				printf("          DQ %2d (",
					index + (DATA_QUEUES * (mc % 2)));
				if (GET_BIT(timing_mask[index].clocking, 0))
					printf(" First Rising");
				if (GET_BIT(timing_mask[index].clocking, 1))
					printf(" First Falling");
				if (GET_BIT(timing_mask[index].clocking, 2))
					printf(" Second Rising");
				if (GET_BIT(timing_mask[index].clocking, 3))
					printf(" Second Falling");
				printf(")\n");
			}
		}
	}

	return 0;
}


static void write_bist_sram_line(struct sram_line_write *sram_line_params, u32 mc, u8 *const curr_pattern)
{
	u32 start;
	u8 line[PATTERN_LINE_LENGTH] = { 0 };

	start = sram_line_params->address * PATTERN_LINE_LENGTH;
	if (GET_BIT(sram_line_params->command, 12))
		memcpy(&ifc_bist[mc][start], curr_pattern,
						PATTERN_LINE_LENGTH);
	start = sram_line_params->address * PATTERN_LINE_LENGTH;
	if (GET_BIT(sram_line_params->command, 13))
		memcpy(&ifc_bist[mc+1][start], curr_pattern,
						PATTERN_LINE_LENGTH);
	memcpy(line, curr_pattern, PATTERN_LINE_LENGTH);
	/* big endianness */
	swap_line(line);
	sram_line_params->data = line;
	write_non_cluster_sram_line(sram_line_params);
}

#if 0
static void write_bist_sram_line(struct sram_line_write *sram_line_params, u32 mc)
{
	u32 start;

	start = sram_line_params->address * PATTERN_LINE_LENGTH;
	if (GET_BIT(sram_line_params->command, 12))
		memcpy(&ifc_bist[mc][start], sram_line_params->data,
						PATTERN_LINE_LENGTH);
	start = sram_line_params->address * PATTERN_LINE_LENGTH;
	if (GET_BIT(sram_line_params->command, 13))
		memcpy(&ifc_bist[mc+1][start], sram_line_params->data,
						PATTERN_LINE_LENGTH);
	write_non_cluster_sram_line(sram_line_params);
}
#endif
static void read_bist_sram_line(u8 *data, u32 line, u32 mc)
{
	u32 start;

	start = line * PATTERN_LINE_LENGTH;
	memcpy(data, &ifc_bist[mc][start], PATTERN_LINE_LENGTH);
}

enum bist_repetition_mode get_bist_rep_mode(char* mode)
{
	if (strcmp(mode, "endless") == 0)
		return ENDLESS;
	else if (strcmp(mode, "until_error") == 0)
		return UNTIL_ERROR;
	else if (strcmp(mode, "single") == 0)
		return SINGLE;
	return ERROR_REP_MODE;
}

enum bist_repetition_mode get_bist_op_mode(char* mode)
{
	if (strcmp(mode, "write") == 0)
		return WRITE_ONLY;
	else if (strcmp(mode, "compare") == 0)
		return COMPARE_ONLY;
	else if (strcmp(mode, "write_compare") == 0)
		return WRITE_COMPARE;
	return ERROR_OP_MODE;
}

int run_default_bist(u32 skip_mc_mask, int size)
{
	u32 status, i;
	struct bist_parameters bist_params;

	bist_params.pattern = bist_pattern_0;
	bist_params.mc_mask = skip_mc_mask;
	bist_params.pattern_length = BIST_PATTERN_0_SIZE;
	bist_params.rep_mode = SINGLE;
	bist_params.op_mode = WRITE_COMPARE;
	bist_params.address = 0x0;
	bist_params.length = ( size / EMEM_MC_NUM_OF_CONTROLLERS )*(1<<10)*(1<<10);
	for(i = 0; i < DATA_QUEUES; i++)
		bist_params.dq_masks[i].mask = 0xf;
	if (!set_bist_pattern(&bist_params)) {
		printf("set_bist_pattern failed\n");
		return -1;
	}
	status = start_bist(&bist_params);

	return status;
}

int init_ddr_by_bist(u32 skip_mc_mask, int size)
{
	u32 status, i;
	struct bist_parameters bist_params;

	bist_params.pattern = bist_pattern_6;
	bist_params.mc_mask = skip_mc_mask;
	bist_params.pattern_length = BIST_PATTERN_6_SIZE;
	bist_params.rep_mode = SINGLE;
	bist_params.op_mode = WRITE_ONLY;
	bist_params.address = 0x0;
	bist_params.length = ( size / EMEM_MC_NUM_OF_CONTROLLERS )*(1<<10)*(1<<10);
	for(i = 0; i < DATA_QUEUES; i++)
		bist_params.dq_masks[i].mask = 0xf;
	if (!set_bist_pattern(&bist_params)) {
		printf("set_bist_pattern failed\n");
		return -1;
	}
	status = start_bist(&bist_params);

	return status;
}

void set_full_ddr_size(int size)
{
	int full_ddr_size, index;

        full_ddr_size = ( size / EMEM_MC_NUM_OF_CONTROLLERS )*(1<<10)*(1<<10);
        rand_pattern_config.length = full_ddr_size;
        for (index = 0; index < BIST_NUM_OF_PATTERNS; index++)
                bist_configs[index].length = full_ddr_size;

}

int do_ddr_bist(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct bist_parameters params;
	u32 index = 0, lram_address, mc_mask, type, seed, iterations;
	u32 read_num;
	bool status, stop, test_status;
	u8 *patterns[BIST_NUM_OF_PATTERNS];
	int change_byte;

	memzero((void *)&params, sizeof(struct bist_parameters));

	patterns[index++] = bist_pattern_0;
	patterns[index++] = bist_pattern_1;
	patterns[index++] = bist_pattern_2;
	patterns[index++] = bist_pattern_3;
	patterns[index++] = bist_pattern_4;
	patterns[index++] = bist_pattern_5;
	patterns[index++] = bist_pattern_6;
	patterns[index++] = bist_pattern_7;

	if (strcmp(argv[1], "show") == 0) {
		type = simple_strtol(argv[2], NULL, 10);
		show_configurations(type);
	} else if (strcmp(argv[1], "run") == 0) {
		type = simple_strtol(argv[3], NULL, 10);
		/* get parameters from LRAM */
		if (type == BIST_TFTP_MODE) { /* get params through ftp */
			params.mc_mask = simple_strtol(argv[2], NULL, 16);
			lram_address = simple_strtol(argv[4], NULL, 16);
			status = get_bist_parameters(&params, lram_address);
			if (!status) {
				printf("get bist parameters failed\n");
				return 0;
			}
		} else if (type == BIST_RANDOM_MODE) { /* random pattern */
			if (argc < 5) {
				error("Seed is mandatory in random mode");
				return -1;
			}
			seed = simple_strtol(argv[4], NULL, 10);
			if (seed == 0) {
				error("0 is an Invalid seed number");
				return -1;
			}
			srand(seed);
			for (index = 0; index < BIST_MAX_PATTERN_SIZE; index++)
				rand_pattern[index] = rand() & 0xFF;
			memcpy(&params, &rand_pattern_config,
					sizeof(struct bist_parameters));
			params.mc_mask = simple_strtol(argv[2], NULL, 16);
			if (argc >= 6) {
				params.rep_mode = get_bist_rep_mode(argv[5]);
				if(params.rep_mode == ERROR_REP_MODE) {
					error("Invalid repetition mode");
					return -1;
				}
			}
			if(argc >= 7) {
				params.op_mode = get_bist_op_mode(argv[6]);
				if(params.op_mode == ERROR_OP_MODE) {
					error("Invalid operation mode");
					return -1;
				}
			}
			params.pattern = rand_pattern;
			print_input_params(&params);
		} else {
			if (type > BIST_NUM_OF_PATTERNS) {
				printf("Invalid configuration\n");
				return -1;
			}
			if(argc < 4) {
				printf("Missing parameters in command line\n");
				return -1;
			}
			memcpy(&params, &bist_configs[type],
					sizeof(struct bist_parameters));
			if (argc >= 5) {
				params.rep_mode = get_bist_rep_mode(argv[4]);
				if(params.rep_mode == ERROR_REP_MODE) {
					error("Invalid repetition mode");
					return -1;
				}
			}
			if (argc >= 6) {
				params.op_mode = get_bist_op_mode(argv[5]);
				if(params.op_mode == ERROR_OP_MODE) {
					error("Invalid operation mode");
					return -1;
				}
			}
			params.pattern = patterns[type];
			params.mc_mask = simple_strtol(argv[2], NULL, 16);
			print_input_params(&params);
		}
		if (!set_bist_pattern(&params)) {
			printf("set_bist_pattern failed\n");
			return -1;
		}
		start_bist(&params);
		if (type == 10)
			free(params.pattern);
	} else if (strcmp(argv[1], "dump") == 0) {
		mc_mask = simple_strtol(argv[2], NULL, 16);
		stop = (argc == 4) ? true:false;
		if(stop) {
			test_status = stop_bist(mc_mask);
			if (!test_status) {
				printf("stop bist failed\n");
				return -1;
			}
			get_bist_results(mc_mask, bist_pattern_0, stop);
			enable_emem_mc_hw(mc_mask, WRITE_COMPARE);
		}
		else
			get_bist_results(mc_mask, bist_pattern_0, stop);
	} else if (strcmp(argv[1], "pattern") == 0) {
		mc_mask = simple_strtol(argv[2], NULL, 16);
		type = simple_strtol(argv[3], NULL, 10);
		if(argc > 3)
			change_byte = simple_strtol(argv[4], NULL, 10);
		else
			change_byte = -1;
		if(type >= BIST_NUM_OF_PATTERNS) {
			error("Invalid config number");
			return -1;
		}
		memcpy(&params, &bist_configs[type],
				sizeof(struct bist_parameters));

		params.mc_mask = mc_mask;
		params.pattern = patterns[type];

		if (change_byte >= params.pattern_length) {
			error("Change byte (%d) is out of range (%d)",
				change_byte, params.pattern_length);
			return -1;
		}
		if (change_byte >= 0)
			params.pattern[change_byte] = ~params.pattern[change_byte];
		if (!set_bist_pattern(&params)) {
			printf("set_bist_pattern failed\n");
			return -1;
		}
		if (change_byte >= 0)
			params.pattern[change_byte] = ~params.pattern[change_byte];
	} else if (strcmp(argv[1], "loop") == 0) {
		mc_mask = simple_strtol(argv[2], NULL, 16);
		iterations = simple_strtol(argv[3], NULL, 10);
		if(argc >= 6) {
			seed = (u32)simple_strtol(argv[5], NULL, 10);
			if (strcmp(argv[4], "start_seed") == 0)
				bist_loop_rand(mc_mask, iterations, false, seed);
			else if (strcmp(argv[4], "fixed_seed") == 0)
				bist_loop_rand(mc_mask, iterations, true, seed);
			else {
				error("invalid start / fixed seed parameter");
				return -1;
			}
		} else
			bist_loop(mc_mask, iterations);
	} else if (strcmp(argv[1], "rd_loop") == 0) {
		if(argc < 6) {
			error("Illegal number of parameters");
			return -1;
		}
		mc_mask = simple_strtol(argv[2], NULL, 16);

		iterations = simple_strtol(argv[3], NULL, 10);
		read_num = simple_strtol(argv[4], NULL, 10);
		seed = simple_strtol(argv[6], NULL, 10);
		if (strcmp(argv[5], "start_seed") == 0)
			read_bist_loop(mc_mask, iterations, read_num, false, seed);
		else if (strcmp(argv[5], "fixed_seed") == 0)
			read_bist_loop(mc_mask, iterations, read_num, true, seed);
		else {
			error("invalid start / fixed seed parameter");
			return -1;
		}
	} else {
		printf("Invalid operation (%s)\n", argv[1]);
		return 0;
	}

	return 0;
}

static void read_bist_loop(u32 mc_mask,  u32 iterations, u32 read_num, bool is_fixed_seed, u32 seed)
{
	u32 bist, index, reads;
	struct bist_parameters params;

	memcpy(&params, &rand_pattern_config,
				sizeof(struct bist_parameters));
	params.mc_mask = mc_mask;
	for (bist = 0; bist < iterations; bist++) {
		srand(seed);
		params.op_mode = WRITE_ONLY;
		printf("writing seed %d\n", seed);
		for (index = 0; index < BIST_MAX_PATTERN_SIZE; index++)
			rand_pattern[index] = rand();
		params.pattern = rand_pattern;
		if (!set_bist_pattern(&params)) {
			printf("set_bist_pattern failed\n");
			return;
		}
		start_bist(&params);
		udelay(10);
		for (reads = 0; reads < read_num; reads++) {
			params.op_mode = COMPARE_ONLY;
			start_bist(&params);
			udelay(10);
			printf(".");
		}
		if(!is_fixed_seed)
			seed = rand()%0xFFFFFF;
		printf("\n");
	}
}

static void bist_loop_rand( u32 mc_mask, u32 iterations, bool is_fixed_seed, u32 seed)
{
	u32 bist, index;
	struct bist_parameters params;

	memzero((void *)loop_results, sizeof(loop_results));
	memcpy(&params, &rand_pattern_config,
				sizeof(struct bist_parameters));
	params.mc_mask = mc_mask;
	for (bist = 0; bist < iterations; bist++) {
		printf("Seed %u\n", seed);
		srand(seed);
		for (index = 0; index < BIST_MAX_PATTERN_SIZE; index++)
			rand_pattern[index] = rand();
		params.pattern = rand_pattern;
		if (!set_bist_pattern(&params)) {
			printf("set_bist_pattern failed\n");
			return;
		}
		if(ctrlc())
			break;
		start_bist(&params);
		udelay(10);
		if(!is_fixed_seed)
			seed = rand()%0xFFFFFFFF;
	}
	printf("\n");
}

static int bist_loop(u32 mc_mask, u32 iterations)
{
	u32 bist, mc, config, index = 0;
	u8 *patterns[BIST_NUM_OF_PATTERNS];
	struct bist_parameters params;

	memzero((void *)&params, sizeof(struct bist_parameters));

	patterns[index++] = bist_pattern_0;
	patterns[index++] = bist_pattern_1;
	patterns[index++] = bist_pattern_2;
	patterns[index++] = bist_pattern_3;
	patterns[index++] = bist_pattern_4;
	patterns[index++] = bist_pattern_5;
	patterns[index++] = bist_pattern_6;
	patterns[index++] = bist_pattern_7;

	memzero((void *)loop_results, sizeof(loop_results));

	for(config = 0; config < BIST_NUM_OF_PATTERNS; config++) {
		current_config = config;
		memcpy(&params, &bist_configs[config],
				sizeof(struct bist_parameters));
		params.mc_mask = mc_mask;
		params.pattern = patterns[config];
		if (!set_bist_pattern(&params)) {
			printf("set_bist_pattern failed\n");
			return -1;
		}
		for (bist = 0; bist < iterations; bist++) {
			printf(".");
			if(ctrlc())
				break;
			start_bist(&params);
			udelay(10);
		}
	}
	printf("\n");
	for(config = 0; config < BIST_NUM_OF_PATTERNS; config++) {
		printf("config %d :\n", config);
		for (mc = 0; mc < EMEM_MC_NUM_OF_CONTROLLERS; mc++)
			printf("\t %d", loop_results[config][mc]);
		printf("\n");
	}

	return 0;
}

int do_bus_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 mc_mask;

	if (argc < 2) {
		error("Invalid command line");
		return -1;
	}
	mc_mask = simple_strtol(argv[1], NULL, 16);
	if (strcmp("address", argv[2]) == 0)
		address_bus_test(mc_mask);
	else if (strcmp("data", argv[2]) == 0)
		data_bus_test(mc_mask);
	else
		error("Invalid bus name");

	return 0;
}

