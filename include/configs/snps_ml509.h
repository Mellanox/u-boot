/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _CONFIG_SNPS_ML509_H_
#define _CONFIG_SNPS_ML509_H_

/*
 * Cpu/Board Config
 */
#define CONFIG_ARC       1
#define CONFIG_ARC_ML509 1

#define CONFIG_SYS_TEXT_BASE 0x3000000

/*
 * Serial console Config
 */
#define CONFIG_BAUDRATE 115200
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 19200, 38400, 57600, 115200 }

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DATE

#define CONFIG_SYS_PROMPT "ARC # " /* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE 256      /* Console I/O Buffer Size */

/*
 * Size of malloc() pool
 */
/* 512kB is suggested, (CONFIG_ENV_SIZE + 128 * 1024) was not enough */
#define CONFIG_SYS_MALLOC_LEN (512 << 10)

#define CONFIG_CMD_CRC32           1
#define CONFIG_CRC32_VERIFY        1
#define CONFIG_SHOW_BOOT_PROGRESS  1

#define CONFIG_ENV_SIZE            0x2000
#define CONFIG_SYS_MAX_FLASH_SECT  1024
#define CONFIG_SYS_MAX_FLASH_BANKS 1

/* Default load address	for bootm command */
#define CONFIG_SYS_LOAD_ADDR 0x80000000

#define CONFIG_ENV_IS_NOWHERE 1 /* In SDRAM */

#define CONFIG_SYS_CBSIZE 256 /* Console I/O Buffer Size */

#define CONFIG_SYS_CACHELINE_SIZE 32

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + \
	sizeof(CONFIG_SYS_PROMPT) + 16)

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE (CONFIG_SYS_CBSIZE)

/*
 * clk, timer setting
 */
#define CONFIG_SYS_HZ 100000000

/*
 * Flash memory is CFI compliant\
 */
#define CONFIG_SYS_FLASH_BASE 0x10000000

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS     1  /* CS1 may or may not be populated */
#define PHYS_MEMORY_START        0x80000000
#define PHYS_MEMORY_SIZE         0x10000000 /* 256 M */
#define CONFIG_SYS_MEMTEST_START 0x80000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END   0x8FD00000	/* 0 ... 1200 MB in DRAM */

#endif
