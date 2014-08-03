/*
*	Copyright (C) 2015 EZchip, Inc. (www.ezchip.com)
*
* 	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License version 2 as
*	published by the Free Software Foundation.
*/

#ifndef _NPS_H_
#define _NPS_H_

#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <linux/ctype.h>
#include <libfdt.h>
#include <image.h>
#include "common.h"
#include "ddr.h"

#define NPS_IMAGE_LRAM_ADDRESS(lram_offset)		(CONFIG_SYS_SDRAM_BASE + lram_offset)
#define NPS_IMAGE_FLASH_ADDRESS(flash_offset)	(CONFIG_SYS_SDRAM_BASE + flash_offset)

#define UBOOT_SIZE		0x80000 /* 512k */

/* LRAM images addresses and offsets */
#define KRN_CMDLINE_LRAM_OFFSET		0x4000
#define PRESENT_CPUS_LRAM_OFFSET	0x5000
#define POSSIBLE_CPUS_LRAM_OFFSET	0xa000
#define UBOOT_LRAM_OFFSET			0xf00000

#define KRN_CMDLINE_LRAM_ADDRESS	NPS_IMAGE_LRAM_ADDRESS(KRN_CMDLINE_LRAM_OFFSET)
#define PRESENT_CPUS_LRAM_ADDRESS	NPS_IMAGE_LRAM_ADDRESS(PRESENT_CPUS_LRAM_OFFSET)
#define POSSIBLE_CPUS_LRAM_ADDRESS	NPS_IMAGE_LRAM_ADDRESS(POSSIBLE_CPUS_LRAM_OFFSET)
#define UBOOT_LRAM_ADDRESS			NPS_IMAGE_LRAM_ADDRESS(UBOOT_LRAM_OFFSET)

/* FLASH images addresses and offsets */
#define DTB_FLASH_OFFSET		0x0
#define KERNEL_FLASH_OFFSET		0x100000
#define ROOTFS_FLASH_OFFSET		0x700000
#define UBOOT_FLASH_OFFSET		0xe80000

#define DTB_FLASH_ADDRESS		NPS_IMAGE_FLASH_ADDRESS(DTB_FLASH_OFFSET)
#define KERNEL_FLASH_ADDRESS	NPS_IMAGE_FLASH_ADDRESS(KERNEL_FLASH_OFFSET)
#define ROOTFS_FLASH_ADDRESS	NPS_IMAGE_FLASH_ADDRESS(ROOTFS_FLASH_OFFSET)

/* EMEM properties */
#define LINUX_BASE_ADDRESS		0x80002000
#define UIMAGE_HEADER_SIZE		0x40
#define UIMAGE_BASE_ADDRESS		(LINUX_BASE_ADDRESS - UIMAGE_HEADER_SIZE)
#define DTB_EMEM_ADDRESS		0x80E00000
#define ROOTFS_EMEM_ADDRESS		0x81000000

#define ROOTFS_MAX_SIZE			0x780000
#define UIMAGE_MAX_SIZE			0x600000
#define DTB_MAX_SIZE			0x100000
#define CPU_MAP_STR_MAX_SIZE	0x5000 /* 20Kb */
#define FDT_PAD_SIZE			(2 * CPU_MAP_STR_MAX_SIZE)
#define UBOOT_MAX_SIZE			0x80000

#define KRN_COMMANDLINE_MAX_SIZE	1024
#define KERNEL_CMDLINE_FS_STR		"root=/dev/ram0 rw initrd=0x81000000,8388608 "

#define MAX_CLUSTERS			16
#define MAX_CORES_PER_CLUSTER	16
#define MAX_THREADS_PER_CORE	16
#define	MAX_CORES				(MAX_CLUSTERS * MAX_CORES_PER_CLUSTER)
#define	MAX_THREADS				(MAX_CLUSTERS * MAX_CORES_PER_CLUSTER * MAX_THREADS_PER_CORE)

#define CORE_MAP_MAX_SIZE		16
#define CPU_MAP_MAX_SIZE		(CORE_MAP_MAX_SIZE * MAX_CORES)

#define FDT_ROOT_NODE_OFFSET	0

int do_file_load(char *file_name_var, char* mode);

#endif /* _NPS_H_ */
