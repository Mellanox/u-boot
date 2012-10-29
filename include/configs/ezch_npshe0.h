/*
 * Copyright (C) 2012 EZchip, Inc. (www.ezchip.com)
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
#define CONFIG_NPSHE0       1 /* EZchip IP */

/*
 * NAND define
 */
#define CONFIG_SYS_NO_FLASH
#include <asm/arch/cpu.h> /* get chip and board defs */

/*
 * Display, CPU and Board information
 */

#define CONFIG_SYS_HZ      83333333   /* clk , timer setting */

#define CONFIG_SYS_CLK     83.333333   /* 83.3Mhz lk@todo redefine it later */
#define CONFIG_CRYSTAL_MHZ 83 /* Crystal 83 Mhz*/

#undef  CONFIG_USE_IRQ        /* no support for IRQs */
#undef  CONFIG_MISC_INIT_R

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
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE	4
#define CONFIG_SYS_NS16550_CLK  CONFIG_SYS_HZ
#define CONFIG_SYS_NS16550_COM1 0xC0000000
#define CONFIG_CONS_INDEX       1 /* Console is on COM1 */

/*
 * commands to include
 */
#include <config_cmd_default.h>

#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
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
#define CONFIG_CMD_IMLS     /* List all found images */
#endif
#define CONFIG_CMD_LOADB    /* loadb */
#define CONFIG_CMD_LOADS    /* loads */
#define CONFIG_CMD_MEMORY   /* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC     /* Misc functions like sleep etc */
#define CONFIG_CMD_NET      /* bootp, tftpboot, rarpboot */
#define CONFIG_CMD_PING     /* ping */
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

#define CONFIG_RAMBOOTCOMMAND   \
                "run load_kernel;"      \
                "run addtty;"   \
                "run miscargs;" \
                "run addip;"    \
                "run addmac;"   \
                "bootm ${loadaddr}"

#define CONFIG_FAST_RAMBOOTCOMMAND      \
                "tftpboot ${krn_file};" \
                "run addtty;"   \
                "run miscargs;" \
                "run addip;"    \
                "run addmac;"   \
                "bootm ${loadaddr}\0"

#define CONFIG_NFSBOOTCOMMAND   \
                "tftpboot ${krn_file};" \
                "run addtty;"   \
                "run miscargs;" \
                "run nfsargs;"  \
                "run addip;" \
                "run addmac;"   \
                "bootm ${loadaddr}"

#define NETWORK_ENV_SETTINGS \
    "load_kernel=sf probe 0;"       \
        "sf read ${loadaddr} ${krn_offs} 40; "  \
        "upkrn_sz; "    \
        "sf read ${loadaddr} ${krn_offs} ${krn_size}\0" \
    "addtty=setenv bootargs "       \
        "console=ttyS0,${baudrate}n8\0" \
    "addip=setenv bootargs ${bootargs} " \
        "ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:" \
        "${hostname}:eth0:off\0" \
    "addmac=setenv bootargs ${bootargs} " \
        "mac=${ethaddr}\0" \
    "miscargs=setenv bootargs ${bootargs} " \
        "${krn_args}\0" \
    "nfsargs=setenv bootargs ${bootargs} "  \
        "root=/dev/nfs rw " \
        "nfsroot=${serverip}:${rootpath}\0"

#define CONFIG_EXTRA_ENV_SETTINGS                       \
    "machid=270f0034\0"                               \
        "preboot_offs=590000\0"                         \
        "preboot_file=\"releases/pre-boot.bin\"\0"       \
        "uboot_offs=500000\0"                           \
        "uboot_file=\"releases/u-boot.bin\"\0"                  \
        "krn_offs=600000\0"                             \
        "krn_file=\"releases/he0/uImage\"\0"                    \
        "krn_args=\0"                   \
        "fastramboot="CONFIG_FAST_RAMBOOTCOMMAND        \
        NETWORK_ENV_SETTINGS

#define CONFIG_BOOTCOMMAND \
	"run nfsboot"

#define CONFIG_AUTO_COMPLETE 1
#define CONFIG_HOSTNAME      he0
#define CONFIG_ETHADDR       00:C0:00:99:AA:FE       /* Ethernet address */
#define CONFIG_IPADDR        10.1.8.254              /* Our ip address */
#define CONFIG_SERVERIP      10.1.3.58               /* Tftp server ip address */
#define CONFIG_GATEWAYIP     10.1.1.10
#define CONFIG_NETMASK       255.255.0.0
#define CONFIG_LOADADDR      0x90000000              /* Default load address for net commands */
#define CONFIG_ROOTPATH      "/opt/ARC/rootfs/default"
#define CONFIG_OVERWRITE_ETHADDR_ONCE

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP    /* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER /* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#define CONFIG_SYS_PROMPT          "nps-he0 # "
#define CONFIG_SYS_CBSIZE 256 /* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + \
	sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS 16 /* max number of command */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE  (CONFIG_SYS_CBSIZE)
#define CONFIG_SYS_LOAD_ADDR 0

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
#define PHYS_MEMORY_SIZE         0x20000000 /* 512 M */
#define CONFIG_SYS_MEMTEST_START 0x80000000 /* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END   0x9FF00000 /* 0 ... 1200 MB in DRAM */

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
#define CONFIG_ENV_IS_IN_SPI_FLASH

#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SF_DEFAULT_SPEED   20000000  // 20M
#endif /* CONFIG_SYS_NO_FLASH */

#define CONFIG_SYS_FLASH_BASE 0x00000000
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

#define CONFIG_CMD_SF
#define CONFIG_CMD_SAVEENV
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_SIZE   0x8000
#define CONFIG_ENV_OFFSET 0x580000
#define CONFIG_ENV_SECT_SIZE    0x40000

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

#endif /* __CONFIG_H */
