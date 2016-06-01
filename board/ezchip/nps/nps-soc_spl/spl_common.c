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
#include <spi.h>
#include <spl.h>
#include <spi_flash.h>
#include <malloc.h>
#include <asm/arcregs.h>
#include "spl_common.h"
#include "../common.h"
#include "../nps.h"
#include "gen3_ucode.h"

DECLARE_GLOBAL_DATA_PTR;
gd_t gdata __attribute__ ((section(".data")));

/* PCI configuration */
static int pci_gen = PCI_GEN1;
static int pci_lanes = PCI_LANES_X1;
static int pci_pfs = USED_PFS;
static int *pci_serdes;

/* x1/x8 serdes */
static int pci_x8_serdes[PCI_LANES_X8] = {SBUS_RCVR_SERDES_40,SBUS_RCVR_SERDES_41,
		SBUS_RCVR_SERDES_42,SBUS_RCVR_SERDES_43,SBUS_RCVR_SERDES_44,
		SBUS_RCVR_SERDES_45,SBUS_RCVR_SERDES_46,SBUS_RCVR_SERDES_47};
static int pci_x1_serdes[PCI_LANES_X1] = {SBUS_RCVR_SERDES_48};

/* uboot environment settings */
static struct spi_flash *flash;
#ifdef CONFIG_TARGET_NPS_SOC
static char uboot_env[UBOOT_ENV_SIZE_TO_READ];
static char current_env[UBOOT_ENV_SIZE_TO_READ];
static char *pci_config_str = "pci_config";
#endif

/* globals */
extern unsigned int xdmem_image_ucode[];
extern unsigned int sbus_master_ucode[];


/* get_pci_config() - read the PCI configuration from flash and update
 *                    global variables accordingly.
 */
static void get_pci_config(void)
{
#ifdef CONFIG_TARGET_NPS_SOC
	int uboot_env_pos, current_env_pos = 0;
	char *pci_config, *env_string = NULL;

	/* read uboot env from the flash */
	spi_flash_read(flash, CONFIG_ENV_OFFSET, UBOOT_ENV_SIZE_TO_READ*sizeof(char), &uboot_env);

	/* find the pci configuration */
	for (uboot_env_pos = 0; uboot_env_pos < UBOOT_ENV_SIZE_TO_READ; uboot_env_pos++) {
		current_env[current_env_pos] = uboot_env[uboot_env_pos];
		current_env_pos++;
		if (!uboot_env[uboot_env_pos]) {
			/* found null termination, check if the string is pci config */
			current_env_pos = 0;
			if (*current_env) {
				if (strncmp(current_env, pci_config_str, 10) == 0) {
					env_string = strtok(current_env, "=");
					env_string = strtok(NULL, "=");
					break;
				}
			}
		}
	}

	if (env_string) {
		/* get pci gen */
		pci_config = strsep(&env_string, ",");
		if (pci_config)
			pci_gen = pci_config[3]-'0';
		/* get pci lanes */
		pci_config = strsep(&env_string, ",");
		if (pci_config)
			pci_lanes = pci_config[1]-'0';
		/* get number of physical functions */
		pci_config = strsep(&env_string, ",");
		if (pci_config)
			pci_pfs = pci_config[3]-'0';
	}
#endif
}

/*
 * update_pci_settings() - update PCI settings
 */
static int update_pci_settings(unsigned int *ref_select,
		unsigned int *pcie_bypass, unsigned int *pcie_modes)
{
	int res;

	printf("SPL: Setting up PCI ");

	get_pci_config();

	printf("x%d ", pci_lanes);
	if (pci_lanes == PCI_LANES_X8) {
		*ref_select = 0x0FF00;
		*pcie_bypass = 0x100;
		*pcie_modes = SERDES_PCIE_X8;
		pci_serdes = pci_x8_serdes;
	} else if (pci_lanes == PCI_LANES_X1) {
		*ref_select = 0x10000;
		*pcie_bypass = 0xFF;
		*pcie_modes = SERDES_PCIE_X1;
		pci_serdes = pci_x1_serdes;
	} else {
		printf("INVALID LANES!");
		goto error;
	}

	printf("[GEN%d] ", pci_gen);
	if (!((pci_gen == PCI_GEN1) | (pci_gen == PCI_GEN2) | (pci_gen == PCI_GEN3))) {
		printf("INVALID GEN!");
		goto error;
	}

	printf("with %d PFs ", pci_pfs);
	if ((pci_pfs < 1) | (pci_pfs > NUM_PFS)) {
		printf("INVALID PFS!");
		goto error;
	}

	res = 0;
	goto out;
error:
	res = 1;
out:
	printf("\n");
	return res;
}

/* write_ro_pci_core_reg() - write read-only PCIe core configuration space register
 *
 * enable writes, write and disable writes
 */
static void write_ro_pci_core_reg(unsigned int reg, unsigned int value)
{
	write_non_cluster_reg(PCIB_BLOCK_ID, DBI_CS2_REG, DBI_CS2_RO_ENABLE_RW);
	write_non_cluster_reg(PCIB_BLOCK_ID, reg, value);
	write_non_cluster_reg(PCIB_BLOCK_ID, DBI_CS2_REG, DBI_CS2_RO_DISABLE_RW);
}

/* set_dbi_pf() - Choose physical function
 *
 * Wait for DBI sts, and choose pf.
 */
static void set_dbi_pf(unsigned int pf_val)
{
	while ((read_non_cluster_reg(PCIB_BLOCK_ID, DBI_STS_REG) & DBI_STS_RDY_VAL) != DBI_STS_RDY_VAL)
		;

	write_ro_pci_core_reg(DBI_CTRL_REG, pf_val << DBI_CTRL_PF_OFFSET);
}

/* limit_mac() - Limit MAC to specific PCI gen
 */
static void limit_mac(void)
{
	write_ro_pci_core_reg(PCIB_LINK_CONTROL2_REG,
			(read_non_cluster_reg(PCIB_BLOCK_ID, PCIB_LINK_CONTROL2_REG)
					& !PCIB_LINK_CONTROL2_TARGET_LINK_SPEED)
					| pci_gen);
}

#ifdef DIRECT_SPEED_CHANGE
static void direct_speed_change_gen3(void)
{
	unsigned long data;

	write_ro_pci_core_reg(PCIB_LINK_CONTROL2_REG,
			(read_non_cluster_reg(PCIB_BLOCK_ID, PCIB_LINK_CONTROL2_REG)
					& !PCIB_LINK_CONTROL2_TARGET_LINK_SPEED)
					| PCIB_LINK_CONTROL2_TARGET_LINK_SPEED_GEN3);

	data = read_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN2_CTRL_REG);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN2_CTRL_REG, data & 0xFFFDFFFF);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN2_CTRL_REG, data | 0x20000);
}
#endif

/* do_poll_rom() - Wait for rom execution completed and acknowledged
 *
 * Register on SERDES interrupt 33, read command, poll for status
 * Defined under SBusMaster SBus controller receiver memory 0x33
 */
static int do_poll_rom(unsigned int serdes_block_id)
{
	unsigned int data;
	int result;

	do {
		result = serdes_sbus_read_cmd(serdes_block_id, SBUS_RCVR_CONTROLLER, &data, SBUS_CONTROLLER_ROM_STS);
		if (result) {
			printf("Failed to read SBUS Controller ROM status!\n");
			return 1;
		}
	} while (data != SBUS_DATA_ROM_ENABLED);

	return 0;
}

#ifdef CHECK_SPICO_READY
/* do_check_spico_ready() - Wait for an indication that SERDES controller is ready
 *
 * Register on SERDES interrupt #4, read command
 */
static int do_check_spico_ready(void)
{
	unsigned int data;
	int result;

	do {
		result = serdes_sbus_read_cmd(serdes_block_id, SBUS_RCVR_LAST_SERDES, &data, SERDES_REG_INT_STATUS);
		if (result) {
			printf("Failed to read last SERDES interrupt status!\n");
			return 1;
		}
	} while (data != SBUS_DATA_ROM_ENABLED);

	return 0;
}
#endif

/* do_poll_link_status() - Wait for PCIE physical and logical links
 *
 * Poll LINK_STATUS register in PEXC, monitor link up bits
 */
static int do_poll_link_status(void)
{
	unsigned int i;
	int err_res;

	err_res = read_non_cluster_reg(CFGB_WEST_BLOCK_ID, CFGB_REG_ERR_RES);
	if (err_res)
		printf("do_poll_link_status: warning! ERR_RES bit is ON (before polling)\n");

	for (i = 0; i < POLL_LINK_STATUS_RETRIES; i++)
		if ((read_non_cluster_reg(PEXC_BLOCK_ID, LINK_STATUS_PEXC) & LINK_STATUS_UP) == LINK_STATUS_UP)
			break;
	spl_print(" [%d retries] ", i);

	err_res = read_non_cluster_reg(CFGB_WEST_BLOCK_ID, CFGB_REG_ERR_RES);
	if (err_res)
		printf("do_poll_link_status: warning! ERR_RES bit is ON (after polling)\n");

	/* Sometimes during the polling ERR_RES occurs for unknown reason.
	 * Since it doesn't effect the polling result, let's clear this bit. */
//	write_non_cluster_reg(CFGB_WEST_BLOCK_ID, CFGB_REG_ERR_RES, 1);

	if (i == POLL_LINK_STATUS_RETRIES) {
		printf("do_poll_link_status: timeout\n");
		return 1;
	}

	return 0;
}

/* update_cs() - Write vendor ids, device ids and base bar
 * in to configuration space for all physical functions
 *
 */
static void update_cs(void)
{
	int i;
	struct msixcap_offset msix_offset;
	unsigned long ethernet_ctrl_class_code;
#ifndef CONFIG_TARGET_NPS_SOC
	struct pci_bar bar;
#endif

	/* read PF1 class code */
	set_dbi_pf(1);
	ethernet_ctrl_class_code = read_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_CLASS_CODE_OFFSET));

	/* update configuration space for each physical function */
	for (i = 0; i < NUM_PFS; i++) {
		/* Choose physical function */
		set_dbi_pf(i);

		/* Fix class code for PF1,2,3 */
		if (i > 0)
			write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_CLASS_CODE_OFFSET), ethernet_ctrl_class_code);

#ifdef CONFIG_TARGET_NPS_SOC
		/* Set BAR sizes (disable BAR0,1,2,3,5)*/
		write_non_cluster_reg(PCIB_BLOCK_ID, DBI_CS2_REG, DBI_CS2_RO_ENABLE_RW);
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_0_BASE_ADDR_OFFSET), 0);
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_1_BASE_ADDR_OFFSET), 0);
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_2_BASE_ADDR_OFFSET), 0);
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_3_BASE_ADDR_OFFSET), 0);
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_4_BASE_ADDR_OFFSET), 0x1FFFFFF);
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_5_BASE_ADDR_OFFSET), 0);
		write_non_cluster_reg(PCIB_BLOCK_ID, DBI_CS2_REG, DBI_CS2_RO_DISABLE_RW);
#endif

		/* MSI-X updates */
		msix_offset.value = read_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_MSIX_CAP_TABLE_OFFSET));
		msix_offset.bir = 2;
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_MSIX_CAP_TABLE_OFFSET), msix_offset.value);
		msix_offset.value = read_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_MSIX_CAP_PBA_OFFSET));
		msix_offset.bir = 2;
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_MSIX_CAP_PBA_OFFSET), msix_offset.value);

		/* SR-IOV disable (set initial, total and & num of VFs to zero) */
		write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_SRIOV_TOTAL_AND_INITIAL_VFS, 0x0);
		write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_SRIOV_NUM_VFS, (read_non_cluster_reg(PCIB_BLOCK_ID, PCIB_SRIOV_NUM_VFS) & 0xFFFF0000));

#ifndef CONFIG_TARGET_NPS_SOC
/* changes came from qemu simulation. real chip works without it */
		/* BAR4 */
		bar.value = read_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_4_BASE_ADDR_OFFSET));
		bar.locatable = 1;
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_4_BASE_ADDR_OFFSET), bar.value);
		/* BAR5 */
		bar.value = read_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_5_BASE_ADDR_OFFSET));
		bar.locatable = 1;
		write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_BAR_5_BASE_ADDR_OFFSET), bar.value);
#endif
	}

#ifdef CONFIG_TARGET_NPS_SOC
	   /* disable unused physical functions */
	   for (i = pci_pfs; i < NUM_PFS; i++) {
			   /* Choose physical function */
			   set_dbi_pf(i);
			   /* Configuration space header */
			   spl_print(" [disabled PF%d] ", i);
			   write_non_cluster_reg(PCIB_BLOCK_ID, CS_OFFSET_TO_ADDR(CS_VENDOR_ID_OFFSET), 0xffffffff);
	   }
#endif

	set_dbi_pf(0);
}

/* do_rom_crc_check() - Check ROM CRC
 *
 * Register on SERDES interrupt #60, read command
 */
static int do_rom_crc_check(void)
{
	int ret_val, i;

	for (i = 0; i < pci_lanes; i++) {
		ret_val = serdes_execute_interrupt(SERDES_WEST_BLOCK_ID, pci_serdes[i], SERDES_INT_CODE_CRC, SERDES_INT_DATA_CRC, 0x0);
		if (ret_val)
			return 1;
	}

	return 0;
}

/* dl_sbus_master_firmware() - Download SBus Master firmware image (Burst Download)
 */
static int dl_sbus_master_firmware(void)
{
	int err;

	/* Set SPICO Reset and Enable off */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0xC0, SBUS_SPICO_CONTROL);
	if (err)
		goto error;

	/* Set IMEM_CNTL_EN on, Enable off & Remove Reset */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0x240, SBUS_SPICO_CONTROL);
	if (err)
		goto error;

	/* Set starting IMEM address for burst download */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0x80000000, SBUS_SPICO_IMEM);
	if (err)
		goto error;

	/* Upload SBus master firmware */
	err = serdes_sbus_burst_upload(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, SBUS_SPICO_IMEM_BURST_DATA, sbus_master_ucode, SBUS_MASTER_UCODE_SIZE);
	if (err)
		goto error;

	/* Set IMEM_CNTL_EN off */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0x40, SBUS_SPICO_CONTROL);
	if (err)
		goto error;

	/* Turn ECC on  */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0xC0000, SBUS_SPICO_ECC);
	if (err)
		goto error;

	/* Set SPICO Enable on */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0x140, SBUS_SPICO_CONTROL);
	if (err)
		goto error;

	return 0;
error:
	printf("dl_sbus_master_firmware: failed to write!\n");
	return 1;
}

/* sbus_master_execute_interrupt() - Initiate SBus Master interrupt
 *
 */
static int sbus_master_execute_interrupt(unsigned int serdes_block_id, unsigned int execute_code, unsigned int execute_data, unsigned int expected_data)
{
	int err, i;
	unsigned int data;

	/* Set the interrupt code */
	data = ((execute_code & SBUS_MASTER_INT_CODE_MASK) << SBUS_MASTER_INT_CODE_OFFS);
	data |= ((execute_data & SBUS_MASTER_INT_DATA_MASK) << SBUS_MASTER_INT_DATA_OFFS);
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, data, SBUS_SPICO_DMEM_DATA_IN);
	if (err)
		goto error;

	/* Issue the interrupt by asserting bit0 */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0x1, SBUS_SPICO_INT_CNTL);
	if (err)
		goto error;

	/* Interrupt de-assertion */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0x0, SBUS_SPICO_INT_CNTL);
	if (err)
		goto error;

	/* Poll for interrupt completion */
	for (i = 0; i < SBUS_MASTER_INT_STATUS_RETRIES; i++) {
		err = serdes_sbus_read_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, &data, SBUS_SPICO_DMEM_DATA_OUT);
		if (err)
			goto error;
#ifdef CONFIG_TARGET_NPS_SOC
		if (!(data & SBUS_MASTER_INT_STATUS_MASK))
			break;
#else
		break;
#endif
	}
	if (i == SBUS_MASTER_INT_STATUS_RETRIES) {
		printf("sbus_master_execute_interrupt: interrupt still in progress, timeout, code: 0x%x\n", execute_code);
		return 1;
	}

#ifdef CONFIG_TARGET_NPS_SOC
	/* Check interrupt result */
	if ((data & SBUS_MASTER_INT_RESULT_MASK) != expected_data) {
		printf("sbus_master_execute_interrupt: interrupt data = 0x%x is not as expected = 0x%x\n",data, expected_data);
		return 1;
	}
#endif

	return 0;
error:
	printf("sbus_master_execute_interrupt: failed to write!\n");
	return 1;
}

/* do_sbus_master_imem_crc_check() - Check SBus Master memory CRC
 *
 */
static int do_sbus_master_imem_crc_check(void)
{
	int ret_val;

	ret_val = sbus_master_execute_interrupt(SERDES_WEST_BLOCK_ID, SBUS_MASTER_INT_CODE_IMEM_CRC, SBUS_MASTER_INT_DATA_CRC, SBUS_MASTER_INT_SUCCESS);
	if (ret_val)
		return 1;

	return 0;
}

/* sbus_master_dmem_upload() - Upload code to SBus Master dmem
 */
static int sbus_master_dmem_upload(unsigned int serdes_block_id, unsigned int start_addr, unsigned int dmem_ucode[], unsigned int ucode_size)
{
#ifdef CONFIG_TARGET_NPS_SOC
	int result;
	unsigned int addr, data;
	int ucode_index;

	/* Upload */
	for (ucode_index = 0; ucode_index < ucode_size; ucode_index++) {
		addr = start_addr + ucode_index;
		data = (addr |
				(1 << 15 ) |
				(dmem_ucode[ucode_index] << 16));
		result = serdes_sbus_write_cmd(serdes_block_id, SBUS_RCVR_SPICO_PROC, data, SBUS_SPICO_DMEM_WRITE);
		if (result) {
			printf("sbus_master_dmem_upload: failed to write!\n");
			return 1;
		}
	}
#endif
	return 0;
}

/* dl_xdmem_full_pcie_firmware() - Download the full-features PCIe firmware
 * into the XDMEM of SBus Master as packed image (file provided by Avago)
 */
static int dl_xdmem_full_pcie_firmware(void)
{
	int err;

	/* Set single step (halts the processor) */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0x1, SBUS_SPICO_SINGLE_STEP);
	if (err)
		goto error;

	/* Enable XDMEM access */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0xD40, SBUS_SPICO_CONTROL);
	if (err)
		goto error;

	/* Upload image to XDMEM */
	err = sbus_master_dmem_upload(SERDES_WEST_BLOCK_ID, 0x400, xdmem_image_ucode, XDMEM_PACKED_IMAGE_UCODE_SIZE);
	if (err)
		goto error;

	/* Disable XDMEM access */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0x140, SBUS_SPICO_CONTROL);
	if (err)
		goto error;

	/* Stop single step. Allow processor to run */
	err = serdes_sbus_write_cmd(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SPICO_PROC, 0x0, SBUS_SPICO_SINGLE_STEP);
	if (err)
		goto error;

	return 0;
error:
	printf("dl_xdmem_full_pcie_firmware: failed to write!\n");
	return 1;
}

/* do_sbus_master_dmem_crc_check() - Check SBus Master XDMEM CRC
 *
 */
static int do_sbus_master_dmem_crc_check(void)
{
	int ret_val;

	ret_val = sbus_master_execute_interrupt(SERDES_WEST_BLOCK_ID, SBUS_MASTER_INT_CODE_DMEM_CRC, SBUS_MASTER_INT_DATA_CRC, SBUS_MASTER_INT_SUCCESS);
	if (ret_val)
		return 1;

	return 0;
}

/* pcie_firmware_swap() - PCIe firmware swap
 */
static int pcie_firmware_swap(void)
{
	int ret_val, i;

	/* PCI PCS reset */
	write_non_cluster_reg(SERDES_WEST_BLOCK_ID, SERDES_REG_PCIE_PCS_RST_N, SERDES_PCIE_PCS_ASSERT);

	/* Issue interrupt #40 for PCI SERDES */
	for (i = 0; i < pci_lanes; i++) {
		ret_val = sbus_master_execute_interrupt(SERDES_WEST_BLOCK_ID, SBUS_MASTER_INT_CODE_FIRMWARE_SWAP, pci_serdes[i], SBUS_MASTER_INT_SUCCESS);
		if (ret_val)
			return 1;
	}

	return 0;
}

/* do_serdes_firmware_check() - CHeck PCIe GEN3 SERDES firmware version
 *
 */
static int do_serdes_firmware_check(void)
{
	int ret_val, i;

	for (i = 0; i < pci_lanes; i++) {
		ret_val = serdes_execute_interrupt(SERDES_WEST_BLOCK_ID, pci_serdes[i], SERDES_INT_CODE_FIRMWARE, SERDES_INT_DATA_FIRMWARE, SERDES_GEN3_FIRMWARE_VERSION);
		if (ret_val)
			return 1;
	}

	return 0;
}

static int fixed_dfe_settings(void)
{
	int err;

	/* Disable PCS interrupts */
	write_non_cluster_reg(SERDES_WEST_BLOCK_ID, SERDES_REG_PCIE_PCS_CFG, SERDES_PCIE_DI_PCS_INTR);

	/* Program seed HF parameter */
	err = serdes_execute_interrupt(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SERDES_BROADCAST, SBUS_MASTER_INT_CODE_SET_DRE, 0x0009, 0x9);
	if (err)
		return 1;

	/* Program seed LF parameter */
	err = serdes_execute_interrupt(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SERDES_BROADCAST, SBUS_MASTER_INT_CODE_SET_DRE, 0x0108, 0x8);
	if (err)
		return 1;

	/* Program seed DC parameter */
	err = serdes_execute_interrupt(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SERDES_BROADCAST, SBUS_MASTER_INT_CODE_SET_DRE, 0x0207, 0x7);
	if (err)
		return 1;

	/* Run before rate change to 8GT/s to determine flag run_ical_on_eq_eval */
	err = serdes_execute_interrupt(SERDES_WEST_BLOCK_ID, SBUS_RCVR_SERDES_BROADCAST, SBUS_MASTER_INT_CODE_SET_DRE, 0x5201, 0x1);
	if (err)
		return 1;

	/* Enable PCS interrupts */
	write_non_cluster_reg(SERDES_WEST_BLOCK_ID, SERDES_REG_PCIE_PCS_CFG, SERDES_PCIE_EN_PCS_INTR);

	return 0;
}

static int equalization_setup(void)
{
	/* Configure PEXC for GEN3 */
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_LOCAL_FS_LF, 0x608);

	/* GEN3 equalization cursor values */
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x0);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x12000);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x1);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x0C000);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x2);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x1F000);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x3);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x09000);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x4);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x00000);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x5);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x00006);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x6);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x00009);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x7);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x0F006);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x8);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x09009);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0x9);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x0000C);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_INDEX, 0xA);
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_PSET_COEF_MAP, 0x18000);

	/* Update equalization control timeout */
	write_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_CONTROL, read_non_cluster_reg(PCIB_BLOCK_ID, PCIB_GEN3_EQ_CONTROL) | 0x20);

	return 0;
}

#ifdef FORCE_X1
static int multilane_x1_only(void)
{
	int ret_val, i;

	if (pci_lanes != PCI_LANES_X8)
		return 0;

	for (i = 1; i < PCI_LANES_X8; i++) {
		ret_val = serdes_execute_interrupt(SERDES_WEST_BLOCK_ID, pci_serdes[i], 0x1, 0x0, 0x1);
		if (ret_val)
			return 1;
	}

	return 0;
}
#endif

/* setup_pci_link() - Setup pcie link
 */
int setup_pci_link(void)
{
	int err;
	unsigned int ref_select, pcie_bypass, pcie_modes;
	volatile int delay = 5000;

	err = update_pci_settings(&ref_select, &pcie_bypass, &pcie_modes);
	if (err)
		goto error;

	/* Set DBI rgr timeout (HW bug fix. Should be fixed on phase2) */
	spl_print("SPL: Set DBI RGR timeout to 0x2000... ");
	write_non_cluster_reg(PCIB_BLOCK_ID, RGR_TIMEOUT_CNTR, 0x2000);
	spl_print("Done\n");

	/* Enable physical functions access to configuration space to prevent
	 * a deadlock when accessing sync registers
	 * (HW bug fix. Should be fixed on phase2) */
	spl_print("SPL: Enable PF access to configuration space... ");
	write_non_cluster_reg(PCIB_BLOCK_ID, ELBI_PF, 0xF);
	spl_print("Done\n");

	/* Set PCI ref clock */
	spl_print("SPL: Set PCI ref clock for lanes (0x%x)... ", ref_select);
	write_non_cluster_reg(SERDES_WEST_BLOCK_ID, SERDES_REG_REF_SELECT_1, ref_select);
	spl_print("Done\n");

	/* Set PCI bypass for unused lanes */
	spl_print("SPL: Set PCI bypass (0x%x)... ", pcie_bypass);
	write_non_cluster_reg(SERDES_WEST_BLOCK_ID, SERDES_REG_PCIE_BYPASS, pcie_bypass);
	spl_print("Done\n");

	/* Wait for ROM enable to finish */
	spl_print("SPL: Waiting for WEST rom enable... ");
	err = do_poll_rom(SERDES_WEST_BLOCK_ID);
	if (err)
		goto error;
	spl_print("Done\n");

	if (pci_gen == PCI_GEN3) {
		spl_print("SPL: GEN3 Equalization setup... ");
		err = equalization_setup();
		if (err)
			goto error;
		spl_print("Done\n");
	}
	if (pci_gen > PCI_GEN1) {
		/*
		 * WA for PCI gen 3 boot issue
		 * TODO change this to debug() when issue is found
		 */
		err = dl_sbus_master_firmware();
		if (err)
			goto error;
		spl_print("Done\n");

		spl_print("SPL: Check SBus master imem CRC... ");
		err = do_sbus_master_imem_crc_check();
		if (err)
			goto error;
		spl_print("Done\n");

		/* Download of the PCIe firmware requires
		 * a delay.
		 */
		while(delay-- > 0);

		spl_print("SPL: Download full-featured PCIe firmware... ");
		err = dl_xdmem_full_pcie_firmware();
		if (err)
			goto error;
		spl_print("Done\n");

		spl_print("SPL: Check SBust master xdmem CRC... ");
		err = do_sbus_master_dmem_crc_check();
		if (err)
			goto error;
		spl_print("Done\n");

		spl_print("SPL: Swapping PCIe firmware... ");
		err = pcie_firmware_swap();
		if (err)
			goto error;
		spl_print("Done\n");
	}

	/* Release PCI PCS reset */
	spl_print("SPL: Release PCI PCS reset... ");
	write_non_cluster_reg(SERDES_WEST_BLOCK_ID, SERDES_REG_PCIE_PCS_RST_N, SERDES_PCIE_PCS_DEASSERT);
	spl_print("Done\n");

	/* Enable PCS interrupts */
	spl_print("SPL: Enable PCS interrupts (0x%x)... ", SERDES_PCIE_EN_PCS_INTR);
	write_non_cluster_reg(SERDES_WEST_BLOCK_ID, SERDES_REG_PCIE_PCS_CFG, SERDES_PCIE_EN_PCS_INTR);
	spl_print("Done\n");

#ifdef CHECK_SPICO_READY
/* no need to wait for SPICO ready because CRC check already does that by initiating interrupt */
	spl_print("SPL: Check SPICO ready... ");
	do_check_spico_ready();
	spl_print("Done\n");
#endif

	if (pci_gen > PCI_GEN1) {
		/*  Check PCIe SERDES firmware version (GEN3) */
		spl_print("SPL: Check PCIe SERDES firmware version... ");
		err = do_serdes_firmware_check();
		if (err)
			goto error;
		spl_print("Done\n");
	}

#ifdef FORCE_X1
	/* Force X1 in X8 mode. Disable SERDES 41-47 */
	spl_print("SPL: Disable SERDES 41-47... ");
	multilane_x1_only();
	spl_print("Done\n");
#endif

	/*  ROM CRC check */
	spl_print("SPL: Check ROM CRC... ");
	err = do_rom_crc_check();
	if (err)
		goto error;
	spl_print("Done\n");

	if (pci_gen == PCI_GEN3) {
		/* Fixed DFE settings configuration (GEN3 only) */
		spl_print("SPL: Fixed DFE settings... ");
		err = fixed_dfe_settings();
		if (err)
			goto error;
		spl_print("Done\n");
	}

	/* Set PCI lanes */
	spl_print("SPL: Set PCI modes (0x%x)... ", pcie_modes);
	write_non_cluster_reg(SERDES_WEST_BLOCK_ID, SERDES_REG_PCIE_MODES, pcie_modes);
	spl_print("Done\n");

	/* Limit MAC */
	spl_print("SPL: Config MAC to GEN%d... ", pci_gen);
	limit_mac();
	spl_print("Done\n");

	/* Retry incoming configuration requests until initialization is complete */
	spl_print("SPL: Set incoming configuration requests retries... ");
	write_non_cluster_reg(PEXC_BLOCK_ID, RSP_CTRL_REG, RSP_CTRL_BOOT_VAL);
	spl_print("Done\n");

	/* Continue link establishment, allow link training status state machine
	 * transition from detect stage after PCIE core configurations are already done*/
	spl_print("SPL: LTSSM enable... ");
	write_non_cluster_reg(PEXC_BLOCK_ID, LTSSM_CTRL_REG, LTSSM_CTRL_VAL);
	spl_print("Done\n");

	spl_print("SPL: Waiting for link status... ");
	err = do_poll_link_status();
	if (!err)
		spl_print("Done\n");

#ifdef DIRECT_SPEED_CHANGE
	/* Direct speed change to GEN3 */
	spl_print("SPL: Direct speed change to GEN3... ");
	direct_speed_change_gen3();
	spl_print("Done\n");
#endif

	/* Update physical functions configuration space */
	spl_print("SPL: Update physical functions configuration space... ");
	update_cs();
	spl_print("Done\n");

	/* Stop retries for incoming configuration requests */
	spl_print("SPL: Unset incoming configuration requests retries... ");
	write_non_cluster_reg(PEXC_BLOCK_ID, RSP_CTRL_REG, RSP_CTRL_DONE_VAL);
	spl_print("Done\n");

	/* Wait for east ROM enable to finish */
	spl_print("SPL: Waiting for EAST rom enable... ");
	do_poll_rom(SERDES_EAST_BLOCK_ID);
	spl_print("Done\n");

	/* return error if polling to link status failed */
	return err;
error:
	return 1;
}


/* jump_to_uboot() - Jump to uboot's address in lram
 */
void jump_to_uboot(void)
{
	void *uboot_ptr = (void *)CONFIG_SYS_TEXT_BASE;
	goto *uboot_ptr;
}

void preload_console(void)
{
	gd = &gdata;

	mem_malloc_init(CONFIG_SYS_SPL_MALLOC_START,
			CONFIG_SYS_SPL_MALLOC_SIZE);
	gd->flags |= GD_FLG_FULL_MALLOC_INIT;

	preloader_console_init();
}

void enable_spi_flash(void)
{
	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	if (!flash) {
		printf("SPL: SPI probe failed.\n");
		hang();
	}
}

void spl_flash_to_mem(u32 offset, void *buf, size_t len)
{
	spi_flash_read(flash, offset, len, buf);
}
