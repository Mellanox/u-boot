/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <common.h>

#define TIMER0_LIMIT 0xffffffff /* Maximum limit for timer */

static ulong timestamp;

/*
 * timer without interrupts
 */
void set_timer(ulong count)
{
	/*
	 * Set up the LIMIT0, COUNT0 & CONTROL0 registers
	 * for timer 0  without interrupt
	 */
	write_new_aux_reg(ARC_REG_TIMER0_CTRL, 0);
	write_new_aux_reg(ARC_REG_TIMER0_LIMIT, TIMER0_LIMIT);
	write_new_aux_reg(ARC_REG_TIMER0_CTRL, 2);
	write_new_aux_reg(ARC_REG_TIMER0_CNT, count);
}

void reset_timer(void)
{
	set_timer(0);
}

int timer_init(void)
{
	reset_timer();
	return 0;
}

ulong get_utimer(ulong base)
{
	return 0;
}

ulong get_timer(ulong base)
{
	/* Read the count value from COUNT0 */
	timestamp = read_new_aux_reg(ARC_REG_TIMER0_CNT);
	return timestamp - base;
}

void uninit_timer(void)
{
	write_new_aux_reg(ARC_REG_TIMER0_LIMIT, 0xffffff);
	write_new_aux_reg(ARC_REG_TIMER0_CNT, 0);
	write_new_aux_reg(ARC_REG_TIMER0_CTRL, 0);
}

/*
 * Delay function
 */
void __udelay(unsigned long usec)
{
	DECLARE_GLOBAL_DATA_PTR;
	ulong uTicks;
	ulong tmp, tmp2, tmp3, start, time;

	uTicks = (gd->cpu_clk / 1000000) * usec;
	tmp2 = get_timer(0);
	tmp3 = get_timer(0);

	start = get_timer(0);
	time  = 0;

	/*
	 * Continue looping till the timer count is more
	 * than the required delay
	 */
	while (time < uTicks) {
		tmp = get_timer(0);

		if (tmp > start)
			time = tmp - start;
		else
			time = (0xffffffff - start) + tmp;
	}
}
