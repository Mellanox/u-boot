/*
 * Copyright (C) 2012 EZchip, Inc. (www.ezchip.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <asm/arch/clock.h>

__u32 get_cpu_clk()
{
	static __u32 sys_freq;

	if (sys_freq == 0)
		sys_freq = (CONFIG_SYS_CLK * 1000000);

	return sys_freq;
}
