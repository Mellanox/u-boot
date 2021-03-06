/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

 #ifndef __ASM_ARC_SYSTEM_H
#define __ASM_ARC_SYSTEM_H

#include <asm/arcregs.h>
#include <asm/ptrace.h>

#ifndef __ASSEMBLY__

#ifdef __KERNEL__

/******************************************************************
 * IRQ Control Macros
 ******************************************************************/

/*
 * Save IRQ state and disable IRQs
 */

#define local_irq_save(x)	 \
	__asm__ __volatile__ (	\
	"lr r20, [status32]\n\t" \
	"mov	%0, r20\n\t"	 \
	"and	r20, r20, %1\n\t"\
	"flag   r20\n\t"		 \
	: "=r" (x)\
	: "n" (~(STATUS_E1_MASK | STATUS_E2_MASK))\
	: "r20", "memory");

/*
 * Conditionally Enable IRQs
 */
extern void local_irq_enable(void);

/*
 * Unconditionally Disable IRQs
 */
#define local_irq_disable()   \
	__asm__ __volatile__ (	\
	"lr r20, [status32]\n\t" \
	"and	r20, r20, %0\n\t"\
	"flag r20\n\t"		 \
	: \
	: "n" (~(STATUS_E1_MASK | STATUS_E2_MASK))\
	: "r20");

/*
 * restore saved IRQ state
 */
#define local_irq_restore(x)\
	__asm__ __volatile__ (  \
	"flag   %0"\
	:		  \
	: "r" (x)   \
	);

/*
 * save IRQ state
 */
#define local_save_flags(x)\
	__asm__ __volatile__ ( \
	"lr r20, [status32]\n\t"\
	"mov	%0, r20\n\t"	\
	: "=r" (x)\
	: \
	: "r20", "memory")

/*
 * Query IRQ state
 */
static inline int irqs_disabled(void)
{
	unsigned long flags;
	local_save_flags(flags);
	return !(flags & (STATUS_E1_MASK
#ifdef CONFIG_ARCH_ARC_LV2_INTR
						| STATUS_E2_MASK
#endif
			));
}

/*
 * mask/unmask an interrupt (@x = IRQ bitmap)
 * e.g. to Disable IRQ 3 and 4, pass 0x18
 *
 * mask = disable IRQ = CLEAR bit in AUX_I_ENABLE
 * unmask = enable IRQ = SET bit in AUX_I_ENABLE
 */

#define mask_interrupt(x)  __asm__ __volatile__ (   \
	"lr	 r19, [status32]\n\t"  \
	"lr r20, [status32]\n\t"   \
	"and	r20, r20, %1\n\t"  \
	"flag   r20\n\t"          \
	"lr r20, [auxienable]\n\t" \
	"and	r20, r20, %0\n\t"  \
	"sr	 r20,[auxienable]\n\t" \
	"flag   r19\n\t" \
	:				 \
	: "r" (~(x)),	\
	"n" (~(STATUS_E1_MASK | STATUS_E2_MASK)) \
	: "r19", "r20", "memory")

#define unmask_interrupt(x)  __asm__ __volatile__ ( \
	"lr	 r19, [status32]\n\t"  \
	"lr r20, [status32]\n\t"   \
	"and	r20, r20, %1\n\t"  \
	"flag   r20\n\t"           \
	"lr r20, [auxienable]\n\t" \
	"or	 r20, r20, %0\n\t"     \
	"sr	 r20, [auxienable]\n\t" \
	"flag   r19\n\t" \
	:				 \
	: "r" (x), "n" (STATUS_E1_MASK | STATUS_E2_MASK) \
	: "r19", "r20", "memory")

/******************************************************************
 * Barriers
 ******************************************************************/

#define mb() __asm__ __volatile__ ("" : : : "memory")
#define rmb() mb()
#define wmb() mb()
#define set_mb(var, value)  do { var = value; mb(); } while (0)
#define set_wmb(var, value) do { var = value; wmb(); } while (0)

#ifdef CONFIG_SMP
#define smp_mb()  mb()
#define smp_rmb() rmb()
#define smp_wmb() wmb()
#else
#define smp_mb()  barrier()
#define smp_rmb() barrier()
#define smp_wmb() barrier()
#endif

#define smp_read_barrier_depends() do { } while (0)

/******************************************************************
 * Arch Depenedent Context Switch Macro  called by sched
 * This in turn calls the regfile switching macro __switch_to ( )
 ******************************************************************/
struct task_struct; /* to prevent cyclic dependencies */

/* switch_to macro based on the ARM implementaion */
extern struct task_struct *__switch_to(struct task_struct *prev,
	struct task_struct *next);

#define switch_to(prev, next, last)	  \
{ \
	do { \
		last = __switch_to(prev, next); \
		mb(); \
		\
	} while (0); \
}

/* Hook into Schedular to be invoked prior to Context Switch
 *  -If ARC H/W profiling enabled it does some stuff
 *  -If event logging enabled it takes a event snapshot
 *
 *  Having a funtion would have been cleaner but to get the correct caller
 *  (from __builtin_return_address) it needs to be inline
 */

/* Things to do for event logging prior to Context switch */
#ifdef CONFIG_ARC_DBG_EVENT_TIMELINE
#define PREP_ARCH_SWITCH_ACT1(next) \
do { \
	if (next->mm) \
		take_snap(SNAP_PRE_CTXSW_2_U, \
			(unsigned int) __builtin_return_address(0), \
			current_thread_info()->preempt_count); \
	else \
		take_snap(SNAP_PRE_CTXSW_2_K, \
			(unsigned int) __builtin_return_address(0), \
			current_thread_info()->preempt_count); \
} \
while (0)
#else
#define PREP_ARCH_SWITCH_ACT1(next)
#endif

/* Things to do for hardware based profiling prior to Context Switch */
#ifdef CONFIG_ARC_PROFILING
extern void arc_ctx_callout(struct task_struct *next);
#define PREP_ARCH_SWITCH_ACT2(next) arc_ctx_callout(next)
#else
#define PREP_ARCH_SWITCH_ACT2(next)
#endif

/* This def is the one used by schedular */
#define prepare_arch_switch(next) \
do {							  \
	PREP_ARCH_SWITCH_ACT1(next);  \
	PREP_ARCH_SWITCH_ACT2(next);  \
}								 \
while (0)

/******************************************************************
 * Miscll stuff
 ******************************************************************/

/*
 * On SMP systems, when the scheduler does migration-cost autodetection,
 * it needs a way to flush as much of the CPU's caches as possible.
 *
 * TODO: fill this in!
 */
static inline void sched_cacheflush(void)
{
	/* Not yet implemented */
}

extern inline unsigned long __xchg(unsigned long with,
	__volatile__ void *ptr, int size)
{
	__asm__ __volatile__ (" ex  %0, [%1]"
				  : "=r" (with), "=r" (ptr)
				  : "0" (with), "1" (ptr)
				  : "memory");

	return with;
}

#define xchg(ptr, with) \
	((__typeof__(*(ptr))) __xchg((unsigned long)(with), \
	(ptr), sizeof(*(ptr))))

#define arch_align_stack(x) (x)

#endif /*__KERNEL__*/

#else  /* !__ASSEMBLY__ */

#endif /* __ASSEMBLY__ */

#endif /* ASM_ARC_SYSTEM_H */
