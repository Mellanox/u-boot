/*
 *  Copyright (C) 2011 Synopsys, INC.
 *
 *  arch\arc\cpu\a770\adr1\clock.c
 * 
 *
 * License terms: GNU General Public License (GPL) version 2
 * Basic register address definitions in physical memory and
 * some block defintions for core devices like the timer.
 *
 *
 * author :
 */
 
#include <asm/arch/clock.h>

__u32 get_cpu_clk()
{
    static __u32 sys_freq=0;
    if(sys_freq==0)
    {
        sys_freq=(CONFIG_SYS_CLK * 1000000);
    }
    return sys_freq;
}

