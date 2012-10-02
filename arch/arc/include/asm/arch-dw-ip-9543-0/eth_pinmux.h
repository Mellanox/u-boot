/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef ETH_PINMUX_HEADER_
#define ETH_PINMUX_HEADER_

#define ETH_BANK0_GPIOC3_C11        0
#define ETH_BANK0_REG1              2
#define ETH_BANK0_REG1_VAL          (0x1ff << 8)

#define ETH_BANK1_GPIOD0_D8         1
#define ETH_BANK1_REG1              4
#define ETH_BANK1_REG1_VAL          (0x1ff << 2)

#define ETH_CLK_IN_GPIOC2_REG2_18   0
#define ETH_CLK_IN_GPIOD9_REG4_0    1

#define ETH_CLK_OUT_GPIOC2_REG2_17  2
#define ETH_CLK_OUT_GPIOD9_REG4_1   3

#endif
