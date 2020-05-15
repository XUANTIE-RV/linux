/*
 * CPU reset routines
 *
 * Copyright (C) 2020-2025 Alibaba Group Holding Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _RISCV_CPU_RESET_H
#define _RISCV_CPU_RESET_H

extern struct resource *standard_resources;
void __cpu_soft_restart(unsigned long entry, unsigned long arg0, unsigned long arg1,
								unsigned long arg2);

__attribute__ ((optimize("-O0"))) static void __noreturn cpu_soft_restart(unsigned long entry,
					       unsigned long arg0,
					       unsigned long arg1,
					       unsigned long arg2)
{
	typeof(__cpu_soft_restart) *restart;
	pgd_t *idmap_pgd;
	pmd_t *idmap_pmd;
	long pa_start, pa_end;
	long i, j, m, n, delta;
	long idmap_pmd_size;

	pa_start = standard_resources->start;
	pa_end = standard_resources->end;

	idmap_pmd_size = (pa_end - pa_start + 1) / PMD_SIZE * sizeof(pmd_t);

	idmap_pgd = (pgd_t *)__va((csr_read(CSR_SATP) & ((1UL<<44)-1))<< PAGE_SHIFT);
	idmap_pmd = (pmd_t *)__get_free_pages(GFP_KERNEL, get_order(idmap_pmd_size));

	m = (pa_start >> PGDIR_SHIFT) % PTRS_PER_PGD;
	n = (pa_end >> PGDIR_SHIFT) % PTRS_PER_PGD;

	for (i = 0; m <= n; m++,i++)
		idmap_pgd[m] = pfn_pgd(PFN_DOWN(__pa(idmap_pmd)) + i,
				__pgprot(_PAGE_TABLE));

	m = pa_start >> PMD_SHIFT;
	n = (pa_end + 1) >> PMD_SHIFT;
	delta = n - m;

	for (i = (pa_start + 1) % PMD_SIZE,j=0; i <= delta; i++,j++)
		idmap_pmd[i] = pfn_pmd(PFN_DOWN(pa_start + j * PMD_SIZE),
				__pgprot(pgprot_val(PAGE_KERNEL) | _PAGE_EXEC));

	restart = (void *)__pa_symbol(__cpu_soft_restart);
	restart(entry, arg0, arg1, arg2);
	unreachable();
}

#endif
