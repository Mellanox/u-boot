/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <common.h>
#include <asm/arcregs.h>
#include <asm/arch/irqs.h>

int disable_interrupts(void)
{
	write_new_aux_reg(AUX_IENABLE, ~(3 << 3));

	return 0;

}

void enable_interrupts(void)
{
	unsigned int status;

	status = read_new_aux_reg(ARC_REG_STATUS32) | STATUS_E1_MASK | \
		STATUS_E2_MASK;
	__asm__("FLAG %0" : : "r" (status));
}

#define Jcc_INSTR_hi  0x2020
#define Jcc_INSTR_lo  0x0f80
#define _Interrupt1
#define _Interrupt2

typedef struct vect_entry {
	unsigned short instruction_hi;
	unsigned short instruction_lo;
	unsigned short target_adr_hi;
	unsigned short target_adr_lo;
} vect_entry_type;

#define VECT_START read_new_aux_reg(AUX_INTR_VEC_BASE)

struct vect_entry vector_table[64]; /* __attribute__ (aligned(1024)); */

struct irq_node {
	void           (*handler)(int, void *, struct pt_regs *);
	unsigned long  flags;
	int            disable_depth;
	void           *dev_id;
	const char     *devname;
	struct         irq_node *next;
	unsigned long  mask;
	int            vertor;
	int            irq_number;
	int            hand_num;
}  irq_node_t;

#ifdef NR_IRQS
#undef NR_IRQS
#endif
#define NR_IRQS (sizeof(vector_table) / sizeof(vector_table[0]) - 1)

extern unsigned handle_int_lv2[];
extern unsigned handle_int_lv1[];
extern unsigned handle_exception[];

static void setvect(int vector, unsigned target)
{
	vect_entry_type *vtab       = (vect_entry_type *)VECT_START;
	unsigned long target_as_int = (unsigned long)target;

	vtab[vector].instruction_hi = Jcc_INSTR_hi;
	vtab[vector].instruction_lo = Jcc_INSTR_lo;
	vtab[vector].target_adr_hi = target_as_int >> 16;
	vtab[vector].target_adr_lo = target_as_int & 0xffff;
}

int  init_interrupt(void)
{
	int i, addr;

	write_new_aux_reg(AUX_INTR_VEC_BASE, (unsigned) &vector_table[0]);
	write_new_aux_reg(AUX_IENABLE, 0x0);
	write_new_aux_reg(AUX_IRQ_LEV, 0x0);

	setvect(0, (unsigned long)handle_exception);
	setvect(1, (unsigned long)handle_int_lv2);
	setvect(2, (unsigned long)handle_exception);

	for (i = 3; i < 0x20; i++)
		setvect(i, (unsigned long)handle_int_lv1);

	setvect(6, (unsigned long)handle_int_lv2);

	for (i = 0x20; i < 0x40; i++)
		setvect(i, (unsigned long)handle_exception);

	return 0;
}

void set_int_level(int vector, int level)
{
	unsigned level_table = read_new_aux_reg(AUX_IRQ_LEV);

	if (level) {
		setvect(vector, (unsigned long)handle_int_lv2);
		write_new_aux_reg(AUX_IRQ_LEV, level_table | (1 << vector));
	} else {
		setvect(vector, (unsigned long)handle_int_lv1);
		write_new_aux_reg(AUX_IRQ_LEV, level_table & (~(1 << vector)));
	}
}

void _setvect1(int vector, _Interrupt1 void (*target)(void))
{
	unsigned long temp;

	if (vector < 3 || vector > 31)
		return;
	if (vector != 1) {
		temp = target ? target : handle_int_lv1;
		set_int_level(vector, 0);
	} else {
		temp = target ? target : handle_int_lv2;
	}
	setvect(vector, (unsigned long)temp);
}

void _setvect2(int vector, _Interrupt2 void (*target)(void))
{
	unsigned long temp;

	if (vector < 3 || vector > 31)
		return;

	if (vector != 1)
		set_int_level(vector, 1);

	temp = target ? target : handle_int_lv2;
	setvect(vector, (unsigned long)temp);
}

void _setexception(int vector, _Interrupt2 void (*target)(void))
{
	if (vector > 2 && vector < 31)
		return;
	if (vector == 1)
		return;
	if (target)
		setvect(vector, (unsigned long)target);
	else
		setvect(vector, (unsigned long)handle_exception);
}

struct interrupt_action {
	interrupt_handler_t *handler;
	void *arg;
};

static struct interrupt_action irq_vecs[64];

void process_interrupt(struct pt_regs *pt)
{
	int vec_num = 0;
	int status = read_new_aux_reg(ARC_REG_STATUS32);

	if ((status & (STATUS_A1_MASK | STATUS_A2_MASK | STATUS_AE_MASK)) == 0)
		while (1)
			/* Nothing to do */;

	if (status & STATUS_A1_MASK)
		vec_num = (read_new_aux_reg(AUX_ICAUSE1)) & 0x1f;
	if (status & STATUS_A2_MASK)
		vec_num = (read_new_aux_reg(AUX_ICAUSE2)) & 0x1f;
	if (status & STATUS_AE_MASK)
		vec_num = (read_new_aux_reg(ARC_REG_ECR) >> 16) & 0xff;

	if (irq_vecs[vec_num].handler == 0)
		while (1)
			/* Nothing to do */;

	void *arg = irq_vecs[vec_num].arg ? irq_vecs[vec_num].arg : pt;
	irq_vecs[vec_num].handler(arg);

	return;
}
