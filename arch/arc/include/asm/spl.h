/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef _ASM_SPL_H_
#define _ASM_SPL_H_

/* Linker symbols */
extern char __bss_start[], __bss_end[];

enum {
        BOOT_DEVICE_SPI,
        BOOT_DEVICE_NONE
};

#endif
