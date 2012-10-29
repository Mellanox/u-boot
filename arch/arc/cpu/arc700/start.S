;#
;# Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
;#
;# This program is free software; you can redistribute it and/or modify
;# it under the terms of the GNU General Public License version 2 as
;# published by the Free Software Foundation.
;#

#include <asm/arcregs.h>	/* required for STATUS_H_SET */
#include <config.h>

	/* text section loaded at CONFIG_SYS_TEXT_BASE defined */
	.section .text , "ax" , @progbits
	.type _start, @function
	.align 4
	.globl _start

	/* pre boot header */
__start:
	.word .
	.word _end
	.word 0
	.word _start

_start:
	/* Disable interrupts */

	flag 0
	/* Whether I am running in CONFIG_SYS_TEXT_BASE */
	lr			r0,[ARC_REG_PC]
	sub			r0,r0,. - __start
	mov 		r1,CONFIG_SYS_TEXT_BASE
	cmp			r0,r1
	bz			relocate_end
	mov %r2,__bss_start
	sub %r2,%r2,%r1
	asr %r2,%r2,2
	mov %lp_count,%r2
	lp copy_end
	ld.ab %r3,[%r0,4]
	st.ab %r3,[%r1,4]
copy_end:
	j relocate_end
relocate_end:
	/* Clear bss */
	mov_s r2, __bss_start
	mov_s r3, __bss_end

_clear_bss:
	st.ab 0,[r2,4]
	brlt  r2,r3,_clear_bss

	/* setup stack pointer */
	mov sp,CONFIG_SYS_TEXT_BASE - CONFIG_SYS_MALLOC_LEN - 4
	mov fp,sp

	/* Enable Irq */

	sr	0xff, [0x40c]
	lr	%r10, [0x200]
	and	%r10, %r10, ~(1<<5)
	or	%r10, %r10, (1<<6)
	sr	%r10, [0x200]

	jal start_arcboot
