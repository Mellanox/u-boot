/*
 *       Copyright (C) 2015 EZchip, Inc. (www.ezchip.com)
 *
 *       This program is free software; you can redistribute it and/or modify
 *       it under the terms of the GNU General Public License version 2 as
 *       published by the Free Software Foundation.
 */

#ifndef _SPL_H_
#define _SPL_H_

/*SERDES block*/
#define SERDES_BLOCK_ID		0x41D

#define PCIE_PCS_RST_N_REG	0x124
#define PCIE_PCS_RES_N_VAL	0x1

#define PCIE_PCS_CFG_REG	0x123
#define PCIE_EN_PCS_INTR	0x5

#define PCIE_MODES_REG		0x122
#define PCIE_MODES_VAL		0x1

#define SBUS_RCVR_ADDR		0x130
#define SBUS_DATA_ADDR		0x131
#define SBUS_DATA			0x132
#define SBUS_COMMAND		0x133
#define SBUS_STS			0x134

#define SBUS_CONTROLLER		0xFE
#define SBUS_INTR_STATUS	0x33
#define INTR_4_STATUS		0x8000

#define LAST_SERDES			0x3B
#define SPICO_INTR_STATUS	0x4
#define INTR_53_STATUS		0x1

#define STATUS_RDY			0x1
#define STATUS_SUCCESS		0x2
#define STATUS_RDY_SUCCESS	(STATUS_SUCCESS | STATUS_RDY)
#define CORE_IFR_READ_CMD	0x22

/*PCIB block*/
#define PCIB_BLOCK_ID		0x42E

#define LINK_STATUS_PCIB	0x428
#define LINK_STATUS_VAL		0x80000000

#define NUM_PFS				0x4

#define DBI_CTRL_REG		0x1B0
#define DBI_CTRL_PF_OFFSET	0xA
#define DBI_CTRL_PF0_VAL	0x0
#define DBI_CTRL_PF1_VAL	0x400
#define DBI_CTRL_PF2_VAL	0x800
#define DBI_CTRL_PF3_VAL	0x1000

#define DBI_CS2_REG			0x1B1
#define DBI_CS2_RO_REG_VAL	0x1
#define DBI_CS2_RW_REG_VAL	0x0

#define DBI_STS_REG			0x1B2
#define DBI_STS_RDY_VAL		0x1

/*PCIB Configuration Space*/
#define PCIB_BLOCK_ID 					0x42E
#define PCIB_PCIE_ADDR					0x400

#define CS_VENDOR_ID_OFFSET					0x0
#define VENDOR_IF_BYTE_SIZE					2

#define CS_DEVICE_ID_OFFSET					0x2
#define DEVICE_ID_BYTE_SIZE					2

#define CS_BAR_0_BASE_ADDR_OFFSET		 	0x10
#define BAR_0_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_1_BASE_ADDR_OFFSET		 	0x14
#define BAR_1_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_2_BASE_ADDR_OFFSET		 	0x18
#define BAR_2_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_3_BASE_ADDR_OFFSET		 	0x1C
#define BAR_3_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_4_BASE_ADDR_OFFSET		 	0x20
#define BAR_4_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_BAR_5_BASE_ADDR_OFFSET		 	0x24
#define BAR_5_BASE_ADDR_ADDRESS_BYTE_SIZE	4

#define CS_SUBSYSTEM_VENDOR_ID_OFFSET 		0x2C
#define SUBSYSTEM_VENDOR_ID_BYTE_SIZE		4

#define CS_SUBSYSTEM_ID_OFFSET 				0x2E
#define SUBSYSTEM_ID_BYTE_SIZE				2

/*PEXC block*/
#define PEXC_BLOCK_ID		0x416

#define RSP_CTRL_REG		0x10D
#define RSP_CTRL_BOOT_VAL	0x80000000
#define RSP_CTRL_DONE_VAL	0

#define LTSSM_CTRL_REG		0x100
#define LTSSM_CTRL_VAL		0x80000000

#define LINK_STATUS_PEXC	0x10B
#define LINK_STATUS_UP		0xC0000000

#endif /* _SPL_H_ */
