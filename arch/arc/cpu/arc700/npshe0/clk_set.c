/*
 * Copyright (C) 2012 EZchip, Inc. (www.ezchip.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <asm/arch/regs.h>

/*
 *    7-2*XTAL
 *    6-PAD
 *    5-video_pll_clk
 *    4-audio_pll_clk
 *    3-ddr_pll_clk
 *    2-misc_pll_clk
 *    1-sys_pll_clk
 *    0-XTAL
 *
 * clk_freq:50M=50000000
 * output_clk:50000000;
 * aways,maybe changed for others?
 */

int eth_clk_set(int selectclk, unsigned long clk_freq, unsigned long out_clk)
{
	int n;

	printf("select eth clk-%d,source=%d,out=%d\n",
		selectclk, clk_freq, out_clk);

	if (((clk_freq) % out_clk) != 0) {
		printf("ERROR:source clk must n times of out_clk ,"
			"source clk=%d\n", out_clk, clk_freq);
		return -1;
	} else
		n = (int)((clk_freq) / out_clk);

	WRITE_CBUS_REG(HHI_ETH_CLK_CNTL,\
		(n - 1) << 0 | selectclk << 9 | 1 << 8);

	udelay(100);

	return 0;
}

int auto_select_eth_clk(void)
{
	return -1;
}
