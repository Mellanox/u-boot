/*
*	Copyright (C) 2014 EZchip, Inc. (www.ezchip.com)
*
*	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License version 2 as
*	published by the Free Software Foundation.
*/

#ifndef _CONFIG_NPS_MINI_HE_H_
#define _CONFIG_NPS_MINI_HE_H_

#define CONFIG_NPS_MINI_HE
/*
 *  CPU configuration
 */
#define CONFIG_ARC_MMU_VER		3
#define CONFIG_SYS_TIMER_RATE		CONFIG_SYS_CLK_FREQ

/*
 * Board configuration
 */
#define CONFIG_ARCH_EARLY_INIT_R

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		0x20000000	/* 512 Mb */
#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_BOOTM_LEN		0x2000000	/* 32 MB */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (1 << 19))	/* 2 MB */
#define CONFIG_SYS_LOAD_ADDR		0x90000000
#define CONFIG_ROOTPATH				"/opt/ARC/rootfs/default"
#define CONFIG_SYS_NO_FLASH

/*
 * UART configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	4
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_NS16550_COM1		0xC0000000
#define CONFIG_CONS_INDEX			1 /* Console is on COM1 */
#define CONFIG_BAUDRATE				115200

/*
 * Command line configuration
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_RUN      /* run command in env variable */
#define CONFIG_CMD_NET      /* bootp, tftpboot, rarpboot */

#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_MAXARGS		16

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE                 0x8000
#define CONFIG_ENV_OFFSET               0x580000
#define CONFIG_ENV_SECT_SIZE    0x40000
#define CONFIG_ENV_OVERWRITE 1

/*
 * Environment configuration
 */
#define CONFIG_BOOTDELAY		3
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_BOOTARGS			"console=ttyARC0,115200n8"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR
#define CONFIG_ROOTPATH			"/opt/ARC/rootfs/default"

/*
 * Console configuration
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"nps# "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)
/*
 * NET
 */
#define CONFIG_ETHADDR       00:C0:00:99:AA:FE       /* Ethernet address */
#define CONFIG_IPADDR        10.1.8.254              /* Our ip address */
#define CONFIG_SERVERIP      10.1.3.58               /* Tftp server ip address */
#define CONFIG_GATEWAYIP     10.1.1.10
#define CONFIG_NETMASK       255.255.0.0

/*
 * Environment information
 */
#define CONFIG_NFSBOOTCOMMAND			\
	"tftpboot ${krn_file};"			\
	"run addtty;"			\
	"run miscargs;"			\
	"run nfsargs;"			\
	"run addip;"			\
	"run addmac;"			\
	"bootm ${loadaddr}"

#define CONFIG_BOOTCOMMAND		\
	"run nfsboot"

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"machid=270f0034\0"				\
	"preboot_offs=590000\0"				\
	"preboot_file=\"releases/nps/pre-boot.bin\"\0"				\
	"uboot_offs=500000\0"				\
	"uboot_file=\"releases/nps/u-boot.bin\"\0"				\
	"krn_offs=600000\0"				\
	"krn_file=\"releases/nps/uImage\"\0"				\
	"krn_args=mem=512M maxcpus=2\0"				\
	"load_kernel=sf probe 0;"				\
	"sf read ${loadaddr} ${krn_offs} 40; "				\
	"upkrn_sz; "				\
	"sf read ${loadaddr} ${krn_offs} ${krn_size}\0"				\
	"addtty=setenv bootargs "				\
	"console=ttyS0,${baudrate}n8\0"				\
	"addip=setenv bootargs ${bootargs} "				\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"				\
	"${hostname}:eth0:off\0"				\
	"addmac=setenv bootargs ${bootargs} "				\
	"mac=${ethaddr}\0"				\
	"miscargs=setenv bootargs ${bootargs} "				\
	"${krn_args}\0"				\
	"nfsargs=setenv bootargs ${bootargs} "				\
	"root=/dev/nfs rw "				\
	"nfsroot=${serverip}:${rootpath}\0"

#endif /* _CONFIG_NPS_MINI_HE_H_ */
