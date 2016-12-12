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

#ifndef _COMMON_H_
#define _COMMON_H_

#include <command.h>
#include <common.h>

#define MAX_CLUSTERS			16
#define MAX_CORES_PER_CLUSTER		16
#define MAX_THREADS_PER_CORE		16
#define	MAX_CORES			(MAX_CLUSTERS * MAX_CORES_PER_CLUSTER)
#define	MAX_THREADS	\
	(MAX_CLUSTERS * MAX_CORES_PER_CLUSTER * MAX_THREADS_PER_CORE)
#define NUM_OF_CLUSTER_PER_COL_ROW	4

/* SBUS */
#define SERDES_SBUS_RCVR_ADDR				0x130
#define SERDES_SBUS_DATA_ADDR				0x131
#define SERDES_SBUS_DATA					0x132
#define SERDES_SBUS_COMMAND					0x133
#define SERDES_SBUS_STS						0x134
#define SERDES_SBUS_COMMAND_RECEIVER_WRITE	0x21
#define SERDES_SBUS_COMMAND_RECEIVER_READ	0x22
#define SERDES_SBUS_STS_RDY					(1 << 0)
#define SERDES_SBUS_STS_SUCCESS				(1 << 1)
#define SERDES_STATUS_REG_RETRY_NUM			1000000
#define SERDES_INT_STATUS_REG_RETRY_NUM		100000

#define SERDES_REG_INT_EXECUTE			0x03
#define SERDES_REG_INT_STATUS			0x04
#define SERDES_REG_INT_EXECUTE_DATA_OFFS	0
#define SERDES_REG_INT_EXECUTE_DATA_MASK	0xFFFF
#define SERDES_REG_INT_EXECUTE_CODE_OFFS	16
#define SERDES_REG_INT_EXECUTE_CODE_MASK	0xFFFF
#define SERDES_REG_INT_STATUS_DATA_OFFS		0
#define SERDES_REG_INT_STATUS_DATA_MASK		0xFFFF
#define SERDES_REG_INT_STATUS_IN_PROGRESS	(1 << 16)

#define CONFIGURATION_BASE		0xF6000000

/* CTOP Aux registers */
#define AUX_REGS_BASE			0xFFFFF800
#define COPY_ACCELERATOR_DST_AUX_REG	(AUX_REGS_BASE + 0xA4)
#define COPY_ACCELERATOR_SRC_AUX_REG	(AUX_REGS_BASE + 0xA8)
#define COPY_ACCELERATOR_SIZE_AUX_REG	(AUX_REGS_BASE + 0xAC)
#define COPY_ACCELERATOR_CMD_AUX_REG	(AUX_REGS_BASE + 0xB0)

#define ACCELERATOR_CMD_TRIGGER		1
#define REGISTER_SIZE_IN_BYTES		4

#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

#define GET_BIT(__x, __pos) \
	(((__x) >> (__pos)) & 1)
#define	GET_BITS(__src, __start_pos, __length)\
		(((__src) >> (__start_pos)) & ((1 << __length) - 1))
#define	WRITE_BIT(__src, __bitNum, __bitValue)\
		(((__src) = ((((__src) & (~(1 << __bitNum))) |\
		((((__bitValue) << (__bitNum))))))))

#define	BYTES_TO_BITS(__Bytes)  ((__Bytes) << 3)

#define NON_CLUSTER_REG_ADDR(BLOCK, REG)\
	(CONFIGURATION_BASE + (BLOCK << 14) + (REG << 2))

struct sram_line_write {
	u32	line_size;
	u32	block;
	u32	address_register;
	u32	address;
	u32	command_register;
	u32	command;
	u8	*data;
};

struct cached_reg {
	const	u32	block;
	const	u32	addr;
	u32	val;
};

enum	emem_mc_cached {
	IFC_BIST_CFG_ID_0,
	IFC_BIST_CFG_ID_1,
	IFC_BIST_CFG_ID_2,
	IFC_BIST_CFG_ID_3,
	IFC_BIST_CFG_ID_4,
	IFC_BIST_CFG_ID_5,
	IFC_BIST_CFG_ID_6,
	IFC_BIST_CFG_ID_7,
	IFC_BIST_CFG_ID_8,
	IFC_BIST_CFG_ID_9,
	IFC_BIST_CFG_ID_10,
	IFC_BIST_CFG_ID_11,
	IFC_BIST_BASE_ADDR_ID_0,
	IFC_BIST_BASE_ADDR_ID_1,
	IFC_BIST_BASE_ADDR_ID_2,
	IFC_BIST_BASE_ADDR_ID_3,
	IFC_BIST_BASE_ADDR_ID_4,
	IFC_BIST_BASE_ADDR_ID_5,
	IFC_BIST_BASE_ADDR_ID_6,
	IFC_BIST_BASE_ADDR_ID_7,
	IFC_BIST_BASE_ADDR_ID_8,
	IFC_BIST_BASE_ADDR_ID_9,
	IFC_BIST_BASE_ADDR_ID_10,
	IFC_BIST_BASE_ADDR_ID_11,
	EXT_MC_LATENCY_0,
	EXT_MC_LATENCY_1,
	EXT_MC_LATENCY_2,
	EXT_MC_LATENCY_3,
	EXT_MC_LATENCY_4,
	EXT_MC_LATENCY_5,
	EXT_MC_LATENCY_6,
	EXT_MC_LATENCY_7,
	EXT_MC_LATENCY_8,
	EXT_MC_LATENCY_9,
	EXT_MC_LATENCY_10,
	EXT_MC_LATENCY_11,
	PHY_UPDATE_0,
	PHY_UPDATE_1,
	PHY_UPDATE_2,
	PHY_UPDATE_3,
	PHY_UPDATE_4,
	PHY_UPDATE_5,
	PHY_UPDATE_6,
	PHY_UPDATE_7,
	PHY_UPDATE_8,
	PHY_UPDATE_9,
	PHY_UPDATE_10,
	PHY_UPDATE_11,
	MC_MI_IF_0,
	MC_MI_IF_1,
	MC_MI_IF_2,
	MC_MI_IF_3,
	MC_MI_IF_4,
	MC_MI_IF_5,
	MC_MI_IF_6,
	MC_MI_IF_7,
	MC_MI_IF_8,
	MC_MI_IF_9,
	MC_MI_IF_10,
	MC_MI_IF_11,
	NUM_OF_CACHED_REGS
};

void flash_to_mem(unsigned int src_addr,
		  unsigned int dst_addr,
		  unsigned int size);
void write_cluster_reg(unsigned int reg, unsigned int sub_block,
			   unsigned int y, unsigned int x, unsigned int value);
void write_non_cluster_reg(unsigned int block, unsigned int reg,
						    unsigned int value);
int do_debug(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_write_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_read_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
u32 read_cached_register(enum emem_mc_cached	reg_id);
void write_cached_register(enum emem_mc_cached	reg_id, u32	value);
void write_non_cluster_sram_line(struct	sram_line_write *sram_line_params);
void read_non_cluster_sram_line(struct	sram_line_write *sram_line_params);
u32 read_non_cluster_reg(u32 block, u32 reg);
u32 read_cluster_reg(u32 cluster_x, u32 cluster_y, u32 sub_block, u32 reg);
void remove_spaces(char	*source);
void set_debug(bool dop);
bool get_debug(void);
int serdes_check_status_reg(unsigned int serdes_block_id);
int serdes_sbus_write_cmd(unsigned int serdes_block_id, unsigned int receiver, unsigned int data, unsigned int data_address);
int serdes_sbus_read_cmd(unsigned int serdes_block_id, unsigned int receiver, unsigned int* ret_data, unsigned int data_address);
int serdes_execute_interrupt(unsigned int serdes_block_id, unsigned int receiver, unsigned int execute_code, unsigned int execute_data, unsigned int expected_data);
int serdes_sbus_burst_upload(unsigned int serdes_block_id, unsigned int receiver, unsigned int burst_addr, unsigned int serdes_ucode[], unsigned int ucode_size);
bool is_east_dgb_lan(void);
#endif /* _COMMON_H_ */
