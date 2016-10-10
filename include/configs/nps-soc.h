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

#ifndef _CONFIG_NPS_SOC_H_
#define _CONFIG_NPS_SOC_H_

#ifndef __ASSEMBLY__
unsigned long get_board_sys_clk(void);
#endif

/*
 *  CPU configuration
 */
#define CONFIG_ARC_MMU_VER		3
#undef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ		get_board_sys_clk()
#define CONFIG_SYS_TIMER_RATE		CONFIG_SYS_CLK_FREQ

/*
 * Board configuration
 */
#define CONFIG_ARCH_EARLY_INIT_R
#define CONFIG_BOARD_LATE_INIT

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
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x70000000)
#define CONFIG_NPS_UIMAGE_EMEM_ADDR	0x80000000
#define CONFIG_NPS_DTB_EMEM_ADDR	0x8ff00000
#define CONFIG_SYS_BOOTM_LEN		0x2000000
#define BIST_CONFIG_LRAM_ADDRESS	0xf8500000

/*
 * External memory configuration
 */

#define CONFIG_NPS_DDR_DEBUG

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
#define CONFIG_BAUDRATE			115200

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
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE		0x8000
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_OFFSET	0xff0000
#define CONFIG_ENV_OVERWRITE 	1

/*
 * Environment configuration
 */
#define CONFIG_BOOTDELAY		3
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR	"\x1b\x1b"
#undef CONFIG_AUTOBOOT_DELAY_STR
#define CONFIG_AUTOBOOT_PROMPT		"Autobooting in %d seconds, " \
					"press \"<Esc><Esc>\" to stop\n", bootdelay
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_LOADADDR			0x90000000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/*
 * Console configuration
 */
#define	CONFIG_SYS_HUSH_PARSER
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
#define CONFIG_SPI_FLASH_BAR
#define CONFIG_SF_DEFAULT_SPEED		25000000
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#define CONFIG_NPS_SPI_BASE		0xF7208C00
#define CONFIG_NPS_UIMAGE_FLASH_OFFS	0x0
#define CONFIG_NPS_UBOOT_FLASH_OFFS	0xe70000
#define CONFIG_NPS_DTB_FLASH_OFFS	0xef0000
#define CONFIG_NPS_BTL_FLASH_OFFS	0xf00000
#define CONFIG_NPS_UIMAGE_SIZE		0xe70000
#define CONFIG_NPS_UBOOT_SIZE		0x80000
#define CONFIG_NPS_BTL_SIZE		0x80000
#define CONFIG_NPS_DTB_SIZE		0x10000

/*
 * NET
 */
#define CONFIG_NPS_ETH
#define CONFIG_NPS_DBG_LAN_WEST
#define CONFIG_OVERWRITE_ETHADDR_ONCE
#define CONFIG_ETHADDR			00:C0:00:F0:04:58	/* Ethernet address */
#define CONFIG_IPADDR			10.1.5.110		/* Our ip address */
#define CONFIG_SERVERIP			10.1.3.132		/* tftp server ip address */
#define CONFIG_GATEWAYIP		10.1.1.10
#define CONFIG_NETMASK			255.255.0.0

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
				"console=ttyS0,115200n8 slub_max_order=0"
#define CONFIG_BOOTCOMMAND	"run ramboot"
#define CONFIG_RAMBOOTCOMMAND	"sf probe 0 && "					\
				"sf read ${fdtaddr} ${dtb_flash_offs} ${dtb_size} && "	\
				"sf read ${loadaddr} ${krn_flash_offs} ${krn_size} && "	\
				"run addip addmisc && "					\
				"bootm ${loadaddr} - ${fdtaddr}"
#define CONFIG_NFSBOOTCOMMAND	"tftpboot ${loaddaddr} ${bootfile} && "			\
				"sf probe 0 && "					\
				"sf read ${fdtaddr} ${dtb_flash_offs} ${dtb_size} && "	\
				"run addip addmisc && "					\
				"bootm ${loadaddr} - ${fdtaddr}"
#define CONFIG_EXTRA_ENV_SETTINGS						\
	"pci_gen=3\0"								\
	"verify=yes\0"								\
	"ddr_freq=1200\0"							\
	"ddr_package=x8\0"							\
	"ddr_type=DDR4\0"							\
	"ddr_debug=0\0"								\
	"ddr_vref_bypass=1\0"							\
	"ddr_skip_bist=0\0"							\
	"ddr_skip_init=0\0"							\
	"phy_rd_vref=850\0"							\
	"fdt_high=0xffffffff\0"							\
	"fdtaddr=" __stringify(CONFIG_NPS_DTB_EMEM_ADDR) "\0"			\
	"btl_flash_offs=" __stringify(CONFIG_NPS_BTL_FLASH_OFFS) "\0"		\
	"btl_size=" __stringify(CONFIG_NPS_BTL_SIZE) "\0"			\
	"krn_flash_offs=" __stringify(CONFIG_NPS_UIMAGE_FLASH_OFFS) "\0"	\
	"krn_size=" __stringify(CONFIG_NPS_UIMAGE_SIZE) "\0"			\
	"uboot_flash_offs=" __stringify(CONFIG_NPS_UBOOT_FLASH_OFFS) "\0"	\
	"uboot_size=" __stringify(CONFIG_NPS_UBOOT_SIZE) "\0"			\
	"dtb_flash_offs=" __stringify(CONFIG_NPS_DTB_FLASH_OFFS) "\0"		\
	"dtb_size=" __stringify(CONFIG_NPS_DTB_SIZE) "\0"			\
	"bistcfgaddr=" __stringify(BIST_CONFIG_LRAM_ADDRESS) "\0"		\
	"btl_file=btl.bin\0"							\
	"uboot_file=u-boot.bin\0"						\
	"krn_file=uImage\0"							\
	"dtb_file=eznps_soc.dtb\0"						\
	"bist_cfg_file=bist_config.cfg\0"					\
	"addip=setenv bootargs ${bootargs} " 					\
		"ip=${ipaddr}:${serverip}:${gatewayip}:" 			\
		"${netmask}:${hostname}:eth0:off\0"				\
	"addmisc=setenv bootargs ${bootargs} ${extra_bootargs}\0"			\
	"btlup=tftpboot ${loadaddr} ${btl_file} &&"				\
		"sf probe 0 && "						\
		"sf erase ${btl_flash_offs} +${filesize} && "			\
		"sf write ${loadaddr} ${btl_flash_offs} ${filesize}\0"		\
	"ubootup=tftpboot ${loadaddr} ${uboot_file} && "			\
		"sf probe 0 && "						\
		"sf erase ${uboot_flash_offs} +${filesize} && "			\
		"sf write ${loadaddr} ${uboot_flash_offs} ${filesize}\0"	\
	"krnup=tftpboot ${loadaddr} ${krn_file} && "				\
		"sf probe 0 && "						\
		"sf erase ${krn_flash_offs} +${filesize} && "			\
		"sf write ${loadaddr} ${krn_flash_offs} ${filesize}\0 "		\
	"dtbup=tftpboot ${loadaddr} ${dtb_file} && "				\
		"sf probe 0 && "						\
		"sf erase ${dtb_flash_offs} +${filesize} && "			\
		"sf write ${loadaddr} ${dtb_flash_offs} ${filesize}\0"		\
	"bistcfgup=tftpboot ${bistcfgaddr} ${bist_cfg_file} && "		\
		"ddr_bist run 0xffffff 10 ${bistcfgaddr}\0"			\

/* SPL defines */
#define CONFIG_SPL_LDSCRIPT	"board/ezchip/nps/nps-soc_spl/u-boot-spl.lds"
#define CONFIG_SPL_START_S_PATH	"board/ezchip/nps/nps-soc_spl/"
#define CONFIG_SPL_TEXT_BASE	0xf8002000

/* SPL related SERIAL defines */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

/* SPL related SPI defines */
#define CONFIG_SYS_SPL_MALLOC_START		0xf8080000
#define CONFIG_SYS_SPL_MALLOC_SIZE		0x80000
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT

#endif /* _CONFIG_NPS_SOC_H_ */
