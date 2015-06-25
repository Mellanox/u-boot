/*
 *       Copyright (C) 2015 EZchip, Inc. (www.ezchip.com)
 *
 *       This program is free software; you can redistribute it and/or modify
 *       it under the terms of the GNU General Public License version 2 as
 *       published by the Free Software Foundation.
 */

#include <asm/arcregs.h>
#include <common.h>
#include "spl.h"
#include "../common.h"
#include "../nps.h"

/* set_dbi_pf() - Choose physical function
 *
 * Wait for DBI sts, and choose pf.
 */
static void set_dbi_pf(unsigned int pf_val)
{
	while((read_non_cluster_register(PCIB_BLOCK_ID,DBI_STS_REG) & DBI_STS_RDY_VAL) != DBI_STS_RDY_VAL);
	/*Allow configuration write to RO registers*/
	write_non_cluster_register(PCIB_BLOCK_ID, DBI_CS2_REG, DBI_CS2_RO_REG_VAL);
	write_non_cluster_register(PCIB_BLOCK_ID, DBI_CTRL_REG, pf_val << DBI_CTRL_PF_OFFSET);
}

/* read_write_to_config_space() - read from relevant config space offset in PCIB
 *
 * Allow 1/2/3/4 bytes
 */
static void read_write_to_config_space(unsigned int cs_offset, unsigned int num_bytes)
{
	unsigned int value, base_reg = PCIB_PCIE_ADDR + (cs_offset)/4;

	if ((num_bytes < 1) || (num_bytes > 4))
		return;

	value = read_non_cluster_register(PCIB_BLOCK_ID, base_reg);
	write_non_cluster_register(PCIB_BLOCK_ID, base_reg, value);
}

/* update_cs() - Write vendor,device ids and base bar to all pfs
 *
 * For now, done by read-write the read value.
 * In the future, values will be taken from uboot configuration
 */
static void update_cs(void)
{
	int i;

	for (i = 0; i < NUM_PFS; i++)
	{
		/* Choose physical function */
		set_dbi_pf(i);
		/* Configuration space header */
		read_write_to_config_space(CS_VENDOR_ID_OFFSET, VENDOR_IF_BYTE_SIZE);
		read_write_to_config_space(CS_DEVICE_ID_OFFSET, DEVICE_ID_BYTE_SIZE);
		read_write_to_config_space(CS_BAR_0_BASE_ADDR_OFFSET, BAR_0_BASE_ADDR_ADDRESS_BYTE_SIZE);
		read_write_to_config_space(CS_BAR_1_BASE_ADDR_OFFSET, BAR_1_BASE_ADDR_ADDRESS_BYTE_SIZE);
		read_write_to_config_space(CS_BAR_2_BASE_ADDR_OFFSET, BAR_2_BASE_ADDR_ADDRESS_BYTE_SIZE);
		read_write_to_config_space(CS_BAR_3_BASE_ADDR_OFFSET, BAR_3_BASE_ADDR_ADDRESS_BYTE_SIZE);
		read_write_to_config_space(CS_BAR_4_BASE_ADDR_OFFSET, BAR_4_BASE_ADDR_ADDRESS_BYTE_SIZE);
		read_write_to_config_space(CS_BAR_5_BASE_ADDR_OFFSET, BAR_5_BASE_ADDR_ADDRESS_BYTE_SIZE);
		read_write_to_config_space(CS_SUBSYSTEM_VENDOR_ID_OFFSET, SUBSYSTEM_VENDOR_ID_BYTE_SIZE);
		read_write_to_config_space(CS_SUBSYSTEM_ID_OFFSET, SUBSYSTEM_ID_BYTE_SIZE);
	}
	/* Updating CS is complete */
	write_non_cluster_register(PEXC_BLOCK_ID, RSP_CTRL_REG, RSP_CTRL_DONE_VAL);
}

/* poll_sbus_sts_data() - Wait for an interrupt and see that it's valid
 *
 * Poll on sbus_sts register in serdes until the status is ready, success
 * poll on the sbus_data for a valid status
 */
static void poll_sbus_sts_data(unsigned int mask, unsigned int value)
{
	while(1) {
		write_non_cluster_register(SERDES_BLOCK_ID, SBUS_COMMAND, CORE_IFR_READ_CMD);
		while((read_non_cluster_register(SERDES_BLOCK_ID,SBUS_STS) & STATUS_RDY_SUCCESS) != STATUS_RDY_SUCCESS);
		if ((read_non_cluster_register(SERDES_BLOCK_ID,SBUS_DATA) & mask) == value)
			break;
	}
}

/* do_poll_rom() - Wait for an interrupt indicating rom is present and up
 *
 * Register on SERDES interrupt #53, read command
 */
static void do_poll_rom(void)
{
	write_non_cluster_register(SERDES_BLOCK_ID, SBUS_RCVR_ADDR, SBUS_CONTROLLER);
	write_non_cluster_register(SERDES_BLOCK_ID, SBUS_DATA_ADDR, SBUS_INTR_STATUS);

	poll_sbus_sts_data(INTR_53_STATUS, INTR_53_STATUS);
}

/* do_check_spico_ready() - Wait for an indication that SERDES controller is ready
 *
 * Register on SERDES interrupt #4, read command
 */
static void do_check_spico_ready(void)
{
	write_non_cluster_register(SERDES_BLOCK_ID, SBUS_RCVR_ADDR, LAST_SERDES);
	write_non_cluster_register(SERDES_BLOCK_ID, SBUS_DATA_ADDR, SPICO_INTR_STATUS);

	poll_sbus_sts_data(INTR_4_STATUS, 0);
}

/* do_poll_link_status() - Wait for PCIE physical and logical links
 *
 * Poll LINK_STATUS register in PEXC, monitor link up bits
 */
static void do_poll_link_status(void)
{
	while((read_non_cluster_register(PEXC_BLOCK_ID,LINK_STATUS_PEXC) & LINK_STATUS_UP) != LINK_STATUS_UP);
}

/* do_boot_config() - Configure necessities and wait for pcie link
 *
 * Once link exists, notify rtl simulator
 */
static void do_boot_config(void)
{
	do_poll_rom();
	/* Release PCI PCS reset */
	write_non_cluster_register(SERDES_BLOCK_ID, PCIE_PCS_RST_N_REG, PCIE_PCS_RES_N_VAL);
	/* Enable PCIE interrupts */
	write_non_cluster_register(SERDES_BLOCK_ID, PCIE_PCS_CFG_REG, PCIE_EN_PCS_INTR);
	do_check_spico_ready();
	/* Single lane (lane 48) */
	write_non_cluster_register(SERDES_BLOCK_ID, PCIE_MODES_REG, PCIE_MODES_VAL);

	/* PCIE link status */
	write_non_cluster_register(PCIB_BLOCK_ID, LINK_STATUS_PCIB, LINK_STATUS_VAL);

	/* Retry incoming configuration requests until initialization is complete*/
	write_non_cluster_register(PEXC_BLOCK_ID, RSP_CTRL_REG, RSP_CTRL_BOOT_VAL);
	/* Continue link establishment, allow link training status state machine
	 * transition from detect stage after PCIE core configurations are already done*/
	write_non_cluster_register(PEXC_BLOCK_ID, LTSSM_CTRL_REG, LTSSM_CTRL_VAL);
	do_poll_link_status();
}

/* jump_to_uboot() - Jump to uboot's address in lram
 */
static void jump_to_uboot(void)
{
	void *uboot_ptr = (void *)UBOOT_LRAM_ADDRESS;
	goto *uboot_ptr;
}

/* board_init_f() - spl's main function
 */
void board_init_f(ulong dummy)
{
	do_boot_config();
	update_cs();
	flash_to_mem(UBOOT_FLASH_OFFSET, UBOOT_LRAM_ADDRESS, UBOOT_MAX_SIZE);
	jump_to_uboot();
}
