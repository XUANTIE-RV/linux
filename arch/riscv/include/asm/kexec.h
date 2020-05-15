/*
 * kexec for riscv
 *
 * Copyright (C) 2020-2025 Alibaba Group Holding Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _RISCV_KEXEC_H
#define _RISCV_KEXEC_H

/* Maximum physical address we can use pages from */

#define KEXEC_SOURCE_MEMORY_LIMIT (-1UL)

/* Maximum address we can reach in physical address mode */

#define KEXEC_DESTINATION_MEMORY_LIMIT (-1UL)

/* Maximum address we can use for the control code buffer */

#define KEXEC_CONTROL_MEMORY_LIMIT (-1UL)

#define KEXEC_CONTROL_PAGE_SIZE 4096

#define KEXEC_ARCH KEXEC_ARCH_RISCV

#ifndef __ASSEMBLY__

/**
 * crash_setup_regs() - save registers for the panic kernel
 *
 * @newregs: registers are saved here
 * @oldregs: registers to be saved (may be %NULL)
 */
static inline void crash_setup_regs(struct pt_regs *newregs,
				    struct pt_regs *oldregs)
{
	if (oldregs) {
		memcpy(newregs, oldregs, sizeof(*newregs));
	} else {
		u64 tmp1, tmp2;

		__asm__ __volatile__ (
			"sd	ra, 8(%2)\n"
			"sd	gp, 24(%2)\n"
			"sd	t0, 40(%2)\n"
			"sd	t1, 48(%2)\n"
			"sd	t2, 56(%2)\n"
			"sd	s0, 64(%2)\n"
			"sd	s1, 72(%2)\n"
			"sd	a0, 80(%2)\n"
			"sd	a1, 88(%2)\n"
			"sd	a2, 96(%2)\n"
			"sd	a3, 104(%2)\n"
			"sd	a4, 112(%2)\n"
			"sd	a5, 120(%2)\n"
			"sd	a6, 128(%2)\n"
			"sd	a7, 136(%2)\n"
			"sd	s2, 144(%2)\n"
			"sd	s3, 152(%2)\n"
			"sd	s4, 160(%2)\n"
			"sd	s5, 168(%2)\n"
			"sd	s6, 176(%2)\n"
			"sd	s7, 184(%2)\n"
			"sd	s8, 192(%2)\n"
			"sd	s9, 200(%2)\n"
			"sd	s10, 208(%2)\n"
			"sd	s11, 216(%2)\n"
			"sd	t3, 224(%2)\n"
			"sd	t4, 232(%2)\n"
			"sd	t5, 240(%2)\n"
			"sd	t6, 248(%2)\n"
			"auipc	%0, 0\n"
			"sd	%0, 0(%2)\n"
			"csrr	%0, sstatus\n"
			"sd	%0, 256(%2)\n"
			"csrr	%0, stval\n"
			"sd	%0, 264(%2)\n"
			"csrr	%0, scause\n"
			"sd	%0, 272(%2)\n"
			"sd tp, 32(%2)\n"
			"sd	sp, 16(%2)\n"
			: "=&r" (tmp1), "=&r" (tmp2)
			: "r" (newregs)
			: "memory"
		);
	}
}

static inline bool crash_is_nosave(unsigned long pfn) {return false; }
static inline void crash_prepare_suspend(void) {}
static inline void crash_post_resume(void) {}

#endif /* __ASSEMBLY__ */

#endif
