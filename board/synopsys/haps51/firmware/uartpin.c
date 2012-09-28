/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <config.h>
#include <asm/arch/firm/reg_addr.h>
#include <asm/arch/firm/uart.h>
#include <asm/arch/firm/io.h>

#ifndef FIRMWARE_IN_ONE_FILE
#define STATIC_PREFIX
#else
#define STATIC_PREFIX static
#endif

STATIC_PREFIX void serial_hw_init(void)
{
	SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, ((1 << 4) | (1 << 5)));
}
