/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __MACH_FIRMWARE_REGS_H
#define __MACH_FIRMWARE_REGS_H

#define IO_CBUS_BASE			0xc1100000
#define IO_AXI_BUS_BASE			0xc1300000
#define IO_AHB_BUS_BASE			0xc9000000
#define IO_APB_BUS_BASE			0xd0040000

#define CBUS_REG_OFFSET(reg) ((reg) << 2)
#define CBUS_REG_ADDR(reg)	 (IO_CBUS_BASE + CBUS_REG_OFFSET(reg))

#define AXI_REG_OFFSET(reg)  ((reg) << 2)
#define AXI_REG_ADDR(reg)	 (IO_AXI_BUS_BASE + AXI_REG_OFFSET(reg))

#define AHB_REG_OFFSET(reg)  ((reg) << 2)
#define AHB_REG_ADDR(reg)	 (IO_AHB_BUS_BASE + AHB_REG_OFFSET(reg))

#define APB_REG_OFFSET(reg)  (reg)
#define APB_REG_ADDR(reg)	 (IO_APB_BUS_BASE + APB_REG_OFFSET(reg))
#define APB_REG_ADDR_VALID(reg) (((unsigned long)(reg) & 3) == 0)

#define PREG_EGPIO_EN_N        0x200c
#define PREG_EGPIO_O           0x200d
#define PREG_EGPIO_I           0x200e
#define PREG_FGPIO_EN_N        0x200f
#define PREG_FGPIO_O           0x2010
#define PREG_FGPIO_I           0x2011
#define PREG_GGPIO_EN_N        0x2012
#define PREG_GGPIO_O           0x2013
#define PREG_GGPIO_I           0x2014
#define PREG_HGPIO_EN_N        0x2015
#define PREG_HGPIO_O           0x2016
#define PREG_HGPIO_I           0x2017

#define PERIPHS_PIN_MUX_0      0x202c
#define PERIPHS_PIN_MUX_1      0x202d
#define PERIPHS_PIN_MUX_2      0x202e
#define PERIPHS_PIN_MUX_3      0x202f
#define PERIPHS_PIN_MUX_4      0x2030
#define PERIPHS_PIN_MUX_5      0x2031
#define PERIPHS_PIN_MUX_6      0x2032
#define PERIPHS_PIN_MUX_7      0x2033
#define PERIPHS_PIN_MUX_8      0x2034
#define PERIPHS_PIN_MUX_9      0x2035
#define PERIPHS_PIN_MUX_10     0x2036
#define PERIPHS_PIN_MUX_11     0x2037
#define PERIPHS_PIN_MUX_12     0x2038

#define GPIO_INTR_EDGE_POL     0x2620
#define GPIO_INTR_GPIO_SEL0    0x2621
#define GPIO_INTR_GPIO_SEL1    0x2622
#define GPIO_INTR_FILTER_SEL0  0x2623

#define WATCHDOG_TC            0x2640
#define WATCHDOG_RESET         0x2641
#define WATCHDOG_ENABLE_BIT    22

#endif
