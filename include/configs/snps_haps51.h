/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ARC700       1 /* in a ARC700 core */
#define CONFIG_HAPS51_DW_IP 1 /* Synopsys DesignWare IP  */

/*
 * NAND define
 */
#include <asm/arch/cpu.h> /* get chip and board defs */

/*
 * Display, CPU and Board information
 */
#define CONFIG_SYS_CLK     50 /* 50Mhz  lk@todo redefine it later */
#define CONFIG_CRYSTAL_MHZ 50 /* Christal 50 Mhz*/

#undef  CONFIG_USE_IRQ        /* no support for IRQs */
#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG 1  /* enable passing of ATAGs */
#define CONFIG_INITRD_TAG  1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN (CONFIG_ENV_SIZE + (1 << 19))
#define CONFIG_SYS_GBL_DATA_SIZE 128 /* bytes reserved for ... */

/*
 * select serial console configuration
 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE           115200
#define CONFIG_SYS_BAUDRATE_TABLE {4800, 9600, 19200, 38400, 57600, 115200}

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_CLK  (12500000)
#define CONFIG_SYS_NS16550_COM1 0xC0000000
#define CONFIG_SYS_NS16550_COM2 0xC0002000
#define CONFIG_SYS_NS16550_COM3 0xC0004000
#define CONFIG_SYS_NS16550_COM4 0xC0006000
#define CONFIG_CONS_INDEX       1 /* Console is on COM1 */

/*
 * commands to include
 */
#include <config_cmd_default.h>

#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_FPGA      /* FPGA configuration Support */
#define CONFIG_CMD_BDI      /* bdinfo */
#define CONFIG_CMD_BOOTD    /* bootd */
#define CONFIG_CMD_CONSOLE  /* coninfo */
#define CONFIG_CMD_ECHO     /* echo arguments */
#define CONFIG_CMD_EDITENV  /* editenv */
#undef CONFIG_CMD_FPGA      /* FPGA configuration Support */
#undef CONFIG_CMD_IMI       /* iminfo */
#undef CONFIG_CMD_ITEST     /* Integer (and string) test */
#ifndef CONFIG_SYS_NO_FLASH
#define CONFIG_CMD_FLASH    /* flinfo, erase, protect */
#endif
#define CONFIG_CMD_IMLS     /* List all found images */
#define CONFIG_CMD_LOADB    /* loadb */
#define CONFIG_CMD_LOADS    /* loads */
#define CONFIG_CMD_MEMORY   /* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC     /* Misc functions like sleep etc */
#undef CONFIG_CMD_NET       /* bootp, tftpboot, rarpboot */
#undef CONFIG_CMD_NFS       /* NFS support */
#define CONFIG_CMD_RUN      /* run command in env variable */
#define CONFIG_CMD_SAVEENV  /* saveenv */
#undef CONFIG_CMD_SETGETDCR /* DCR support on 4xx */
#define CONFIG_CMD_SOURCE   /* "source" command support */
#undef CONFIG_CMD_XIMG      /* Load part of Multi Image */
#undef CONFIG_CMD_KGDB      /* KGDB support */

#define MTDIDS_DEFAULT    "nand0=nand"
#define MTDPARTS_DEFAULT  "mtdparts=nand:512k(x-loader), "\
	"1920k(u-boot), 128k(u-boot-env), 4m(kernel), -(fs)"

/*
 * Environment information
 */
#define CONFIG_BOOTDELAY 3
#define CONFIG_BOOTFILE "uImage"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"usbtty=cdc_acm\0" \
	"console=ttyS1,115200n8\0" \
	"mmcargs=setenv bootargs console=${console} " \
	"root=/dev/cardblksd2 rw " \
	"rootfstype=ext3 rootwait\0" \
	"nandargs=setenv bootargs console=${console} " \
	"root=/dev/mtdblock4 rw " \
	"rootfstype=yaffs2\0" \
	"loadbootscript=fatload mmc 0 ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
	"source ${loadaddr}\0" \
	"loaduimage=fatload mmc 0 ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
	"run mmcargs; " \
	"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
	"run nandargs; " \
	"onenand read ${loadaddr} 280000 400000; " \
	"bootcmd=bootm ${loadaddr}\0" \
	"boardname=snps-haps51\0" \
	"chipname=dw-ip-9543-0\0" \
	"machid=0x34\0" \
	"bootargs=root=254:2 rw init=/init console=ttyS0,115200 rootfstype=ext3\0"

#define CONFIG_BOOTCOMMAND \
	"version"
/*"mmcinfo; fatls mmc 0:1; fatload mmc 0:1 82000000 uImage; bootm 82000000" \*/

#define CONFIG_AUTO_COMPLETE 1

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP    /* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER /* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#define CONFIG_SYS_PROMPT          "haps51 # "
#define CONFIG_SYS_CBSIZE 256 /* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + \
	sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS 16 /* max number of command */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE  (CONFIG_SYS_CBSIZE)
#define CONFIG_SYS_LOAD_ADDR 0

/* clk , timer setting */
#define CONFIG_SYS_HZ 100000000

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE     (128 << 10) /* regular stack 128 KiB */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ (4 << 10) /* IRQ stack 4 KiB */
#define CONFIG_STACKSIZE_FIQ (4 << 10) /* FIQ stack 4 KiB */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS     1 /* CS1 may or may not be populated */
#define PHYS_MEMORY_START        0x80000000
#define PHYS_MEMORY_SIZE         0x10000000 /* 256 M */
#define CONFIG_SYS_MEMTEST_START 0x80000000 /* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END   0x8FD00000 /* 0 ... 1200 MB in DRAM */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#ifndef CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_MAX_FLASH_BANKS 1
#define CONFIG_SYS_MAX_FLASH_SECT  1024

/*
 * Flash memory is CFI compliant
 */
#define CONFIG_SYS_FLASH_CFI    1
#define CONFIG_FLASH_CFI_DRIVER 1 /* Use drivers/cfi_flash.c */
#define CONFIG_FLASH_CFI_MTD    1 /* Use drivers/cfi_mtd.c */

#define CONFIG_SYS_FLASH_PROTECTION 1 /* Use h/w sector protection*/
#define CONFIG_SYS_MONITOR_LEN      (0x60000) /* Reserve 2 sectors */
#define CONFIG_ENV_IS_IN_FLASH
#else
#undef CONFIG_CMD_SF
#define CONFIG_ENV_IS_NOWHERE
#endif /* CONFIG_SYS_NO_FLASH */

#define CONFIG_SYS_FLASH_BASE 0x10000000
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

#undef CONFIG_CMD_SF
#define CONFIG_CMD_SAVEENV
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_SIZE   0x20000
#define CONFIG_ENV_OFFSET 0x60000

/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */

/* timeout values are in ticks */
#ifndef CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_ERASE_TOUT (100 * CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT (100 * CONFIG_SYS_HZ)

/*
 * Flash banks JFFS2 should use
 */
#define CONFIG_SYS_MAX_MTD_BANKS (CONFIG_SYS_MAX_FL57600ASH_BANKS + \
	CONFIG_SYS_MAX_NAND_DEVICE)
#define CONFIG_SYS_JFFS2_MEM_NAND
/* use flash_info[2] */
#define CONFIG_SYS_JFFS2_FIRST_BANK	CONFIG_SYS_MAX_FLASH_BANKS
#define CONFIG_SYS_JFFS2_NUM_BANKS 1

#ifndef __ASSEMBLY__
extern unsigned int boot_flash_base;
extern volatile unsigned int boot_flash_env_addr;
extern unsigned int boot_flash_off;
extern unsigned int boot_flash_sec;
extern unsigned int boot_flash_type;
#endif /* __ASSEMBLY__ */
#endif /* CONFIG_SYS_NO_FLASH */

/*-----------------------------------------------------------------------
 * Other driver setup
 */
#undef CONFIG_CALXEDA_XGMAC /* STMMAC ethernet driver ? */

/*----------------------------------------------------------------------------
 * ... Ethernet from ... family
 *----------------------------------------------------------------------------
 */
#if defined(CONFIG_CMD_NET)
#define CONFIG_RESET_PHY_R 1
#define CONFIG_HOSTNAME    haps51_fd
#define CONFIG_ETHADDR     00 : 01 : 02 : 65 : 04 : 75 /* Ethernet address */
#define CONFIG_IPADDR      10.100.24.197     /* Our ip address */
#define CONFIG_GATEWAYIP   10.100.24.1       /* Our getway ip address */
#define CONFIG_SERVERIP    10.100.24.228     /* Tftp server ip address */
#define CONFIG_NETMASK     255.255.255.0
#endif /* (CONFIG_CMD_NET) */

/*
 * BOOTP fields
 */
#if defined(CONFIG_CMD_NET)
#define CONFIG_BOOTP_SUBNETMASK 0x00000001
#define CONFIG_BOOTP_GATEWAY    0x00000002
#define CONFIG_BOOTP_HOSTNAME   0x00000004
#define CONFIG_BOOTP_BOOTPATH   0x00000010
#endif /* (CONFIG_CMD_NET) */

#endif /* __CONFIG_H */
