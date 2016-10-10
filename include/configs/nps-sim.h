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

#ifndef _CONFIG_NPS_SIM_H_
#define _CONFIG_NPS_SIM_H_

/*
 *  CPU configuration
 */
#define CONFIG_ARC_MMU_VER		3
#define CONFIG_SYS_TIMER_RATE		CONFIG_SYS_CLK_FREQ

/*
 * Board configuration
 */
#define CONFIG_ARCH_EARLY_INIT_R
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_MISC_INIT_R	/* Call misc_init_r */

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	0xf8000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		0x1000000	/* 16 Mb */
#define CONFIG_SYS_INIT_SP_ADDR	\
		(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_MALLOC_LEN		0x200000	/* 2 MB */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_NPS_KERNEL_MSID_SIZE	0x100000000
#define CONFIG_NPS_UIMAGE_EMEM_ADDR	0x80001fc0
#define CONFIG_NPS_DTB_EMEM_ADDR	0x80f00000
#define CONFIG_SYS_BOOTM_LEN		0x2000000

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
#define CONFIG_CMD_SF
#define CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot */
#define CONFIG_CMD_PING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_CMDLINE_EDITING	1	/* add command line history(up arrow & down arrow)	*/

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
#define CONFIG_LOADADDR			0x80001fc0
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR	

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
 * FLASH
 */
#define CONFIG_NPS_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SF_DEFAULT_SPEED		128500
#define CONFIG_ENV_SPI_MAX_HZ		128500
#define CONFIG_NPS_SPI
#define CONFIG_NPS_SPI_BASE		0xF7208C00
#define CONFIG_NPS_UIMAGE_FLASH_OFFS	0x0
#define CONFIG_NPS_UBOOT_FLASH_OFFS	0xe70000
#define CONFIG_NPS_DTB_FLASH_OFFS	0xef0000
#define CONFIG_NPS_UIMAGE_SIZE		0xe70000
#define CONFIG_NPS_UBOOT_SIZE		0x80000
#define CONFIG_NPS_DTB_SIZE		0x10000

/*
 * NET
 */
#define CONFIG_NPS_ETH
#define CONFIG_OVERWRITE_ETHADDR_ONCE

/*
 * fdt
 */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP

/*
 * Misc
 */
#define CONFIG_LIB_RAND

/*
 * Environment information
 */
#define CONFIG_BOOTARGS		"earlycon=uart8250,mmio32be,0xf7209000,115200n8 "	\
				"console=ttyS0,115200n8 " 				\
				"rcupdate.rcu_cpu_stall_suppress=1"
#define CONFIG_BOOTCOMMAND	"run ramboot"
#define CONFIG_RAMBOOTCOMMAND	"run addip addmisc; "		\
				"bootm ${loadaddr} - ${fdtaddr}"
#define CONFIG_EXTRA_ENV_SETTINGS				\
	"verify=no\0"						\
	"fdt_high=0xffffffff\0"					\
	"fdtaddr=" __stringify(CONFIG_NPS_DTB_EMEM_ADDR) "\0"	\
	"addip=setenv bootargs ${bootargs} " 			\
		"ip=${ipaddr}:${serverip}:${gatewayip}:" 	\
		"${netmask}:${hostname}:eth0:off\0"		\
	"addmisc=setenv bootargs ${bootargs} ${extra_bootargs}\0"

/* SPL defines */
#define CONFIG_SPL_LDSCRIPT	"board/ezchip/nps/nps-sim_spl/u-boot-spl.lds"
#define CONFIG_SPL_START_S_PATH	"board/ezchip/nps/nps-sim_spl/"
#define CONFIG_SPL_TEXT_BASE	0xf8002000

/* SPL related SERIAL defines */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

/* SPL related SPI defines */
#define CONFIG_SYS_SPL_MALLOC_START	0xf8080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT

#endif /* _CONFIG_NPS_SIM_H_ */
