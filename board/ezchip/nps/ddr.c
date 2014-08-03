/*
*	Copyright (C) 2015 EZchip, Inc. (www.ezchip.com)
*
* 	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License version 2 as
*	published by the Free Software Foundation.
*/

#include <asm/io.h>
#include "ddr.h"
#include "common.h"

/* Memory Interface */
static void configure_mi(void)
{
	u32 blockID;

	for (blockID = EMEM_MI_START_BLOCK1_ID; blockID <= EMEM_MI_END_BLOCK2_ID ; blockID++)
	{
		if (blockID == (EMEM_MI_END_BLOCK1_ID + 1))
		{
			blockID = EMEM_MI_START_BLOCK2_ID - 1;
			continue;
		}
		write_non_cluster_register(blockID, 0x107, 0x00000011);// DRAM_ECC
		write_non_cluster_register(blockID, 0x12F, 0xF8000205);
	}
	write_non_cluster_register(EMEM_MI_START_BLOCK1_ID, 0x110, 0x00033333);// CACHE_MI_IF
	write_non_cluster_register(EMEM_MI_START_BLOCK1_ID, 0x13D, 0x00006423);// RD_SORT_THR_7
}

/* Level 2 Cache */
static void configure_l2c(void)
{
	u32 blockID;

	for (blockID = L2C_START_BLOCK1_ID; blockID <= L2C_END_BLOCK2_ID ; blockID++)
	{
		if (blockID == (L2C_END_BLOCK1_ID + 1))
		{
			blockID = L2C_START_BLOCK2_ID - 1;
			continue;
		}
		write_non_cluster_register(blockID, 0x00, 0x00010001);// INDIRECT_REGISTER_DATA
		write_non_cluster_register(blockID, 0x14, 0x00000032);// MS_L2C_USG
		write_non_cluster_register(blockID, 0x10, 0x00000109);// INDIRECT_REGISTER_WRITE

		write_non_cluster_register(blockID, 0x00, 0x00000202);// INDIRECT_REGISTER_DATA
		write_non_cluster_register(blockID, 0x14, 0x00000032);// MS_ATU_TYPE
		write_non_cluster_register(blockID, 0x10, 0x00000209);// INDIRECT_REGISTER_WRITE

		write_non_cluster_register(blockID, 0x00, 0x0000000C);// INDIRECT_REGISTER_DATA
		write_non_cluster_register(blockID, 0x14, 0x00000000);// DVCS_AMNT
		write_non_cluster_register(blockID, 0x10, 0x00000309);// INDIRECT_REGISTER_WRITE

		write_non_cluster_register(blockID, 0x00, 0x76543210);// INDIRECT_REGISTER_DATA
		write_non_cluster_register(blockID, 0x14, 0x00000010);// DVCS_PTRN_1
		write_non_cluster_register(blockID, 0x10, 0x00000309);// INDIRECT_REGISTER_WRITE

		write_non_cluster_register(blockID, 0x00, 0x0000BA98);// INDIRECT_REGISTER_DATA
		write_non_cluster_register(blockID, 0x14, 0x00000020);// DVCS_PTRN_2
		write_non_cluster_register(blockID, 0x10, 0x00000309);// INDIRECT_REGISTER_WRITE

		/* SRAMS */
		write_non_cluster_register(blockID, 0x00, 0xC3100000);// IND_DATA
		write_non_cluster_register(blockID, 0x01, 0x33BABA00);// IND_DATA
		write_non_cluster_register(blockID, 0x02, 0x00000001);// IND_DATA
		write_non_cluster_register(blockID, 0x14, 0x00000032);// MS_DFN
		write_non_cluster_register(blockID, 0x10, 0x00000002);// SRAM_WRITE
	}

}

/* Memory Controller */
static void configure_mc(void)
{
	u32 blockID;

	for (blockID = MEMORY_CONTROLLER_START_BLOCK1_ID; blockID <= MEMORY_CONTROLLER_END_BLOCK2_ID ; blockID++)
	{
		if (blockID == (MEMORY_CONTROLLER_END_BLOCK1_ID+ 1))
		{
			blockID = MEMORY_CONTROLLER_START_BLOCK2_ID - 1;
			continue;
		}
		write_non_cluster_register(blockID, 0x162, 0x02040067);// EZREF_CNTR
		write_non_cluster_register(blockID, 0x163, 0x02042718);// EZREF_TIMING
		write_non_cluster_register(blockID, 0x100, 0x172F121D);// TIMING1
		write_non_cluster_register(blockID, 0x101, 0x00050B0B);// TIMING2
		write_non_cluster_register(blockID, 0x102, 0x1517190C);// TIMING3
		write_non_cluster_register(blockID, 0x103, 0x01459040);// RFRSH_PARAMS_1X
		write_non_cluster_register(blockID, 0x104, 0x01459040);// RFRSH_PARAMS_2X
		write_non_cluster_register(blockID, 0x105, 0x01459040);// RFRSH_PARAMS_4X
		write_non_cluster_register(blockID, 0x106, 0x002B0000);// CALIB_PARAMS
		write_non_cluster_register(blockID, 0x10A, 0x00004130);// DDR_PROPERTIES
		write_non_cluster_register(blockID, 0x113, 0x000A0178);// MNG_CMD_PARAMS
		write_non_cluster_register(blockID, 0x116, 0x00000000);// MC_DDR_IF
		write_non_cluster_register(blockID, 0x117, 0x77FF813C);// MC_MI_IF
		write_non_cluster_register(blockID, 0x118, 0x7A021208);// MC_SYNC1
		write_non_cluster_register(blockID, 0x119, 0x00000208);// MC_SYNC2
		write_non_cluster_register(blockID, 0x11B, 0x00000C0C);// EXT_MC_LATENCY
		write_non_cluster_register(blockID, 0x11C, 0x00000008);// MRS_RFRSH_ACK

		write_non_cluster_register(blockID, 0x110, 0x00000002);// APB_IFC
		write_non_cluster_register(blockID, 0x110, 0x80000002);// APB_IFC
		write_non_cluster_register(blockID, 0x110, 0x80000003);// APB_IFC
		write_non_cluster_register(blockID, 0x110, 0x84000001);// APB_IFC

		write_non_cluster_register(blockID, 0x00, 0x0204C620);// INDIRECT_DATA_REGISTERS
		write_non_cluster_register(blockID, 0x14, 0x00000005);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x0FC00172);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x0000014B);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x80002183);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000080);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00038000);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000020);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x014008D0);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000010);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x04200030);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000011);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000033);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000001);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00040001);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000001);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000024);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000060);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000008);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000061);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x000002A8);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000062);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000000);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000063);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x06240E08);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000044);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x281B0400);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000045);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00060200);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000046);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x02000000);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000047);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x0116081A);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000048);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00320E08);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000049);
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x120, 0x00000001);

		write_non_cluster_register(blockID, 0x00, 0x044402A8);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00001004);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000000);
		write_non_cluster_register(blockID, 0x10, 0x00000001);

		write_non_cluster_register(blockID, 0x00, 0x044402A8);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00001004);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x14, 0x00000000);
		write_non_cluster_register(blockID, 0x10, 0x00000011);

		write_non_cluster_register(blockID, 0x00, 0x04460000);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00001004);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000001);

		write_non_cluster_register(blockID, 0x00, 0x04460000);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00001004);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000011);

		write_non_cluster_register(blockID, 0x00, 0x04420008);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00001004);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000001);

		write_non_cluster_register(blockID, 0x00, 0x04420008);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00001004);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000011);

		write_non_cluster_register(blockID, 0x00, 0x04400124);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00021004);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000001);

		write_non_cluster_register(blockID, 0x00, 0x04400124);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00021004);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000011);

		write_non_cluster_register(blockID, 0x00, 0x05C00400);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x0002100C);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000001);

		write_non_cluster_register(blockID, 0x00, 0x05C00400);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x0002100C);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000011);

		write_non_cluster_register(blockID, 0x00, 0x00000000);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00000137);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000052);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x000001E2);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00020002);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x000001F0);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000052);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00000222);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00020002);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00000230);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000001);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00000137);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000052);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00000262);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00020002);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00000270);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000052);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x000002A2);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00020002);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x000002B0);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x00, 0x00000000);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x01, 0x00000137);// INDIRECT_DATA_REGISTER
		write_non_cluster_register(blockID, 0x10, 0x00000101);

		write_non_cluster_register(blockID, 0x116, 0x00030003);// MC_DDR_IF

		write_non_cluster_register(blockID, 0x121, 0x00004801);// PHY_UPDATE
	}
}

/* Clock Reset Generator */
static void configure_crg(void)
{
	u32 reg;

	for (reg = CRG_START_BLOCK_ID; reg <= CRG_END_BLOCK_ID ; reg++)
	{
		write_non_cluster_register(0x480, reg, 0x7A007F);// EMI_RST
	}
}

/* Message Scheduling Unit */
static void configure_msu(void)
{
	write_cluster_register(0x80, MSU_BLOCK_ID, CLUSTER_Y_VALUE, CLUSTER_X_VALUE, 0x000F0001); //EN_CFG
}

/* Cluster Interface Unit */
static void configure_ciu(void)
{
	write_cluster_register(0xA4, CIU_BLOCK_ID, CLUSTER_Y_VALUE, CLUSTER_X_VALUE, 0xB202000A); //GLB_MSID_CFG_1
	write_cluster_register(0xC4, CIU_BLOCK_ID, CLUSTER_Y_VALUE, CLUSTER_X_VALUE, 0xC200000F); //GLB_MSID_CFG_2
}

void configure_emem(void)
{
	configure_mi();
	configure_l2c();
	configure_mc();
	configure_crg();
	configure_msu();
	configure_ciu();
}
