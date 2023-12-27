/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#ifndef _ASM_RISCV_PGTABLE_32_H
#define _ASM_RISCV_PGTABLE_32_H

#include <asm-generic/pgtable-nopmd.h>
#include <linux/bits.h>
#include <linux/const.h>

/* Size of region mapped by a page global directory */
#define PGDIR_SHIFT     22
#define PGDIR_SIZE      (_AC(1, UL) << PGDIR_SHIFT)
#define PGDIR_MASK      (~(PGDIR_SIZE - 1))

#define MAX_POSSIBLE_PHYSMEM_BITS 34

/*
 * rv32 PTE format:
 * | XLEN-1  10 | 9             8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
 *       PFN      reserved for SW   D   A   G   U   X   W   R   V
 */
#define _PAGE_PFN_MASK  GENMASK(31, 10)

/*
 * [31:30] Svpbmt Memory Type definitions:
 *
 *  00 - PMA    Normal Cacheable, No change to implied PMA memory type
 *  01 - NC     Non-cacheable, idempotent, weakly-ordered Main Memory
 *  10 - IO     Non-cacheable, non-idempotent, strongly-ordered I/O memory
 *  11 - Rsvd   Reserved for future standard use
 */
#define _PAGE_NOCACHE_SVPBMT	(1UL << 30)
#define _PAGE_IO_SVPBMT		(1UL << 31)
#define _PAGE_MTMASK_SVPBMT	(_PAGE_NOCACHE_SVPBMT | _PAGE_IO_SVPBMT)

#define _PAGE_PMA_THEAD		0UL
#define _PAGE_NOCACHE_THEAD	0UL
#define _PAGE_IO_THEAD		0UL
#define _PAGE_MTMASK_THEAD	0UL

static inline u32 riscv_page_mtmask(void)
{
	u32 val;

	ALT_SVPBMT(val, _PAGE_MTMASK);
	return val;
}

static inline u32 riscv_page_nocache(void)
{
	u32 val;

	ALT_SVPBMT(val, _PAGE_NOCACHE);
	return val;
}

static inline u32 riscv_page_io(void)
{
	u32 val;

	ALT_SVPBMT(val, _PAGE_IO);
	return val;
}

#define _PAGE_NOCACHE		riscv_page_nocache()
#define _PAGE_IO		riscv_page_io()
#define _PAGE_MTMASK		riscv_page_mtmask()

/* Set of bits to preserve across pte_modify() */
#define _PAGE_CHG_MASK  (~(unsigned long)(_PAGE_PRESENT | _PAGE_READ |	\
					  _PAGE_WRITE | _PAGE_EXEC |	\
					  _PAGE_USER | _PAGE_GLOBAL))

#endif /* _ASM_RISCV_PGTABLE_32_H */
