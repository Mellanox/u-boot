/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _CPU_H
#define _CPU_H

/*
 * M1 default Settings
 * We should support SD/SDHC/MMC , SPI , NAND as defautly
 *
 * For NAND and SPI , we should support MTD device
 */
#define CONFIG_MTD_DEVICE     1
#define CONFIG_MTD_PARTITIONS 1
#define CONFIG_CMD_MTDPARTS   1

#define CONFIG_CMD_SF         1

#endif /* _CPU_H */
