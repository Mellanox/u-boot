/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/arcregs.h>

/*
 * Note on the LD/ST addressing modes with address register write-back
 *
 * LD.a same as LD.aw
 *
 * LD.a    reg1, [reg2, x]  => Pre Incr
 *      Eff Addr for load = [reg2 + x]
 *
 * LD.ab   reg1, [reg2, x]  => Post Incr
 *      Eff Addr for load = [reg2]
 */

.macro PUSH reg
	st.a	\reg, [%sp, -4]
.endm

.macro PUSHAX aux
	lr	%r9, [\aux]
	PUSH	%r9
.endm

.macro  SAVE_R1_TO_R24
	PUSH	%r1
	PUSH	%r2
	PUSH	%r3
	PUSH	%r4
	PUSH	%r5
	PUSH	%r6
	PUSH	%r7
	PUSH	%r8
	PUSH	%r9
	PUSH	%r10
	PUSH	%r11
	PUSH	%r12
	PUSH	%r13
	PUSH	%r14
	PUSH	%r15
	PUSH	%r16
	PUSH	%r17
	PUSH	%r18
	PUSH	%r19
	PUSH	%r20
	PUSH	%r21
	PUSH	%r22
	PUSH	%r23
	PUSH	%r24
.endm

.macro SAVE_ALL_SYS

	st	%r0, [%sp]
	lr	%r0, [%ecr]
	st	%r0, [%sp, 8]	/* ECR */
	st	%sp, [%sp, 4]

	SAVE_R1_TO_R24
	PUSH	%r25
	PUSH	%gp
	PUSH	%fp
	PUSH	%blink
	PUSHAX	%eret
	PUSHAX	%erstatus
	PUSH	%lp_count
	PUSHAX	%lp_end
	PUSHAX	%lp_start
	PUSHAX	%erbta
.endm

.align 4
.globl _start


_start:
	/* Critical system events */
	j	reset			/* 0 - 0x000 */

reset:
	/* Setup interrupt vector base that matches "__text_start" */
	sr	__text_start, [ARC_AUX_INTR_VEC_BASE]

	/* Setup stack pointer */
	mov	%sp, CONFIG_SYS_INIT_SP_ADDR
	mov	%fp, %sp

	/* Clear bss */
	mov	%r0, __bss_start
	mov	%r1, __bss_end

clear_bss:
	st.ab	0, [%r0, 4]
	brlt	%r0, %r1, clear_bss

	/* Zero the one and only argument of "board_init_f" */
	mov_s	%r0, 0
	j	board_init_f