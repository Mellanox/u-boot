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
#include <asm/arcregs.h>
#include <asm/io.h>
#include <spi.h>
#include <spi_flash.h>
#include "common.h"
#ifdef CONFIG_TARGET_NPS_HE
#include "ddr_he.h"
#else
#include "ddr.h"
#include "bist.h"
#include "chip.h"
#endif

static int dump_reg(u32 block, char *reg_list);
static bool disable_print = true;

static struct cached_reg reg_db[NUM_OF_CACHED_REGS] = {
	{ EMEM_MC_0_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_1_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_2_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_3_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_4_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_5_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_6_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_7_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_8_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_9_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_10_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_11_ADDR, IFC_BIST_CFG_ADDR, 0x80 },
	{ EMEM_MC_0_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_1_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_2_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_3_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_4_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_5_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_6_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_7_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_8_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_9_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_10_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_11_ADDR, IFC_BIST_BASE_ADDR, 0x0 },
	{ EMEM_MC_0_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_1_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_2_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_3_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_4_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_5_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_6_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_7_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_8_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_9_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_10_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_11_ADDR, EMEM_MC_REG_EXT_MC_LATENCY, 0x0 },
	{ EMEM_MC_0_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_1_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_2_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_3_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_4_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_5_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_6_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_7_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_8_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_9_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_10_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_11_ADDR, EMEM_MC_REG_PHY_UPDATE, 0x801 },
	{ EMEM_MC_0_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_1_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_2_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_3_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_4_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_5_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_6_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_7_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_8_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_9_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_10_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL },
	{ EMEM_MC_11_ADDR, EMEM_MC_REG_MC_MI_IF, MC_MI_IF_INIT_VAL }
};

#ifdef CONFIG_TARGET_NPS_SIM
void flash_to_mem(u32 src_addr, u32 dst_addr, u32 size)
{
	/* source address */
	write_aux_reg(COPY_ACCELERATOR_SRC_AUX_REG, src_addr);

	/* destination address */
	write_aux_reg(COPY_ACCELERATOR_DST_AUX_REG, dst_addr);

	/* size */
	write_aux_reg(COPY_ACCELERATOR_SIZE_AUX_REG, size);

	/* command register */
	write_aux_reg(COPY_ACCELERATOR_CMD_AUX_REG, ACCELERATOR_CMD_TRIGGER);
}
#else
void flash_to_mem(u32 src_addr, u32 dst_addr, u32 size)
{
	struct spi_flash *flash;

	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	if (!flash) {
		puts("SPL: SPI probe failed.\n");
		hang();
	}

	spi_flash_read(flash, src_addr, size, (void *)dst_addr);
}
#endif

void write_cluster_reg(u32 cluster_x, u32 cluster_y,
					u32 sub_block, u32 reg, u32 value)
{
	u32 reg_addr;

	reg_addr = CONFIGURATION_BASE + (cluster_x << 20) + (cluster_y << 16)
			+ (sub_block << 10) + (reg << 2);

	out_be32((void *)reg_addr, value);
}

void write_non_cluster_reg(u32 block, u32 reg, u32 value)
{
	u32 reg_addr;

	reg_addr = CONFIGURATION_BASE + (block << 14) + (reg << 2);
#ifndef CONFIG_SPL_BUILD
	if (!disable_print)
		printf("R_W A 0x%03X%03X D 0x%08X\n", block, reg, value);
#endif
	out_be32((void *) reg_addr, value);
}

void write_cached_register(enum emem_mc_cached reg_id, u32 value)
{
	reg_db[reg_id].val = value;
	write_non_cluster_reg(reg_db[reg_id].block,
			reg_db[reg_id].addr, value);
}

u32 read_cached_register(enum emem_mc_cached reg_id)
{
#ifndef CONFIG_SPL_BUILD
	if (!disable_print)
		printf("R_R(Cached) A 0x%03X%03X D 0x%08X\n", reg_db[reg_id].block,
				reg_db[reg_id].addr, reg_db[reg_id].val);
#endif
	return reg_db[reg_id].val;
}

u32 read_cluster_reg(u32 cluster_x, u32 cluster_y, u32 sub_block, u32 reg)
{
	u32 reg_addr;

	reg_addr = CONFIGURATION_BASE + (cluster_x << 20) + (cluster_y << 16)
			+ (sub_block << 10) + (reg << 2);

	return in_be32((void *)reg_addr);
}

u32 read_non_cluster_reg(u32 block, u32 reg)
{
	u32 reg_value, reg_addr;
	reg_addr = CONFIGURATION_BASE + (block << 14) + (reg << 2);
	reg_value = in_be32((void *)reg_addr);
#ifndef CONFIG_SPL_BUILD
	if (!disable_print)
		printf("R_R A 0x%03X%03X D 0x%08X\n", block, reg, reg_value);
#endif
	return reg_value;
}

void write_non_cluster_sram_line(struct sram_line_write *sram_line_params)
{
	u32 data_index, value, line_size;
	u32 data_registers;
	u32 *pattern = (u32 *)sram_line_params->data;

	line_size = sram_line_params->line_size;
	data_registers = line_size / REGISTER_SIZE_IN_BYTES;
	for (data_index = 0; data_index < data_registers; data_index++) {
		value = *pattern++;
		write_non_cluster_reg(sram_line_params->block, data_index, value);
	}
	write_non_cluster_reg(sram_line_params->block,
			sram_line_params->address_register, sram_line_params->address);
	write_non_cluster_reg(sram_line_params->block,
			sram_line_params->command_register, sram_line_params->command);
}

void read_non_cluster_sram_line(struct sram_line_write *sram_line_params)
{
	u32 data_registers, line_size, data_index, value;
	u32 try, retries = 1000;
	u32 sts_reg_addr = 0x15;/*EMEM_MC_IND_STS_REG_ADDR;*/

	line_size = sram_line_params->line_size;
	data_registers = line_size / REGISTER_SIZE_IN_BYTES;
	write_non_cluster_reg(sram_line_params->block,
			sram_line_params->address_register, sram_line_params->address);
	write_non_cluster_reg(sram_line_params->block,
			sram_line_params->command_register, sram_line_params->command);
	for (try = 0; try < retries; try++) {
		value = read_non_cluster_reg(sram_line_params->block, sts_reg_addr);
		if ((value & 0x3) == 0x3)
			break;
	}
	if ((value & 0x3) != 0x3) {
		printf("Read sram failed. Status = 0x%x\n", value);
		return;
	}
	for (data_index = 0; data_index < data_registers; data_index++) {
		value = read_non_cluster_reg(sram_line_params->block, data_index);
		((u32 *) (sram_line_params->data))[data_index] = value;
	}
}

void remove_spaces(char *source)
{
	char *i = source;
	char *j = source;

	while (*j != 0) {
		*i = *j++;
		if (*i != ' ')
			i++;
	}
	*i = 0;
}

void set_debug(bool dop)
{
	disable_print = !dop;
}

bool get_debug(void)
{
	return !disable_print;
}

int do_write_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 block, reg, value, x, y;

	if (strcmp("ALL_MC", argv[1]) == 0) {
		reg = (u32) simple_strtol(argv[2], NULL, 16);
		value = (u32) simple_strtol(argv[3], NULL, 16);
		for (block = 0; block < EMEM_MC_NUM_OF_BLOCKS; block++)
			write_non_cluster_reg(GET_EMEM_MC_BLOCK_ADDR(block), reg, value);
	} else {
		block = (u32) simple_strtol(argv[1], NULL, 16);
		if(argc < 5) {
			reg = (u32) simple_strtol(argv[2], NULL, 16);
			value = (u32) simple_strtol(argv[3], NULL, 16);
			write_non_cluster_reg(block, reg, value);
		} else {
			x = (u32) simple_strtol(argv[2], NULL, 10);
			y = (u32) simple_strtol(argv[3], NULL, 10);
			reg = (u32) simple_strtol(argv[4], NULL, 16);
			value = (u32) simple_strtol(argv[5], NULL, 16);
			write_cluster_reg(x, y, block, reg, value);
		}
	}

	return 0;
}

int do_read_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 block, reg, value, x, y;
	u32 idx = 1;

	block = (u32) simple_strtol(argv[idx++], NULL, 16);
	/* check if list of registers was requested*/
	if(dump_reg(block, argv[idx]) < 0) {
		if(argc < 4) {
			reg = (u32) simple_strtol(argv[idx++], NULL, 16);
			value = read_non_cluster_reg(block, reg);
			printf("Value = 0x%x\n", value);
		}
		else {
			x = (u32) simple_strtol(argv[idx++], NULL, 10);
			y = (u32) simple_strtol(argv[idx++], NULL, 10);
			reg = (u32) simple_strtol(argv[idx++], NULL, 16);
			value = read_cluster_reg(x, y, block, reg);
			printf("Value = 0x%x\n", value);
		}
	}

	return 0;
}

int do_debug(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bool param = (bool) simple_strtol(argv[1], NULL, 10);
	set_debug(param);

	printf("Debug flag was %s\n", (param) ? "enabled" : "disabled");

	return 0;
}

int dump_reg(u32 block, char *reg_list)
{
	u32 reg, first_reg, last_reg, value;
	char *ptr_dash;
	char *ptr_comma;

	ptr_dash = strchr(reg_list, '-');
	ptr_comma = strchr(reg_list, ',');
	if((ptr_comma == NULL) && (ptr_dash == NULL))
		return -1;
	while (ptr_comma != NULL) {
		if (ptr_dash < ptr_comma && ptr_dash != NULL) {
			*ptr_dash = '\0';
			first_reg = simple_strtol(reg_list, NULL, 16);
			reg_list = ptr_dash + 1;
			ptr_comma = strchr(reg_list, ',');
			if (ptr_comma != NULL)
				*ptr_comma = '\0';
			last_reg = simple_strtol(reg_list, NULL, 16);
			for (reg = first_reg; reg <= last_reg; reg++) {
				value = read_non_cluster_reg(block, reg);
				printf("[0x%x][0x%x] = 0x%x\n", block, reg, value);
			}
			ptr_dash = strchr(reg_list, '-');
		} else {
			*ptr_comma = '\0';
			reg = simple_strtol(reg_list, NULL, 16);
			value = read_non_cluster_reg(block, reg);
			printf("[0x%x][0x%x] = 0x%x\n", block, reg, value);
			reg_list = ptr_comma + 1;
			ptr_comma = strchr(reg_list, ',');
		}
	}
	if (ptr_dash == NULL) {
		reg = simple_strtol(reg_list, NULL, 16);
		value = read_non_cluster_reg(block, reg);
		printf("[0x%x][0x%x] = 0x%x\n", block, reg, value);
	} else {
		*ptr_dash = '\0';
		first_reg = simple_strtol(reg_list, NULL, 16);
		reg_list = ptr_dash + 1;
		last_reg = simple_strtol(reg_list, NULL, 16);
		for (reg = first_reg; reg <= last_reg; reg++) {
			value = read_non_cluster_reg(block, reg);
			printf("[0x%x][0x%x] = 0x%x\n", block, reg, value);
		}
	}

	return 0;
}

/* SERDES common */

int serdes_check_status_reg(unsigned int serdes_block_id) {
	int retry_num;
	unsigned int status;

	for (retry_num = 0; retry_num < SERDES_STATUS_REG_RETRY_NUM; retry_num++) {
		status = read_non_cluster_reg(serdes_block_id, SERDES_SBUS_STS);
		if (status & SERDES_SBUS_STS_RDY)
			break;
	}

	if (retry_num == SERDES_STATUS_REG_RETRY_NUM) {
		printf(
				"serdes_check_status_reg: %u retries Exceeded for status register, register last value = 0x%08X\n",
				retry_num, status);
		return 1;
	}

	if (!(status & SERDES_SBUS_STS_SUCCESS)) {
		printf("serdes_check_status_reg: read status register failed!\n");
		return 1;
	}

	return 0;
}

int serdes_sbus_write_cmd(unsigned int serdes_block_id, unsigned int receiver,
		unsigned int data, unsigned int data_address)
{
	write_non_cluster_reg(serdes_block_id, SERDES_SBUS_RCVR_ADDR, receiver);
	write_non_cluster_reg(serdes_block_id, SERDES_SBUS_DATA_ADDR, data_address);
	write_non_cluster_reg(serdes_block_id, SERDES_SBUS_DATA, data);
	write_non_cluster_reg(serdes_block_id, SERDES_SBUS_COMMAND,
			SERDES_SBUS_COMMAND_RECEIVER_WRITE);
	if (serdes_check_status_reg(serdes_block_id)) {
		printf(
				"serdes_sbus_write_cmd: write failed for data = 0x%08X, data_address = 0x%08X\n",
				data, data_address);
		return 1;
	}

	return 0;
}

int serdes_sbus_read_cmd(unsigned int serdes_block_id, unsigned int receiver,
		unsigned int* ret_data, unsigned int data_address)
{
	write_non_cluster_reg(serdes_block_id, SERDES_SBUS_RCVR_ADDR, receiver);
	write_non_cluster_reg(serdes_block_id, SERDES_SBUS_DATA_ADDR, data_address);
	write_non_cluster_reg(serdes_block_id, SERDES_SBUS_COMMAND,
			SERDES_SBUS_COMMAND_RECEIVER_READ);
	if (serdes_check_status_reg(serdes_block_id)) {
		printf("serdes_sbus_read_cmd: read failed for data_address = 0x%08X\n",
				data_address);
		return 1;
	}

	*ret_data = read_non_cluster_reg(serdes_block_id, SERDES_SBUS_DATA);

	return 0;
}

int serdes_execute_interrupt(unsigned int serdes_block_id,
		unsigned int receiver, unsigned int execute_code,
		unsigned int execute_data, unsigned int expected_data)
{
	unsigned int data;
	int i, result;

	/* Generate interrupt */
	data = ((execute_data & SERDES_REG_INT_EXECUTE_DATA_MASK)
			<< SERDES_REG_INT_EXECUTE_DATA_OFFS);
	data |= ((execute_code & SERDES_REG_INT_EXECUTE_CODE_MASK)
			<< SERDES_REG_INT_EXECUTE_CODE_OFFS);
	result = serdes_sbus_write_cmd(serdes_block_id, receiver, data,
			SERDES_REG_INT_EXECUTE);
	if (result) {
		printf("serdes_execute_interrupt: failed to write!\n");
		return 1;
	}

	/* Retrieve interrupt status */
	for (i = 0; i < SERDES_INT_STATUS_REG_RETRY_NUM; i++) {
		result = serdes_sbus_read_cmd(serdes_block_id, receiver, &data,
				SERDES_REG_INT_STATUS);
		if (result) {
			printf("serdes_execute_interrupt: failed to read!\n");
			return 1;
		}
#if defined(CONFIG_TARGET_NPS_SOC) || defined(CONFIG_TARGET_NPS_IDG4400)
		if (!(data & SERDES_REG_INT_STATUS_IN_PROGRESS))
		break;
#else
		break;
#endif
	}

	if (i == SERDES_INT_STATUS_REG_RETRY_NUM) {
		printf("serdes_execute_interrupt: interrupt still in progress, timeout, code: 0x%x\n",
				execute_code);
		return 1;
	}

#if defined(CONFIG_TARGET_NPS_SOC) || defined(CONFIG_TARGET_NPS_IDG4400)
	data = ((data & SERDES_REG_INT_STATUS_DATA_MASK) >> SERDES_REG_INT_STATUS_DATA_OFFS);
	if (expected_data != data) {
		printf("serdes_execute_interrupt: interrupt data = 0x%x is not as expected = 0x%x\n",data, expected_data);
		return 1;
	}
#endif

	return 0;
}

int serdes_sbus_burst_upload(unsigned int serdes_block_id,
		unsigned int receiver, unsigned int burst_addr,
		unsigned int serdes_ucode[], unsigned int ucode_size)
{
#if defined(CONFIG_TARGET_NPS_SOC) || defined(CONFIG_TARGET_NPS_IDG4400)
	int result;
	unsigned int data;
	int ucode_index, num_writes;

	/* Upload */
	for (ucode_index = 0; ucode_index < ucode_size;) {
		if ((ucode_size - 3) >= ucode_index)
			num_writes = 3;
		else if ((ucode_size - 2) >= ucode_index)
			num_writes = 2;
		else
			num_writes = 1;
		data = ((serdes_ucode[ucode_index] & 0x3FF) |
				((serdes_ucode[ucode_index + 1] & 0x3FF ) << 10 ) |
				((serdes_ucode[ucode_index + 2] & 0x3FF ) << 20 ) |
				(num_writes << 30));

		result = serdes_sbus_write_cmd(serdes_block_id, receiver, data, burst_addr);
		if (result) {
			printf("serdes_sbus_burst_upload: failed to write!\n");
			return 1;
		}
		ucode_index += 3;
	}
#endif
	return 0;
}

/*
 * Getting the current frequency of the sys clock,
 * calculated from the sys_pll register in the crg block
 * The frequency is calculated through this formula:
 * 
 * F(PLL) = ref_clk / pll_refcnt * 2 * (2 + 2 * prog_fb_div4) *
 * 				prog_fbdiv255 / pll_out_divcnt / 2
*/
unsigned long get_board_sys_clk(void)
{      
	union sys_pll_pllcfg pll_register;
	union boot_cfg boot_register;
	unsigned long frequency_dividend;
	unsigned long frequency_divisor;   
	unsigned short ref_clk;
	unsigned short number_of_2 = 7;
	unsigned short number_of_5 = 8;
	unsigned long multiply_last = 50000000;
	static unsigned long frequency = 0;
	
	/* this function is used a lot of times - no need to execute
	 * the calculation each time, so a static variable is used */
	if (frequency)
		return frequency;
               
	pll_register.value = read_non_cluster_reg(CRG_BLOCK_ID,
		CRG_REG_SYS_PLL_PLLCFG);
               
	boot_register.value = read_non_cluster_reg(CRG_BLOCK_ID,
		CRG_REG_BOOT_CFG);
	
	/* multiplying by 50M or 100M only at the end
	 * to avoid having values over 4 bytes.
	 * Multiplying by 1 or 2 for now, instead
	 */
	ref_clk = boot_register.fields.cfg_ref_clk_freq + 1;
	
	frequency_dividend = ref_clk * 2 * (2 + 2 *
		pll_register.fields.prog_fb_div4) *
		pll_register.fields.prog_fb_div255;
               
	frequency_divisor = pll_register.fields.pll_refcnt * 
		pll_register.fields.pll_out_divcnt * 2;
	
	/* Because of constraints including the size of the values
	 * in the formula, we cannot use values larger than 4 bytes,
	 * and we cannot use floating points.
	 * That's why we divide the divisor and the value to multiply
	 * with at the end (50M) by 2 or 5 (the only factors of 50M) as
	 * much as possible, to ensure there are no floating points with
	 * the 'dividend' / 'divisor' operation, and that the values will
	 * be small
	 */ 
	while ((!(frequency_divisor % 2)) && number_of_2) {
		multiply_last /= 2;
		frequency_divisor /= 2;
		number_of_2--;
	}

	while ((!(frequency_divisor % 5)) && number_of_5) {
		multiply_last /= 5;
		frequency_divisor /= 5;
		number_of_5--;
	}
               
	frequency = (frequency_dividend / frequency_divisor) * 
			multiply_last;

	return frequency;
}

bool is_east_dgb_lan(void)
{
	char *env_dbg_lan_side = NULL;

	env_dbg_lan_side = getenv("dbg_lan_side");
	if (env_dbg_lan_side)
		if ((strcasecmp(env_dbg_lan_side, "EAST") == 0))
			return true;
	return false;
}
