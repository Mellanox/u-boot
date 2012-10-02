/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H

#define IRQ_FLG_LOCK    (0x0001)     /* handler is not replaceable */
#define IRQ_FLG_REPLACE (0x0002)     /* replace existing handler   */
#define IRQ_FLG_STD     (0x8000)     /* internally used            */
#define IRQ_ISA_FAST    (0x00000020) /* same as IRQF_DISABLED      */

#define mask_interrupt(x)  __asm__ __volatile__ ( \
	"lr r20, [auxienable]\n\t" \
	"and    r20, r20, %0\n\t" \
	"sr     r20,[auxienable]\n\t" \
	: \
	: "r" (~(x)) \
	: "r20", "memory")

#define unmask_interrupt(x)  __asm__ __volatile__ ( \
	"lr r20, [auxienable]\n\t" \
	"or     r20, r20, %0\n\t" \
	"sr     r20, [auxienable]\n\t" \
	: \
	: "r" (x) \
	: "r20", "memory")

#endif
