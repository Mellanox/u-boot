/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <command.h>
#include <common.h>
#include <asm/arcregs.h>
#include <asm/arch/regs.h>

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	WRITE_CBUS_REG(WATCHDOG_RESET, 0);
	WRITE_CBUS_REG(WATCHDOG_TC, 1 << 22 | 100); /* 100*10uS=1ms */
	WRITE_CBUS_REG(WATCHDOG_RESET, 0);
	udelay(10000); /* make sure reset is through */

	printf("Chip watchdog reset error!!!please reset it by hardware\n");

	while (1)
		/* do nothing */

	return 0;
}

/*
 * disable dcache and icahce
 */
int cpu_init(void)
{
	disable_interrupts();

	return 0;
}
