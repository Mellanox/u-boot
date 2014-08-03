/*
*	Copyright (C) 2014 EZchip, Inc. (www.ezchip.com)
*
*	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License version 2 as
*	published by the Free Software Foundation.
*/

#ifndef _CONFIG_NPS400_H_
#define _CONFIG_NPS400_H_

/*
 *  CPU configuration
 */
#define CONFIG_ARC_MMU_VER		3
#define CONFIG_SYS_TIMER_RATE		CONFIG_SYS_CLK_FREQ

/*
 * Board configuration
 */
#define CONFIG_BOARD_EARLY_INIT_R

#define CONFIG_ARCH_EARLY_INIT_R

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	0xf8000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		0x1000000	/* 16 Mb */
#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_MALLOC_LEN		0x200000	/* 2 MB */
#define CONFIG_SYS_LOAD_ADDR		0xf8c00000	/* 12MB offset in lram */
#define CONFIG_SYS_NO_FLASH

/*
 * UART configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	4
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_NS16550_COM1		0xF7209000
#define CONFIG_CONS_INDEX		1 /* Console is on COM1 */
#define CONFIG_BAUDRATE			1200

/*
 * Command line configuration
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_RUN		/* run command in env variable */
#define CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_MAXARGS	16

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE		0x8000
#define CONFIG_ENV_SECT_SIZE	0x40000

/*
 * Environment configuration
 */
#define CONFIG_BOOTDELAY		0
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT		\
	"\nEnter password(nps) - autoboot in %d seconds...\n", CONFIG_BOOTDELAY
#define CONFIG_AUTOBOOT_DELAY_STR	"nps"
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

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
#define CONFIG_OVERWRITE_ETHADDR_ONCE
#define CONFIG_ETHADDR       00:C0:00:99:AA:FE       /* Ethernet address */
#define CONFIG_IPADDR        10.1.8.254              /* Our ip address */
#define CONFIG_SERVERIP      10.1.3.58               /* Tftp server ip address */
#define CONFIG_GATEWAYIP     10.1.1.10
#define CONFIG_NETMASK       255.255.0.0

#define CONFIG_OF_LIBFDT	1

/*
 * Environment information
 */
#define CONFIG_RAMBOOTCOMMAND				\
	"run miscargs;"					\
	"boot_prepare;"                                 \
	"bootm ${loadaddr} - ${fdtaddr}"

#define CONFIG_BOOTCOMMAND	"run ramboot"

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"verify=no\0"						\
	"machid=270f0034\0"					\
	"krn_args=\0"						\
	"miscargs=setenv bootargs ${bootargs} ${krn_args}\0"	\
	"loadaddr=0x80001fc0\0"					\
	"fdt_high=0xffffffff\0"					\
	"fdtaddr=0x80e00000\0"					\

#endif /* _CONFIG_NPS400_H_ */
