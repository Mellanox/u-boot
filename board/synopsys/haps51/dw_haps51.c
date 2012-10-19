/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <common.h>
#include <asm/arch/memory.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/nand.h>
#include <stdio_dev.h>

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	/*
	 * @todo implement this function
	 */
	printf(" @TODO: [%s]:%s()\n", __FILE__, __func__);

	DECLARE_GLOBAL_DATA_PTR;
#ifdef CONFIG_CMD_KGDB
	kgdb_init();
#endif

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Final stage, so udelay works
 */
int misc_init_r(void)
{
	/*
	 * @todo implement this function
	 */
	printf(" @TODO: [%s]:%s()\n", __FILE__, __func__);

	return 0;
}

struct arc_nand_platform arc_nand_mid_platform[] = {
	{
		.name            = NAND_BOOT_NAME,
		.chip_enable_pad = ARC_NAND_CE0,
		.ready_busy_pad  = ARC_NAND_CE0,
		.platform_nand_data = {
			.chip =  {
				.nr_chips = 1,
				.options  = (NAND_TIMING_MODE5 | \
					NAND_ECC_SHORT_MODE),
			},
		},
		.rbpin_mode = 1,
		.short_pgsz = 384,
		.ran_mode   = 0,
		.T_REA      = 20,
		.T_RHOH     = 15,
	},
	{
		.name            = NAND_NORMAL_NAME,
		.chip_enable_pad = (ARC_NAND_CE0) ,
		.ready_busy_pad  = (ARC_NAND_CE0) ,
		.platform_nand_data = {
			.chip =  {
				.nr_chips = 1,
				.options  = (NAND_TIMING_MODE5 | \
					NAND_ECC_BCH30_MODE),
			},
		},
		.rbpin_mode = 1,
		.short_pgsz = 0,
		.ran_mode   = 0,
		.T_REA      = 20,
		.T_RHOH     = 15,
	}
};

struct arc_nand_device arc_nand_mid_device = {
	.arc_nand_platform = arc_nand_mid_platform,
	.dev_num = ARRAY_SIZE(arc_nand_mid_platform),
};

void board_nand_pinmux(unsigned int en_dis)
{
	printf(" @TODO: [%s]:%s()\n", __FILE__, __func__);

	if (en_dis)
		set_mio_mux(4, (0x1<<11) | (0xff<<14));
	else
		clear_mio_mux(4, (0x1<<11) | (0xff<<14));
	return;
}

#if defined(CONFIG_CMD_KGDB)
void kgdb_serial_init(void)
{
	/*
	 * @todo implement this function
	 */
	printf(" @TODO: [%s]:%s()\n", __FILE__, __func__);
}

void putDebugChar(int c)
{
	serial_putc(c);
}

void putDebugStr(const char *str)
{
	serial_puts(str);
}

int getDebugChar(void)
{
	return serial_getc();
}

void kgdb_interruptible(int yes)
{
	return;
}

int kgdb_serial_tstc(void)
{
	return 0;
}
#endif /* CONFIG_CMD_KGDB */
