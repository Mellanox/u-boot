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

#ifndef _SPL_COMMON_H_
#define _SPL_COMMON_H_

void spl_flash_to_mem(u32 offset, void *buf, size_t len);
int setup_pci_link(void);
void jump_to_uboot(void);
void preload_console(void);
void enable_spi_flash(void);
void spl_print(const char* s);

#ifdef SPL_PRINTS
#define spl_print(...) printf(__VA_ARGS__)
#else
#define spl_print(...)
#endif

#define POLL_LINK_STATUS_RETRIES	50000000
#define UBOOT_ENV_SIZE_TO_READ 3000

/* PCI lanes */
#define PCI_LANES_X8	8
#define PCI_LANES_X1	1

/* PCI gen */
#define PCI_GEN1	1
#define PCI_GEN2	2
#define PCI_GEN3	3

/* Physical functions */
#define NUM_PFS			4
#ifdef CONFIG_TARGET_NPS_SOC
#define USED_PFS		1
#else
#define USED_PFS		2
#endif

/* CFGB block */
#define CFGB_WEST_BLOCK_ID		0x41A
#define CFGB_REG_ERR_RES		0x105

/*SERDES block*/
#define SERDES_WEST_BLOCK_ID		0x41D
#define SERDES_EAST_BLOCK_ID		0x51D

#define SERDES_REG_REF_SELECT_1		0x111
#define SERDES_REG_PCIE_BYPASS		0x120
#define SERDES_REG_PCIE_PCS_RST_N	0x124
#define SERDES_REG_PCIE_PCS_CFG		0x123
#define SERDES_REG_PCIE_MODES		0x122

#define SERDES_PCIE_PCS_ASSERT		0x0
#define SERDES_PCIE_PCS_DEASSERT	0x1
#define SERDES_PCIE_EN_PCS_INTR		0x5
#define SERDES_PCIE_DI_PCS_INTR		0x7
#define SERDES_PCIE_X1				0x1
#define SERDES_PCIE_X8				0x4

#define SERDES_INT_CODE_CRC				0x3C
#define SERDES_INT_DATA_CRC				0x0

#define SERDES_INT_CODE_FIRMWARE		0x0
#define SERDES_INT_DATA_FIRMWARE		0x0

#define SERDES_GEN3_FIRMWARE_VERSION	0x1055

/* SBUS */
#define SBUS_RCVR_CONTROLLER		0xFE
#define SBUS_CONTROLLER_ROM_STS		0x33
#define SBUS_DATA_ROM_ENABLED		0x3 /* exec completed & acknowledged */

#define SBUS_RCVR_LAST_SERDES		0x3B
#define SBUS_RCVR_SERDES_40			0x29
#define SBUS_RCVR_SERDES_41			0x2B
#define SBUS_RCVR_SERDES_42			0x2D
#define SBUS_RCVR_SERDES_43			0x2F
#define SBUS_RCVR_SERDES_44			0x31
#define SBUS_RCVR_SERDES_45			0x33
#define SBUS_RCVR_SERDES_46			0x35
#define SBUS_RCVR_SERDES_47			0x37
#define SBUS_RCVR_SERDES_48			0x39

#define SBUS_RCVR_SERDES_BROADCAST	0xFF

/* SBUS GEN3 */

#define SBUS_RCVR_SPICO_PROC		0xFD
#define SBUS_SPICO_CONTROL			0x1
#define SBUS_SPICO_DMEM_DATA_IN		0x2
#define SBUS_SPICO_DMEM_WRITE		0x4
#define SBUS_SPICO_SINGLE_STEP		0x5
#define SBUS_SPICO_INT_CNTL			0x7
#define SBUS_SPICO_DMEM_DATA_OUT	0x8
#define SBUS_SPICO_IMEM				0x3
#define SBUS_SPICO_IMEM_BURST_DATA	0x14
#define SBUS_SPICO_ECC				0x16

#define SBUS_MASTER_INT_CODE_OFFS	0
#define SBUS_MASTER_INT_CODE_MASK	0xFFFF
#define SBUS_MASTER_INT_DATA_OFFS	16
#define SBUS_MASTER_INT_DATA_MASK	0xFFFF
#define SBUS_MASTER_INT_STATUS_RETRIES	100000
#define SBUS_MASTER_INT_STATUS_MASK		0x8000
#define SBUS_MASTER_INT_RESULT_MASK		0x7FFF
#define SBUS_MASTER_INT_SUCCESS			0x0001

#define SBUS_MASTER_INT_CODE_IMEM_CRC		0x2
#define SBUS_MASTER_INT_CODE_DMEM_CRC		0x4
#define SBUS_MASTER_INT_CODE_FIRMWARE_SWAP	0x28
#define SBUS_MASTER_INT_CODE_SET_DRE		0x26
#define SBUS_MASTER_INT_DATA_CRC			0x0


/*PCIB block*/
#define PCIB_BLOCK_ID		0x42E
#define PCIB_PCIE_ADDR		0x400
#define CS_OFFSET_TO_ADDR(offset) (PCIB_PCIE_ADDR + (offset)/4)

#define PCIB_GEN3_EQ_LOCAL_FS_LF	CS_OFFSET_TO_ADDR(0x894)
#define PCIB_GEN3_EQ_PSET_INDEX		CS_OFFSET_TO_ADDR(0x89C)
#define PCIB_GEN3_EQ_PSET_COEF_MAP	CS_OFFSET_TO_ADDR(0x898)
#define PCIB_GEN3_EQ_CONTROL		CS_OFFSET_TO_ADDR(0x8A8)

#define PCIB_GEN2_CTRL_REG							CS_OFFSET_TO_ADDR(0x80C)
#define PCIB_LINK_CONTROL2_REG						CS_OFFSET_TO_ADDR(0xA0)
#define PCIB_LINK_CAPABILITIES_REG					CS_OFFSET_TO_ADDR(0x7C)
#define PCIB_LINK_CONTROL2_TARGET_LINK_SPEED		0x0000000F
#define PCIB_LINK_CONTROL2_TARGET_LINK_SPEED_GEN1	0x1
#define PCIB_LINK_CONTROL2_TARGET_LINK_SPEED_GEN2	0x2
#define PCIB_LINK_CONTROL2_TARGET_LINK_SPEED_GEN3	0x3

#define RGR_TIMEOUT_CNTR	0x1C0
#define ELBI_PF				0x171

#define DBI_CTRL_REG		0x1B0
#define DBI_CTRL_PF_OFFSET	0xA
#define DBI_CTRL_PF0_VAL	0x0
#define DBI_CTRL_PF1_VAL	0x400
#define DBI_CTRL_PF2_VAL	0x800
#define DBI_CTRL_PF3_VAL	0x1000

#define DBI_CS2_REG					0x1B1
#define DBI_CS2_RO_ENABLE_RW		0x1
#define DBI_CS2_RO_DISABLE_RW		0x0

#define DBI_STS_REG		0x1B2
#define DBI_STS_RDY_VAL		0x1

/*PCIB Configuration Space*/
#define CS_VENDOR_ID_OFFSET	0x0
#define VENDOR_IF_BYTE_SIZE	2

#define CS_DEVICE_ID_OFFSET	0x2
#define DEVICE_ID_BYTE_SIZE	2

#define CS_CLASS_CODE_OFFSET	0x8

#define CS_BAR_0_BASE_ADDR_OFFSET		0x10
#define BAR_0_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_1_BASE_ADDR_OFFSET		0x14
#define BAR_1_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_2_BASE_ADDR_OFFSET		0x18
#define BAR_2_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_3_BASE_ADDR_OFFSET		0x1C
#define BAR_3_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_4_BASE_ADDR_OFFSET		0x20
#define BAR_4_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_5_BASE_ADDR_OFFSET		0x24
#define BAR_5_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_SUBSYSTEM_VENDOR_ID_OFFSET		0x2C
#define SUBSYSTEM_VENDOR_ID_BYTE_SIZE		4

#define CS_SUBSYSTEM_ID_OFFSET			0x2E
#define SUBSYSTEM_ID_BYTE_SIZE			2

struct pci_bar {
	union {
		struct {
			u32 base_address	: 28,
			prefetchable		: 1,
			locatable			: 2,
			region_type			: 1;
		};
		u32 value;
	};
};

#define CS_MSIX_CAP_TABLE_OFFSET		0xB4
#define CS_MSIX_CAP_PBA_OFFSET			0xB8

struct msixcap_offset {
	union {
		struct {
			u32 offset	: 29,
			bir			: 3;
		};
		u32 value;
	};
};

/* SR-IOV */
#define PCIB_SRIOV_TOTAL_AND_INITIAL_VFS	CS_OFFSET_TO_ADDR(0x1B4)
#define PCIB_SRIOV_NUM_VFS					CS_OFFSET_TO_ADDR(0x1B8)


/*PEXC block*/
#define PEXC_BLOCK_ID		0x416

#define RSP_CTRL_REG		0x10D
#define RSP_CTRL_BOOT_VAL	0x80000000
#define RSP_CTRL_DONE_VAL	0

#define LTSSM_CTRL_REG		0x100
#define LTSSM_CTRL_VAL		0x80000000

#define LINK_STATUS_PEXC	0x10B
#define LINK_STATUS_UP		0xC0000000

#endif /* _SPL_COMMON_H_ */
